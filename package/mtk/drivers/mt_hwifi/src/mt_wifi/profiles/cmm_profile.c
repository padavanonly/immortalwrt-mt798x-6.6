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
 ****************************************************************************

    Module Name:
	cmm_profile.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#include "rt_config.h"
#include "hdev/hdev.h"

/*
    ========================================================================

    Routine Description:
	Find key section for Get key parameter.

    Arguments:
	buffer                      Pointer to the buffer to start find the key section
	section                     the key of the secion to be find

    Return Value:
	NULL                        Fail
	Others                      Success
    ========================================================================
*/
RTMP_STRING *RTMPFindSection(RTMP_STRING *buffer)
{
	RTMP_STRING temp_buf[32];
	RTMP_STRING *ptr, *ret = NULL;

	strlcpy(temp_buf, "Default", strlen("Default") + 1);
	ptr = rtstrstr(buffer, temp_buf);

	if (ptr != NULL) {
		ret = ptr + strlen("\n");
		return ret;
	} else
		return NULL;
}

/*
    ========================================================================

    Routine Description:
	Get key parameter.

    Arguments:
	key			Pointer to key string
	dest			Pointer to destination
	destsize		The datasize of the destination
	buffer		Pointer to the buffer to start find the key
	bTrimSpace	Set true if you want to strip the space character of the result pattern

    Return Value:
	TRUE                        Success
	FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
	For SSID and security key related parameters, we SHALL NOT trim the space(' ') character.
    ========================================================================
*/
INT RTMPGetKeyParameter(
	IN RTMP_STRING *key,
	OUT RTMP_STRING *dest,
	IN INT destsize,
	IN RTMP_STRING *buffer,
	IN BOOLEAN bTrimSpace)
{
	RTMP_STRING *pMemBuf, *temp_buf1 = NULL, *temp_buf2 = NULL;
	RTMP_STRING *start_ptr, *end_ptr;
	RTMP_STRING *ptr;
	RTMP_STRING *offset = NULL;
	INT  len, keyLen;

	keyLen = strlen(key);
	os_alloc_mem(NULL, (PUCHAR *)&pMemBuf, MAX_PARAM_BUFFER_SIZE  * 2);

	if (pMemBuf == NULL)
		return FALSE;

	memset(pMemBuf, 0, MAX_PARAM_BUFFER_SIZE * 2);
	temp_buf1 = pMemBuf;
	temp_buf2 = (RTMP_STRING *)(pMemBuf + MAX_PARAM_BUFFER_SIZE);
	/*find section*/
	offset = RTMPFindSection(buffer);

	if (offset == NULL) {
		os_free_mem((PUCHAR)pMemBuf);
		return FALSE;
	}

	strlcpy(temp_buf1, "\n", strlen("\n") + 1);
	strncat(temp_buf1, key, strlen(key));
	strncat(temp_buf1, "=", strlen("="));
	/*search key*/
	start_ptr = rtstrstr(offset, temp_buf1);

	if (start_ptr == NULL) {
		os_free_mem((PUCHAR)pMemBuf);
		return FALSE;
	}

	start_ptr += strlen("\n");
	end_ptr = rtstrstr(start_ptr, "\n");

	if (end_ptr == NULL)
		end_ptr = start_ptr + strlen(start_ptr);

	if (end_ptr < start_ptr) {
		os_free_mem((PUCHAR)pMemBuf);
		return FALSE;
	}

	NdisMoveMemory(temp_buf2, start_ptr, end_ptr - start_ptr);
	temp_buf2[end_ptr - start_ptr] = '\0';
	start_ptr = rtstrstr(temp_buf2, "=");

	if (start_ptr == NULL) {
		os_free_mem((PUCHAR)pMemBuf);
		return FALSE;
	}

	ptr = (start_ptr + 1);

	/*trim special characters, i.e.,  TAB or space*/
	while (*start_ptr != 0x00) {
		if (((*ptr == ' ') && bTrimSpace) || (*ptr == '\t'))
			ptr++;
		else
			break;
	}

	len = strlen(start_ptr);
	memset(dest, 0x00, destsize);
	strlcpy(dest, ptr, destsize);
	os_free_mem((PUCHAR)pMemBuf);
	return TRUE;
}

#ifndef CONFIG_PROFILE_OFF
#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
unsigned char glb_npu_en_node = 1;
EXPORT_SYMBOL(glb_npu_en_node);
#endif

static UCHAR GetDefaultChannel(struct wifi_dev *wdev)
{
	/*priority must the same as Default PhyMode*/
	if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G)
		return 36;

	return 1;
}

static VOID RTMPChannelCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UINT32 i;
	CHAR *macptr;
	struct wifi_dev *wdev;
	UCHAR Channel;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	struct Chan_Config *pChCfg = NULL;

#ifdef CONFIG_AP_SUPPORT
	UINT32 j = 0;
#endif

	if (!pChCtrl) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "can't find pChCtrl !\n");
		return;
	}

	pChCfg = &pChCtrl->ch_cfg;

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		Channel = os_str_tol(macptr, 0, 10);
#ifdef CONFIG_AP_SUPPORT

		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (i >= CFG_WIFI_RAM_BAND_NUM)
				break;

			pChCfg->boot_chan = Channel;

			for (j = 0; j < pAd->ApCfg.BssidNum; j++) {
				wdev = &pAd->ApCfg.MBSSID[j].wdev;
				/*only when AutoChannelSelect enabled, ACS will run*/
				if (Channel == 0) {
					if (pAd->ApCfg.bAutoChannelAtBootup)
						wdev->channel = Channel;
					else {
						wdev->channel = GetDefaultChannel(wdev);
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_WARN,
							"AutoChannelSelect is invalid for ACS, use default channel %d\n", wdev->channel);
					}
				} else
					wdev->channel = Channel;

				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"BssIdx(%d) wdev->channel=%d\n", j, wdev->channel);
			}
#ifdef BW_VENDOR10_CUSTOM_FEATURE
			SET_APCLI_SYNC_PEER_DEAUTH_ENBL(pAd, FALSE);
#endif
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "Index%d Channel=%d\n", i, Channel);
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (i < MAX_MULTI_STA) {
				wdev = &pAd->StaCfg[i].wdev;
				wdev->channel = Channel;
			} else
				break;
		}
#endif /*CONFIG_STA_SUPPORT*/
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (pAd->StaCfg[MAIN_MSTA_ID].wdev.channel == 0)
			pAd->StaCfg[MAIN_MSTA_ID].wdev.channel = GetDefaultChannel(&pAd->StaCfg[MAIN_MSTA_ID].wdev);

		for (i = 0; i < MAX_MULTI_STA; i++) {
			wdev = &pAd->StaCfg[i].wdev;

			if (wdev->channel == 0)
				wdev->channel = pAd->StaCfg[MAIN_MSTA_ID].wdev.channel;
		}
	}
#endif /*CONFIG_STA_SUPPORT*/
}


#ifdef MULTI_INTR_SUPPORT
static VOID RTMPMultiIntrCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	BOOLEAN pre_condition;
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);

	pAd->CommonCfg.MultiIntr = os_str_tol(Buffer, 0, 10);

	/* The multiple interrupt will be adopted only When WHNAT is off
	 * since the current HWNAT design requires be aware of all interrupt
	 * events through single irq
	 */
#ifdef WHNAT_SUPPORT
	pre_condition = PD_GET_WHNAT_ENABLE(pAd->physical_dev) == 0;
#else
	pre_condition = TRUE;
#endif
	if (!pre_condition)
		pAd->CommonCfg.MultiIntr = 0;

	/* update some hif setting configured during driver probe phase */
	if (chip_ops && chip_ops->post_config_hif)
		chip_ops->post_config_hif(pAd);

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
		"MultiIntr = %d\n", pAd->CommonCfg.MultiIntr);
}
#endif

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
static VOID RTMPOldChannelCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UINT32 i;
	CHAR *macptr;
	struct wifi_dev *wdev;
	UCHAR BackupChannel;

	if ((Buffer == NULL) || (IS_SUPPORT_V10_DFS(pAd) == FALSE))
		return;

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
#ifdef CONFIG_AP_SUPPORT
		/* V10 -- AP Mode Only */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			/* V10 -- Only 5G Mode */
			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			if ((wdev == NULL) || (!WMODE_CAP_5G(wdev->PhyMode)))
				continue;

			/* Extract Channel */
			BackupChannel = os_str_tol(macptr, 0, 10);

			/* Disallow Zero or Invalid Values */
			if ((!BackupChannel) || (DfsV10CheckChnlGrp(pAd, BackupChannel) == NA_GRP)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"Backup Channel=%d\n",
					BackupChannel);
				continue;
			}

			/* Valid Old Channel Processing */
			if (BackupChannel && (!IS_V10_OLD_CHNL_VALID(wdev)))
				SET_V10_OLD_CHNL_VALID(wdev, TRUE);

			/* Update Channel */
			wdev->channel = BackupChannel;

			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BSS%d Channel=%d\n",
				i, wdev->channel);
		}
#endif /* CONFIG_AP_SUPPORT */
	}
}
#endif

static VOID RTMPWirelessModeCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UCHAR i;
	UCHAR cfg_mode, *macptr;
	struct wifi_dev *wdev = NULL;

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		cfg_mode = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
		if (i >= pAd->ApCfg.BssidNum)
			break;

		wdev = &pAd->ApCfg.MBSSID[i].wdev;
		wdev->PhyMode = cfgmode_2_wmode(cfg_mode);
		wlan_config_set_ch_band(wdev, wdev->PhyMode);
#ifdef DOT11_HE_AX
		if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G))
			wlan_config_set_he6g_op_present(wdev, 1);
		else
			wlan_config_set_he6g_op_present(wdev, 0);
#endif /* #ifdef DOT11_HE_AX */
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			"Init: BSS%d PhyMode=%d\n", i, wdev->PhyMode);

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

		if (i < MAX_MULTI_STA) {
			wdev = &pAd->StaCfg[i].wdev;
			wdev->PhyMode = cfgmode_2_wmode(cfg_mode);
			wlan_config_set_ch_band(wdev, wdev->PhyMode);
		}

#endif /*CONFIG_STA_SUPPORT*/
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
		/* for first time, update all phy mode is same as ra0 */
		for (i = 0; i < MAX_APCLI_NUM; i++) {
			pAd->StaCfg[i].wdev.PhyMode = pAd->ApCfg.MBSSID[0].wdev.PhyMode;
			wlan_config_set_ch_band(&pAd->StaCfg[i].wdev,
						pAd->ApCfg.MBSSID[0].wdev.PhyMode);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"Final: Apcli%d, PhyMode=%d\n",
					i, pAd->StaCfg[i].wdev.PhyMode);
		}

#endif /*APCLI_SUPPORT*/

#ifdef MBSS_SUPPORT

	/*Check if any wdev not configure a wireless mode, apply MSSID value to it.*/
	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;

		if (wdev->PhyMode == WMODE_INVALID) {
			wdev->PhyMode = pAd->ApCfg.MBSSID[0].wdev.PhyMode;
#ifdef DOT11_HE_AX
			if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G))
				wlan_config_set_he6g_op_present(wdev, 1);
			else
				wlan_config_set_he6g_op_present(wdev, 0);
#endif /* #ifdef DOT11_HE_AX */
			wlan_config_set_ch_band(wdev, wdev->PhyMode);
		}
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"Final: BSS%d PhyMode=%d\n", i, wdev->PhyMode);
	}

#endif/*MBSS_SUPPORT*/
#endif /*CONFIG_AP_SUPPORT*/

}

/*
   ==========================================================================
   Description:
       Set EDCCA mode by reading profile settings.
       0:disable, 1: set ED threshold to -49 dB for 2.4G or 5G

    Return:
	None.
   ==========================================================================
*/
VOID edcca_mode_set_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *buffer)
{
	UINT8 band_idx = 0;
	RTMP_STRING *ptr;

	for (band_idx = 0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), band_idx++) {
		if (band_idx < CFG_WIFI_RAM_BAND_NUM) {
			pAd->CommonCfg.u1EDCCAMode = simple_strtol(ptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				" edcca_mode band mode [%d]= %d\n",
				hc_get_hw_band_idx(pAd), pAd->CommonCfg.u1EDCCAMode);
		} else
			break;
	}
}

/*
    ========================================================================

    Routine Description:
	Add key parameter.

    Arguments:
	key			Pointer to key string
	value			Pointer to destination
	destsize		The datasize of the destination
	bTrimSpace	Set true if you want to strip the space character of the result pattern

    Return Value:
	TRUE                        Success
	FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
	For SSID and security key related parameters, we SHALL NOT trim the space(' ') character.
    ========================================================================
*/
INT RTMPAddKeyParameter(
	IN RTMP_STRING *key,
	IN CHAR *value,
	IN INT destsize,
	IN RTMP_STRING *buffer)
{
	ULONG str_tem_len = MAX_INI_BUFFER_SIZE - strlen(buffer);
	int ret = snprintf(buffer + strlen(buffer), str_tem_len,
		"%s=%s\n", key, value);
	if (os_snprintf_error(str_tem_len, ret)) {
		MTWF_PRINT("%s: snprintf error!\n", __func__);
		return FALSE;
	}
	return TRUE;
}

/*
    ========================================================================

    Routine Description:
	Set key parameter.

    Arguments:
	key			Pointer to key string
	value			Pointer to destination
	destsize		The datasize of the destination
	bTrimSpace	Set true if you want to strip the space character of the result pattern

    Return Value:
	TRUE                        Success
	FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
	For SSID and security key related parameters, we SHALL NOT trim the space(' ') character.
    ========================================================================
*/
INT RTMPSetKeyParameter(
	IN RTMP_STRING *key,
	OUT CHAR *value,
	IN INT destsize,
	IN RTMP_STRING *buffer,
	IN BOOLEAN bTrimSpace)
{
	RTMP_STRING buf[512] = "", *temp_buf1 = NULL;
	RTMP_STRING *start_ptr;
	RTMP_STRING *end_ptr;
	RTMP_STRING *offset = NULL;
	INT keyLen;
	INT start_len;
	INT end_len;
	INT len;

	keyLen = strlen(key);
	temp_buf1 = buf;

	/*find section*/
	offset = RTMPFindSection(buffer);
	if (offset == NULL)
		return FALSE;

	if (keyLen > (sizeof(buf) - 3)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			 "Input parameter incorrect, keyLen = %d\n", keyLen);
		return FALSE;
	}

	strlcpy(temp_buf1, "\n", strlen("\n") + 1);
	strncat(temp_buf1, key, strlen(key));
	strncat(temp_buf1, "=", strlen("="));

	/*search key*/
	start_ptr = rtstrstr(offset, temp_buf1);
	if (start_ptr == NULL) {
		/*can't searched, add directly*/
		RTMPAddKeyParameter(key, value, destsize, buffer);
		return TRUE;
	}

	/*remove original*/
	start_ptr += strlen("\n");
	start_len = strlen(start_ptr);

	end_ptr = rtstrstr(start_ptr, "\n");
	if (end_ptr == NULL)
		end_ptr = start_ptr + start_len;

	if (end_ptr < start_ptr)
		return FALSE;

	/*clear original setting*/
	end_ptr += strlen("\n");
	end_len = strlen(end_ptr);
	os_move_mem(start_ptr, end_ptr, end_len);
	start_ptr += end_len;
	len = start_len - end_len;
	os_zero_mem(start_ptr, len);
	/*fill new field & value*/
	RTMPAddKeyParameter(key, value, destsize, buffer);
	return TRUE;
}

/*
    ========================================================================

    Routine Description:
	Get multiple key parameter.

    Arguments:
	key                         Pointer to key string
	dest                        Pointer to destination
	destsize                    The datasize of the destination
	buffer                      Pointer to the buffer to start find the key

    Return Value:
	TRUE                        Success
	FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
    ========================================================================
*/
INT RTMPGetKeyParameterWithOffset(
	IN  RTMP_STRING *key,
	OUT RTMP_STRING *dest,
	OUT PUINT end_offset,
	IN  INT     destsize,
	IN  RTMP_STRING *buffer,
	IN	BOOLEAN	bTrimSpace)
{
	RTMP_STRING *temp_buf1 = NULL;
	RTMP_STRING *temp_buf2 = NULL;
	RTMP_STRING *start_ptr;
	RTMP_STRING *end_ptr;
	RTMP_STRING *ptr;
	RTMP_STRING *offset = 0;
	INT  len;

	os_alloc_mem(NULL, (PUCHAR *)&temp_buf1, MAX_PARAM_BUFFER_SIZE);

	if (temp_buf1 == NULL)
		return FALSE;

	os_alloc_mem(NULL, (PUCHAR *)&temp_buf2, MAX_PARAM_BUFFER_SIZE);

	if (temp_buf2 == NULL) {
		os_free_mem((PUCHAR)temp_buf1);
		return FALSE;
	}

	/*find section		*/
	if (*end_offset == 0) {
		offset = RTMPFindSection(buffer);

		if (offset == NULL) {
			os_free_mem((PUCHAR)temp_buf1);
			os_free_mem((PUCHAR)temp_buf2);
			return FALSE;
		}
	} else
		offset = buffer + (*end_offset);

	strlcpy(temp_buf1, "\n", strlen("\n") + 1);
	strncat(temp_buf1, key, strlen(key));
	strncat(temp_buf1, "=", strlen("="));
	/*search key*/
	start_ptr = rtstrstr(offset, temp_buf1);

	if (start_ptr == NULL) {
		os_free_mem((PUCHAR)temp_buf1);
		os_free_mem((PUCHAR)temp_buf2);
		return FALSE;
	}

	start_ptr += strlen("\n");
	end_ptr = rtstrstr(start_ptr, "\n");

	if (end_ptr == NULL)
		end_ptr = start_ptr + strlen(start_ptr);

	if (end_ptr < start_ptr) {
		os_free_mem((PUCHAR)temp_buf1);
		os_free_mem((PUCHAR)temp_buf2);
		return FALSE;
	}

	*end_offset = end_ptr - buffer;
	NdisMoveMemory(temp_buf2, start_ptr, end_ptr - start_ptr);
	temp_buf2[end_ptr - start_ptr] = '\0';
	strlcpy(temp_buf1, temp_buf2, strlen(temp_buf2) + 1);
	start_ptr = rtstrstr(temp_buf1, "=");

	if (start_ptr == NULL) {
		os_free_mem((PUCHAR)temp_buf1);
		os_free_mem((PUCHAR)temp_buf2);
		return FALSE;
	}

	strlcpy(temp_buf2, start_ptr + 1, strlen(start_ptr + 1) + 1);
	ptr = temp_buf2;

	/*trim space or tab*/
	while (*ptr != 0x00) {
		if ((bTrimSpace && (*ptr == ' ')) || (*ptr == '\t'))
			ptr++;
		else
			break;
	}

	len = strlen(ptr);
	memset(dest, 0x00, destsize);
	strlcpy(dest, ptr, len >= destsize ?  destsize : len);
	os_free_mem((PUCHAR)temp_buf1);
	os_free_mem((PUCHAR)temp_buf2);
	return TRUE;
}

#ifdef CONFIG_AP_SUPPORT

#ifdef APCLI_SUPPORT
static void rtmp_read_ap_client_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *buffer)
{
	RTMP_STRING *macptr = NULL;
	INT			i = 0, j = 0;
	UCHAR		macAddress[MAC_ADDR_LEN];
	PSTA_ADMIN_CONFIG   pApCliEntry = NULL;
	struct wifi_dev *wdev = NULL;
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
	UINT_8 idx = 0;
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */


	for (i = 0; i < MAX_APCLI_NUM; i++) {

#ifdef DOT11W_PMF_SUPPORT
		pAd->StaCfg[i].wdev.SecConfig.PmfCfg.Desired_MFPC = FALSE;
		pAd->StaCfg[i].wdev.SecConfig.PmfCfg.Desired_MFPR = FALSE;
		pAd->StaCfg[i].wdev.SecConfig.PmfCfg.Desired_PMFSHA256 = FALSE;
#endif /* DOT11W_PMF_SUPPORT */

		pAd->StaCfg[i].BssType = BSS_INFRA;
	}



#ifdef CONVERTER_MODE_SWITCH_SUPPORT
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCFG_PROFILE, DBG_LVL_DEBUG,
			"Initially Start any BSS or AP with Default settings\n");
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
			pAd->ApCfg.MBSSID[idx].APStartPseduState = AP_STATE_ALWAYS_START_AP_DEFAULT;
		for (idx = 0; idx < MAX_APCLI_NUM; idx++)
			pAd->StaCfg[idx].ApCliMode = APCLI_MODE_ALWAYS_START_AP_DEFAULT;
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */

	/*ApCliEnable*/
	if (RTMPGetKeyParameter("ApCliEnable", tmpbuf, 128, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			pApCliEntry = &pAd->StaCfg[i];

			if ((strncmp(macptr, "0", 1) == 0))
				pApCliEntry->ApcliInfStat.Enable = FALSE;
			else if ((strncmp(macptr, "1", 1) == 0))
				pApCliEntry->ApcliInfStat.Enable = TRUE;
			else {
				pApCliEntry->ApcliInfStat.Enable = FALSE;

#ifdef CONVERTER_MODE_SWITCH_SUPPORT
				if (strncmp(macptr, "2", 1) == 0) {
						pApCliEntry->ApcliInfStat.Enable = TRUE;
						pApCliEntry->ApCliMode = APCLI_MODE_START_AP_AFTER_APCLI_CONNECTION;
						MTWF_DBG(pAd,
						DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"Start any BSS or AP, after ApcliConnection with RootAP\n");
				} else if (strncmp(macptr, "3", 1) == 0) {
						pApCliEntry->ApcliInfStat.Enable = TRUE;
						pApCliEntry->ApCliMode = APCLI_MODE_NEVER_START_AP;
						MTWF_DBG(pAd,
						DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"Do Not start any BSS or AP\n");
				}

				for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
					/* When ApCliEnable=2: Start AP, when Apcli connect to RootAP */
					/* When ApCliEnable=3: Never start any(BSS) AP */
					if (strncmp(macptr, "2", 1) == 0) {
						pAd->ApCfg.MBSSID[idx].APStartPseduState =
							AP_STATE_START_AFTER_APCLI_CONNECTION;
					} else if (strncmp(macptr, "3", 1) == 0) {
							pAd->ApCfg.MBSSID[idx].APStartPseduState = AP_STATE_NEVER_START_AP;
					}
				}
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */

			}

			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"ApCliEntry[%d].Enable=%d\n", i, pApCliEntry->ApcliInfStat.Enable);
		}
	}

	/*ApCliSsid*/
	if (RTMPGetKeyParameter("ApCliSsid", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, FALSE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {

			pApCliEntry = &pAd->StaCfg[i];
			/*Ssid acceptable strlen must be less than 32 and bigger than 0.*/
			pApCliEntry->CfgSsidLen = (UCHAR)strlen(macptr);

			if (pApCliEntry->CfgSsidLen > 32) {
				pApCliEntry->CfgSsidLen = 0;
				continue;
			}

			if (pApCliEntry->CfgSsidLen > 0) {
				memcpy(&pApCliEntry->CfgSsid, macptr, pApCliEntry->CfgSsidLen);
				pApCliEntry->ApcliInfStat.Valid = FALSE;/* it should be set when successfuley association*/
			} else {
				NdisZeroMemory(&(pApCliEntry->CfgSsid), MAX_LEN_OF_SSID);
				continue;
			}

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"ApCliEntry[%d].CfgSsidLen=%d, CfgSsid=%s\n", i,
				pApCliEntry->CfgSsidLen, pApCliEntry->CfgSsid);
		}
	}

	/*ApCliWirelessMode*/
	if (RTMPGetKeyParameter("ApCliWirelessMode", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			UCHAR cfg_mode;

			cfg_mode = os_str_tol(macptr, 0, 10);
			pApCliEntry = &pAd->StaCfg[i];
			pApCliEntry->wdev.PhyMode = cfgmode_2_wmode(cfg_mode);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"ApCliEntry[%d].wdev.PhyMode=%d\n", i,
				pApCliEntry->wdev.PhyMode);
		}
	}

	/*ApCliBssid*/
	if (RTMPGetKeyParameter("ApCliBssid", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {

			pApCliEntry = &pAd->StaCfg[i];

			if (strlen(macptr) != 17) /*Mac address acceptable format 01:02:03:04:05:06 length 17*/
				continue;

			if (strcmp(macptr, "00:00:00:00:00:00") == 0)
				continue;

			for (j = 0; j < MAC_ADDR_LEN; j++) {
				AtoH(macptr, &macAddress[j], 1);
				macptr = macptr + 3;
			}

			memcpy(pApCliEntry->CfgApCliBssid, &macAddress, MAC_ADDR_LEN);
			pApCliEntry->ApcliInfStat.Valid = FALSE;/* it should be set when successfuley association*/
		}
	}

	/* ApCliTxMode*/
	if (RTMPGetKeyParameter("ApCliTxMode", tmpbuf, 25, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {

			wdev = &pAd->StaCfg[i].wdev;
			wdev->DesiredTransmitSetting.field.FixedTxMode =
				RT_CfgSetFixedTxPhyMode(macptr);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"I/F(apcli%d) Tx Mode = %d\n", i,
				wdev->DesiredTransmitSetting.field.FixedTxMode);
		}
	}

	/* ApCliTxMcs*/
	if (RTMPGetKeyParameter("ApCliTxMcs", tmpbuf, 50, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {

			wdev = &pAd->StaCfg[i].wdev;
			wdev->DesiredTransmitSetting.field.MCS =
				RT_CfgSetTxMCSProc(macptr, &wdev->bAutoTxRateSwitch);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"I/F(apcli%d) Tx MCS = %s(%d)\n", i,
				(wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO ? "AUTO" : ""),
				wdev->DesiredTransmitSetting.field.MCS);
		}
	}

#ifdef WSC_AP_SUPPORT

	/* Wsc4digitPinCode = TRUE use 4-digit Pin code, otherwise 8-digit Pin code */
	if (RTMPGetKeyParameter("ApCli_Wsc4digitPinCode", tmpbuf, 32, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {

			if (os_str_tol(macptr, 0, 10) != 0)
				pAd->StaCfg[i].wdev.WscControl.WscEnrollee4digitPinCode = TRUE;
			else /* Disable */
				pAd->StaCfg[i].wdev.WscControl.WscEnrollee4digitPinCode = FALSE;

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"I/F(apcli%d) ApCli_Wsc4digitPinCode=%d\n", i,
				pAd->StaCfg[i].wdev.WscControl.WscEnrollee4digitPinCode);
		}
	}

#ifdef APCLI_SUPPORT
	/* ApCliWscScanMode */
	if (RTMPGetKeyParameter("ApCliWscScanMode", tmpbuf, 32, buffer, TRUE)) {
		UCHAR Mode;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			Mode = simple_strtol(macptr, 0, 10);
			if (Mode != TRIGGER_PARTIAL_SCAN)
				Mode = TRIGGER_FULL_SCAN;

			pAd->StaCfg[i].wdev.WscControl.WscApCliScanMode = Mode;
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"I/F(apcli%d) WscApCliScanMode=%d\n", i, Mode);
		}
	}
#endif /* APCLI_SUPPORT */

#endif /* WSC_AP_SUPPORT */
#ifdef UAPSD_SUPPORT

	/*UAPSDCapable*/
	if (RTMPGetKeyParameter("ApCliUAPSDCapable", tmpbuf, 10, buffer, TRUE) ||
		RTMPGetKeyParameter("ApCliAPSDCapable", tmpbuf, 10, buffer, TRUE)) { /* backward compatible with old SDK */
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;
		pAd->ApCfg.FlgApCliIsUapsdInfoUpdated = TRUE;

		for (i = 0, macptr = rstrtok(tmpbuf, ";");
			 (macptr && i < MAX_APCLI_NUM);
			 macptr = rstrtok(NULL, ";"), i++) {

			pApCliEntry = &pAd->StaCfg[i];
			pApCliEntry->wdev.UapsdInfo.bAPSDCapable = \
					(UCHAR) os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"ApCliUAPSDCapable[%d]=%d\n", i,
				pApCliEntry->wdev.UapsdInfo.bAPSDCapable);
		}
	}

#endif /* UAPSD_SUPPORT */

	/* ApCliNum */
	if (RTMPGetKeyParameter("ApCliNum", tmpbuf, 10, buffer, TRUE)) {
		if (os_str_tol(tmpbuf, 0, 10) <= MAX_APCLI_NUM)
			pAd->ApCfg.ApCliNum = os_str_tol(tmpbuf, 0, 10);

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"I/F(apcli) ApCliNum=%d\n", pAd->ApCfg.ApCliNum);
	}

#ifdef APCLI_CONNECTION_TRIAL
	pAd->ApCfg.ApCliNum++;

	/* ApCliTrialCh */
	if (RTMPGetKeyParameter("ApCliTrialCh", tmpbuf, 128, buffer, TRUE)) {
		/* last IF is for apcli connection trial */
		pApCliEntry = &pAd->StaCfg[pAd->ApCfg.ApCliNum - 1];
		pApCliEntry->TrialCh = (UCHAR) os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"TrialChannel=%d\n", pApCliEntry->TrialCh);
	}

#endif /* APCLI_CONNECTION_TRIAL */
#ifdef DOT11W_PMF_SUPPORT

	/* Protection Management Frame Capable */
	if (RTMPGetKeyParameter("ApCliPMFMFPC", tmpbuf, 32, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0,
			 macptr = rstrtok(tmpbuf, ";");
			 (macptr && i < MAX_APCLI_NUM);
			 macptr = rstrtok(NULL, ";"),
			 i++) {
			POS_COOKIE pObj;
			INT backup_ioctl_if;
			INT backup_ioctl_if_type;

			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pObj = (POS_COOKIE) pAd->OS_Cookie;
			backup_ioctl_if = pObj->ioctl_if;
			backup_ioctl_if_type = pObj->ioctl_if_type;
			pObj->ioctl_if = i;
			pObj->ioctl_if_type = INT_APCLI;
			Set_ApCliPMFMFPC_Proc(pAd, macptr);
			pObj->ioctl_if = backup_ioctl_if;
			pObj->ioctl_if_type = backup_ioctl_if_type;
		}
	}

	/* Protection Management Frame Required */
	if (RTMPGetKeyParameter("ApCliPMFMFPR", tmpbuf, 32, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0,
			 macptr = rstrtok(tmpbuf, ";");
			 (macptr && i < MAX_APCLI_NUM);
			 macptr = rstrtok(NULL, ";"),
			 i++) {
			POS_COOKIE pObj;
			INT backup_ioctl_if;
			INT backup_ioctl_if_type;

			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pObj = (POS_COOKIE) pAd->OS_Cookie;
			backup_ioctl_if = pObj->ioctl_if;
			backup_ioctl_if_type = pObj->ioctl_if_type;
			pObj->ioctl_if = i;
			pObj->ioctl_if_type = INT_APCLI;
			Set_ApCliPMFMFPR_Proc(pAd, macptr);
			pObj->ioctl_if = backup_ioctl_if;
			pObj->ioctl_if_type = backup_ioctl_if_type;
		}
	}

	if (RTMPGetKeyParameter("ApCliPMFSHA256", tmpbuf, 32, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0,
			 macptr = rstrtok(tmpbuf, ";");
			 (macptr && i < MAX_APCLI_NUM);
			 macptr = rstrtok(NULL, ";"),
			 i++) {
			POS_COOKIE pObj;
			INT backup_ioctl_if;
			INT backup_ioctl_if_type;

			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pObj = (POS_COOKIE) pAd->OS_Cookie;
			backup_ioctl_if = pObj->ioctl_if;
			backup_ioctl_if_type = pObj->ioctl_if_type;
			pObj->ioctl_if = i;
			pObj->ioctl_if_type = INT_APCLI;
			Set_ApCliPMFSHA256_Proc(pAd, macptr);
			pObj->ioctl_if = backup_ioctl_if;
			pObj->ioctl_if_type = backup_ioctl_if_type;
		}
	}

#endif /* DOT11W_PMF_SUPPORT */

#ifdef DOT11_HE_AX
	/* ApCliMuOfdmaDlEnable */
	if (RTMPGetKeyParameter("ApCliMuOfdmaDlEnable", tmpbuf, 128, buffer, FALSE)) {
		UCHAR mu_dl_ofdma = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			mu_dl_ofdma = os_str_tol(macptr, 0, 10);

			if (IF_COMBO_HAVE_AP_STA(pAd)) {
				wdev = &pAd->StaCfg[i].wdev;
			}

			if (wdev) {
				wlan_config_set_mu_dl_ofdma(wdev, mu_dl_ofdma);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"APCLI[%d] ApCliMuOfdmaDlEnable = %d\n", i, mu_dl_ofdma);
			}
		}
	}

	/* ApCliMuOfdmaUlEnable */
	if (RTMPGetKeyParameter("ApCliMuOfdmaUlEnable", tmpbuf, 128, buffer, FALSE)) {
		UCHAR mu_ul_ofdma = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			mu_ul_ofdma = os_str_tol(macptr, 0, 10);

			if (IF_COMBO_HAVE_AP_STA(pAd)) {
				wdev = &pAd->StaCfg[i].wdev;
			}

			if (wdev) {
				wlan_config_set_mu_ul_ofdma(wdev, mu_ul_ofdma);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"APCLI[%d] ApCliMuOfdmaUlEnable = %d\n", i, mu_ul_ofdma);
			}
		}
	}

	/* ApCliMuMimoDlEnable */
	if (RTMPGetKeyParameter("ApCliMuMimoDlEnable", tmpbuf, 128, buffer, FALSE)) {
		UCHAR mu_dl_mimo = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			mu_dl_mimo = os_str_tol(macptr, 0, 10);

			if (IF_COMBO_HAVE_AP_STA(pAd)) {
				wdev = &pAd->StaCfg[i].wdev;
			}

			if (wdev) {
				wlan_config_set_mu_dl_mimo(wdev, mu_dl_mimo);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"APCLI[%d] ApCliMuMimoDlEnable = %d\n", i, mu_dl_mimo);
			}
		}
	}

	/* ApCliMuMimoUlEnable */
	if (RTMPGetKeyParameter("ApCliMuMimoUlEnable", tmpbuf, 128, buffer, FALSE)) {
		UCHAR mu_ul_mimo = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			mu_ul_mimo = os_str_tol(macptr, 0, 10);

			if (IF_COMBO_HAVE_AP_STA(pAd)) {
				wdev = &pAd->StaCfg[i].wdev;
			}

			if (wdev) {
				wlan_config_set_mu_ul_mimo(wdev, mu_ul_mimo);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"APCLI[%d] ApCliMuMimoUlEnable = %d\n", i, mu_ul_mimo);
			}
		}
	}
#endif /* #ifdef DOT11_HE_AX */
}
#endif /* APCLI_SUPPORT */

#endif /* CONFIG_AP_SUPPORT */

INT rtmp_band_index_get_by_order(struct _RTMP_ADAPTER *pAd, UCHAR order)
{
	INT ret = DBDC_BAND0;

	if (order == 0)
		ret = DBDC_BAND0;

	if (order == 1)
		ret = DBDC_BAND1;

	return ret;
}


static UCHAR band_order_check(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR order)
{
	UCHAR ret = 0;

	if (pAd->CommonCfg.dbdc_mode) {
		if (((order == 0) && WMODE_CAP_2G(wdev->PhyMode))
			|| ((order == 1) && WMODE_CAP_5G(wdev->PhyMode)))
			ret = 1;
	} else
		ret = 1;

	return ret;
}

static struct wifi_dev *get_curr_wdev(struct _RTMP_ADAPTER *pAd, UCHAR idx)
{
#ifdef APCLI_SUPPORT
	UCHAR curidx;
#endif

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if ((idx < MAX_MBSSID_NUM(pAd))
			&& (idx < pAd->ApCfg.BssidNum)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BSSID[%d]\n", idx);
			return &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].wdev;
		}
#ifdef APCLI_SUPPORT
		curidx = idx - pAd->ApCfg.BssidNum;
		if ((curidx < MAX_MULTI_STA)
			&& (curidx < MAX_APCLI_NUM)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"APCLI[%d]\n", curidx);
			return &pAd->StaCfg[curidx].wdev;
		}
#endif
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (idx < MAX_MULTI_STA) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"STA[%d]\n", idx);
			return &pAd->StaCfg[idx].wdev;
		}
	}
#endif /*CONFIG_STA_SUPPORT*/
	return NULL;
}

static void read_frag_thld_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i = 0;
	UINT32 frag_thld = 0;
	RTMP_STRING *macptr = NULL;
	struct wifi_dev *wdev = NULL;

	if (RTMPGetKeyParameter("FragThreshold", tmpbuf, PER_BSS_SIZE_5(pAd), buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			frag_thld = os_str_tol(macptr, 0, 10);

			if (frag_thld > MAX_FRAG_THRESHOLD || frag_thld < MIN_FRAG_THRESHOLD)
				frag_thld = MAX_FRAG_THRESHOLD;
			else if (frag_thld % 2 == 1)
				frag_thld -= 1;

			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					 "profile: FragThreshold[%d]=%d\n", i, frag_thld);

			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_frag_thld(wdev, frag_thld);
			else
				break;
		}
	}
}

static VOID read_rts_pkt_thld_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i = 0;
	UINT32 rts_pkt_thld = 0;
	RTMP_STRING *macptr = NULL;
	struct wifi_dev *wdev = NULL;

	if (RTMPGetKeyParameter("RTSPktThreshold", tmpbuf, PER_BSS_SIZE_5(pAd), buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			rts_pkt_thld = os_str_tol(macptr, 0, 10);

			if ((rts_pkt_thld < 1) || (rts_pkt_thld > MAX_RTS_PKT_THRESHOLD))
				rts_pkt_thld = MAX_RTS_PKT_THRESHOLD;

			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					 "profile: RTSPktThreshold[%d]=%d\n", i, rts_pkt_thld);

			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_rts_pkt_thld(wdev, rts_pkt_thld);
			else
				break;
		}
	}
}

static VOID read_rts_len_thld_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i = 0;
	UINT32 rts_thld = 0;
	RTMP_STRING *macptr = NULL;
	struct wifi_dev *wdev = NULL;


	if (RTMPGetKeyParameter("RTSThreshold", tmpbuf, PER_BSS_SIZE_5(pAd), buf, FALSE)) {
		if (pAd->CommonCfg.dbdc_mode) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				rts_thld = (UINT32)os_str_tol(macptr, 0, 10);
				if ((rts_thld > MAX_RTS_THRESHOLD) || (rts_thld < 1))
					rts_thld = MAX_RTS_THRESHOLD;
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					 "profile: RTSThreshold[%d]=%d\n", i, rts_thld);
				wdev = get_curr_wdev(pAd, i);
				if (wdev)
					wlan_config_set_rts_len_thld(wdev, rts_thld);
				else
					break;
			}
		} else {
#ifdef CONFIG_AP_SUPPORT
			UCHAR mbss_idx = 0;
#endif
			rts_thld = (UINT32)os_str_tol(tmpbuf, 0, 10);
			if ((rts_thld > MAX_RTS_THRESHOLD) || (rts_thld < 1))
				rts_thld = MAX_RTS_THRESHOLD;
#ifdef CONFIG_AP_SUPPORT
			for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
				struct wifi_dev *mbss_wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, mbss_idx)].wdev;
				wlan_config_set_rts_len_thld(mbss_wdev, rts_thld);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"mbss[idx=%d], RTSThreshold[%d]=%d\n", mbss_idx, mbss_idx,
						 rts_thld);
			}
#endif
		}
	}
}

#ifdef CONFIG_STA_SUPPORT
static void rtmp_read_sta_wmm_parms_from_file(IN  PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	RTMP_STRING *macptr;
	UINT i = 0;
#if MAX_MULTI_STA > 1
	UINT j = 0;
#endif
	BOOLEAN bWmmEnable = FALSE;

	/*WmmCapable*/
	if (RTMPGetKeyParameter("WmmCapable", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= MAX_MULTI_STA) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					 "Input parameter[WmmCapable] incorrect\n");
				return;
			}
			if (os_str_tol(macptr, 0, 10) != 0) { /*Enable*/
				pAd->StaCfg[i].wdev.bWmmCapable = TRUE;
				bWmmEnable = TRUE;
			} else /*Disable*/
				pAd->StaCfg[i].wdev.bWmmCapable = FALSE;

#if MAX_MULTI_STA > 1
			if (i == 0) {
				/* First setting is also the default value for others */
				for (j = 1; j < MAX_MULTI_STA; j++)
					pAd->StaCfg[j].wdev.bWmmCapable = pAd->StaCfg[i].wdev.bWmmCapable;
			}
#endif

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "WmmCapable=%d\n", pAd->StaCfg[i].wdev.bWmmCapable);
		}
	}

	/*AckPolicy for AC_BK, AC_BE, AC_VI, AC_VO*/
	if (RTMPGetKeyParameter("AckPolicy", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= WMM_NUM_OF_AC) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					 "Input parameter[WmmCapable] incorrect\n");
				return;
			}
			pAd->CommonCfg.AckPolicy[i] = (UCHAR)os_str_tol(macptr, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "AckPolicy[%d]=%d\n", i, pAd->CommonCfg.AckPolicy[i]);
		}

		wlan_config_set_ack_policy_all(&pAd->wpf, pAd->CommonCfg.AckPolicy);
	}

#ifdef UAPSD_SUPPORT

	if (bWmmEnable) {
		/*UAPSDCapable*/
		if (RTMPGetKeyParameter("UAPSDCapable", tmpbuf, 10, buffer, TRUE) ||
			RTMPGetKeyParameter("APSDCapable", tmpbuf, 10, buffer, TRUE)) { /* backward compatible with old SDK */
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				if (i >= MAX_MULTI_STA) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						 "Input parameter[UAPSDCapable] incorrect\n"
						);
					return;
				}
				if (os_str_tol(tmpbuf, 0, 10) != 0) /*Enable*/
					pAd->StaCfg[i].wdev.UapsdInfo.bAPSDCapable = TRUE;
				else
					pAd->StaCfg[i].wdev.UapsdInfo.bAPSDCapable = FALSE;

#if MAX_MULTI_STA > 1
				if (i == 0) {
					/* First setting is also the default value for others */
					for (j = 1; j < MAX_MULTI_STA; j++)
						pAd->StaCfg[j].wdev.UapsdInfo.bAPSDCapable = pAd->StaCfg[i].wdev.UapsdInfo.bAPSDCapable;
				}
#endif

				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"UAPSDCapable=%d\n",
					pAd->StaCfg[i].wdev.UapsdInfo.bAPSDCapable);
			}
		}

		/*MaxSPLength*/
		if (RTMPGetKeyParameter("MaxSPLength", tmpbuf, 10, buffer, TRUE)) {
			pAd->CommonCfg.MaxSPLength = os_str_tol(tmpbuf, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"MaxSPLength=%d\n", pAd->CommonCfg.MaxSPLength);
		}

		/*APSDAC for AC_BE, AC_BK, AC_VI, AC_VO*/
		if (RTMPGetKeyParameter("APSDAC", tmpbuf, 32, buffer, TRUE)) {
			BOOLEAN apsd_ac[WMM_NUM_OF_AC] = { 0 };

			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				if (i >= WMM_NUM_OF_AC) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						 "Input parameter[APSDAC] incorrect\n"
						);
					return;
				}
				apsd_ac[i] = (BOOLEAN)os_str_tol(macptr, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"APSDAC%d  %d\n", i,  apsd_ac[i]);
			}

			pAd->CommonCfg.bAPSDAC_BE = apsd_ac[0];
			pAd->CommonCfg.bAPSDAC_BK = apsd_ac[1];
			pAd->CommonCfg.bAPSDAC_VI = apsd_ac[2];
			pAd->CommonCfg.bAPSDAC_VO = apsd_ac[3];
			pAd->CommonCfg.bACMAPSDTr[0] = apsd_ac[0];
			pAd->CommonCfg.bACMAPSDTr[1] = apsd_ac[1];
			pAd->CommonCfg.bACMAPSDTr[2] = apsd_ac[2];
			pAd->CommonCfg.bACMAPSDTr[3] = apsd_ac[3];
		}
	}

#endif /* UAPSD_SUPPORT */
}

#ifdef XLINK_SUPPORT
static void rtmp_get_psp_xlink_mode_from_file(IN  PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	/* Xlink Mode*/
	if (RTMPGetKeyParameter("PSP_XLINK_MODE", tmpbuf, 32, buffer, TRUE)) {
		if (os_str_tol(tmpbuf, 0, 10) != 0) /* enable*/
			pAd->StaCfg[0].PSPXlink = TRUE;
		else /* disable*/
			pAd->StaCfg[0].PSPXlink = FALSE;

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"PSP_XLINK_MODE=%d\n", pAd->StaCfg[0].PSPXlink);
	}
}
#endif /* XLINK_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef DOT11_VHT_AC
static VOID read_vht_sgi(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;

	if (RTMPGetKeyParameter("VHT_SGI", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				wlan_config_set_vht_sgi(wdev, val);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"VHT: Short GI for 80Mhz/160Mhz  = %s\n",
						 (val == GI_800) ? "Disabled" : "Enable");
			} else
				break;
		}
	}
}

static VOID read_vht_stbc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;

	if (RTMPGetKeyParameter("VHT_STBC", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_vht_stbc(wdev, val);
			else
				break;
		}
	}
}

static VOID read_vht_ldpc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;

	if (RTMPGetKeyParameter("VHT_LDPC", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_vht_ldpc(wdev, val);
			else
				break;
		}
	}
}

static VOID read_vht_bw_sig(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;
	UCHAR *bwsig_str[] = {"NONE", "STATIC", "DYNAMIC"};

	if (RTMPGetKeyParameter("VHT_BW_SIGNAL", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);

			if (wdev) {
				if (val > BW_SIGNALING_DYNAMIC)
					val = BW_SIGNALING_DISABLE;
				wlan_config_set_vht_bw_sig(wdev, val);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"VHT: BW SIGNALING = %s\n", bwsig_str[val]);
			} else
				break;
		}
	}
}

static VOID read_vht_max_ampdu_length_exponent(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct _RTMP_CHIP_CAP *chip_cap;
	UINT32 max_ampdu_len;

	if (RTMPGetKeyParameter("AF_VHT", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {

		max_ampdu_len = os_str_tol(tmpbuf, 0, 10);
		if (max_ampdu_len == 0)
			max_ampdu_len = VHT_AMPDU_2_OF_POWER_13_M1;
		else if (max_ampdu_len == 1)
			max_ampdu_len = VHT_AMPDU_2_OF_POWER_14_M1;
		else if (max_ampdu_len == 2)
			max_ampdu_len = VHT_AMPDU_2_OF_POWER_15_M1;
		else if (max_ampdu_len == 3)
			max_ampdu_len = VHT_AMPDU_2_OF_POWER_16_M1;
		else if (max_ampdu_len == 4)
			max_ampdu_len = VHT_AMPDU_2_OF_POWER_17_M1;
		else if (max_ampdu_len == 5)
			max_ampdu_len = VHT_AMPDU_2_OF_POWER_18_M1;
		else if (max_ampdu_len == 6)
			max_ampdu_len = VHT_AMPDU_2_OF_POWER_19_M1;
		else if (max_ampdu_len == 7)
			max_ampdu_len = VHT_AMPDU_2_OF_POWER_20_M1;

		chip_cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);

		if (chip_cap)
			chip_cap->ppdu.vht_max_ampdu_len_exp = max_ampdu_len;

	}
}

static VOID read_vht_param_from_file(struct _RTMP_ADAPTER *pAd,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;
	long Value;
	UCHAR vht_bw;
	UCHAR cen_ch_2 = 0;
#ifdef CONFIG_AP_SUPPORT
	UCHAR IdBss = 0;
#endif /* CONFIG_AP_SUPPORT */
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* Channel Width */
	if (RTMPGetKeyParameter("VHT_BW", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			Value = os_str_tol(macptr, 0, 10);

			if (Value <= VHT_BW_8080)
				vht_bw = Value;
			else
				vht_bw = VHT_BW_2040;

			if (vht_bw == VHT_BW_8080 && !IS_PHY_CAPS(pChipCap->phy_caps, fPHY_CAP_BW160NC))
				vht_bw = VHT_BW_80;

			if (pAd->CommonCfg.dbdc_mode && (vht_bw > VHT_BW_80) &&
				!IS_PHY_CAPS(pChipCap->phy_caps, fPHY_CAP_BW160C_STD))
				vht_bw = VHT_BW_80;

			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				wlan_config_set_vht_bw(wdev, vht_bw);
			} else
				break;

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
			if (IS_SUPPORT_V10_DFS(pAd) && vht_bw == VHT_BW_2040) {
				/* Boot Time HT BW Update when VHT BW if VHT2040 */
				wlan_config_set_ht_bw(wdev, HT_BW_20);
#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
				pAd->CommonCfg.MCastPhyMode.field.BW = HT_BW_20;
				pAd->CommonCfg.MCastPhyMode_5G.field.BW = HT_BW_20;
#else
				pAd->CommonCfg.mcastphymode.field.BW = HT_BW_20;
#endif /* MCAST_VENDOR10_CUSTOM_FEATURE */
#endif /* MCAST_RATE_SPECIFIC */
			}
#endif
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				 "wdev[%d] VHT: Channel Width = %s MHz\n", i,
				  VhtBw2Str(vht_bw));

#ifdef CONFIG_AP_SUPPORT
			/* for first time, update all same as ra0 */
			if (i == 0) {
				for (IdBss = 1; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
					wdev = get_curr_wdev(pAd, PF_TO_BSS_IDX(pAd, IdBss));
					if (wdev)
						wlan_config_set_vht_bw(wdev, vht_bw);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}
	}

	/* VHT_SGI */
	read_vht_sgi(pAd, tmpbuf, buf);
	/* VHT_STBC */
	read_vht_stbc(pAd, tmpbuf, buf);
	/* VHT_LDPC */
	read_vht_ldpc(pAd, tmpbuf, buf);
	/* VHT_BW_SIGNAL */
	read_vht_bw_sig(pAd, tmpbuf, buf);
	/* MAX AMPDU length exponent */
	read_vht_max_ampdu_length_exponent(pAd, tmpbuf, buf);

	/* Disallow non-VHT connection */
	if (RTMPGetKeyParameter("VHT_DisallowNonVHT", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value == 0)
			pAd->CommonCfg.bNonVhtDisallow = FALSE;
		else
			pAd->CommonCfg.bNonVhtDisallow = TRUE;

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VHT: VHT_DisallowNonVHT = %d\n",
			pAd->CommonCfg.bNonVhtDisallow);
	}

	/* VHT Secondary80 */
	if (RTMPGetKeyParameter("VHT_Sec80_Channel", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			Value = os_str_tol(tmpbuf, 0, 10);

			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				UCHAR ch_band = wlan_config_get_ch_band(wdev);
				cen_ch_2 = vht_cent_ch_freq((UCHAR)Value, VHT_BW_80, ch_band);
				wlan_config_set_cen_ch_2(wdev, cen_ch_2);
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
				wdev->vht_sec_80_channel = Value;
#endif
			} else
				break;

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					 "wdev[%d] VHT: Secondary80 = %ld, Center = %d\n", i,
					  Value, cen_ch_2);
			}
	}

	/* 2.4G 256QAM */
	if (RTMPGetKeyParameter("G_BAND_256QAM", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);
		pAd->CommonCfg.g_band_256_qam = (Value) ? TRUE : FALSE;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				 "VHT: G_BAND_256QAM = %ld\n", Value);
	}

	/* Use VHT Rate for 2G Band */
	if (RTMPGetKeyParameter("UseVhtRateFor2g", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);
		pAd->CommonCfg.bUseVhtRateFor2g = (Value) ? TRUE : FALSE;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				 "VHT: UseVhtRateFor2g = %ld\n", Value);
	}
}

#endif /* DOT11_VHT_AC */

#ifdef DOT11_HE_AX
#ifdef FIXED_HE_GI_SUPPORT
static VOID read_he_gi_ltf(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;

	if (RTMPGetKeyParameter("FgiFltf", tmpbuf, PER_BSS_SIZE_8(pAd), buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				set_fgi_and_ltf_profile(pAd, wdev, val);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"WDEV[%d], FgiFltf = %d\n", i, val);
			} else
				break;
		}
	}
}
#endif

static VOID read_he_ldpc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;

	if (RTMPGetKeyParameter("HeLdpc", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				wlan_config_set_he_ldpc(wdev, val);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"WDEV[%d], HeLdpc = %d\n", i, val);
			} else
				break;
		}
	}
}

static VOID read_he_txop_dur_rts_thld(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;

	if (RTMPGetKeyParameter("HE_TXOP_RTS_THLD", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				wlan_config_set_he_txop_dur_rts_thld(wdev, val);

				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"WDEV[%d], HE_TXOP_RTS_THLD = %d\n", i, val);
			} else
				break;
		}
	}
}

static VOID read_ppdu_tx_type(struct _RTMP_ADAPTER *ad,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UINT8 ppdu_tx_type = 0;
	INT i;

	if (RTMPGetKeyParameter("PPDUTxType", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
				macptr = rstrtok(NULL, ";"), i++) {
			ppdu_tx_type = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(ad, i);
			if (wdev)
				wlan_config_set_ppdu_tx_type(wdev, ppdu_tx_type);
			else
				break;
			MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"WDEV[%d], PPDUTxType = %d\n", i, ppdu_tx_type);
		}
	}
}

static VOID read_num_users_ofdma(struct _RTMP_ADAPTER *ad,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UINT8 user_cnt = 0;
	INT i;

	if (RTMPGetKeyParameter("NumUsersOFDMA", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
				macptr = rstrtok(NULL, ";"), i++) {
			user_cnt = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(ad, i);
			if (wdev)
				wlan_config_set_ofdma_user_cnt(wdev, user_cnt);
			else
				break;
			MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"WDEV[%d], NumUsersOFDMA = %d\n", i, user_cnt);
		}
	}
}

static VOID read_mu_edca_override(struct _RTMP_ADAPTER *ad,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UINT8 input = 0;
	BOOLEAN override = FALSE;
	INT i;

	if (RTMPGetKeyParameter("MuEdcaOverride", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
				macptr = rstrtok(NULL, ";"), i++) {
			input = os_str_tol(macptr, 0, 10);
			override = (input == 0) ? FALSE : TRUE;
			wdev = get_curr_wdev(ad, i);
			if (wdev)
				wlan_config_set_mu_edca_override(wdev, override);
			else
				break;
			MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"WDEV[%d], MuEdcaOverride = %d\n", i, override);
		}
	}
}

static VOID read_non_tx_bss_idx(struct _RTMP_ADAPTER *ad,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UINT8 bss_idx = 0;
	INT i;

	if (RTMPGetKeyParameter("NonTxBSSIndex", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
				macptr = rstrtok(NULL, ";"), i++) {
			bss_idx = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(ad, i);
			if (wdev)
				wlan_config_set_non_tx_bss_idx(wdev, bss_idx);
			else
				break;
			MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"NonTxBSSIndex = %d\n", bss_idx);
		}
	}
}

static VOID read_ofdma_direction(struct _RTMP_ADAPTER *ad,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	INT i;
	UINT8 ofdma_dir = 0;

	if (RTMPGetKeyParameter("OFDMA", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
				macptr = rstrtok(NULL, ";"), i++) {
			ofdma_dir = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(ad, i);
			if (wdev)
				wlan_config_set_ofdma_direction(wdev, ofdma_dir);
			else
				break;
			MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"OFDMA = %d\n", ofdma_dir);
		}
	}
}

static VOID read_vht_1024qam_support(struct _RTMP_ADAPTER *ad,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	long Value;

	/* VHT 1024QAM */
	if (RTMPGetKeyParameter("Vht1024QamSupport", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);
		ad->CommonCfg.vht_1024_qam = (Value) ? TRUE : FALSE;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				 "Vht1024QamSupport = %ld\n", Value);
	}
}

static VOID read_he_max_ampdu_length_exponent(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct _RTMP_CHIP_CAP *chip_cap;
	UINT16 max_ampdu_len;

	if (RTMPGetKeyParameter("AF_HE", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {

		max_ampdu_len = os_str_tol(tmpbuf, 0, 10);
		if (max_ampdu_len == 0)
			max_ampdu_len = HE_AMPDU_2_OF_POWER_13_M1;
		else if (max_ampdu_len == 1)
			max_ampdu_len = HE_AMPDU_2_OF_POWER_14_M1;
		else if (max_ampdu_len == 2)
			max_ampdu_len = HE_AMPDU_2_OF_POWER_15_M1;
		else if (max_ampdu_len == 3)
			max_ampdu_len = HE_AMPDU_2_OF_POWER_16_M1;

		chip_cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);

		if (chip_cap)
			chip_cap->ppdu.he_max_ampdu_len_exp = max_ampdu_len;

	}
}

static VOID read_he_param_from_file(struct _RTMP_ADAPTER *pAd,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	read_he_ldpc(pAd, tmpbuf, buf);
#if defined(DOT11_HE_AX) && defined(FIXED_HE_GI_SUPPORT)
	read_he_gi_ltf(pAd, tmpbuf, buf);
#endif
	read_he_txop_dur_rts_thld(pAd, tmpbuf, buf);
	read_ppdu_tx_type(pAd, tmpbuf, buf);
	read_num_users_ofdma(pAd, tmpbuf, buf);
	read_mu_edca_override(pAd, tmpbuf, buf);
	read_non_tx_bss_idx(pAd, tmpbuf, buf);
	read_ofdma_direction(pAd, tmpbuf, buf);
	read_vht_1024qam_support(pAd, tmpbuf, buf);
	read_he_max_ampdu_length_exponent(pAd, tmpbuf, buf);
}
#endif


#ifdef DOT11_N_SUPPORT
static VOID read_fixed_mcs(struct _RTMP_ADAPTER *ad,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	INT i = 0;
	UCHAR fixed_mcs;

	if (RTMPGetKeyParameter("FixedMcs", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
				macptr = rstrtok(NULL, ";"), i++) {
			fixed_mcs = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(ad, i);
			if (wdev)
				wlan_config_set_fixed_mcs(wdev, fixed_mcs);
			else
				break;
			MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"WDEV[%d], FixedMcs = %d\n", i, fixed_mcs);
		}
	}
}

static VOID read_min_mpdu_start_space(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	INT i = 0;
	UCHAR mpdu_density = 0;

	if (RTMPGetKeyParameter("HT_MpduDensity", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			mpdu_density = os_str_tol(macptr, 0, 10);
			if (mpdu_density > 7)
				mpdu_density = 4;
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"HT: MPDU Density = %d\n", (INT) mpdu_density);

			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_min_mpdu_start_space(wdev, mpdu_density);
			else
				break;
		}
	}
}

static VOID read_ht_protect(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht_protect_en = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_PROTECT", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht_protect_en = os_str_tol(macptr, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					 "HT_PROTECT=%s\n", (ht_protect_en) ? "Enable" : "Disable");
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_ht_protect_en(wdev, ht_protect_en);
			else
				break;
		}
	}
}

static VOID read_ht_gi(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht_gi;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_GI", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht_gi = os_str_tol(macptr, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					 "HT_GI = %s\n", (ht_gi == GI_400) ? "GI_400" : "GI_800");
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_ht_gi(wdev, ht_gi);
			else
				break;
		}
	}
}

static VOID read_40M_intolerant(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht40_intolerant = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_40MHZ_INTOLERANT", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht40_intolerant = os_str_tol(macptr, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"HT: 40MHZ INTOLERANT = %d\n", ht40_intolerant);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_40M_intolerant(wdev, ht40_intolerant);
			else
				break;
		}
	}
}

static VOID read_ht_ldpc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht_ldpc = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_LDPC", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht_ldpc = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_ht_ldpc(wdev, ht_ldpc);
			else
				break;
		}
	}
}

static VOID read_ht_stbc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht_stbc = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_STBC", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht_stbc = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_ht_stbc(wdev, ht_stbc);
			else
				break;
		}
	}
}

static VOID read_ht_mode(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht_mode = 0;
	INT i = 0;

	/* HT Operation Mode : Mixed Mode , Green Field*/
	if (RTMPGetKeyParameter("HT_OpMode", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht_mode = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_ht_mode(wdev, ht_mode);
			else
				break;
		}
	}
}

static VOID read_txrx_stream_num(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	struct mcs_nss_caps *nss_cap = MCS_NSS_CAP(pAd);
	RTMP_STRING *macptr = NULL;
	UCHAR tx_stream = 0, rx_stream = 0;
	INT i = 0, j = 0, boundary_idx = MAX_MULTI_STA, start_idx = 0;

#if defined(CONFIG_AP_SUPPORT)
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		boundary_idx = pAd->ApCfg.BssidNum;
#endif

	if (RTMPGetKeyParameter("HT_TxStream", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			tx_stream = os_str_tol(macptr, 0, 10);

			for (j = i*start_idx; j < boundary_idx; j++) {
				wdev = get_curr_wdev(pAd, j);
				if (wdev) {
					wlan_config_set_tx_stream(wdev, min(tx_stream, nss_cap->max_nss));
#ifdef DOT11_HE_AX
					wlan_config_set_he_tx_nss(wdev, min(tx_stream, nss_cap->max_nss));
#endif /* DOT11_HE_AX */
#ifdef DOT11_EHT_BE
					wlan_config_set_eht_tx_nss(wdev, min(tx_stream, nss_cap->max_nss));
#endif /* DOT11_EHT_BE */
				}
			}
		}
	}

	if (RTMPGetKeyParameter("HT_RxStream", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			rx_stream = os_str_tol(macptr, 0, 10);

			for (j = i*start_idx; j < boundary_idx; j++) {
				wdev = get_curr_wdev(pAd, j);
				if (wdev) {
					wlan_config_set_rx_stream(wdev, min(rx_stream, nss_cap->max_nss));
#ifdef DOT11_HE_AX
					wlan_config_set_he_rx_nss(wdev, min(rx_stream, nss_cap->max_nss));
#endif /* DOT11_HE_AX */
#ifdef DOT11_EHT_BE
					wlan_config_set_eht_rx_nss(wdev, min(tx_stream, nss_cap->max_nss));
#endif /* DOT11_EHT_BE */
				}
			}
		}
	}
}

static VOID read_amsdu_enable(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR amsdu_enable = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_AMSDU", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			amsdu_enable = os_str_tol(macptr, 0, 10);

			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_amsdu_en(wdev, amsdu_enable);
			else
				break;
		}
	}
}

static VOID read_amsdu_num(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UINT8 amsdu_num, max_mpdu_len = MPDU_7991_OCTETS;
	INT i = 0;

	if (RTMPGetKeyParameter("AMSDU_NUM", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			amsdu_num = os_str_tol(macptr, 0, 10);
			if (amsdu_num < 3)
				max_mpdu_len = MPDU_3895_OCTETS;
			if (amsdu_num > 4)
				max_mpdu_len = MPDU_11454_OCTETS;

			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_vht_max_mpdu_len(wdev, max_mpdu_len);
			else
				break;
		}
	}
}

static VOID read_mmps(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR mmps = 0;
	INT i = 0, idx = 0;

	if (RTMPGetKeyParameter("HT_MIMOPSMode", tmpbuf, 25, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i > CFG_WIFI_RAM_BAND_NUM)
				break;

			mmps = os_str_tol(macptr, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"HT: MIMOPS Mode  = %d\n", mmps);
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].wdev;
					if (band_order_check(pAd, wdev, i))
						wlan_config_set_mmps(wdev, mmps);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				for (idx = 0; idx < MAX_MULTI_STA; idx++) {
					wdev = &pAd->StaCfg[idx].wdev;
					if (band_order_check(pAd, wdev, i))
						wlan_config_set_mmps(wdev, mmps);
				}
			}
#endif /* CONFIG_STA_SUPPORT */
		}
	}
}

static VOID read_ba_related(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UINT16 val = 0;
	UINT32 i = 0;

	if (RTMPGetKeyParameter("HT_BADecline", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);

			if (wdev)
				wlan_config_set_ba_decline(wdev, (UINT8)val);
			else
				break;
		}
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"HT: BA Decline  = %s\n", (val == 0) ? "Disable" : "Enable");
	}

	if (RTMPGetKeyParameter("HT_AutoBA", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);

			if (wdev)
				wlan_config_set_ba_enable(wdev, (UINT8)val);
			else
				break;
		}
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"HT: Auto BA  = %s\n", (val == 0) ? "Disable" : "Enable");
	}

	if (RTMPGetKeyParameter("HT_BAWinSize", tmpbuf, PER_BSS_SIZE_4(pAd), buf, TRUE)) {
		UINT16 max_ba_wsize = 0;
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				max_ba_wsize = ba_get_default_max_ba_wsize(wdev, pAd);
				wlan_config_set_ba_txrx_wsize(wdev, val, val, max_ba_wsize);
			} else
				break;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"HT_BAWinSize: wdev[%d]: (TX=%d, RX=%d), max_ba_wsize=%u\n", i, val, val, max_ba_wsize);
		}
	}
}

static VOID read_ht_bw(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	UCHAR ht_bw;

	if (RTMPGetKeyParameter("HT_BW", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		ht_bw = os_str_tol(tmpbuf, 0, 10);
		wlan_config_set_ht_bw_all(&pAd->wpf, ht_bw);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"HT: Channel Width = %s\n",
				 (ht_bw == HT_BW_40) ? "40 MHz" : "20 MHz");
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			RTMP_STRING *Bufptr;
			struct wifi_dev *wdev;
			INT i;
			UCHAR IdBss = 0;

			for (i = 0, Bufptr = rstrtok(tmpbuf, ";"); (Bufptr && (i < MAX_MBSSID_NUM(pAd)));
				 Bufptr = rstrtok(NULL, ";"), i++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				ht_bw = os_str_tol(Bufptr, 0, 10);
				wlan_config_set_ht_bw(wdev, ht_bw);
#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
				wdev->rate.MCastPhyMode_5G.field.BW = HT_BW_20;
				wdev->rate.MCastPhyMode.field.BW = HT_BW_20;
#else
				wdev->rate.mcastphymode.field.BW = HT_BW_20;
#endif /* #ifdef MCAST_VENDOR10_CUSTOM_FEATURE */
#endif /* MCAST_RATE_SPECIFIC */

				/* for first time, update all same as ra0 */
				if (i == 0) {
					for (IdBss = 1; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
						wdev = get_curr_wdev(pAd, PF_TO_BSS_IDX(pAd, IdBss));
						if (wdev != NULL) {
							wlan_config_set_ht_bw(wdev, ht_bw);
#ifdef MCAST_RATE_SPECIFIC
							wdev->rate.mcastphymode.field.BW = HT_BW_20;
#endif /* MCAST_RATE_SPECIFIC */
						} else {
							MTWF_DBG(pAd,
								DBG_CAT_CFG, CATCFG_PROFILE,
								DBG_LVL_ERROR, "wdev is NULL\n");
						}
					}
				}
			}
		}
#endif /*CONFIG_AP_SUPPORT*/
	}
}

static VOID read_ht_max_ampdu_length_exponent(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct _RTMP_CHIP_CAP *chip_cap;
	UINT16 max_ampdu_len;

	if (RTMPGetKeyParameter("AF_HT", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {

		max_ampdu_len = os_str_tol(tmpbuf, 0, 10);
		if (max_ampdu_len == 0)
			max_ampdu_len = HT_AMPDU_2_OF_POWER_13_M1;
		else if (max_ampdu_len == 1)
			max_ampdu_len = HT_AMPDU_2_OF_POWER_14_M1;
		else if (max_ampdu_len == 2)
			max_ampdu_len = HT_AMPDU_2_OF_POWER_15_M1;
		else if (max_ampdu_len == 3)
			max_ampdu_len = HT_AMPDU_2_OF_POWER_16_M1;

		chip_cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);

		if (chip_cap)
			chip_cap->ppdu.ht_max_ampdu_len_exp = max_ampdu_len;

	}
}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
static VOID RTMPOldBWCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer, BOOLEAN isVHT)
{
	UCHAR bw = 0;
	RTMP_STRING *Bufptr = NULL;
	struct wifi_dev *wdev = NULL;
	INT i = 0;

	if ((Buffer == NULL) || (IS_SUPPORT_V10_DFS(pAd) == FALSE))
		return;

#ifdef CONFIG_AP_SUPPORT
	/* V10 -- AP Mode Only */
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		bw = os_str_tol(Buffer, 0, 10);

		/* Disallow Invalid Values */
		if ((!isVHT && bw > BW_40) || (isVHT && bw > BW_80)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
				"Incorrect BW=%d\n",
				bw);
			return;
		}

		for (i = 0, Bufptr = rstrtok(Buffer, ";"); (Bufptr && (i < MAX_MBSSID_NUM(pAd)));
			Bufptr = rstrtok(NULL, ";"), i++) {
			bw = os_str_tol(Bufptr, 0, 10);

			wdev = get_curr_wdev(pAd, i);
			if (!wdev || !IS_V10_OLD_CHNL_VALID(wdev))
				continue;

			if (isVHT) {
				wlan_config_set_vht_bw(wdev, bw);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"wdev[%d] VHT: Channel Width = %s MHz\n"
					, i, VhtBw2Str(bw));
			} else {
				wlan_config_set_ht_bw(wdev, bw);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"HT: Channel Width = %s\n",
					(bw == HT_BW_40) ? "40 MHz" : "20 MHz");
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}
#endif

static VOID read_ht_param_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	UINT32 Value = 0;
	INT i = 0;
#ifdef CONFIG_AP_SUPPORT
	RTMP_STRING *Bufptr;
#endif /* CONFIG_AP_SUPPORT */

	/* HT_BW */
	read_ht_bw(pAd, tmpbuf, buf);
	/* Tx/Rx Stream */
	read_txrx_stream_num(pAd, tmpbuf, buf);
	/* HT_OpMode */
	read_ht_mode(pAd, tmpbuf, buf);
	/* HT_PROTECT */
	read_ht_protect(pAd, tmpbuf, buf);
	/* HT_GI */
	read_ht_gi(pAd, tmpbuf, buf);
	/* HT_LDPC */
	read_ht_ldpc(pAd, tmpbuf, buf);
	/* HT_STBC */
	read_ht_stbc(pAd, tmpbuf, buf);
	/* MPDU Density*/
	read_min_mpdu_start_space(pAd, tmpbuf, buf);
	/* 40_Mhz_Intolerant*/
	read_40M_intolerant(pAd, tmpbuf, buf);
	/* Tx A-MSUD */
	read_amsdu_enable(pAd, tmpbuf, buf);
	/* A-MSDU Number */
	read_amsdu_num(pAd, tmpbuf, buf);
	/* MMPS */
	read_mmps(pAd, tmpbuf, buf);
	/* Block Ack */
	read_ba_related(pAd, tmpbuf, buf);
	/* Fixed MCS */
	read_fixed_mcs(pAd, tmpbuf, buf);
	/* MAX AMPDU length exponent */
	read_ht_max_ampdu_length_exponent(pAd, tmpbuf, buf);


	/* Fixed Tx mode : CCK, OFDM*/
	if (RTMPGetKeyParameter("FixedTxMode", tmpbuf, 25, buf, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, Bufptr = rstrtok(tmpbuf, ";"); (Bufptr && i < MAX_MBSSID_NUM(pAd));
				 Bufptr = rstrtok(NULL, ";"), i++) {
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.DesiredTransmitSetting.field.FixedTxMode =
					RT_CfgSetFixedTxPhyMode(Bufptr);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"(IF-ra%d) Fixed Tx Mode = %d\n", i,
						 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.DesiredTransmitSetting.field.FixedTxMode);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			pAd->StaCfg[0].wdev.DesiredTransmitSetting.field.FixedTxMode =
				RT_CfgSetFixedTxPhyMode(tmpbuf);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"Fixed Tx Mode = %d\n",
					 pAd->StaCfg[0].wdev.DesiredTransmitSetting.field.FixedTxMode);
		}
#endif /* CONFIG_STA_SUPPORT */
	}


	if (RTMPGetKeyParameter("HT_EXTCHA", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		struct wifi_dev *wdev;
		UCHAR ext_cha;
#ifdef CONFIG_STA_SUPPORT
		Value = os_str_tol(tmpbuf, 0, 10);

		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			wdev = &pAd->StaCfg[MAIN_MSTA_ID].wdev;

			if (Value == 0)
				ext_cha = EXTCHA_BELOW;
			else
				ext_cha = EXTCHA_ABOVE;

			wlan_config_set_ext_cha(wdev, ext_cha);

			for (i = 0; i < MAX_MULTI_STA; i++) {
				wdev = &pAd->StaCfg[i].wdev;

				if (wlan_config_get_ext_cha(wdev) == EXTCHA_NOASSIGN)
					wlan_config_set_ext_cha(wdev, ext_cha);
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"HT: Ext Channel = %s\n", (Value == 0) ? "BELOW" : "ABOVE");
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

		for (i = 0, Bufptr = rstrtok(tmpbuf, ";"); (Bufptr && (i < MAX_MBSSID_NUM(pAd)));
			 Bufptr = rstrtok(NULL, ";"), i++) {
			Value = os_str_tol(Bufptr, 0, 10);
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;

			if (Value == 0)
				ext_cha = EXTCHA_BELOW;
			else
				ext_cha = EXTCHA_ABOVE;

			wlan_config_set_ext_cha(wdev, ext_cha);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"HT: WDEV[%x] Ext Channel = %s\n", i,
				(Value == 0) ? "BELOW" : "ABOVE");
		}
		}

		ext_cha = wlan_config_get_ext_cha(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);

		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;

			if (wlan_config_get_ext_cha(wdev) == EXTCHA_NOASSIGN) {
				wlan_config_set_ext_cha(wdev, ext_cha);
			}
		}

#endif /*CONFIG_AP_SUPPORT*/
	}

	/* MSC*/
	if (RTMPGetKeyParameter("HT_MCS", tmpbuf, 50, buf, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, Bufptr = rstrtok(tmpbuf, ";"); (Bufptr && i < MAX_MBSSID_NUM(pAd));
				 Bufptr = rstrtok(NULL, ";"), i++) {
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;

				Value = os_str_tol(Bufptr, 0, 10);

				if (Value <= MCS_32)
					wdev->DesiredTransmitSetting.field.MCS = Value;
				else
					wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;

				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"(IF-ra%d) HT: MCS = %s(%d)\n",
						 i, (wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO ? "AUTO" : "Fixed"),
						 wdev->DesiredTransmitSetting.field.MCS);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;

			Value = os_str_tol(tmpbuf, 0, 10);

			if (Value <= MCS_32) {
				wdev->DesiredTransmitSetting.field.MCS  = Value;
				wdev->bAutoTxRateSwitch = FALSE;
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"HT: MCS = %d\n", wdev->DesiredTransmitSetting.field.MCS);
			} else {
				wdev->DesiredTransmitSetting.field.MCS  = MCS_AUTO;
				wdev->bAutoTxRateSwitch = TRUE;
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"HT: MCS = AUTO\n");
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	}


#ifdef GREENAP_SUPPORT

	/*Green AP*/
	if (RTMPGetKeyParameter("GreenAP", tmpbuf, 10, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value == 0)
			greenap_set_capability(pAd, FALSE);
		else
			greenap_set_capability(pAd, TRUE);

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"HT: greenap_cap = %d\n", greenap_get_capability(pAd));
	}

#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT

	/* PcieAspm */
	if (RTMPGetKeyParameter("PcieAspm", tmpbuf, 10, buf, TRUE)) {

		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value == 0)
			set_pcie_aspm_dym_ctrl_cap(pAd, FALSE);
		else
			set_pcie_aspm_dym_ctrl_cap(pAd, TRUE);

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"ChipI=%x, Value=%d, pcie_aspm in profile=%d\n",
			pAd->ChipID,
			Value,
			get_pcie_aspm_dym_ctrl_cap(pAd));
	}

#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	if (RTMPGetKeyParameter("TWTSupport", tmpbuf, 128, buf, TRUE)) {
		struct wifi_dev *wdev;
		RTMP_STRING *macptr = NULL;
		UINT i, twt_support[CFG_WIFI_RAM_BAND_NUM];
#ifdef BCN_EXTCAP_VAR_LEN
		long cfg_mode;
#endif /* BCN_EXTCAP_VAR_LEN */
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i < CFG_WIFI_RAM_BAND_NUM) {
				twt_support[i] = os_str_tol(macptr, 0, 10);
#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
					if (i < MAX_MULTI_STA) {
						wdev = &pAd->StaCfg[i].wdev;
						wlan_config_set_he_twt_support(wdev, twt_support[i]);
						MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"%d:%s TWTsupport = %d\n", i,
							RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev), twt_support[i]);
					}
				}
#endif /* CONFIG_STA_SUPPORT */
			} else
				break;
		}
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				UCHAR BandIdx = DBDC_BAND0;
				wdev = get_curr_wdev(pAd, i);
#ifdef BCN_EXTCAP_VAR_LEN
				cfg_mode = wmode_2_cfgmode(wdev->PhyMode);
#endif /* BCN_EXTCAP_VAR_LEN */
				if (!wdev) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"Dereference null return value\n");
					return;
				}
				if (pAd->CommonCfg.dbdc_mode) {
					if (WMODE_CAP_5G(wdev->PhyMode))
						BandIdx = DBDC_BAND1;
					else
						BandIdx = DBDC_BAND0;
				}
#ifdef BCN_EXTCAP_VAR_LEN
				if (twt_support[BandIdx] == 0) {
					wdev->BcnExtCapLen = EXT_CAP_COM_LENGTH;
				} else if (twt_support[BandIdx] >= 1) {
					if (cfg_mode > PHY_11VHT_N_MIXED)
						wdev->BcnExtCapLen = EXT_CAP_MIN_SAFE_LENGTH;
					else
						wdev->BcnExtCapLen = EXT_CAP_COM_LENGTH;
				}
#endif /* BCN_EXTCAP_VAR_LEN */
				wlan_config_set_he_twt_support(wdev, twt_support[BandIdx]);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"%d:%s TWTsupport = %d\n", i,
					RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev), twt_support[BandIdx]);
#ifdef DOT11_EHT_BE
				/*
					shall set the Restricted TWT Support subfield in its transmitted EHT Capabilities element to 1
					and shall set the Broadcast TWT Support subfield in its transmitted HE Capabilities element to 1
				*/
				if (!TWT_SUPPORT_BTWT(twt_support[BandIdx]) &&
					wlan_config_get_eht_restricted_twt(wdev)) {
					wlan_config_set_he_twt_support(wdev,
							(twt_support[BandIdx] | TWT_PROFILE_SUPPORT_BTWT));

					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"%d:%s Also need bTWT support due to rTWT support!\n", i,
						RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev));
				}
#endif /* DOT11_EHT_BE */
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	if (RTMPGetKeyParameter("TWTInfoFrame", tmpbuf, 128, buf, TRUE)) {
		RTMP_STRING *macptr = NULL;
		UINT i, twt_info_frame[CFG_WIFI_RAM_BAND_NUM];
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i < CFG_WIFI_RAM_BAND_NUM) {
				twt_info_frame[i] = os_str_tol(macptr, 0, 10);
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
			} else
				break;
		}
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			struct wifi_dev *wdev;
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				UCHAR BandIdx = DBDC_BAND0;
				wdev = get_curr_wdev(pAd, i);
				if (wdev) {
					if (pAd->CommonCfg.dbdc_mode) {
						if (WMODE_CAP_5G(wdev->PhyMode))
							BandIdx = DBDC_BAND1;
						else
							BandIdx = DBDC_BAND0;
					}
					wlan_config_set_he_twt_info_frame(wdev, twt_info_frame[BandIdx]);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"%d:%s TWTInfoFrame = %d\n", i,
						RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev), twt_info_frame[BandIdx]);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

	/* HT_DisallowTKIP*/
	if (RTMPGetKeyParameter("HT_DisallowTKIP", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value == 1)
			pAd->CommonCfg.HT_DisallowTKIP = TRUE;
		else
			pAd->CommonCfg.HT_DisallowTKIP = FALSE;

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"HT: Disallow TKIP mode = %s\n",
			(pAd->CommonCfg.HT_DisallowTKIP == TRUE) ? "ON" : "OFF");
	}

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	if (RTMPGetKeyParameter("OBSSScanParam", tmpbuf, 32, buf, TRUE)) {
		int ObssScanValue, idx;
		RTMP_STRING *macptr;
		for (idx = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (idx < 7);
			macptr = rstrtok(NULL, ";"), idx++) {
			ObssScanValue = os_str_tol(macptr, 0, 10);

			switch (idx) {
			case 0:
				if (ObssScanValue < 5 || ObssScanValue > 1000)
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							 "Invalid OBSSScanParam for Dot11OBssScanPassiveDwell(%d), should in range 5~1000\n", ObssScanValue);
				else {
					pAd->CommonCfg.Dot11OBssScanPassiveDwell = ObssScanValue;	/* Unit : TU. 5~1000*/
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"OBSSScanParam for Dot11OBssScanPassiveDwell=%d\n",
						ObssScanValue);
				}

				break;

			case 1:
				if (ObssScanValue < 10 || ObssScanValue > 1000)
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							 "Invalid OBSSScanParam for Dot11OBssScanActiveDwell(%d), should in range 10~1000\n", ObssScanValue);
				else {
					pAd->CommonCfg.Dot11OBssScanActiveDwell = ObssScanValue;	/* Unit : TU. 10~1000*/
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"OBSSScanParam for Dot11OBssScanActiveDwell=%d\n",
						ObssScanValue);
				}

				break;

			case 2:
				pAd->CommonCfg.Dot11BssWidthTriggerScanInt = ObssScanValue;	/* Unit : Second*/
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"OBSSScanParam for Dot11BssWidthTriggerScanInt=%d\n",
					ObssScanValue);
				break;

			case 3:
				if (ObssScanValue < 200 || ObssScanValue > 10000)
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							 "Invalid OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel(%d), should in range 200~10000\n", ObssScanValue);
				else {
					pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = ObssScanValue;	/* Unit : TU. 200~10000*/
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel=%d\n",
						ObssScanValue);
				}

				break;

			case 4:
				if (ObssScanValue < 20 || ObssScanValue > 10000)
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							 "Invalid OBSSScanParam for Dot11OBssScanActiveTotalPerChannel(%d), should in range 20~10000\n", ObssScanValue);
				else {
					pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = ObssScanValue;	/* Unit : TU. 20~10000*/
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"OBSSScanParam for Dot11OBssScanActiveTotalPerChannel=%d\n",
						ObssScanValue);
				}

				break;

			case 5:
				pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = ObssScanValue;
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"OBSSScanParam for Dot11BssWidthChanTranDelayFactor=%d\n",
					ObssScanValue);
				break;

			case 6:
				pAd->CommonCfg.Dot11OBssScanActivityThre = ObssScanValue;	/* Unit : percentage*/
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"OBSSScanParam for Dot11BssWidthChanTranDelayFactor=%d\n",
					ObssScanValue);
				break;
			}
		}

		if (idx != 7) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					 "Wrong OBSSScanParamtetrs format in dat file!!!!! Use default value.\n");
			pAd->CommonCfg.Dot11OBssScanPassiveDwell = dot11OBSSScanPassiveDwell;	/* Unit : TU. 5~1000*/
			pAd->CommonCfg.Dot11OBssScanActiveDwell = dot11OBSSScanActiveDwell;	/* Unit : TU. 10~1000*/
			pAd->CommonCfg.Dot11BssWidthTriggerScanInt = dot11BSSWidthTriggerScanInterval;	/* Unit : Second	*/
			pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = dot11OBSSScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000*/
			pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = dot11OBSSScanActiveTotalPerChannel;	/* Unit : TU. 20~10000*/
			pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = dot11BSSWidthChannelTransactionDelayFactor;
			pAd->CommonCfg.Dot11OBssScanActivityThre = dot11BSSScanActivityThreshold;	/* Unit : percentage*/
		}

		pAd->CommonCfg.Dot11BssWidthChanTranDelay = ((UINT32)pAd->CommonCfg.Dot11BssWidthTriggerScanInt *
				(UINT32)pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"OBSSScanParam for Dot11BssWidthChanTranDelay=%ld\n",
			pAd->CommonCfg.Dot11BssWidthChanTranDelay);
	}

	if (RTMPGetKeyParameter("HT_BSSCoexistence", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);
		pAd->CommonCfg.bBssCoexEnable = ((Value == 1) ? TRUE : FALSE);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"HT: 20/40 BssCoexSupport = %s\n",
			(pAd->CommonCfg.bBssCoexEnable == TRUE) ? "ON" : "OFF");
	}

	if (RTMPGetKeyParameter("HT_BSSCoexApCntThr", tmpbuf, 25, buf, TRUE)) {
		pAd->CommonCfg.BssCoexApCntThr = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"HT: 20/40 BssCoexApCntThr = %d\n",
			pAd->CommonCfg.BssCoexApCntThr);
	}

#endif /* DOT11N_DRAFT3 */

#endif /* DOT11_N_SUPPORT */

	if (RTMPGetKeyParameter("TXRX_RXV_ON", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);
		pAd->CommonCfg.bTXRX_RXV_ON = Value;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"TXRX_RXV_ON = %s\n", (Value == 1) ? "ON" : "OFF");
	}
}
#endif /* DOT11_N_SUPPORT */

#ifdef TXBF_SUPPORT
#if defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT)
static VOID read_itxbf(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ITxBfEn = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("ITxBfEn", tmpbuf, 128, buf, FALSE)) {
		/* Reset pAd->CommonCfg.ITxBfEn */
		pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ITxBfEn = os_str_tol(macptr, 0, 10);
			pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn |= ITxBfEn;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"ITxBfEn = %d\n", ITxBfEn);
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"BSSID[%d]\n", i);
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				}
			}
#endif /* CONFIG_AP_SUPPORT */
#if defined(STA_ITXBF_SUPPORT) && defined(CONFIG_STA_SUPPORT)
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"STA[%d]\n", i);
					wdev = &pAd->StaCfg[i].wdev;
				}
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				wlan_config_set_itxbf(wdev, ITxBfEn);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"MBSS[%d] ITxBfEn = %d\n", i, ITxBfEn);
			} else
				break;
		}

		/* If wdev num > ITxBfEn num in profile, set wdev with the final ITxBfEn */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"More BSSID[%d]\n", i);
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_itxbf(wdev, ITxBfEn);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] ITxBfEn = %d\n", i, ITxBfEn);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#if defined(STA_ITXBF_SUPPORT) && defined(CONFIG_STA_SUPPORT)
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"More STA[%d]\n", i);
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_itxbf(wdev, ITxBfEn);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] ITxBfEn = %d\n", i, ITxBfEn);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "Common.ITxBfEn = %d\n",
		pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn);
}
#endif /* defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT) */

static VOID read_etxbf(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ETxBfEnCond = SUBF_OFF;
	INT i = 0;

	if (RTMPGetKeyParameter("ETxBfEnCond", tmpbuf, PER_BSS_SIZE_2(pAd), buf, FALSE)) {
		/* Reset pAd->CommonCfg.ETxBfEnCond */
		pAd->CommonCfg.ETxBfEnCond = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ETxBfEnCond = os_str_tol(macptr, 0, 10);
			pAd->CommonCfg.ETxBfEnCond |= ETxBfEnCond;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					 "ETxBfEnCond = %d\n", ETxBfEnCond);
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"BSSID[%d]\n", i);
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				}
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"STA[%d]\n", i);
					wdev = &pAd->StaCfg[i].wdev;
				}
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				wlan_config_set_etxbf(wdev, ETxBfEnCond);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"MBSS[%d] ETxBfEnCond = %d\n", i, ETxBfEnCond);
			} else
				break;
		}

		/* If wdev num > ETxBfEnCond num in profile, set wdev with the final ETxBfEnCond */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"More BSSID[%d]\n", i);
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_etxbf(wdev, ETxBfEnCond);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] ETxBfEnCond = %d\n", i, ETxBfEnCond);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"More STA[%d]\n", i);
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_etxbf(wdev, ETxBfEnCond);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] ETxBfEnCond = %d\n", i, ETxBfEnCond);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}
}

static VOID read_txbf_smth_intl_bypas(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	RTMP_STRING *macptr = NULL;
	UINT i = 0;

	if (RTMPGetKeyParameter("BfSmthIntlBbypass", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= CFG_WIFI_RAM_BAND_NUM)
				break;

			pAd->CommonCfg.BfSmthIntlBypass = os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					 "BfSmthIntlBbypass[%d] = %d\n", i, pAd->CommonCfg.BfSmthIntlBypass);
		}
	}
}

static VOID read_hera_stbc_priority(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	RTMP_STRING *macptr = NULL;
	UINT i = 0;

	if (RTMPGetKeyParameter("HeraStbcPriority", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= CFG_WIFI_RAM_BAND_NUM)
				break;

			pAd->CommonCfg.HeraStbcPriority = os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					 "HeraStbcPriority[%d] = %d\n", i, pAd->CommonCfg.HeraStbcPriority);
		}
	}
}


#ifdef DSCP_PRI_SUPPORT
static VOID read_dscp_pri_param(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr = NULL;
	RTMP_STRING tok_str[32];
	UCHAR i = 0, bss_idx = 0;
	int ret;

	if (RTMPGetKeyParameter("DscpPriMapEnable", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
		ULONG enable = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < MAX_BEACON_NUM);
			macptr = rstrtok(NULL, ";"), i++) {
			enable = os_str_tol(macptr, 0, 10);

			if (enable < 2)
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].dscp_pri_map_enable = (UCHAR) (enable);
			else
				MTWF_DBG(pAd, DBG_CAT_SEC, CATCFG_PROFILE, DBG_LVL_ERROR,
							"invalid dscp_pri_map_enable value\n");

			MTWF_DBG(NULL, DBG_CAT_SEC, CATCFG_PROFILE, DBG_LVL_INFO,
				"(%s)) ==> enable:%d\n", __func__,
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].dscp_pri_map_enable);
		}
		pAd->ApCfg.DscpPriMapSupport = 1;
#ifdef MAP_R5
		for (i = 0; i < MAX_MULTI_STA; i++)
			pAd->StaCfg[i].dscp_pri_map_enable = enable;
#endif
	}

	for (bss_idx = 0; bss_idx < pAd->ApCfg.BssidNum; bss_idx++) {
		ret = snprintf(tok_str, sizeof(tok_str), "DscpPriMapBss%d", bss_idx);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATCFG_PROFILE,
				DBG_LVL_ERROR, "snprintf error!\n");
			return;
		}
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 512, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				RTMP_STRING	*this_char;
				UINT8	dscpValue;
				INT8 pri;

				if (i > 63)
					break;

				this_char = strsep((char **)&macptr, ":");
				if (this_char == NULL) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"value not defined for Dscp and Priority\n");
					break;
				}

				dscpValue = simple_strtol(this_char, 0, 10);
				if (dscpValue > 63) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"Invalid Dscp Value Valid Value between 0 to 63\n");
					break;
				}
				if (macptr == NULL) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"Priority not defined for Dscp %d\n", dscpValue);
					break;
				}
				pri = simple_strtol(macptr, 0, 10);
				if (pri < -1  || pri > 7) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"Invalid Priority value Valid value between 0 to 7\n");
					break;
				}
				if (pri == 0)
					pri = 3;
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				 "Setting Pri %d for Dscp=%d\n", pri, dscpValue);
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, bss_idx)].dscp_pri_map[dscpValue] = pri;
			}
		}
	}
}
#endif /*DSCP_PRI_SUPPORT*/

static VOID read_txbf_param_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
#if defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT)
	/* ITxBfEn */
	read_itxbf(pAd, tmpbuf, buf);
#endif /* defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT) */

	/* ETxBfEnCond */
	read_etxbf(pAd, tmpbuf, buf);

	/* BfSmthIntlBbypass*/
	read_txbf_smth_intl_bypas(pAd, tmpbuf, buf);

	/* HeraStbcPriority */
	read_hera_stbc_priority(pAd, tmpbuf, buf);
}
#endif /* TXBF_SUPPORT */

#ifdef DOT11_HE_AX
static VOID read_mu_dl_ofdma(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR mu_dl_ofdma = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("MuOfdmaDlEnable", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			mu_dl_ofdma = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum)
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA)
					wdev = &pAd->StaCfg[i].wdev;
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				wlan_config_set_mu_dl_ofdma(wdev, mu_dl_ofdma);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"MBSS[%d] MuOfdmaDlEnable = %d\n", i, mu_dl_ofdma);
			} else
				break;
		}

		/* If wdev num > ETxBfEnCond num in profile, set wdev with the final ETxBfEnCond */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_mu_dl_ofdma(wdev, mu_dl_ofdma);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"More MBSS[%d] MuOfdmaDlEnable = %d\n", i, mu_dl_ofdma);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_mu_dl_ofdma(wdev, mu_dl_ofdma);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"More MBSS[%d] MuOfdmaDlEnable = %d\n", i, mu_dl_ofdma);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}

#ifdef DOT11_EHT_BE
	if (RTMPGetKeyParameter("PpOfdmaDlEnable", tmpbuf, 128, buf, FALSE)) {
		mu_dl_ofdma = 0;
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			mu_dl_ofdma = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum)
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA)
					wdev = &pAd->StaCfg[i].wdev;
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				wlan_config_set_pp_dl_ofdma(wdev, mu_dl_ofdma);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"MBSS[%d] pp_dl_ofdma = %d\n", i, mu_dl_ofdma);
			} else
				break;
		}

		/* If wdev num > ETxBfEnCond num in profile, set wdev with the final ETxBfEnCond */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_pp_dl_ofdma(wdev, mu_dl_ofdma);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"More MBSS[%d] MuOfdmaDlEnable = %d\n", i, mu_dl_ofdma);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_pp_dl_ofdma(wdev, mu_dl_ofdma);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"More MBSS[%d] MuOfdmaDlEnable = %d\n", i, mu_dl_ofdma);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}
#endif
}

static VOID read_mu_ul_ofdma(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR mu_ul_ofdma = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("MuOfdmaUlEnable", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			mu_ul_ofdma = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum)
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA)
					wdev = &pAd->StaCfg[i].wdev;
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				wlan_config_set_mu_ul_ofdma(wdev, mu_ul_ofdma);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"MBSS[%d] MuOfdmaUlEnable = %d\n", i, mu_ul_ofdma);
			} else
				break;
		}

		/* If wdev num > ETxBfEnCond num in profile, set wdev with the final ETxBfEnCond */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_mu_ul_ofdma(wdev, mu_ul_ofdma);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] MuOfdmaUlEnable = %d\n", i, mu_ul_ofdma);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_mu_ul_ofdma(wdev, mu_ul_ofdma);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] MuOfdmaUlEnable = %d\n", i, mu_ul_ofdma);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}

#ifdef DOT11_EHT_BE
	if (RTMPGetKeyParameter("PpOfdmaUlEnable", tmpbuf, 128, buf, FALSE)) {
		mu_ul_ofdma = 0;
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			mu_ul_ofdma = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum)
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA)
					wdev = &pAd->StaCfg[i].wdev;
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				wlan_config_set_pp_ul_ofdma(wdev, mu_ul_ofdma);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"MBSS[%d] PpOfdmaUlEnable = %d\n", i, mu_ul_ofdma);
			} else
				break;
		}

		/* If wdev num > ETxBfEnCond num in profile, set wdev with the final ETxBfEnCond */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_pp_ul_ofdma(wdev, mu_ul_ofdma);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] PpOfdmaUlEnable = %d\n", i, mu_ul_ofdma);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_pp_ul_ofdma(wdev, mu_ul_ofdma);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] PpOfdmaUlEnable = %d\n", i, mu_ul_ofdma);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}
#endif
}

static VOID read_mu_dl_mimo(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	ULONG etxbf_en_cond = 0;
	UCHAR mu_dl_mimo = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("MuMimoDlEnable", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			mu_dl_mimo = os_str_tol(macptr, 0, 10);


#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum)
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA)
					wdev = &pAd->StaCfg[i].wdev;
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				/* DL Mu-Mimo should be disabled if ETxBfEnCond is disabled */
				etxbf_en_cond = wlan_config_get_etxbf(wdev);
				if (etxbf_en_cond == 0)
					mu_dl_mimo = 0;

				wlan_config_set_mu_dl_mimo(wdev, mu_dl_mimo);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"MBSS[%d] MuMimoDlEnable = %d\n", i, mu_dl_mimo);
			} else
				break;
		}

		/* If wdev num > ETxBfEnCond num in profile, set wdev with the final ETxBfEnCond */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_mu_dl_mimo(wdev, mu_dl_mimo);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] MuMimoDlEnable = %d\n", i, mu_dl_mimo);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_mu_dl_mimo(wdev, mu_dl_mimo);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] MuMimoDlEnable = %d\n", i, mu_dl_mimo);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}

#ifdef DOT11_EHT_BE
	if (RTMPGetKeyParameter("PpMuMimoDlEnable", tmpbuf, 128, buf, FALSE)) {
		mu_dl_mimo = 0;
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			mu_dl_mimo = os_str_tol(macptr, 0, 10);


#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum)
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA)
					wdev = &pAd->StaCfg[i].wdev;
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				/* DL Mu-Mimo should be disabled if ETxBfEnCond is disabled */
				etxbf_en_cond = wlan_config_get_etxbf(wdev);
				if (etxbf_en_cond == 0)
					mu_dl_mimo = 0;

				wlan_config_set_pp_dl_mu_mimo(wdev, mu_dl_mimo);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"MBSS[%d] PpMuMimoDlEnable = %d\n", i, mu_dl_mimo);
			} else
				break;
		}

		/* If wdev num > ETxBfEnCond num in profile, set wdev with the final ETxBfEnCond */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_pp_dl_mu_mimo(wdev, mu_dl_mimo);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] PpMuMimoDlEnable = %d\n", i, mu_dl_mimo);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_pp_dl_mu_mimo(wdev, mu_dl_mimo);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] PpMuMimoDlEnable = %d\n", i, mu_dl_mimo);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}
#endif
}

static VOID read_mu_ul_mimo(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR mu_ul_mimo = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("MuMimoUlEnable", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			mu_ul_mimo = os_str_tol(macptr, 0, 10);


#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum)
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA)
					wdev = &pAd->StaCfg[i].wdev;
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				wlan_config_set_mu_ul_mimo(wdev, mu_ul_mimo);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"MBSS[%d] MuMimoUlEnable = %d\n", i, mu_ul_mimo);
			} else
				break;
		}

		/* If wdev num > ETxBfEnCond num in profile, set wdev with the final ETxBfEnCond */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_mu_ul_mimo(wdev, mu_ul_mimo);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] MuMimoUlEnable = %d\n", i, mu_ul_mimo);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_mu_ul_mimo(wdev, mu_ul_mimo);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] MuMimoUlEnable = %d\n", i, mu_ul_mimo);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}

#ifdef DOT11_EHT_BE
	if (RTMPGetKeyParameter("PpMuMimoUlEnable", tmpbuf, 128, buf, FALSE)) {
		mu_ul_mimo = 0;
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			mu_ul_mimo = os_str_tol(macptr, 0, 10);


#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum)
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA)
					wdev = &pAd->StaCfg[i].wdev;
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				wlan_config_set_pp_ul_mu_mimo(wdev, mu_ul_mimo);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"MBSS[%d] PpMuMimoUlEnable = %d\n", i, mu_ul_mimo);
			} else
				break;
		}

		/* If wdev num > ETxBfEnCond num in profile, set wdev with the final ETxBfEnCond */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_pp_ul_mu_mimo(wdev, mu_ul_mimo);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] PpMuMimoUlEnable = %d\n", i, mu_ul_mimo);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_pp_ul_mu_mimo(wdev, mu_ul_mimo);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"More MBSS[%d] PpMuMimoUlEnable = %d\n", i, mu_ul_mimo);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}
#endif
}

static VOID read_mu_param_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct _RTMP_CHIP_CAP *chip_cap;
	enum PHY_CAP phy_caps;

	chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	phy_caps = chip_cap->phy_caps;

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_DL_MUOFDMA))
		read_mu_dl_ofdma(pAd, tmpbuf, buf);

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_UL_MUOFDMA))
		read_mu_ul_ofdma(pAd, tmpbuf, buf);

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_DL_MUMIMO))
		read_mu_dl_mimo(pAd, tmpbuf, buf);

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_UL_MUMIMO))
		read_mu_ul_mimo(pAd, tmpbuf, buf);
}

static VOID read_er_su_param_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct _RTMP_CHIP_CAP *chip_cap;
	enum PHY_CAP phy_caps;
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR HeErSuRxDisable = 0;
	INT i = 0;

	chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	phy_caps = chip_cap->phy_caps;

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_ER_SU)) {
		if (RTMPGetKeyParameter("HeErSuRxDisable", tmpbuf, 128, buf, FALSE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && VALID_MBSS(pAd, i);
				macptr = rstrtok(NULL, ";"), i++) {
				HeErSuRxDisable = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
					if (i < pAd->ApCfg.BssidNum)
						wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				}
#endif /* CONFIG_AP_SUPPORT */

				if (wdev) {
					wlan_config_set_er_su_rx_disable(wdev, HeErSuRxDisable);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"MBSS[%d] HeErSuRxDisable = %d\n", i, HeErSuRxDisable);
				}
			}
		}
	}
}

static VOID read_he_dyn_smps_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR he_dyn_smps = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HeDynSmps", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && VALID_MBSS(pAd, i);
			macptr = rstrtok(NULL, ";"), i++) {
			he_dyn_smps = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum)
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */

			if (wdev) {
				wlan_config_set_he_dyn_smps(wdev, he_dyn_smps);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"MBSS[%d] HeDynSmps = %d\n",
					i, wlan_config_get_he_dyn_smps(wdev));
			}
		}
	}
}

static VOID read_ul_mu_data_param_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR HeOmiUlMuDataDisableRx = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HeOmiUlMuDataDisableRx", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && VALID_MBSS(pAd, i);
			macptr = rstrtok(NULL, ";"), i++) {
			HeOmiUlMuDataDisableRx = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum)
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */

			if (wdev) {
				wlan_config_set_ul_mu_data_disable_rx(wdev, HeOmiUlMuDataDisableRx);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"MBSS[%d] HeOmiUlMuDataDisableRx = %d\n", i, HeOmiUlMuDataDisableRx);
			}
		}
	}
}

#endif /* DOT11_HE_AX */

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
#define CHECK_NSS_RANGE(_nss, _config_nss) ((_nss > _config_nss) ? _config_nss : _nss)
static VOID read_eaprate_param_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	/*MgmRateSet*/
	INT eaprateidx;
	RTMP_STRING *macptr = NULL;
	char		tok_str[128];
	int i = 0;
	int ret;

	for (eaprateidx = BCN_TYPE; eaprateidx < MGMT_MAX_TYPE; eaprateidx++) {
		ret = snprintf(tok_str, sizeof(tok_str), "mgmrateset%d", eaprateidx);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE,
				DBG_LVL_ERROR, "snprintf error!\n");
			return;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buf, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				UINT32 ratetype = 0, phymode = 0, mcs = 0;
				INT i4Recv = 0;
				union _HTTRANSMIT_SETTING *transmit = NULL;
				struct wifi_dev *wdev = NULL;

				if (i >= pAd->ApCfg.BssidNum)
					break;
				wdev = &pAd->ApCfg.MBSSID[i].wdev;

				if (macptr) {
					i4Recv = sscanf(macptr, "%d-%d-%d", &(ratetype), &(phymode), &(mcs));
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"RateType %d,PhyMode %d,Mcs %d\n", ratetype, phymode, (mcs));

					if (i4Recv != 3) {
						MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"Format Error!\n");
						break;
					}

					if (ratetype >= MGMT_MAX_TYPE) {
						MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"error ratetype(%d)\n", ratetype);
						break;
					}
					transmit = &wdev->eap.mgmphymode[ratetype];

					switch (phymode) {
					case EAP_RATE_DISABLE: /* disable */
						if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G) {
							transmit->field.MODE = MODE_CCK;
							transmit->field.BW =  BW_20;
							transmit->field.MCS = RATE_1;
						} else {
							transmit->field.MODE = MODE_OFDM;
							transmit->field.BW =  BW_20;
							transmit->field.MCS = OfdmRateToRxwiMCS[RATE_6];
						}

						break;
					case EAP_CCK:	/* CCK */
						transmit->field.MODE = MODE_CCK;
						transmit->field.BW =  BW_20;
						if (wlan_config_get_ch_band(wdev) != CMD_CH_BAND_24G
							|| mcs > 3) {
							MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
								"CCK mode: err band(%d) 5/6G, error MCS(%d)!\n",
								wlan_config_get_ch_band(wdev), mcs);
							return;
						}

						if (pAd->CommonCfg.TxPreamble == Rt802_11PreambleShort) {
							if (mcs == 0) {
								mcs = mcs + 1;
								MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
										"Invalid MCS(CCK-1M) value when short preamble is enabled!\n");
							}
							mcs |= 0x4;
						}
						transmit->field.MCS = mcs;

						break;
					case EAP_OFDM:	/* OFDM */
						transmit->field.MODE = MODE_OFDM;
						transmit->field.BW =  BW_20;
						if (mcs > 7) {
							MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
									 "MCS must in range from 0 to 7 for OFDM Mode.\n");
							mcs = 0;
						} else
							transmit->field.MCS = mcs;

						break;
					default:
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
								 "unknown mgmt ratetype/phymode/mcs %d/%d/%d.\n", ratetype, phymode, mcs);
						return;
					}

					if (phymode == EAP_RATE_DISABLE)
						wdev->eap.eap_mgmrate_en &= ~(1 << ratetype);
					else
						wdev->eap.eap_mgmrate_en |= (1 << ratetype);

				} else {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"Format Error!\n");
					break;
				}
			}
		}
	}

	/* HighPri Rate*/
	for (eaprateidx = HIGHPRI_ARP; eaprateidx < HIGHPRI_MAX_TYPE; eaprateidx++) {
		ret = snprintf(tok_str, sizeof(tok_str), "hprirateset%d", eaprateidx);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE,
				DBG_LVL_ERROR, "snprintf error!\n");
			return;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buf, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				UINT32 ratetype = 0, phymode = 0, mcs = 0;
				INT i4Recv = 0;
				union _HTTRANSMIT_SETTING *transmit = NULL;
				struct wifi_dev *wdev = NULL;

				if (i >= pAd->ApCfg.BssidNum)
					break;
				wdev = &pAd->ApCfg.MBSSID[i].wdev;

				if (macptr) {
					i4Recv = sscanf(macptr, "%d-%d-%d", &(ratetype), &(phymode), &(mcs));
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"RateType %d,PhyMode %d,Mcs %d\n", ratetype, phymode, (mcs));

					if (i4Recv != 3) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
								"Format Error!\n");
						break;
					}

					if (ratetype >= HIGHPRI_MAX_TYPE) {
						MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"error ratetype(%d)\n", ratetype);
						break;
					}
					transmit = &wdev->eap.hpriphymode[ratetype];

					switch (phymode) {
					case EAP_RATE_DISABLE: /* disable */
						if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G) {
							transmit->field.MODE = MODE_CCK;
							transmit->field.BW =  BW_20;
							transmit->field.MCS = RATE_1;
						} else {
							transmit->field.MODE = MODE_OFDM;
							transmit->field.BW =  BW_20;
							transmit->field.MCS = OfdmRateToRxwiMCS[RATE_6];
						}

						break;
					case EAP_CCK:	/* CCK */
						transmit->field.MODE = MODE_CCK;
						transmit->field.BW =  BW_20;
						if (wlan_config_get_ch_band(wdev) != CMD_CH_BAND_24G
							|| mcs > 3) {
							MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
								"CCK mode: err band(%d) 5/6G, error MCS(%d)!\n",
								wlan_config_get_ch_band(wdev), mcs);
							return;
						}

						if (pAd->CommonCfg.TxPreamble == Rt802_11PreambleShort) {
							if (mcs == 0) {
								mcs = mcs + 1;
								MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
										"Invalid MCS(CCK-1M) value when short preamble is enabled!\n");
							}
							mcs |= 0x4;
						}
						transmit->field.MCS = mcs;

						break;
					case EAP_OFDM:	/* OFDM */
						transmit->field.MODE = MODE_OFDM;
						transmit->field.BW =  BW_20;
						if (mcs > 7) {
							MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
									 "MCS must in range from 0 to 7 for OFDM Mode.\n");
							mcs = 0;
						} else
							transmit->field.MCS = mcs;

						break;
					default:
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
								 "unknown hpri ratetype/phymode/mcs %d/%d/%d.\n", ratetype, phymode, mcs);
						return;
					}

					if (phymode == EAP_RATE_DISABLE)
						wdev->eap.eap_hprirate_en &= ~(1 << ratetype);
					else
						wdev->eap.eap_hprirate_en |= (1 << ratetype);

				} else {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"Format Error!\n");
					break;
				}
			}
		}
	}

/* SupRateSet */
	if (RTMPGetKeyParameter("suprateset", tmpbuf, 32, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			INT supratesetbitmap = 0, j = 0;
			UCHAR rate[] = {
				0x82, 0x84, 0x8b, 0x96, 0x8C, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6c};
			struct wifi_dev *wdev;
			struct legacy_rate *eap_legacy_rate;

			if (i >= pAd->ApCfg.BssidNum)
				break;

			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			eap_legacy_rate = &wdev->eap.eap_legacy_rate;

			supratesetbitmap = os_str_tol(macptr, 0, 10);
			wdev->eap.eap_suprate_en = TRUE;
			eap_legacy_rate->sup_rate_len = 0;
			eap_legacy_rate->ext_rate_len = 0;
			for (j = 0; j < MAX_LEN_OF_SUPPORTED_RATES; j++) {
				if (supratesetbitmap & (1 << j)) {
					if (WMODE_EQUAL(wdev->PhyMode, WMODE_B)
						&& (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G)) {
						eap_legacy_rate->sup_rate[eap_legacy_rate->sup_rate_len] = rate[j];
						eap_legacy_rate->sup_rate_len++;
						wdev->eap.eapsupportcckmcs |= (1 << j);
						wdev->eap.eapsupportratemode |= SUPPORT_CCK_MODE;
					} else if ((wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G
						|| wlan_config_get_ch_band(wdev) == CMD_CH_BAND_6G) && (j > 3)) {
						eap_legacy_rate->sup_rate[eap_legacy_rate->sup_rate_len] = rate[j];
						eap_legacy_rate->sup_rate_len++;
						wdev->eap.eapsupportofdmmcs |= (1 << (j - 4));
						wdev->eap.eapsupportratemode |= SUPPORT_OFDM_MODE;
					} else {
						if ((j < 4) || (j == 5) || (j == 7) || (j == 9) || (j == 11)) {
							eap_legacy_rate->sup_rate[eap_legacy_rate->sup_rate_len] = rate[j];
							eap_legacy_rate->sup_rate_len++;
							if (j < 4) {
								wdev->eap.eapsupportcckmcs |= (1 << j);
								wdev->eap.eapsupportratemode |= SUPPORT_CCK_MODE;
							} else {
								wdev->eap.eapsupportofdmmcs |= (1 << (j - 4));
								wdev->eap.eapsupportratemode |= SUPPORT_OFDM_MODE;
							}
						} else {
							eap_legacy_rate->ext_rate[eap_legacy_rate->ext_rate_len] = rate[i] & 0x7f;
							eap_legacy_rate->ext_rate_len++;
							wdev->eap.eapsupportofdmmcs |= (1 << (j - 4));
							wdev->eap.eapsupportratemode |= SUPPORT_OFDM_MODE;
						}
					}
				}
			}
		}
	}
	/* HtSupRateSet */
	if (RTMPGetKeyParameter("htsuprateset", tmpbuf, 32, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			UINT32 htsupratesetbitmap = 0;
			struct wifi_dev *wdev;

			if (i >= pAd->ApCfg.BssidNum)
				break;

			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			htsupratesetbitmap = (UINT32) os_str_tol(macptr, 0, 10);
			wdev->eap.eap_htsuprate_en = TRUE;
			wdev->eap.eapsupporthtmcs = htsupratesetbitmap;
			wdev->eap.eapmcsset[0] = htsupratesetbitmap & 0x000000ff;
			wdev->eap.eapmcsset[1] = (htsupratesetbitmap & 0x0000ff00) >> 8;
			wdev->eap.eapmcsset[2] = (htsupratesetbitmap & 0x00ff0000) >> 16;
			wdev->eap.eapmcsset[3] = (htsupratesetbitmap & 0xff000000) >> 24;
		}
	}
	/* VhtSupRateSet */
	if (RTMPGetKeyParameter("vhtsuprateset", tmpbuf, 32, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			UINT32 vhtsupratesetbitmap = 0;
			struct wifi_dev *wdev;

			if (i >= pAd->ApCfg.BssidNum)
				break;

			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			vhtsupratesetbitmap = (UINT32) os_str_tol(macptr, 0, 10);
			wdev->eap.eap_vhtsuprate_en = TRUE;
			wdev->eap.rx_mcs_map.mcs_ss1 = vhtsupratesetbitmap & 0x0000003;
			wdev->eap.rx_mcs_map.mcs_ss2 = (vhtsupratesetbitmap & 0x0000000c) >> 2;
			wdev->eap.rx_mcs_map.mcs_ss3 = (vhtsupratesetbitmap & 0x00000030) >> 4;
			wdev->eap.rx_mcs_map.mcs_ss4 = (vhtsupratesetbitmap & 0x000000c0) >> 6;
			wdev->eap.rx_mcs_map.mcs_ss5 = (vhtsupratesetbitmap & 0x00000300) >> 8;
			wdev->eap.rx_mcs_map.mcs_ss6 = (vhtsupratesetbitmap & 0x00000c00) >> 10;
			wdev->eap.rx_mcs_map.mcs_ss7 = (vhtsupratesetbitmap & 0x00003000) >> 12;
			wdev->eap.rx_mcs_map.mcs_ss8 = (vhtsupratesetbitmap & 0x0000c000) >> 14;

			wdev->eap.tx_mcs_map.mcs_ss1 = (vhtsupratesetbitmap & 0x00030000) >> 16;
			wdev->eap.tx_mcs_map.mcs_ss2 = (vhtsupratesetbitmap & 0x000c0000) >> 18;
			wdev->eap.tx_mcs_map.mcs_ss3 = (vhtsupratesetbitmap & 0x00300000) >> 20;
			wdev->eap.tx_mcs_map.mcs_ss4 = (vhtsupratesetbitmap & 0x00c00000) >> 22;
			wdev->eap.tx_mcs_map.mcs_ss5 = (vhtsupratesetbitmap & 0x03000000) >> 24;
			wdev->eap.tx_mcs_map.mcs_ss6 = (vhtsupratesetbitmap & 0x0c000000) >> 26;
			wdev->eap.tx_mcs_map.mcs_ss7 = (vhtsupratesetbitmap & 0x30000000) >> 28;
			wdev->eap.tx_mcs_map.mcs_ss8 = (vhtsupratesetbitmap & 0xc0000000) >> 30;
		}
	}
#ifdef DOT11_HE_AX
	if (RTMPGetKeyParameter("hesuprateset", tmpbuf, 32 * 4, buf, TRUE)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			 "hesuprateset=%s\n", tmpbuf);
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
		     macptr = rstrtok(NULL, ";"), i++) {
			struct wifi_dev *wdev;
			UINT32 mcs_set[3];
			INT j;

			if (i >= pAd->ApCfg.BssidNum)
				break;

			wdev = &pAd->ApCfg.MBSSID[i].wdev;

			memset(mcs_set, 0xff, sizeof(mcs_set));
			for (j = 0, macptr = rstrtok(macptr, ",");
			     macptr && (j < ARRAY_SIZE(mcs_set));
			     macptr = rstrtok(NULL, ","), j++) {
				mcs_set[j] = (UINT32)os_str_tol(macptr, 0, 0);
			}
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"mcs_set: 0x%x 0x%x 0x%x\n",
					mcs_set[0], mcs_set[1], mcs_set[2]);

			for (j = 0; j < HE_MAX_SUPPORT_STREAM; j++ ) {
				wdev->eap.rate.he80_rx_nss_mcs[j] = (mcs_set[0] >> (j*2)) & 0x3;
				wdev->eap.rate.he80_tx_nss_mcs[j] = (mcs_set[0] >> (16+j*2)) & 0x3;
				wdev->eap.rate.he160_rx_nss_mcs[j] = (mcs_set[1] >> (j*2)) & 0x3;
				wdev->eap.rate.he160_tx_nss_mcs[j] = (mcs_set[1] >> (16+j*2)) & 0x3;
				wdev->eap.rate.he8080_rx_nss_mcs[j] = (mcs_set[2] >> (j*2)) & 0x3;
				wdev->eap.rate.he8080_tx_nss_mcs[j] = (mcs_set[2] >> (16+j*2)) & 0x3;
			}

			wdev->eap.eap_hesuprate_en = TRUE;
		}
	}
#endif /* DOT11_HE_AX */

#ifdef DOT11_EHT_BE
	if (RTMPGetKeyParameter("ehtsuprateset", tmpbuf, 32 * 4, buf, TRUE)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			 "ehtsuprateset=%s\n", tmpbuf);
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
		     macptr = rstrtok(NULL, ";"), i++) {
			struct wifi_dev *wdev;
			UINT32 nss_set[3];
			INT j;
			UINT8 tx_nss;
			UINT8 rx_nss;

			if (i >= pAd->ApCfg.BssidNum)
				break;

			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			tx_nss = wlan_config_get_tx_stream(wdev);
			rx_nss = wlan_config_get_rx_stream(wdev);

			memset(nss_set, 0x00, sizeof(nss_set));
			for (j = 0, macptr = rstrtok(macptr, ",");
			     macptr && (j < ARRAY_SIZE(nss_set));
			     macptr = rstrtok(NULL, ","), j++) {
				nss_set[j] = (UINT32)os_str_tol(macptr, 0, 0);
			}
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"nss_set: 0x%x 0x%x 0x%x\n",
					nss_set[0], nss_set[1], nss_set[2]);

			for (i = 1; i < EHT_MCS_NSS_BW_NUM; i++) {
				wdev->eap.rate.rx_nss_mcs[i].mcs0_9 = CHECK_NSS_RANGE((nss_set[i-1] & 0xf), rx_nss);
				wdev->eap.rate.tx_nss_mcs[i].mcs0_9 = CHECK_NSS_RANGE(((nss_set[i-1] >> 4) & 0xf), tx_nss);
				wdev->eap.rate.rx_nss_mcs[i].mcs10_11 = CHECK_NSS_RANGE(((nss_set[i-1] >> 8) & 0xf), rx_nss);
				wdev->eap.rate.tx_nss_mcs[i].mcs10_11 = CHECK_NSS_RANGE(((nss_set[i-1] >> 12) & 0xf), tx_nss);
				wdev->eap.rate.rx_nss_mcs[i].mcs12_13 = CHECK_NSS_RANGE(((nss_set[i-1] >> 16) & 0xf), rx_nss);
				wdev->eap.rate.tx_nss_mcs[i].mcs12_13 = CHECK_NSS_RANGE(((nss_set[i-1] >> 20) & 0xf), tx_nss);
			}

			wdev->eap.eap_ehtsuprate_en = TRUE;
		}
	}
#endif /* DOT11_EHT_BE */
}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
void RTMPSetSTASSID(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RTMP_STRING *SSID)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	pStaCfg->SsidLen = (UCHAR) strlen(SSID);
	NdisZeroMemory(pStaCfg->Ssid, NDIS_802_11_LENGTH_SSID);
	NdisMoveMemory(pStaCfg->Ssid, SSID, pStaCfg->SsidLen);
	pStaCfg->LastSsidLen = pStaCfg->SsidLen;
	NdisZeroMemory(pStaCfg->LastSsid, NDIS_802_11_LENGTH_SSID);
	NdisMoveMemory(pStaCfg->LastSsid, SSID, pStaCfg->LastSsidLen);
	pStaCfg->MlmeAux.AutoReconnectSsidLen = pStaCfg->SsidLen;
	NdisZeroMemory(pStaCfg->MlmeAux.AutoReconnectSsid, NDIS_802_11_LENGTH_SSID);
	NdisMoveMemory(pStaCfg->MlmeAux.AutoReconnectSsid, SSID, pStaCfg->MlmeAux.AutoReconnectSsidLen);
	pStaCfg->MlmeAux.SsidLen = pStaCfg->SsidLen;
	NdisZeroMemory(pStaCfg->MlmeAux.Ssid, NDIS_802_11_LENGTH_SSID);
	NdisMoveMemory(pStaCfg->MlmeAux.Ssid, SSID, pStaCfg->MlmeAux.SsidLen);
}


void RTMPSetSTAPassPhrase(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RTMP_STRING *PassPh)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	int ret;

	PassPh[strlen(PassPh)] = '\0'; /* make STA can process .$^& for WPAPSK input */

	if ((!IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap))
	   )
		ret = FALSE;
	else
		ret = SetWPAPSKKey(pAd, PassPh, strlen(PassPh), (PUCHAR)pStaCfg->Ssid, pStaCfg->SsidLen, pStaCfg->PMK);

	/*TODO: review it, if the ret is already FALSE. */
	if (IS_AKM_OWE(wdev->SecConfig.AKMMap))
		ret = TRUE;

	if (ret == TRUE) {
		RTMPZeroMemory(pStaCfg->WpaPassPhrase, 64);
		RTMPMoveMemory(pStaCfg->WpaPassPhrase, PassPh, strlen(PassPh));
		pStaCfg->WpaPassPhraseLen = strlen(PassPh);

		if (IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap) ||
		    IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap) ||
		    IS_AKM_OWE(wdev->SecConfig.AKMMap)) {
			/* Start STA supplicant state machine*/
			pStaCfg->WpaState = SS_START;
		} else if IS_AKM_WPANONE(wdev->SecConfig.AKMMap)
			pStaCfg->WpaState = SS_NOTUSE;

#ifdef WSC_STA_SUPPORT
		NdisZeroMemory(pStaCfg->wdev.WscControl.WpaPsk, 64);
		pStaCfg->wdev.WscControl.WpaPskLen = 0;

		if ((strlen(PassPh) >= 8) && (strlen(PassPh) <= 64)) {
			NdisMoveMemory(pStaCfg->wdev.WscControl.WpaPsk, PassPh, strlen(PassPh));
			pStaCfg->wdev.WscControl.WpaPskLen = strlen(PassPh);
		}

#endif /* WSC_STA_SUPPORT */
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "(WPAPSK=%s)\n", PassPh);
	}
}


inline void RTMPSetSTACipherSuites(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, NDIS_802_11_ENCRYPTION_STATUS WepStatus)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	/* Update all wepstatus related*/
	pStaCfg->PairwiseCipher = SecEncryModeOldToNew(WepStatus);
	pStaCfg->GroupCipher = SecEncryModeOldToNew(WepStatus);
}

#endif /* CONFIG_STA_SUPPORT */


void RTMPSetCountryCode(RTMP_ADAPTER *pAd, RTMP_STRING *CountryCode)
{
	if (strlen(CountryCode) == 2) {
		NdisMoveMemory(pAd->CommonCfg.CountryCode, CountryCode, 2);
		pAd->CommonCfg.CountryCode[2] = ' ';
#ifdef CONFIG_STA_SUPPORT
#ifdef EXT_BUILD_CHANNEL_LIST
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		NdisMoveMemory(pAd->StaCfg[0].StaOriCountryCode, CountryCode, 2);
#endif /* EXT_BUILD_CHANNEL_LIST */
#endif /* CONFIG_STA_SUPPORT */

		pAd->CommonCfg.bCountryFlag = TRUE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			"CountryCode(%s) is invalid, length!=2.\n", CountryCode);
		pAd->CommonCfg.bCountryFlag = FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
		"CountryCode=%s\n", pAd->CommonCfg.CountryCode);
}

#ifdef DOT11_EHT_BE
void rtmp_set_pp_black_list(RTMP_ADAPTER *pAd, RTMP_STRING *CountryCode, UINT8 member)
{
	if (member >= MAX_PP_BLACK_LIST_LEN)
		return;
	if (strlen(CountryCode) == 2) {
		NdisMoveMemory(pAd->CommonCfg.pp_bl[member].CountryCode, CountryCode, 2);
#ifdef CONFIG_STA_SUPPORT
		NdisMoveMemory(pAd->StaCfg[0].pp_bl[member].CountryCode, CountryCode, 2);
#endif /* CONFIG_STA_SUPPORT */
	} else
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			"CountryCode(%s) is invalid, length!=2.\n",
			CountryCode);

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
		"CountryCode=%s\n", CountryCode);
}
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11V_MBSSID_SUPPORT
static VOID rtmp_read_dot11v_mbssid_cfg_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UINT32 i, j;
	CHAR *macptr;
	bool bDot11vMbss;

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		bDot11vMbss = os_str_tol(macptr, 0, 10);
		if (i >= pAd->ApCfg.BssidNum)
			break;

		/* build 11v mbssid mapping */
		if (bDot11vMbss) {

			/* init */
			if (i == 0) {
				for (j = 0; j < MAX_TX_BSS_CNT; j++) {
					pAd->ApCfg.dot11v_BssidNum[j] =  0;
					pAd->ApCfg.dot11v_max_indicator[j] =  0;
				}
			}

			pAd->ApCfg.dot11v_mbssid_bitmap |= (1 << i);

			/*
			 * Old profile setting - only support one T-BSS(ra0)
			 */
			if (i == 0)
				pAd->ApCfg.MBSSID[i].mbss_11v.mbss_11v_enable = MBSS_11V_T;
			else
				pAd->ApCfg.MBSSID[i].mbss_11v.mbss_11v_enable = MBSS_11V_NT;

			pAd->ApCfg.MBSSID[i].mbss_11v.mbss_11v_t_bss_idx = 0;

			pAd->ApCfg.dot11v_BssidNum[0]++;
		}
	}

	if (pAd->ApCfg.dot11v_BssidNum[0]) {
		pAd->ApCfg.dot11v_max_indicator[0] = mbss_11v_bssid_num_to_max_indicator(pAd->ApCfg.dot11v_BssidNum[0]);
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_NOTICE,
			"Dot11v_11vGrp[%d] : NumBss = %u, max_indicator=%u\n", 0, pAd->ApCfg.dot11v_BssidNum[0], pAd->ApCfg.dot11v_max_indicator[0]);
	}
}
#endif


#ifdef DOT11V_MBSSID_SUPPORT
static VOID rtmp_read_dot11v_mbssid_cfg_from_file_ext(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UINT32 i, j;
	CHAR *macptr;
	UCHAR grp_idx = 0;
	UINT last_tx_idx = 0;

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		if (i >= pAd->ApCfg.BssidNum)
			break;

		/* TX-x  or NT-x  or CH-x, x=0~3 */
		if (strlen(macptr) == 4) {
			if (i == 0) {
				for (j = 0; j < MAX_TX_BSS_CNT; j++) {
					pAd->ApCfg.dot11v_BssidNum[j] =  0;
					pAd->ApCfg.dot11v_max_indicator[j] =  0;
				}
			}

			if (!strncmp(macptr, "TX-", 3)) {
				grp_idx = (u8)os_str_tol(macptr+3, 0, 10);
				last_tx_idx = i;
				MTWF_DBG(NULL, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_NOTICE,
					"%u, Role: %s, group id : %u\n", i, macptr, grp_idx);
				pAd->ApCfg.MBSSID[i].mbss_11v.mbss_11v_grp_idx = grp_idx;
				pAd->ApCfg.MBSSID[i].mbss_11v.mbss_11v_enable = MBSS_11V_T;
				pAd->ApCfg.MBSSID[i].mbss_11v.mbss_11v_t_bss_idx = last_tx_idx;
				pAd->ApCfg.dot11v_BssidNum[grp_idx]++;

				/* only Tx & NT BSS need set global bitmap */
				pAd->ApCfg.dot11v_mbssid_bitmap |= (1 << i);
			} else if (!strncmp(macptr, "NT-", 3)) {
				grp_idx = (u8)os_str_tol(macptr+3, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_NOTICE,
					"%u, Role: %s, group id : %u\n", i, macptr, grp_idx);
				pAd->ApCfg.MBSSID[i].mbss_11v.mbss_11v_grp_idx = grp_idx;
				pAd->ApCfg.MBSSID[i].mbss_11v.mbss_11v_enable = MBSS_11V_NT;
				pAd->ApCfg.MBSSID[i].mbss_11v.mbss_11v_t_bss_idx = last_tx_idx;
				pAd->ApCfg.dot11v_BssidNum[grp_idx]++;

				/* only Tx & NT BSS need set global bitmap */
				pAd->ApCfg.dot11v_mbssid_bitmap |= (1 << i);
			} else if (!strncmp(macptr, "CH-", 3)) {
				grp_idx = (u8)os_str_tol(macptr+3, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_NOTICE,
					"%u, Role: %s, group id : %u\n", i, macptr, grp_idx);
				pAd->ApCfg.MBSSID[i].mbss_11v.mbss_11v_grp_idx = grp_idx;
				pAd->ApCfg.MBSSID[i].mbss_11v.mbss_11v_enable = MBSS_11V_CH;
				pAd->ApCfg.MBSSID[i].mbss_11v.mbss_11v_t_bss_idx = last_tx_idx;
				pAd->ApCfg.dot11v_BssidNum[grp_idx]++;
			}

		} else {
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_ERROR,
				"!!! ERROR!!! %u, Unknown Role: %s !!! CHECK !!!\n", i, macptr);
		}
	}


	for (j = 0; j < MAX_TX_BSS_CNT; j++) {
		if (pAd->ApCfg.dot11v_BssidNum[j])
			pAd->ApCfg.dot11v_max_indicator[j] = mbss_11v_bssid_num_to_max_indicator(pAd->ApCfg.dot11v_BssidNum[j]);

		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_NOTICE,
			"Dot11v_11vGrp[%d] : NumBss = %u, max_indicator=%u\n", j, pAd->ApCfg.dot11v_BssidNum[j], pAd->ApCfg.dot11v_max_indicator[j]);
	}
}
#endif

#ifdef CONFIG_6G_SUPPORT
static VOID rtmp_read_6g_discov_cfg_from_file(RTMP_ADAPTER *pAd, char *tmpbuf, char *pBuffer)
{
	UINT8	apidx;
	UCHAR	iob_type = 0, iob_interval = 0, iob_mode = 0;
	UCHAR	rnr_6g_in_probe = 0;
	BOOLEAN cfg_valid[4] = {FALSE};
	BOOLEAN iob_type_by_cfg = FALSE;

	/* read 6g iob config */
	if (RTMPGetKeyParameter("He6gIobType", tmpbuf, 25, pBuffer, TRUE)) {
		if (strlen(tmpbuf) != 0) {
			iob_type = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			cfg_valid[0] = TRUE;
			iob_type_by_cfg = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"He6gIobType=%d\n", iob_type);
		}
	}

	if (RTMPGetKeyParameter("He6gIobTu", tmpbuf, 25, pBuffer, TRUE)) {
		if (strlen(tmpbuf) != 0) {
			iob_interval = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			cfg_valid[1] = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"He6gIobTu=%d\n", iob_interval);
		}
	} else
		iob_interval = UNSOLICIT_TX_INTERVAL;

	if (RTMPGetKeyParameter("He6gIobMode", tmpbuf, 25, pBuffer, TRUE)) {
		if (strlen(tmpbuf) != 0) {
			iob_mode = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			if (iob_mode <= UNSOLICIT_TXMODE_HE_SU)
				cfg_valid[2] = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"He6gIobMode=%d\n", iob_mode);
		}
	}

	/* read 6g oob config */
	if (RTMPGetKeyParameter("He6gOob", tmpbuf, 25, pBuffer, TRUE)) {
		if (strlen(tmpbuf) != 0) {
			rnr_6g_in_probe = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			cfg_valid[3] = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"He6gOob=%d\n", rnr_6g_in_probe);
		}
	}

	/* apply setting to wdev */
	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;

		if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G)) {
			/* 6g: default config if parameter not found */
			if (!cfg_valid[0])
				iob_type =		UNSOLICIT_TX_PROBE_RSP;
			if (!cfg_valid[1])
				iob_interval =	UNSOLICIT_TX_INTERVAL;
			if (!cfg_valid[2])
				iob_mode = (pAd->CommonCfg.wifi_cert) ?
							UNSOLICIT_TXMODE_HE_SU : UNSOLICIT_TXMODE_NON_HT;
			if (!cfg_valid[3]) {
				rnr_6g_in_probe = RNR_REPORTING_NONE; /* No 6G RNR in-band */
			}
		} else {
			/* 2g/5g: default config */
			iob_type =			UNSOLICIT_TX_DISABLE;
			iob_interval =		0;
			iob_mode =			UNSOLICIT_TXMODE_NON_HT;

			if (!cfg_valid[3])
				rnr_6g_in_probe = RNR_REPORTING_ALL_BSS; /* 2G/5G report 6G*/
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"Bss(%d): phymode=%d, iob_type=%d%s, iob_interval=%d, iob_mode=%d, oob_rnr=%d\n",
			apidx, wdev->PhyMode, iob_type, iob_type_by_cfg ? "(byCfg)" : "(auto)",
			iob_interval, iob_mode, rnr_6g_in_probe);

		wlan_operate_set_he_6g_qos_state(wdev, false);
		wlan_operate_set_he_6g_qos_tu(wdev, 0);
		in_band_discovery_update_conf(wdev, iob_type, iob_interval, iob_mode, iob_type_by_cfg);
		out_band_discovery_update(wdev, rnr_6g_in_probe);
	}

}
#endif

static VOID RTMPDtimPeriodCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UINT32 i;
	CHAR *macptr;
	BSS_STRUCT *pMbss = NULL;
	UINT8 DtimPeriod;

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		DtimPeriod = os_str_tol(macptr, 0, 10);
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			if (i == 0)
				pAd->ApCfg.DtimPeriod = DtimPeriod;

			pMbss = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)];
			pMbss->DtimPeriod = DtimPeriod;
		}
	}

#ifdef MBSS_SUPPORT
	/* Check if any 0 case, apply MSSID 0 DtimPeriod to it.*/
	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		pMbss = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)];

		if (pMbss->DtimPeriod == 0)
			pMbss->DtimPeriod = pAd->ApCfg.DtimPeriod;

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"BSS%d DtimPeriod=%d\n", i, pMbss->DtimPeriod);
	}
#endif/*MBSS_SUPPORT*/

}
#endif /*CONFIG_AP_SUPPORT*/

NDIS_STATUS	RTMPSetPreProfileParameters(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *tmpbuf;

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (tmpbuf == NULL)
		return NDIS_STATUS_FAILURE;


#ifdef MULTI_INTR_SUPPORT
	if (RTMPGetKeyParameter("MultiIntr", tmpbuf, 10,  pBuffer, TRUE))
		RTMPMultiIntrCfg(pAd, tmpbuf);
#endif

	os_free_mem(tmpbuf);
	return NDIS_STATUS_SUCCESS;
}

#ifdef MGMT_TXPWR_CTRL
void rtmp_read_mgmt_pwr_parms_from_file(
		IN RTMP_ADAPTER *pAd,
		IN RTMP_STRING *tmpbuf,
		IN RTMP_STRING *pBuffer)
{
	UINT32 i4Recv, value0, value1;
	UINT8 i;
	RTMP_STRING *macptr;

	if (RTMPGetKeyParameter("MgmtTxPwr", tmpbuf, 256, pBuffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;
			i4Recv = sscanf(macptr, "%d.%d", &(value0), &(value1));

			 /* in 0.5 db scale*/
			value0 *= 2;
			if (i4Recv == 2 && value1 == 5)
				value0++;

			pAd->ApCfg.MBSSID[i].wdev.MgmtTxPwr = value0;
			pAd->ApCfg.MBSSID[i].wdev.MgmtTxPwrBak = value0;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"I/F(ra%d) MgmtTxPwr=%d\n", i, value0);
		}
	}
}
#endif

#ifdef ANTENNA_CONTROL_SUPPORT
void rtmp_read_ant_ctrl_parms_from_file(
		IN RTMP_ADAPTER *pAd,
		IN RTMP_STRING *tmpbuf,
		IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;
	UINT32 Txstream = 0, Rxstream = 0;
	UINT8 i, Band_idx = 0;
	struct mcs_nss_caps *nss_cap = MCS_NSS_CAP(pAd);

	if (RTMPGetKeyParameter("AntCtrl", tmpbuf, 32, pBuffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			if (wdev == NULL) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"Incorrect BSS\n");
				continue;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_NOTICE,
				"Default:nss_cap->max_nss = %d, nss_cap->max_path =(tx:%d /rx:%d)!!\n",
				nss_cap->max_nss, nss_cap->max_path[0], nss_cap->max_path[1]);

			Band_idx = HcGetBandByWdev(wdev);

			/* Default value of antenna when input 0 */
			if (0 == os_str_tol(macptr, 0, 10)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"Band_idx = %d default Antenna number!!\n", Band_idx);
				goto set_default;
			}

			if (strlen(macptr) != 4) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"Please use input format like xTxR (x <= %d)!!\n",
						nss_cap->max_nss);
				goto set_default;
			}

			if (((macptr[1] == 'T') || (macptr[1] == 't')) && ((macptr[3] == 'R') || (macptr[3] == 'r'))) {
				Txstream = simple_strtol(&macptr[0], 0, 10);
				Rxstream = simple_strtol(&macptr[2], 0, 10);

				if (Txstream != Rxstream) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"Band_idx = %d Tx & Rx Antenna number different, Set to Default!!\n",
							Band_idx);
					goto set_default;
				}
				if (Txstream > nss_cap->max_nss) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"Band_idx=%d Wrong Configuration Ant number > MAX Support == %d!!\n",
							Band_idx, nss_cap->max_nss);
					goto set_default;
				}
				pAd->TxStream = Txstream;
				pAd->RxStream = Rxstream;
				pAd->bAntennaSetAPEnable = 1;
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_WARN,
					"%s : Band_idx=%d, Tx_Stream=%d, Rx_Stream=%d\n",
						__func__, Band_idx, pAd->TxStream, pAd->RxStream);
				continue;
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"Invalid input\n");
				continue;
			}
set_default:
#ifdef DOT11_HE_AX
			if (WMODE_CAP_AX(wdev->PhyMode)) {
				pAd->RxStream = wlan_config_get_he_rx_nss(wdev);
				pAd->TxStream = wlan_config_get_he_tx_nss(wdev);
			} else
#endif
			{
				pAd->RxStream = wlan_config_get_rx_stream(wdev);
				pAd->TxStream = wlan_config_get_tx_stream(wdev);
			}
			pAd->bAntennaSetAPEnable = 0;

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_WARN,
				"%s : Band_idx=%d, Tx_Stream=%d, Rx_Stream=%d\n",
				__func__, Band_idx, pAd->TxStream, pAd->RxStream);
		}
	}
}
#endif /* ANTENNA_CONTROL_SUPPORT */
#ifdef ANDLINK_FEATURE_SUPPORT
void rtmp_read_andlink_parms_from_file(
		IN RTMP_ADAPTER *pAd,
		IN RTMP_STRING *tmpbuf,
		IN RTMP_STRING *pBuffer)
{
	UCHAR bandidx = 0;
	UINT16 tmp_value = 0;
	RTMP_STRING *macptr = NULL;

	/*init the value,default is disable*/
	pAd->CommonCfg.andlink_enable = FALSE;
	pAd->CommonCfg.andlink_ip_hostname_en = FALSE;
	bandidx = hc_get_hw_band_idx(pAd);

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "andlink_en=%d, in BAND: %d\n",
					pAd->CommonCfg.andlink_enable, bandidx);
	/*andlink_en*/
	if (RTMPGetKeyParameter("andlink_en", tmpbuf, 256, pBuffer, FALSE)) {
		tmp_value = 0;
		bandidx = 0;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"andlink_en=%d, in BAND: %d\n",
			pAd->CommonCfg.andlink_enable, bandidx);
		for (bandidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), bandidx++) {
			if (bandidx < CFG_WIFI_RAM_BAND_NUM) {
				tmp_value = (UINT16)os_str_tol(macptr, 0, 10);
				pAd->CommonCfg.andlink_enable = tmp_value > 0 ? TRUE : FALSE;
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"andlink_en=%d, in BAND: %d\n",
					pAd->CommonCfg.andlink_enable, bandidx);
			} else
				break;
		}
	}

	/*andlink_ip_hostname_en*/
	if (RTMPGetKeyParameter("andlink_ip_hostname_en", tmpbuf, 256, pBuffer, FALSE)) {
			tmp_value = 0;
			bandidx = 0;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"andlink_ip_hostname_en=%d, in BAND: %d\n",
				pAd->CommonCfg.andlink_ip_hostname_en, bandidx);
			for (bandidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), bandidx++) {
				if (bandidx < CFG_WIFI_RAM_BAND_NUM) {
					tmp_value = (UINT16)os_str_tol(macptr, 0, 10);
					pAd->CommonCfg.andlink_ip_hostname_en = tmp_value > 0 ? TRUE : FALSE;
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"andlink_ip_hostname_en=%d, in BAND: %d\n",
					pAd->CommonCfg.andlink_ip_hostname_en, bandidx);
				} else
					break;
			}
		}
}

#endif/*ANDLINK_FEATURE_SUPPORT*/
#ifdef ACK_CTS_TIMEOUT_SUPPORT
void rtmp_read_ackcts_timeout_parms_from_file(
		IN RTMP_ADAPTER *pAd,
		IN RTMP_STRING *tmpbuf,
		IN RTMP_STRING *pBuffer) {

	RTMP_STRING *macptr = NULL;

	/*init the value, then will not set. keep CR default value*/
	pAd->CommonCfg.ack_cts_enable = FALSE;
	pAd->CommonCfg.cck_timeout = DEFALT_TMAC_CDTR_VALUE;
	pAd->CommonCfg.ofdm_timeout = DEFALT_TMAC_ODTR_VALUE;
	pAd->CommonCfg.ofdma_timeout = DEFALT_TMAC_OMDTR_VALUE;
	pAd->CommonCfg.distance = INVALID_DISTANCE;

	if (RTMPGetKeyParameter("ACK_CTS_TOUT_EN", tmpbuf, 256, pBuffer, FALSE)) {
		UINT16 tmp_value = 0;
		UCHAR bandidx = 0;

		for (bandidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), bandidx++) {
			if (bandidx < CFG_WIFI_RAM_BAND_NUM) {
				tmp_value = (UINT16)os_str_tol(macptr, 0, 10);
				pAd->CommonCfg.ack_cts_enable = tmp_value > 0 ? TRUE : FALSE;

			} else
				break;
		}
	}

	if (RTMPGetKeyParameter("CCK_ACK_TOUT", tmpbuf, 256, pBuffer, FALSE)) {
		UINT16 ack_timeout = 0;
		UCHAR bandidx = 0;

		for (bandidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), bandidx++) {
			if (bandidx < CFG_WIFI_RAM_BAND_NUM) {
				ack_timeout = (UINT16)os_str_tol(macptr, 0, 10);
				pAd->CommonCfg.cck_timeout = ack_timeout;

			} else
				break;
		}
	}

	if (RTMPGetKeyParameter("OFDM_ACK_TOUT", tmpbuf, 256, pBuffer, FALSE)) {
		UINT16 ack_timeout = 0;
		UCHAR bandidx = 0;

		for (bandidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), bandidx++) {
			if (bandidx < CFG_WIFI_RAM_BAND_NUM) {
				ack_timeout = (UINT16)os_str_tol(macptr, 0, 10);
				pAd->CommonCfg.ofdm_timeout = ack_timeout;
			} else
				break;
		}
	}

	if (RTMPGetKeyParameter("OFDMA_ACK_TOUT", tmpbuf, 256, pBuffer, FALSE)) {
		UINT16 ack_timeout = 0;
		UCHAR bandidx = 0;

		for (bandidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), bandidx++) {
			if (bandidx < CFG_WIFI_RAM_BAND_NUM) {
				ack_timeout = (UINT16)os_str_tol(macptr, 0, 10);
				pAd->CommonCfg.ofdma_timeout = ack_timeout;
			} else
				break;
		}
	}

	if (RTMPGetKeyParameter("Distance", tmpbuf, 256, pBuffer, FALSE)) {
		UINT16 distance = 0;
		UCHAR bandidx = 0;

		for (bandidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), bandidx++) {
			if (bandidx < CFG_WIFI_RAM_BAND_NUM) {
				distance = (UINT16)os_str_tol(macptr, 0, 10);
				pAd->CommonCfg.distance = distance;
			} else
				break;
		}
	}

	return;
}

#endif/*ACK_CTS_TIMEOUT_SUPPORT*/

#ifdef MLME_MULTI_QUEUE_SUPPORT
void rtmp_read_mlme_multiqueue_parms_from_file(
		IN RTMP_ADAPTER *pAd,
		IN RTMP_STRING *tmpbuf,
		IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr = NULL;
	UCHAR idx = 0;
	UCHAR ration_val;
	pAd->Mlme.MultiQEnable = FALSE;
	pAd->Mlme.HPQueue.Ration = RATION_OF_MLME_HP_QUEUE;
	pAd->Mlme.Queue.Ration = RATION_OF_MLME_QUEUE;
	pAd->Mlme.LPQueue.Ration = RATION_OF_MLME_LP_QUEUE;

	if (RTMPGetKeyParameter("MlmeMultiQEnable", tmpbuf, 128, pBuffer, TRUE)) {
		if ((UCHAR) simple_strtol(tmpbuf, 0, 10) != 0)
			pAd->Mlme.MultiQEnable = TRUE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"Mlme.MultiQEnable=%d\n", pAd->Mlme.MultiQEnable);
	}

	if (RTMPGetKeyParameter("MlmeMultiQCtrl", tmpbuf, 128, pBuffer, TRUE)) {
		for (idx = 0, macptr = rstrtok(tmpbuf, "-"); macptr && (idx < 3);
			macptr = rstrtok(NULL, "-"), idx++) {
			ration_val = (UINT16)os_str_tol(macptr, 0, 10);
			switch(idx) {
				case 0:
					pAd->Mlme.HPQueue.Ration = ration_val;
					break;
				case 1:
					pAd->Mlme.Queue.Ration = ration_val;
					break;
				case 2:
					pAd->Mlme.LPQueue.Ration = ration_val;
					break;
				default:
					break;

			}
		}
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"[hp_q_ration]-[np_q_ration]-[lp_q_ration] = %d-%d-%d\n",
			pAd->Mlme.HPQueue.Ration, pAd->Mlme.Queue.Ration, pAd->Mlme.LPQueue.Ration);
	}
}
#endif

void rtmp_read_quick_channel_switch_parms_from_file(
		IN RTMP_ADAPTER *pAd,
		IN RTMP_STRING *tmpbuf,
		IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr = NULL;
	UINT16 tmp_value = 0;
	INT i = 0;
	struct wifi_dev *wdev = NULL;

	if (RTMPGetKeyParameter("QuickChannelSwitch", tmpbuf, 256, pBuffer, FALSE)) {
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;

					if (macptr)
						tmp_value = (UINT16)os_str_tol(macptr, 0, 10);

					if (tmp_value > QUICK_CH_SWICH_ENABLE)
						tmp_value = QUICK_CH_SWICH_ENABLE;

					wdev->quick_ch_change = tmp_value;
				}
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), i++) {
					wdev = &pAd->StaCfg[i].wdev;

					if (macptr)
						tmp_value = (UINT16)os_str_tol(macptr, 0, 10);

					if (tmp_value > QUICK_CH_SWICH_ENABLE)
						tmp_value = QUICK_CH_SWICH_ENABLE;

					wdev->quick_ch_change = tmp_value;
				}
			}
#endif /* CONFIG_STA_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
					wdev->quick_ch_change = QUICK_CH_SWICH_ENABLE; /* Enable quick_ch_change by default */
				}
			}
#endif /* CONFIG_AP_SUPPORT */
	}
}

#ifdef WIFI_MD_COEX_SUPPORT
static void rtmp_read_coex_param_from_file(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	UINT16 powerBackoff;
	BOOLEAN idcState;

	// IDC State parameter
	if (RTMPGetKeyParameter("IdcState", tmpbuf, 32, buffer, TRUE)) {
		idcState = (BOOLEAN)os_str_tol(tmpbuf, 0, 10);
		if (idcState != 0 && idcState != 1)
			MTWF_DBG(pAd, DBG_CAT_COEX, CATCFG_PROFILE, DBG_LVL_ERROR,
				 "input parameter[IdcState=%d] incorrect! It should be 0 or 1\n", idcState);
		else {
			pAd->idcState = idcState;
			MTWF_DBG(pAd, DBG_CAT_COEX, CATCFG_PROFILE, DBG_LVL_NOTICE,
				"IdcState set to %s.\n", pAd->idcState ? "Enable" : "Disable");
		}
	}

	// Amplifier Power Backoff parameter
	if (RTMPGetKeyParameter("AmplifierPowerBackoff", tmpbuf, 32, buffer, TRUE)) {
		if (kstrtou16(tmpbuf, 10, &powerBackoff))
			MTWF_DBG(pAd, DBG_CAT_COEX, CATCFG_PROFILE, DBG_LVL_ERROR,
				"input parameter[AmplifierPowerBackoff=%s] incorrect!\n", tmpbuf);
		else {
			if (powerBackoff == 0)
				MTWF_DBG(pAd, DBG_CAT_COEX, CATCFG_PROFILE, DBG_LVL_ERROR,
					"AmplifierPowerBackoff can not set to 0.\n");
			else {
				pAd->CommonCfg.powerBackoff = powerBackoff;
				MTWF_DBG(pAd, DBG_CAT_COEX, CATCFG_PROFILE, DBG_LVL_NOTICE,
					"AmplifierPowerBackoff set to %d.\n", powerBackoff);
			}
		}
	}
}
#endif

#ifdef VLAN_SUPPORT
static VOID readProfileVlanIDWdev(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev = NULL;

	/*The order of parameter is ap firstly and apcli lastly in dat file*/
	/*VLANID=[ra0];[ra1];[ra2];[apcli0]*/
	/*VLANID=1;2;3;4, it implies VID of ra0 is 1, and apcli0 is 4.*/
	if (RTMPGetKeyParameter("VLANID", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				ConfigVlanID(pAd, wdev, val);
			else
				break;
		}
	}
}
static VOID readProfileVlanPriorityWdev(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev = NULL;

	/*order of parameter is same as VLANID*/
	if (RTMPGetKeyParameter("VLANPriority", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				ConfigVlanPriority(pAd, wdev, val);
			else
				break;
		}
	}
}
static VOID readProfileVlanTagWdev(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev = NULL;

	/*order of parameter is same as VLANID*/
	if (RTMPGetKeyParameter("VLANTag", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				ConfigVlanTag(pAd, wdev, val);
			else
				break;
		}
	}
}
static VOID readProfileVlanTxPolicyWdev(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev = NULL;

	/*order of parameter is same as VLANID*/
	if (RTMPGetKeyParameter("VLANTxPolicy", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				ConfigVlanTxPolicy(pAd, wdev, val);
			else
				break;
		}
	}
}
static VOID readProfileVlanRxPolicyWdev(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev = NULL;

	/*order of parameter is same as VLANID*/
	if (RTMPGetKeyParameter("VLANRxPolicy", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				ConfigVlanRxPolicy(pAd, wdev, val);
			else
				break;
		}
	}
}
#endif
NDIS_STATUS	RTMPSetProfileParameters(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *tmpbuf;
	RTMP_STRING *macptr = NULL;
#ifdef DOT11_EHT_BE
	RTMP_STRING *tmpptr = NULL;
#endif
	INT		i = 0, retval;
#if defined(TX_POWER_CONTROL_SUPPORT) || defined(MBSS_AS_WDS_AP_SUPPORT)
	CHAR	* value = 0;
#endif
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#if defined(TX_POWER_CONTROL_SUPPORT)
	CHAR	*value2 = 0;
	UINT8	ucPwrBoostReg[2] = {0};
#endif
#ifdef CONFIG_AP_SUPPORT
	INT		j = 0;
	RTMP_STRING tok_str[16];
	UCHAR BssidCountSupposed = 0;
	BOOLEAN bSSIDxIsUsed = FALSE;
	long readvalue;
	INT ret = 0;
#endif /* CONFIG_AP_SUPPORT */

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (tmpbuf == NULL)
		return NDIS_STATUS_FAILURE;

	/* If profile parameter is set, channel lists of HW bands need to be reset*/
	hc_init_ChCtrl(pAd);

#ifdef CONFIG_AP_SUPPORT
	/* If profile parameter is set, ACS parameters of HW bands need to be reset*/
	hc_init_ACSChCtrl(pAd);
#endif

	do {
		/* wifi certification */
		if (RTMPGetKeyParameter("WifiCert", tmpbuf, 25, pBuffer, TRUE)) {
			UCHAR wifi_cert = os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.wifi_cert = wifi_cert;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"WifiCert=%d\n", wifi_cert);
		}
		/* wifi tgn certification wmm test*/
		if (RTMPGetKeyParameter("TgnWMMCert", tmpbuf, 25, pBuffer, TRUE)) {
			UCHAR tgn_wmm_cert = os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.tgn_wmm_cert = tgn_wmm_cert;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"tgn_wmm_cert=%d\n", tgn_wmm_cert);
		}

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef MBSS_SUPPORT

			/*BSSIDNum; This must read first of other multiSSID field, so list this field first in configuration file*/
			if (RTMPGetKeyParameter("BssidNum", tmpbuf, 25, pBuffer, TRUE)) {
				pAd->ApCfg.BssidNum = (UCHAR) os_str_tol(tmpbuf, 0, 10);
				if (pAd->ApCfg.BssidNum > MAX_MBSSID_NUM(pAd)) {
					pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							 "BssidNum=%d(MAX_MBSSID_NUM is %d)\n",
							  pAd->ApCfg.BssidNum, MAX_MBSSID_NUM(pAd));
				} else
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"BssidNum=%d\n", pAd->ApCfg.BssidNum);
			}

			/* assign default value */
			for (i = 0; i < pAd->ApCfg.BssidNum; i++)
				pAd->ApCfg.Pf2MbssIdxMap[i] = i;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"Pf2MbssIdxMap:\n");
			for (i = 0; i < pAd->ApCfg.BssidNum; i++)
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "%2d ",
					pAd->ApCfg.Pf2MbssIdxMap[i]);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "\n");
#else
			pAd->ApCfg.BssidNum = 1;
#endif /* MBSS_SUPPORT */
		}

		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			UCHAR txcmd_mode;
			if (RTMPGetKeyParameter("TxCmdMode", tmpbuf, 25, pBuffer, TRUE)) {
				txcmd_mode = (UCHAR) os_str_tol(tmpbuf, 0, 10);
				hc_set_txcmd_mode(pAd->hdev_ctrl, txcmd_mode);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
		if (RTMPGetKeyParameter("RROSupport", tmpbuf, 25, pBuffer, TRUE))
			cap->hw_rro_en = (UCHAR) os_str_tol(tmpbuf, 0, 10);
		else
			cap->hw_rro_en = 0;

#ifdef ICAP_FW_SUPPORT
		if (RTMPGetKeyParameter("IsICAPFW", tmpbuf, 25, pBuffer, TRUE)) {
			UCHAR IcapFW = 0;

			IcapFW = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			if (IcapFW) {
				cap->wtbl_max_entries = 30;
				}
		}
#endif /* ICAP_FW_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (RTMPGetKeyParameter("MStaNum", tmpbuf, 25, pBuffer, TRUE)) {
				pAd->MaxMSTANum = (UCHAR) os_str_tol(tmpbuf, 0, 10);

				if (pAd->MaxMSTANum > MAX_MULTI_STA) {
					pAd->MaxMSTANum = MAX_MULTI_STA;
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							 "MStaNum=%d(MAX_MULTI_STA is %d)\n",
							  pAd->MaxMSTANum, MAX_MULTI_STA);
				} else
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"MStaNum=%d\n", pAd->MaxMSTANum);
			}
		}
#endif

		/* set file parameter to portcfg*/
#ifdef MT_MAC
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef MBSS_SUPPORT

			/* for MT7615, we could assign extend BSSID mac address by ourself. */
			/* extend index starts from 1.*/
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				if (i == 0)
					ret = snprintf(tok_str, sizeof(tok_str), "MacAddress");
				else
					ret = snprintf(tok_str, sizeof(tok_str), "MacAddress%d", i);
				if (os_snprintf_error(sizeof(tok_str), ret))
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"snprintf error!\n");

				if (RTMPGetKeyParameter(tok_str, tmpbuf, 25, pBuffer, TRUE)) {
					retval = RT_CfgSetMacAddress(pAd, tmpbuf, i, OPMODE_AP);

					if (retval)
						MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"MacAddress%d = "MACSTR"\n",
							i, MAC2STR(pAd->ExtendMBssAddr[i]));
				}
			}

#endif /* MBSS_SUPPORT */

#ifdef CONFIG_APSTA_MIXED_SUPPORT
			if (RTMPGetKeyParameter("ApcliMacAddress", tmpbuf, 25, pBuffer, TRUE)) {
				retval = RT_CfgSetMacAddress(pAd, tmpbuf, 0, OPMODE_STA);

				if (retval)
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"ApcliMacAddress = "MACSTR"\n",
						MAC2STR(pAd->ApcliAddr[0]));
			}
			if (RTMPGetKeyParameter("ApcliMacAddress1", tmpbuf, 25, pBuffer, TRUE)) {
				retval = RT_CfgSetMacAddress(pAd, tmpbuf, 1, OPMODE_STA);

				if (retval)
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"ApcliMacAddress1 = "MACSTR"\n",
						MAC2STR(pAd->ApcliAddr[1]));
			}
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

		}
#endif /*CONFIG_AP_SUPPORT*/
#endif /*MT_MAC*/

		/*CountryRegion*/
		if (RTMPGetKeyParameter("CountryRegion", tmpbuf, 25, pBuffer, TRUE)) {
			retval = RT_CfgSetCountryRegion(pAd, tmpbuf, BAND_24G);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"CountryRegion=%d\n", pAd->CommonCfg.CountryRegion);
		}

		/*CountryRegionABand*/
		if (RTMPGetKeyParameter("CountryRegionABand", tmpbuf, 25, pBuffer, TRUE)) {
			retval = RT_CfgSetCountryRegion(pAd, tmpbuf, BAND_5G);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"CountryRegionABand=%d\n", pAd->CommonCfg.CountryRegionForABand);
		}

		/*FreqList*/
		if (RTMPGetKeyParameter("FreqList", tmpbuf, 300, pBuffer, TRUE))
			CfgSetFreqList(pAd, tmpbuf);
#ifdef CAL_FREE_IC_SUPPORT

		/* DisableCalFree */
		if (RTMPGetKeyParameter("DisableCalFree", tmpbuf, 25, pBuffer, TRUE)) {
			UCHAR DisableCalFree = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

			if (DisableCalFree)
				ops->is_cal_free_ic = NULL;

			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"DisableCalFree=%d\n", DisableCalFree);
		}

#endif /* DisableCalFree */

		/*CountryCode*/
		if (pAd->CommonCfg.bCountryFlag == 0) {
			if (RTMPGetKeyParameter("CountryCode", tmpbuf, 25, pBuffer, TRUE))
				RTMPSetCountryCode(pAd, tmpbuf);
		}

		/*TpePSDsingleBW20*/
		if (RTMPGetKeyParameter("TpePSDsingleBW20", tmpbuf, 25, pBuffer, TRUE)) {
			UCHAR bTpePSDsingleBW20Flag = (UCHAR) os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.bTpePSDsingleBW20Flag = (BOOLEAN) bTpePSDsingleBW20Flag;
		}

#ifdef EXT_BUILD_CHANNEL_LIST

		/*ChannelGeography*/
		if (RTMPGetKeyParameter("ChannelGeography", tmpbuf, 25, pBuffer, TRUE)) {
			UCHAR Geography = (UCHAR) os_str_tol(tmpbuf, 0, 10);

			if (Geography <= BOTH) {
				pAd->CommonCfg.Geography = Geography;
				pAd->CommonCfg.CountryCode[2] =
					(pAd->CommonCfg.Geography == BOTH) ? ' ' : ((pAd->CommonCfg.Geography == IDOR) ? 'I' : 'O');
#ifdef CONFIG_STA_SUPPORT
#ifdef EXT_BUILD_CHANNEL_LIST
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				pAd->StaCfg[0].StaOriGeography = pAd->CommonCfg.Geography;
#endif /* EXT_BUILD_CHANNEL_LIST */
#endif /* CONFIG_STA_SUPPORT */
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"ChannelGeography=%d\n", pAd->CommonCfg.Geography);
			}
		} else {
			pAd->CommonCfg.Geography = BOTH;
			pAd->CommonCfg.CountryCode[2] = ' ';
		}

#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/* SSID*/
			if (TRUE) {
				/* PRINT(DBG_LVL_INFO, ("pAd->ApCfg.BssidNum=%d\n", pAd->ApCfg.BssidNum)); */
				for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					ret = snprintf(tok_str, sizeof(tok_str), "SSID%d", i + 1);
					if (os_snprintf_error(sizeof(tok_str), ret))
						MTWF_DBG(pAd,
						DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"snprintf error!\n");
					if (RTMPGetKeyParameter(tok_str, tmpbuf, 33, pBuffer, FALSE)) {
						NdisMoveMemory(pAd->ApCfg.MBSSID[i].Ssid,
							tmpbuf, strlen(tmpbuf));
						pAd->ApCfg.MBSSID[i].Ssid[strlen(tmpbuf)] = '\0';
						pAd->ApCfg.MBSSID[i].SsidLen =
							strlen((RTMP_STRING *)
								pAd->ApCfg.MBSSID[i].Ssid);

						if (bSSIDxIsUsed == FALSE)
							bSSIDxIsUsed = TRUE;
#ifdef MASK_PARTIAL_MACADDR  /* Sensitive log */
							MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"SSID[%d]=***, EdcaIdx=%d\n", i,
							pAd->ApCfg.MBSSID[i].wdev.EdcaIdx);
#else
							MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"SSID[%d]=%s, EdcaIdx=%d\n", i,
							pAd->ApCfg.MBSSID[i].Ssid,
							pAd->ApCfg.MBSSID[i].wdev.EdcaIdx);
#endif
					}
				}

				if (bSSIDxIsUsed == FALSE) {
					if (RTMPGetKeyParameter("SSID", tmpbuf, 256, pBuffer, FALSE)) {
						BssidCountSupposed = delimitcnt(tmpbuf, ";") + 1;

						if (pAd->ApCfg.BssidNum != BssidCountSupposed)
							MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"No. of SSIDs(=%d) != BssidNum(= %d)!\n",
							BssidCountSupposed, pAd->ApCfg.BssidNum);

						if (pAd->ApCfg.BssidNum > 1) {
							/* Anyway, we still do the legacy dissection of the whole SSID string.*/
							for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
								int apidx = 0;

								if (i < pAd->ApCfg.BssidNum)
									apidx = i;
								else
									break;

								NdisMoveMemory(pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].Ssid, macptr, strlen(macptr));
								pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].Ssid[strlen(macptr)] = '\0';
								pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].SsidLen = strlen((RTMP_STRING *)pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].Ssid);
								MTWF_DBG(NULL,
									DBG_CAT_CFG, CATCFG_PROFILE,
									DBG_LVL_INFO,
									"SSID[%d]=%s\n",
									i,
									pAd->ApCfg.MBSSID[i].Ssid);
							}
						} else {
							if ((strlen(tmpbuf) > 0) && (strlen(tmpbuf) <= 32)) {
								NdisMoveMemory(pAd->ApCfg.MBSSID[BSS0].Ssid, tmpbuf, strlen(tmpbuf));
								pAd->ApCfg.MBSSID[BSS0].Ssid[strlen(tmpbuf)] = '\0';
								pAd->ApCfg.MBSSID[BSS0].SsidLen = strlen((RTMP_STRING *) pAd->ApCfg.MBSSID[BSS0].Ssid);
								MTWF_DBG(NULL,
									DBG_CAT_CFG, CATCFG_PROFILE,
									DBG_LVL_INFO,
									"SSID=%s\n",
									pAd->ApCfg.MBSSID[0].Ssid);
							}
						}
					}
				}

				if (RTMPGetKeyParameter("EdcaIdx", tmpbuf, 256, pBuffer, FALSE)) {
					UCHAR edca_idx = 0;

					for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
						if (i < pAd->ApCfg.BssidNum) {
							edca_idx = os_str_tol(macptr, 0, 10);
							pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.EdcaIdx = edca_idx;
						}
					}
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			/*SSID*/
			if (RTMPGetKeyParameter("SSID", tmpbuf, 256, pBuffer, FALSE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), i++) {
					if (strlen(macptr) <= 32) {
						RTMPSetSTASSID(pAd, &pAd->StaCfg[i].wdev, macptr);
						MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"(SSID=%s)\n", macptr);
					}
				}
			}
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			/*NetworkType*/
			if (RTMPGetKeyParameter("NetworkType", tmpbuf, 25, pBuffer, TRUE)) {
				int ii;

				if (strcmp(tmpbuf, "Adhoc") == 0)
					pAd->StaCfg[0].BssType = BSS_ADHOC;
				else { /*Default Infrastructure mode*/
					for (ii = 0; ii < MAX_MULTI_STA; ii++)
						pAd->StaCfg[ii].BssType = BSS_INFRA;
				}

				/* Reset Ralink supplicant to not use, it will be set to start when UI set PMK key*/
				for (ii = 0; ii < MAX_MULTI_STA; ii++) {
					pAd->StaCfg[ii].WpaState = SS_NOTUSE;
					pAd->StaCfg[ii].bConfigChanged = TRUE;
				}

				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"(NetworkType=%d)\n", pAd->StaCfg[0].BssType);
			}
		}
#endif /* CONFIG_STA_SUPPORT */
		if (RTMPGetKeyParameter("MaxProbeRspTh", tmpbuf, 25, pBuffer, TRUE)) {
			UINT16 max_rsp = (UINT16)os_str_tol(tmpbuf, 0, 10);

			cap->ProbeRspMaxNum = max_rsp;
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
				"MaxProbeRspTh=%d\n", max_rsp);
		}
#ifdef MT_DFS_SUPPORT

		if (RTMPGetKeyParameter("DfsCalibration", tmpbuf, 25, pBuffer, TRUE)) {
			UINT_32 DisableDfsCal = os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.DfsParameter.DisableDfsCal = DisableDfsCal;
		}

		if (RTMPGetKeyParameter("DfsEnable", tmpbuf, 25, pBuffer, TRUE)) {
			UINT_32 DfsEnable = os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.DfsParameter.bDfsEnable = DfsEnable;
		}
#ifdef MT_BAND4_DFS_SUPPORT /*302502*/
		if (RTMPGetKeyParameter("Band4DfsEnable", tmpbuf, 25, pBuffer, TRUE)) {
			UINT_32 Band4DfsEnable = os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.DfsParameter.band4DfsEnable = Band4DfsEnable;
			if (pAd->CommonCfg.DfsParameter.band4DfsEnable) {
				pAd->Dot11_H.CSPeriod = 2;
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_NOTICE,
				"pAd->Dot11_H.CSPeriod:Band4 DFS CSA counter <= 2\n\n");
			}
		}
#endif
#ifdef CONFIG_RCSA_SUPPORT
		if (RTMPGetKeyParameter("DfsUseCsaCfg", tmpbuf, 25, pBuffer, TRUE)) {
			UINT_8 UseCsaCfg = os_str_tol(tmpbuf, 0, 10);

			if (UseCsaCfg > 0)
				pAd->CommonCfg.DfsParameter.fUseCsaCfg = TRUE;
			else
				pAd->CommonCfg.DfsParameter.fUseCsaCfg = FALSE;
		}
		if (RTMPGetKeyParameter("DfsRCSAEn", tmpbuf, 25, pBuffer, TRUE)) {
			UINT_8 RCSAEn = os_str_tol(tmpbuf, 0, 10);

			if (RCSAEn > 0)
				pAd->CommonCfg.DfsParameter.bRCSAEn = TRUE;
			else
				pAd->CommonCfg.DfsParameter.bRCSAEn = FALSE;
		}
#endif
#endif

		/*WirelessMode*/
		/*Note: BssidNum must be put before WirelessMode in dat file*/
		if (RTMPGetKeyParameter("WirelessMode", tmpbuf, 128, pBuffer, TRUE))
			RTMPWirelessModeCfg(pAd, tmpbuf);

#ifdef CONFIG_AP_SUPPORT
		/*EDCCA_Mode*/
		if (RTMPGetKeyParameter("EDCCA_Mode", tmpbuf, 10, pBuffer, TRUE))
			edcca_mode_set_cfg(pAd, tmpbuf);

		/*AutoChannelSelect*/
		if (RTMPGetKeyParameter("AutoChannelSelect", tmpbuf, 10, pBuffer, TRUE))
			auto_ch_select_set_cfg(pAd, tmpbuf);
		if (RTMPGetKeyParameter("ACSScanMode", tmpbuf, 10, pBuffer, TRUE))
			acs_set_scan_mode(pAd, tmpbuf);
		if (RTMPGetKeyParameter("ACSScanDwell", tmpbuf, 10, pBuffer, TRUE))
			acs_set_scan_dwell(pAd, tmpbuf);
		if (RTMPGetKeyParameter("ACSRestoreDwell", tmpbuf, 10, pBuffer, TRUE))
			acs_set_restore_dwell(pAd, tmpbuf);
		if (RTMPGetKeyParameter("ACSPartialScanChNum", tmpbuf, 10, pBuffer, TRUE))
			acs_set_partial_scan_ch_num(pAd, tmpbuf);
		if (RTMPGetKeyParameter("ACSChUtilThr", tmpbuf, 10, pBuffer, TRUE))
			acs_set_ch_util_thr(pAd, tmpbuf);
		if (RTMPGetKeyParameter("ACSStaNumThr", tmpbuf, 10, pBuffer, TRUE))
			acs_set_sta_num_thr(pAd, tmpbuf);
#ifdef CONFIG_6G_SUPPORT
		if (RTMPGetKeyParameter("PSC_ACS", tmpbuf, 10, pBuffer, TRUE))
			auto_ch_select_PSC_cfg(pAd, tmpbuf);
#ifdef CONFIG_6G_AFC_SUPPORT
		if (RTMPGetKeyParameter("AfcDeviceType", tmpbuf, 32, pBuffer, TRUE))
			rtmp_read_afc_device_type_param(pAd, tmpbuf);
		if (RTMPGetKeyParameter("AfcSpBwDup", tmpbuf, 32, pBuffer, TRUE))
			rtmp_read_afc_dup_en_param(pAd, tmpbuf);
		if (RTMPGetKeyParameter("AfcFreqrange", tmpbuf, 128, pBuffer, TRUE))
			rtmp_read_afc_freq_range_param(pAd, tmpbuf);
		if (RTMPGetKeyParameter("ACSAfterAFC", tmpbuf, 32, pBuffer, TRUE))
			rtmp_read_acs_after_afc_param(pAd, tmpbuf);
		if (RTMPGetKeyParameter("AfcSpectrumType", tmpbuf, 32, pBuffer, TRUE))
			rtmp_read_afc_spectrum_type_param(pAd, tmpbuf);
		if (RTMPGetKeyParameter("AfcOpClass131", tmpbuf, 128, pBuffer, TRUE))
			rtmp_read_afc_opclass_131_param(pAd, tmpbuf);
		if (RTMPGetKeyParameter("AfcOpClass132", tmpbuf, 128, pBuffer, TRUE))
			rtmp_read_afc_opclass_132_param(pAd, tmpbuf);
		if (RTMPGetKeyParameter("AfcOpClass133", tmpbuf, 128, pBuffer, TRUE))
			rtmp_read_afc_opclass_133_param(pAd, tmpbuf);
		if (RTMPGetKeyParameter("AfcOpClass134", tmpbuf, 128, pBuffer, TRUE))
			rtmp_read_afc_opclass_134_param(pAd, tmpbuf);
		if (RTMPGetKeyParameter("AfcOpClass135", tmpbuf, 128, pBuffer, TRUE))
			rtmp_read_afc_opclass_135_param(pAd, tmpbuf);
		if (RTMPGetKeyParameter("AfcOpClass136", tmpbuf, 128, pBuffer, TRUE))
			rtmp_read_afc_opclass_136_param(pAd, tmpbuf);
		if (RTMPGetKeyParameter("AfcOpClass137", tmpbuf, 128, pBuffer, TRUE))
			rtmp_read_afc_opclass_137_param(pAd, tmpbuf);
		if (RTMPGetKeyParameter("AfcDGgainOffset", tmpbuf, 128, pBuffer, TRUE))
			rtmp_read_afc_DGgain_offset_param(pAd, tmpbuf);
#endif /*CONFIG_6G_AFC_SUPPORT*/
#endif
#endif/* CONFIG_AP_SUPPORT */

		/* Channel Group */
		if (RTMPGetKeyParameter("ChannelGrp", tmpbuf, 25, pBuffer, TRUE))
			MTSetChGrp(pAd, tmpbuf);
		/*SeamlessCSA*/
		if (RTMPGetKeyParameter("SeamlessCSA", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) != 0)  /*Enable*/
				pAd->CommonCfg.bIEEE80211H = TRUE;
			else /*Disable*/
				pAd->CommonCfg.bIEEE80211H = FALSE;
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"SeamlessCSA=%d\n", pAd->CommonCfg.bIEEE80211H);
		} else
			pAd->CommonCfg.bIEEE80211H = FALSE;

		/*Channel*/
		if (RTMPGetKeyParameter("Channel", tmpbuf, 128, pBuffer, TRUE))
			RTMPChannelCfg(pAd, tmpbuf);

		/* EtherTrafficBand */
		if (RTMPGetKeyParameter("EtherTrafficBand", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->CommonCfg.EtherTrafficBand = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"EtherTrafficBand=%d\n", pAd->CommonCfg.EtherTrafficBand);

			if (pAd->CommonCfg.EtherTrafficBand > EtherTrafficBand5G)
				pAd->CommonCfg.EtherTrafficBand = EtherTrafficBand5G;
		}

		/* Wf_fwd_ */
		if (RTMPGetKeyParameter("WfFwdDisabled", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->CommonCfg.WfFwdDisabled = os_str_tol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"WfFwdDisabled=%d\n", pAd->CommonCfg.WfFwdDisabled);
		}

		/*BasicRate*/
		if (RTMPGetKeyParameter("BasicRate", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->CommonCfg.BasicRateBitmap = (ULONG) os_str_tol(tmpbuf, 0, 10);
			pAd->CommonCfg.BasicRateBitmapOld = (ULONG) os_str_tol(tmpbuf, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BasicRate=%ld\n", pAd->CommonCfg.BasicRateBitmap);
		}

#ifdef GN_MIXMODE_SUPPORT
		/*GNMixMode*/
		if (RTMPGetKeyParameter("GNMixMode", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->CommonCfg.GNMixMode = (ULONG) os_str_tol(tmpbuf, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"GNMixMode=%d\n", pAd->CommonCfg.GNMixMode);
		}
#endif /*GN_MIXMODE_SUPPORT*/

#ifdef DBDC_ONE_BAND_SUPPORT
		/*DbdcBandSupport::0:both band, 1:2.4G, 2:5G*/
		if (RTMPGetKeyParameter("DbdcBandSupport", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->CommonCfg.DbdcBandSupport = (ULONG) os_str_tol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"DbdcBandSupport=%d\n", pAd->CommonCfg.DbdcBandSupport);
		}
#endif /*DBDC_ONE_BAND_SUPPORT*/

		/*BeaconPeriod*/
		if (RTMPGetKeyParameter("BeaconPeriod", tmpbuf, PER_BSS_SIZE_5(pAd), pBuffer, TRUE)) {
			UCHAR i_counter = 0;
			CHAR *remain_buf;
			RTMP_STRING *pa_buf = tmpbuf;
			USHORT bcn_val;

			do {
				if (i_counter >= CFG_WIFI_RAM_BAND_NUM)
					break;
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"BeaconPeriod:tmpbuf=%s\n", pa_buf);
				bcn_val = (USHORT) os_str_tol(pa_buf, (CHAR **)&remain_buf, 10);
				/* The acceptable is 20~1000 ms. Refer to WiFi test plan. */
				if (bcn_val >= 20 && bcn_val <= 1000)
					pAd->CommonCfg.BeaconPeriod = bcn_val;
				else
					pAd->CommonCfg.BeaconPeriod = 100;	/* Default value*/
				pa_buf = &remain_buf[1];
				i_counter++;
			} while (remain_buf[0] == ';');
#ifdef APCLI_CONNECTION_TRIAL
			pAd->CommonCfg.BeaconPeriod = 200;
#endif /* APCLI_CONNECTION_TRIAL */
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"Band %d BeaconPeriod=%d\n", hc_get_hw_band_idx(pAd), pAd->CommonCfg.BeaconPeriod);
		}


#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef CONFIG_DOT11V_WNM
			WNM_ReadParametersFromFile(pAd, tmpbuf, pBuffer);
#endif

			/*DtimPeriod*/
			if (RTMPGetKeyParameter("DtimPeriod", tmpbuf, 128, pBuffer, TRUE)) {
				/* apply per-BSSs setting */
				RTMPDtimPeriodCfg(pAd, tmpbuf);
			}

#ifdef BAND_STEERING
			/* Read BandSteering profile parameters */
			BndStrgSetProfileParam(pAd, tmpbuf, pBuffer);
#endif /* BAND_STEERING */
#ifdef MBSS_AS_WDS_AP_SUPPORT
			if (RTMPGetKeyParameter("WDSEnable", tmpbuf, 50, pBuffer, TRUE)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"WDS=%s\n", tmpbuf);
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < MAX_BEACON_NUM);
					macptr = rstrtok(NULL, ";"), i++) {
					pAd->ApCfg.MBSSID[i].wdev.wds_enable = simple_strtoul(macptr, 0, 10);
				}
			}

			if (RTMPGetKeyParameter("WdsMac", tmpbuf, 50, pBuffer, TRUE)) {
				/*Mac address acceptable format 01:02:03:04:05:06 length 17 */
				if (strlen(tmpbuf) != 17) {
					for (i = 0, value = rstrtok(tmpbuf, ":"); value && (i < MAC_ADDR_LEN);
						value = rstrtok(NULL, ":")) {
						if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
							return FALSE;  /*Invalid */
					AtoH(value, (UCHAR *)&pAd->ApCfg.wds_mac[i++], 1);
						}
				}
			}
#endif
		}
#endif /* CONFIG_AP_SUPPORT */

		/* TxPower */
		if (RTMPGetKeyParameter("TxPower", tmpbuf, 10, pBuffer, TRUE)) {
			long tmp_val = 0;

			/* parameter parsing */
			if (kstrtol(tmpbuf, 10, &tmp_val))
				MTWF_PRINT("%s: read value failed(=%s)\n", __func__, tmpbuf);
			else
				pAd->CommonCfg.ucTxPowerPercentage = tmp_val;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"[TxPower]: %d\n",
				pAd->CommonCfg.ucTxPowerPercentage);
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				pAd->CommonCfg.ucTxPowerDefault =
					pAd->CommonCfg.ucTxPowerPercentage;
			}
#endif /* CONFIG_STA_SUPPORT */
		}

#ifdef TX_POWER_CONTROL_SUPPORT
		/* Power Boost (CCK) */
		if (RTMPGetKeyParameter("PowerUpCck", tmpbuf, 32,
					pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_cck); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_cck)
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"[PowerUpCck] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0.cPowerUpCck[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1.cPowerUpCck[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
		}

		/* Power Boost (OFDM) */
		if (RTMPGetKeyParameter("PowerUpOfdm", tmpbuf, 32,
					pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_ofdm); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_ofdm)
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"[PowerUpOfdm] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0.cPowerUpOfdm[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1.cPowerUpOfdm[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
		}

		/* Power Boost (HT20) */
		if (RTMPGetKeyParameter("PowerUpHT20", tmpbuf, 32,
					pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_ht20); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_ht20)
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"[PowerUpHT20] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0.cPowerUpHt20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1.cPowerUpHt20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
		}

		/* Power Boost (HT40) */
		if (RTMPGetKeyParameter("PowerUpHT40", tmpbuf, 32,
					pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_ht40); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_ht40)
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"[PowerUpHT40] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0.cPowerUpHt40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1.cPowerUpHt40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
		}

		/* Power Boost (VHT20) */
		if (RTMPGetKeyParameter("PowerUpVHT20", tmpbuf, 32,
					pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_vht20); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht20)
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"[PowerUpVHT20] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0.cPowerUpVht20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1.cPowerUpVht20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
		}

		/* Power Boost (VHT40) */
		if (RTMPGetKeyParameter("PowerUpVHT40", tmpbuf, 32,
					pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_vht40); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht40)
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"[PowerUpVHT40] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0.cPowerUpVht40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1.cPowerUpVht40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
		}

		/* Power Boost (VHT80) */
		if (RTMPGetKeyParameter("PowerUpVHT80", tmpbuf, 32,
					pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_vht80); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht80)
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"[PowerUpVHT80] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0.cPowerUpVht80[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1.cPowerUpVht80[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
		}

		/* Power Boost (VHT160) */
		if (RTMPGetKeyParameter("PowerUpVHT160", tmpbuf, 32,
					pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_vht160); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht160)
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"[PowerUpVHT160] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0.cPowerUpVht160[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1.cPowerUpVht160[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
		}
#endif /* TX_POWER_CONTROL_SUPPORT */

#if defined(TX_POWER_CONTROL_SUPPORT) || defined(TX_POWER_CONTROL_SUPPORT_V2)
		{
			struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

			if (arch_ops && arch_ops->arch_txpower_boost_profile)
				arch_ops->arch_txpower_boost_profile(pAd, tmpbuf, pBuffer);
		}
#endif

#ifdef SINGLE_SKU_V2

		/* TxPower SKU */
		if (RTMPGetKeyParameter("SKUenable", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			pAd->CommonCfg.SKUenable = simple_strtol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"[SKUenable] BAND%d: %d\n",
				hc_get_hw_band_idx(pAd), pAd->CommonCfg.SKUenable);
		}

#endif /*SINGLE_SKU_V2 */

		/* TxPower SKU DUP PATCH*/
		if (RTMPGetKeyParameter("SkuDupPatch", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			pAd->CommonCfg.SKU_DUP_Patch_enable = os_str_tol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"[SkuTableIdx]: %d\n", pAd->CommonCfg.SKUTableIdx);
		}

		/* TxPower SKU */
		if (RTMPGetKeyParameter("SkuTableIdx", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			pAd->CommonCfg.SKUTableIdx = simple_strtol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"[SkuTableIdx]: %d\n", pAd->CommonCfg.SKUTableIdx);
		}

		/* CCKTxStream */
		if (RTMPGetKeyParameter("CCKTxStream", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			pAd->CommonCfg.CCKTxStream = simple_strtol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"[CCKTxStream] BAND%d: %d\n",
				hc_get_hw_band_idx(pAd), pAd->CommonCfg.CCKTxStream);
		}

		/* TxPower Percentage */
		if (RTMPGetKeyParameter("PERCENTAGEenable", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			pAd->CommonCfg.PERCENTAGEenable = simple_strtol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"[PERCENTAGEenable] BAND%d: %d\n",
				hc_get_hw_band_idx(pAd), pAd->CommonCfg.PERCENTAGEenable);
		}

		/* TxPower BF Backoff */
		if (RTMPGetKeyParameter("BFBACKOFFenable", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			pAd->CommonCfg.BFBACKOFFenable = simple_strtol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"[BFBACKOFFenable] BAND%d: %d\n",
				hc_get_hw_band_idx(pAd), pAd->CommonCfg.BFBACKOFFenable);
		}

#ifdef CFG_SUPPORT_FALCON_MURU
		if (RTMPGetKeyParameter("Disable160RuMu", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			pAd->CommonCfg.Dis160RuMu = simple_strtoul(tmpbuf, 0, 16);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"[Disable160RuMu] BAND%d: %d\n",
				hc_get_hw_band_idx(pAd), pAd->CommonCfg.Dis160RuMu);
		}

		if (RTMPGetKeyParameter("MaxRuOfdma", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			pAd->CommonCfg.MaxRuOfdma = simple_strtol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"[MaxRuOfdma] BAND0: %d\n", pAd->CommonCfg.MaxRuOfdma);
		}

		if (RTMPGetKeyParameter("MaxDLMuMimo", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			pAd->CommonCfg.MaxDLMuMimo = simple_strtol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"[MaxDLMuMimo] BAND0: %d\n", pAd->CommonCfg.MaxDLMuMimo);
		}

		if (RTMPGetKeyParameter("MaxULMuMimo", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			pAd->CommonCfg.MaxULMuMimo = simple_strtol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"[MaxULMuMimo] BAND0: %d\n", pAd->CommonCfg.MaxULMuMimo);
		}
#endif /* CFG_SUPPORT_FALCON_MURU */

#ifdef LINK_TEST_SUPPORT

		/* Link Test Support */
		if (RTMPGetKeyParameter("LinkTestSupport", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			pAd->CommonCfg.LinkTestSupportTemp = simple_strtol(tmpbuf, 0, 10);

			/* LinkTestSupport can be enabled by any profile */

			if (pAd->CommonCfg.LinkTestSupportTemp)
				pAd->CommonCfg.LinkTestSupport = TRUE;
			else
				pAd->CommonCfg.LinkTestSupport = FALSE;

			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"LinkTestSupport = %d\n", pAd->CommonCfg.LinkTestSupport);
		}

#endif /* LINK_TEST_SUPPORT */
#ifdef RLM_CAL_CACHE_SUPPORT

		/* Calibration Cache Support */
		if (RTMPGetKeyParameter("CalCacheApply", tmpbuf, 32, pBuffer, TRUE)) {
			pAd->CommonCfg.CalCacheApply = (ULONG) simple_strtol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"CalCacheApply = %d\n", pAd->CommonCfg.CalCacheApply);
		}

#endif /* RLM_CAL_CACHE_SUPPORT */

		/*BGProtection*/
		if (RTMPGetKeyParameter("BGProtection", tmpbuf, 10, pBuffer, TRUE)) {
			/*#if 0	#ifndef WIFI_TEST*/
			/*		pAd->CommonCfg.UseBGProtection = 2; disable b/g protection for throughput test*/
			/*#else*/
			switch (os_str_tol(tmpbuf, 0, 10)) {
			case 1: /*Always On*/
				pAd->CommonCfg.UseBGProtection = 1;
				break;

			case 2: /*Always OFF*/
				pAd->CommonCfg.UseBGProtection = 2;
				break;

			case 0: /*AUTO*/
			default:
				pAd->CommonCfg.UseBGProtection = 0;
				break;
			}

			/*#endif*/
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BGProtection=%ld\n", pAd->CommonCfg.UseBGProtection);
		}

#ifdef CONFIG_AP_SUPPORT

		/*OLBCDetection*/
		if (RTMPGetKeyParameter("DisableOLBC", tmpbuf, 10, pBuffer, TRUE)) {
			switch (os_str_tol(tmpbuf, 0, 10)) {
			case 1: /*disable OLBC Detection*/
				pAd->CommonCfg.DisableOLBCDetect = 1;
				break;

			case 0: /*enable OLBC Detection*/
				pAd->CommonCfg.DisableOLBCDetect = 0;
				break;

			default:
				pAd->CommonCfg.DisableOLBCDetect = 0;
				break;
			}

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"OLBCDetection=%ld\n", pAd->CommonCfg.DisableOLBCDetect);
		}

#endif /* CONFIG_AP_SUPPORT */

		/*TxPreamble*/
		if (RTMPGetKeyParameter("TxPreamble", tmpbuf, 10, pBuffer, TRUE)) {
			switch (os_str_tol(tmpbuf, 0, 10)) {
			case Rt802_11PreambleShort:
				pAd->CommonCfg.TxPreamble = Rt802_11PreambleShort;
				break;

			case Rt802_11PreambleAuto:
				pAd->CommonCfg.TxPreamble = Rt802_11PreambleAuto;
				break;

			case Rt802_11PreambleLong:
			default:
				pAd->CommonCfg.TxPreamble = Rt802_11PreambleLong;
				break;
			}

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"TxPreamble=%ld\n", pAd->CommonCfg.TxPreamble);
		}

		/*RTSPktThreshold*/
		read_rts_pkt_thld_from_file(pAd, tmpbuf, pBuffer);
		/*RTSThreshold*/
		read_rts_len_thld_from_file(pAd, tmpbuf, pBuffer);
		/*FragThreshold*/
		read_frag_thld_from_file(pAd, tmpbuf, pBuffer);

#ifdef VLAN_SUPPORT
		/*VLANTag*/
#ifdef CONFIG_AP_SUPPORT
		readProfileVlanTagWdev(pAd, tmpbuf, pBuffer);
		readProfileVlanIDWdev(pAd, tmpbuf, pBuffer);
		readProfileVlanPriorityWdev(pAd, tmpbuf, pBuffer);
		readProfileVlanTxPolicyWdev(pAd, tmpbuf, pBuffer);
		readProfileVlanRxPolicyWdev(pAd, tmpbuf, pBuffer);
		if (RTMPGetKeyParameter("VLANEn", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) != 0)
				pAd->CommonCfg.bEnableVlan = TRUE;
			else
				pAd->CommonCfg.bEnableVlan = FALSE;
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"VLANEn=%d\n", pAd->CommonCfg.bEnableVlan);
		}
#endif /*CONFIG_AP_SUPPORT*/
#endif /*VLAN_SUPPORT*/

		/*TxBurst*/
		if (RTMPGetKeyParameter("TxBurst", tmpbuf, 10, pBuffer, TRUE)) {
			/*#ifdef WIFI_TEST*/
			/*						pAd->CommonCfg.bEnableTxBurst = FALSE;*/
			/*#else*/
			if (os_str_tol(tmpbuf, 0, 10) != 0) /*Enable*/
				pAd->CommonCfg.bEnableTxBurst = TRUE;
			else /*Disable*/
				pAd->CommonCfg.bEnableTxBurst = FALSE;

			/*#endif*/
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"TxBurst=%d\n", pAd->CommonCfg.bEnableTxBurst);
		}
		/* WmmCapable*/
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		rtmp_read_ap_wmm_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			rtmp_read_sta_wmm_parms_from_file(pAd, tmpbuf, pBuffer);
#ifdef XLINK_SUPPORT
			rtmp_get_psp_xlink_mode_from_file(pAd, tmpbuf, pBuffer);
#endif /* XLINK_SUPPORT */
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/* RejectMgmtRx */
			if (RTMPGetKeyParameter("RejectMgmtRx", tmpbuf, 25, pBuffer, TRUE)) {
				if (kstrtol(tmpbuf, 10, &readvalue) == 0) {
					if (readvalue >= MGMT_RX_ACTION_MAX)
						MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_DEBUG,
							"RejectMgmtRx: Invalid parameter(=%ld)\n",
							readvalue);
					else {
						pAd->ApCfg.reject_mgmt_rx = (u8)readvalue;
						MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"Read reject_mgmt_rx(=%d)\n",
							pAd->ApCfg.reject_mgmt_rx);
					}
				} else
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_DEBUG,
						"Read RejectMgmtRx(=%s) failed\n", tmpbuf);

			}
			/* BandMaxStaNum*/
			if (RTMPGetKeyParameter("BandMaxStaNum", tmpbuf, 10, pBuffer, TRUE)) {
				UINT16 tmpBandMaxStaNum = (UINT16)os_str_tol(tmpbuf, 0, 10);

				if (tmpBandMaxStaNum > 0) {
					pAd->ApCfg.BandMaxStaNum = (UINT16)tmpBandMaxStaNum;
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"%s::(BandMaxStaNum=%d)\n", __func__, pAd->ApCfg.BandMaxStaNum);
				}
			}

			/* MbssMaxStaNum*/
			if (RTMPGetKeyParameter("MbssMaxStaNum", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					ApCfg_Set_PerMbssMaxStaNum_Proc(pAd, i, macptr);
				}
			}

			/* BssMaxIdleEn */
			if (RTMPGetKeyParameter("BssMaxIdleEn", tmpbuf, 128, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					if (os_str_tol(macptr, 0, 10) != 0) /*Enable*/
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].max_idle_ie_en = TRUE;
					else /*Disable*/
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].max_idle_ie_en = FALSE;

					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"I/F(ra%d) BssMaxIdleEn=%d\n",
							i, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].max_idle_ie_en);
				}
			}

			/* BssMaxIdle */
			if (RTMPGetKeyParameter("BssMaxIdle", tmpbuf, 128, pBuffer, TRUE)) {
				UINT16 idle_period;

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					idle_period = (USHORT) os_str_tol(macptr, 0, 10);
					if ((idle_period < 1) || (idle_period > 0xffff))
						idle_period = MAC_TABLE_AGEOUT_TIME;

					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].max_idle_period = idle_period;

					/* Apply BSS[0] setting to all as default */
					for (j = 1; (i == 0) && (j < pAd->ApCfg.BssidNum); j++)
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, j)].max_idle_period = idle_period;
				}
			}

			/*NoForwarding*/
			if (RTMPGetKeyParameter("NoForwarding", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].IsolateInterStaTraffic = FALSE;

					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"I/F(ra%d) NoForwarding=%ld\n", i,
							 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].IsolateInterStaTraffic);
				}
			}

			/*no_bcn*/
			if (RTMPGetKeyParameter("no_bcn", tmpbuf, 32, pBuffer, TRUE)) {
				struct wifi_dev *wdev = NULL;

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;
					/*Disable Tx beacon when interface up*/
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
					if (os_str_tol(macptr, 0, 10) != 0)
						wdev->bcn_buf.stop_tx |= STOP_BCN_TX_NO_BCN;
					else /*Enable beacon default*/
						wdev->bcn_buf.stop_tx = STOP_BCN_TX_DISABLE;
				}
			}

			/*NoForwardingBTNBSSID*/
			if (RTMPGetKeyParameter("NoForwardingBTNBSSID", tmpbuf, 10, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) != 0) /*Enable*/
					pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = FALSE;
				else /*Disable*/
					pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = FALSE;

				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"NoForwardingBTNBSSID=%ld\n",
						 pAd->ApCfg.IsolateInterStaTrafficBTNBSSID);
			}

			/*HideSSID*/
			if (RTMPGetKeyParameter("HideSSID", tmpbuf, 100, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					int apidx = i;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					if (os_str_tol(macptr, 0, 10) != 0) { /*Enable*/
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].bHideSsid = TRUE;
#ifdef WSC_V2_SUPPORT
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.WscControl.WscV2Info.bWpsEnable = FALSE;
#endif /* WSC_V2_SUPPORT */
					} else /*Disable*/
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].bHideSsid = FALSE;

					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"I/F(ra%d) HideSSID=%d\n", i,
							 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].bHideSsid);
				}
			}

			/*StationKeepAlive*/
			if (RTMPGetKeyParameter("StationKeepAlive", tmpbuf, PER_BSS_SIZE_2(pAd), pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					int apidx = i;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].StationKeepAliveTime = os_str_tol(macptr, 0, 10);
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"I/F(ra%d) StationKeepAliveTime=%d\n", i,
							 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].StationKeepAliveTime);
				}
			}

			/*AutoChannelSkipList*/
			if (RTMPGetKeyParameter("AutoChannelSkipList", tmpbuf, 128, pBuffer, FALSE)) {
				pAd->ApCfg.AutoChannelSkipListNum = delimitcnt(tmpbuf, ";") + 1;
				if (delimitcnt(tmpbuf, ";"))
					pAd->ApCfg.AutoChannelSkipListNum =
						delimitcnt(tmpbuf, ";") + 1;
				else
					pAd->ApCfg.AutoChannelSkipListNum = 0;

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i < pAd->ApCfg.AutoChannelSkipListNum) {
						pAd->ApCfg.AutoChannelSkipList[i] = os_str_tol(macptr, 0, 10);
						MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							" AutoChannelSkipList[%d]= %d\n", i,
							pAd->ApCfg.AutoChannelSkipList[i]);
					} else
						break;
				}
#ifdef CONFIG_6G_AFC_SUPPORT
				afc_save_autochannel_skip_init(pAd);
#endif /*CONFIG_6G_AFC_SUPPORT*/
			}

#ifdef BACKGROUND_SCAN_SUPPORT

			if (RTMPGetKeyParameter("DfsZeroWait", tmpbuf, 50, pBuffer, FALSE)) {
				UINT8 DfsZeroWait = os_str_tol(tmpbuf, 0, 10);

				if ((DfsZeroWait == 1)
#ifdef MT_DFS_SUPPORT
					&& IS_SUPPORT_MT_DFS(pAd)
#endif
				   ) {
					pAd->BgndScanCtrl.DfsZeroWaitSupport = TRUE;/*Enable*/
#ifdef MT_DFS_SUPPORT
					UPDATE_MT_ZEROWAIT_DFS_Support(pAd, TRUE);
#endif
				} else {
					pAd->BgndScanCtrl.DfsZeroWaitSupport = FALSE;
#ifdef MT_DFS_SUPPORT
					UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
#endif
				}

				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"DfsZeroWait Support=%d/%d\n", DfsZeroWait,
					pAd->BgndScanCtrl.DfsZeroWaitSupport);
			}

#ifdef MT_DFS_SUPPORT
			if (RTMPGetKeyParameter("DfsDedicatedZeroWait", tmpbuf, 25, pBuffer, TRUE)) {
				UCHAR DfsDedicatedZeroWait = (UCHAR) simple_strtol(tmpbuf, 0, 10);


#if ((RDD_PROJECT_TYPE_2 == 1) || (RDD_2_SUPPORTED == 1))
				switch (DfsDedicatedZeroWait) {
				case DFS_DEDICATED_ZERO_WAIT_DISABLED:
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
					break;

				case DFS_DEDICATED_ZERO_WAIT_ENABLED:
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
					break;

				case DFS_DEDICATED_ZERO_WAIT_DEFAULT_FLOW_ENABLED:
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = TRUE;
					break;

				case DFS_DEDICATED_ZERO_WAIT_DNMC:
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
					pAd->CommonCfg.DfsParameter.ucDisTm = TRUE;
					break;

				case DFS_DEDICATED_ZERO_WAIT_BW80_ENABLE:
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
					pAd->CommonCfg.DfsParameter.BW80DedicatedZWSupport = TRUE;
					break;

				case DFS_SWBASED_ZERO_WAIT_BW160_ENABLE:
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = TRUE;
					break;

				default:
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
					break;
				}

#else
				if (!pAd->CommonCfg.dbdc_mode) {
					switch (DfsDedicatedZeroWait) {
					case DFS_DEDICATED_ZERO_WAIT_DISABLED:
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
						break;

					case DFS_DEDICATED_ZERO_WAIT_ENABLED:
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
						break;

					case DFS_DEDICATED_ZERO_WAIT_DEFAULT_FLOW_ENABLED:
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = TRUE;
						break;

					case DFS_SWBASED_ZERO_WAIT_BW160_ENABLE:
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = TRUE;
						break;

					default:
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
						break;
					}
				} else {
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
				}

#endif
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
				if (DfsDedicatedZeroWait == 4)
					pAd->CommonCfg.DfsParameter.bDFSV10Support = TRUE;
				else
					pAd->CommonCfg.DfsParameter.bDFSV10Support = FALSE;
#endif
#ifdef DFS_ADJ_BW_ZERO_WAIT
				if ((DfsDedicatedZeroWait == DFS_SWBASED_ZERO_WAIT_BW160_ENABLE) &&
					(pAd->CommonCfg.DfsParameter.bDfsEnable))
					pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport = TRUE;
				else
					pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport = FALSE;
#endif
			}
			if (RTMPGetKeyParameter("DfsZeroWaitDefault", tmpbuf, 25, pBuffer, TRUE)) {
				UCHAR DfsZeroWaitDefault = (UCHAR) simple_strtol(tmpbuf, 0, 10);
				pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = DfsZeroWaitDefault;
			}

			if (RTMPGetKeyParameter("DfsZeroWaitBandidx", tmpbuf, 25, pBuffer, TRUE)) {
				switch (os_str_tol(tmpbuf, 0, 10)) {
				case 1:
					pAd->CommonCfg.DfsParameter.ZeroWaitBandidx = 1;
					break;
				case 2:
					pAd->CommonCfg.DfsParameter.ZeroWaitBandidx = 2;
					break;
				default:
					pAd->CommonCfg.DfsParameter.ZeroWaitBandidx = 1;
					break;
				}
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"DfsZeroWaitBandidx=%d\n",
					pAd->CommonCfg.DfsParameter.ZeroWaitBandidx);
			}

			if (RTMPGetKeyParameter("DfsTargetCh", tmpbuf, 25, pBuffer, TRUE)) {
				UCHAR Channel = 0, Bw = 0;
				USHORT CacValue = 0;
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < 3);
					macptr = rstrtok(NULL, ";"), i++) {
					switch (i) {
					case 0:
						Channel = simple_strtol(macptr, 0, 10);
						break;
					case 1:
						Bw = simple_strtol(macptr, 0, 10);
						break;
					case 2:
						CacValue = simple_strtol(macptr, 0, 10);
						break;
					default:
						break;
					}
				}
				ZeroWait_DFS_Pre_Assign_Next_Target_Channel(pAd, Channel, Bw, CacValue);
			}

			/*CACMemoEn*/
			if (RTMPGetKeyParameter("CACMemoEn", tmpbuf, 10, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10))
					pAd->CommonCfg.DfsParameter.CACMemoEn = TRUE;
				else
					pAd->CommonCfg.DfsParameter.CACMemoEn = FALSE;

				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_NOTICE,
					"CACMemoEn=%d\n",
					pAd->CommonCfg.DfsParameter.CACMemoEn);
			}

			/* DFS Pre CAC */
			if (RTMPGetKeyParameter("DfsPreCac", tmpbuf, 10, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10))
					pAd->CommonCfg.DfsParameter.bPreCacEn = TRUE;
				else
					pAd->CommonCfg.DfsParameter.bPreCacEn = FALSE;

				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_NOTICE,
					"DfsPreCac=%d\n",
					pAd->CommonCfg.DfsParameter.bPreCacEn);
			}

			if (RTMPGetKeyParameter("DfsChSelPrefer", tmpbuf, 25, pBuffer, TRUE)) {
				if (kstrtol(tmpbuf, 10, &readvalue) == 0) {
					if ((UCHAR) readvalue >= RadarDetectSelectNum)
						MTWF_PRINT("Invalid parameter!\n");
					else
						pAd->CommonCfg.DfsParameter.DfsChSelPrefer = (UCHAR) readvalue;
				} else
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"Read DfsChSelPrefer(=%s) failed\n", tmpbuf);

			}

			if (RTMPGetKeyParameter("DfsNopExpireSetChPolicy", tmpbuf, 30, pBuffer, TRUE)) {
				if (kstrtol(tmpbuf, 10, &readvalue) == 0) {
					if ((UCHAR) readvalue >= DFS_NOP_EXPIRE_SET_CH_POLICY_NUM)
						MTWF_PRINT("Invalid parameter!\n");
					else
						pAd->CommonCfg.DfsParameter.DfsNopExpireSetChPolicy = (UCHAR) readvalue;
				} else
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"Read DfsNopExpireSetChPolicy(=%s) failed\n", tmpbuf);
			}

			if (RTMPGetKeyParameter("DfsDedicatedRxPreselectCh", tmpbuf, 25, pBuffer, TRUE)) {
				if (kstrtol(tmpbuf, 10, &readvalue) == 0) {
					if ((UCHAR) readvalue >= DFS_PRE_SELECT_CH_NUM)
						MTWF_PRINT("Invalid parameter!\n");
					else
						pAd->CommonCfg.DfsParameter.bDfsDedicatedRxPreselectCh = (BOOLEAN) readvalue;
				} else
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"Read DfsDedicatedRxPreselectCh(=%s) failed\n", tmpbuf);
			}
#ifdef DFS_SDB_SUPPORT
			if (RTMPGetKeyParameter("DfsSdbEnable", tmpbuf, 25, pBuffer, TRUE)) {
				if (kstrtol(tmpbuf, 10, &readvalue) == 0) {
					pAd->CommonCfg.DfsParameter.bDfsSdbEnable = (BOOLEAN) readvalue;
				} else
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"Read DfsSdbEnable(=%s) failed\n", tmpbuf);
			}
#endif /* DFS_SDB_SUPPORT */

#endif
			if (RTMPGetKeyParameter("BgndScanSkipCh", tmpbuf, 50, pBuffer, FALSE)) {
				pAd->BgndScanCtrl.SkipChannelNum = delimitcnt(tmpbuf, ";") + 1;

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i < pAd->BgndScanCtrl.SkipChannelNum) {
						pAd->BgndScanCtrl.SkipChannelList[i] = os_str_tol(macptr, 0, 10);
						MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"Background Skip Channel list[%d]= %d\n", i,
							pAd->BgndScanCtrl.SkipChannelList[i]);
					} else
						break;
				}
			}

#endif /* BACKGROUND_SCAN_SUPPORT */

			if (RTMPGetKeyParameter("EDCCACfgMode", tmpbuf, 10, pBuffer, FALSE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < CFG_WIFI_RAM_BAND_NUM);
					macptr = rstrtok(NULL, ";"), i++)
					if (kstrtol(macptr, 10, &readvalue) == 0)
						pAd->CommonCfg.u1EDCCACfgMode = (UCHAR) readvalue;
			}

			if (RTMPGetKeyParameter("EDCCAEnable", tmpbuf, 10, pBuffer, FALSE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i < CFG_WIFI_RAM_BAND_NUM) {

						pAd->CommonCfg.u1EDCCACtrl = simple_strtol(macptr, 0, 10);
						MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							" EDCCA band[%d]= %d by profile\n", i, pAd->CommonCfg.u1EDCCACtrl);

					} else
						break;
				}
			}

			if (RTMPGetKeyParameter("EDCCAThreshold", tmpbuf, 32, pBuffer, FALSE)) {
				u_long tmp_thresh = 0;

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr != NULL && i < EDCCA_MAX_BW_NUM; macptr = rstrtok(NULL, ";"), i++) {
					pAd->CommonCfg.u1EDCCAThreshold[i] = 0x7f;
					ret = kstrtol(macptr, 0, &tmp_thresh);
					if (ret == 0)
						pAd->CommonCfg.u1EDCCAThreshold[i] = tmp_thresh;
					if (pAd->CommonCfg.u1EDCCAThreshold[i] > 0 && pAd->CommonCfg.u1EDCCAThreshold[i] < 0x7f)
						pAd->CommonCfg.u1EDCCAThreshold[i] = ~pAd->CommonCfg.u1EDCCAThreshold[i] + 1;
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						" EDCCAthreshold= %d by profile\n", pAd->CommonCfg.u1EDCCAThreshold[i]);
				}
			}

#ifdef MT_DFS_SUPPORT

			if (RTMPGetKeyParameter("DfsZeroWaitCacTime", tmpbuf, 50, pBuffer, FALSE)) {
				UINT8 OffChnlCacTime = os_str_tol(tmpbuf, 0, 10);

				pAd->CommonCfg.DfsParameter.DfsZeroWaitCacTime = OffChnlCacTime; /* Unit is minute */
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"DfsZeroWaitCacTime=%d/%d\n",
					OffChnlCacTime,
					pAd->CommonCfg.DfsParameter.DfsZeroWaitCacTime);
			}
			if (RTMPGetKeyParameter("OcacEnable", tmpbuf, 10, pBuffer, FALSE)) {
				switch (os_str_tol(tmpbuf, 0, 10)) {
				case 0: /*Off Channel CAC Disable*/
					pAd->CommonCfg.DfsParameter.bOcacEnable = 0;
					break;
				case 1: /*Off Channel CAC Non-Channel Switch*/
					pAd->CommonCfg.DfsParameter.bOcacEnable = 1;
					break;
				case 2: /*Off Channel CAC Channel Switch*/
					pAd->CommonCfg.DfsParameter.bOcacEnable = 2;
					break;
				default:
					pAd->CommonCfg.DfsParameter.bOcacEnable = 0;
					break;
				}
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"OcacEnable=%d\n",
					pAd->CommonCfg.DfsParameter.bOcacEnable);
			}

#endif /* MT_DFS_SUPPORT  */
#ifdef AP_SCAN_SUPPORT

			/*ACSCheckTime*/
			if (RTMPGetKeyParameter("ACSCheckTime", tmpbuf, 32, pBuffer, TRUE)) {
				UINT8 i = 0;
				UINT32 time = 0;
				RTMP_STRING *ptr;

				for (i = 0, ptr = rstrtok(tmpbuf, ";"); ptr; ptr = rstrtok(NULL, ";"), i++) {
					if (i >= CFG_WIFI_RAM_BAND_NUM)
						break;

					time = simple_strtol(ptr, 0, 10);
					pAd->ApCfg.AcsCfg.ACSCheckTime = time;
				}

				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_NOTICE,
					"ACSCheckTime[%d]=%u seconds\n",
					hc_get_hw_band_idx(pAd), pAd->ApCfg.AcsCfg.ACSCheckTime);
			}
			/*ACSCheckMinTime*/
			if (RTMPGetKeyParameter("ACSCheckMinTime", tmpbuf, 32, pBuffer, TRUE)) {
				UINT8 i = 0;
				UINT32 time = 0;
				RTMP_STRING *ptr;

				for (i = 0, ptr = rstrtok(tmpbuf, ";"); ptr; ptr = rstrtok(NULL, ";"), i++) {
					if (i >= CFG_WIFI_RAM_BAND_NUM)
						break;

					time = simple_strtol(ptr, 0, 10);
					time = time * 60;/* minute to second */
					pAd->ApCfg.AcsCfg.ACSCheckTime = time;
				}

				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_NOTICE,
					"ACSCheckTime[%d]=%u seconds\n",
					hc_get_hw_band_idx(pAd), pAd->ApCfg.AcsCfg.ACSCheckTime);
			}
#endif /* AP_SCAN_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

		/*ShortSlot*/
		if (RTMPGetKeyParameter("ShortSlot", tmpbuf, 10, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				UCHAR ShortSlot = 0;
				INT status = TRUE;
				if (i < 2) {
					status = RT_CfgSetShortSlot(pAd, macptr, i);
					if (status) {
						ShortSlot = os_str_tol(macptr, 0, 10);
						MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"band%d ShortSlot=%d\n", i, ShortSlot);
					} else
						MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"band%d wrong ShortSlot\n", i);
				} else
					break;
			}

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"ShortSlot=%d\n", TRUE);
		}
#ifdef CONFIG_AP_SUPPORT
		if (RTMPGetKeyParameter("SlotTime", tmpbuf, 10, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				UCHAR SlotTime = 0;
				struct wifi_dev *wdev = NULL;
				UCHAR BandIdx;

				if (i >= pAd->ApCfg.BssidNum)
					break;

				wdev = &pAd->ApCfg.MBSSID[i].wdev;
				BandIdx = HcGetBandByWdev(wdev);
				SlotTime = os_str_tol(macptr, 0, 10);
				if ((SlotTime < 9) || (SlotTime > 25))
					SlotTime = 9;

				pAd->CommonCfg.bUseShortSlotTime = (SlotTime == 9) ? TRUE : FALSE;
				pAd->CommonCfg.SlotTime = SlotTime;

				if (WMODE_CAP_5G(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode)) {
					pAd->CommonCfg.bUseShortSlotTime = TRUE;
					pAd->CommonCfg.SlotTime = 9;
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef TXBF_SUPPORT

		if (cap->FlgHwTxBfCap) {
			/* Set ETxBfEnCond to wdev->wpf_cfg->phy_conf.ETxBfEnCond and pAd->CommonCfg.ETxBfEnCond */
			read_txbf_param_from_file(pAd, tmpbuf, pBuffer);

#if defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT)
			/* ITxBfTimeout */
			if (RTMPGetKeyParameter("ITxBfTimeout", tmpbuf, 32, pBuffer, TRUE)) {
				pAd->CommonCfg.ITxBfTimeout = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"ITxBfTimeout = %ld\n", pAd->CommonCfg.ITxBfTimeout);
			}
#endif /* defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT) */

			/* ETxBfEnCond */
			if (RTMPGetKeyParameter("ETxBfEnCond", tmpbuf, PER_BSS_SIZE_2(pAd), pBuffer, TRUE)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"CommnCfg.ETxBfEnCond = %ld\n", pAd->CommonCfg.ETxBfEnCond);

				if (pAd->CommonCfg.ETxBfEnCond)
					pAd->CommonCfg.RegTransmitSetting.field.TxBF = TRUE;
				else
					pAd->CommonCfg.RegTransmitSetting.field.TxBF = FALSE;

				/* MUTxRxEnable*/
				if (RTMPGetKeyParameter("MUTxRxEnable", tmpbuf, 32, pBuffer, TRUE)) {
					pAd->CommonCfg.MUTxRxEnable = os_str_tol(tmpbuf, 0, 10);
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"MUTxRxEnable = %ld\n", pAd->CommonCfg.MUTxRxEnable);
				}
			}

#ifdef CFG_SUPPORT_MU_MIMO_RA
				if (RTMPGetKeyParameter("MuEnable7615HwPatch", tmpbuf, 10, pBuffer, TRUE)) {
					UINT8 IsEnable = FALSE;

					IsEnable = simple_strtol(tmpbuf, 0, 10);
					pAd->MuHwSwPatch = IsEnable;
				}
#endif

			/* ETxBfTimeout*/
			if (RTMPGetKeyParameter("ETxBfTimeout", tmpbuf, 32, pBuffer, TRUE)) {
				pAd->CommonCfg.ETxBfTimeout = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"ETxBfTimeout = %ld\n", pAd->CommonCfg.ETxBfTimeout);
			}

			/* ETxBfNoncompress*/
			if (RTMPGetKeyParameter("ETxBfNoncompress", tmpbuf, 32, pBuffer, TRUE)) {
				pAd->CommonCfg.ETxBfNoncompress = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"ETxBfNoncompress = %d\n", pAd->CommonCfg.ETxBfNoncompress);
			}

			/* ETxBfIncapable */
			if (RTMPGetKeyParameter("ETxBfIncapable", tmpbuf, 32, pBuffer, TRUE)) {
				pAd->CommonCfg.ETxBfIncapable = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"ETxBfIncapable = %d\n", pAd->CommonCfg.ETxBfIncapable);
			}

			/* MaxBfeeSS */
			if (RTMPGetKeyParameter("MaxBfeeSS", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->CommonCfg.MaxBfeeSS = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"MaxBfeeSS = %d\n", pAd->CommonCfg.MaxBfeeSS);
			}

			/* BfNDPARate */
			if (RTMPGetKeyParameter("BfNDPARate", tmpbuf, 10, pBuffer, TRUE)) {
				if (strcmp(tmpbuf, "OFDM_6M") == 0)
					pAd->CommonCfg.BfNDPARate = 75;
				else if (strcmp(tmpbuf, "OFDM_9M") == 0)
					pAd->CommonCfg.BfNDPARate = 79;
				else if (strcmp(tmpbuf, "OFDM_12M") == 0)
					pAd->CommonCfg.BfNDPARate = 74;
				else if (strcmp(tmpbuf, "OFDM_18M") == 0)
					pAd->CommonCfg.BfNDPARate = 78;
				else if (strcmp(tmpbuf, "OFDM_24M") == 0)
					pAd->CommonCfg.BfNDPARate = 73;
				else if (strcmp(tmpbuf, "OFDM_36M") == 0)
					pAd->CommonCfg.BfNDPARate = 77;
				else if (strcmp(tmpbuf, "OFDM_48M") == 0)
					pAd->CommonCfg.BfNDPARate = 72;
				else if (strcmp(tmpbuf, "OFDM_54M") == 0)
					pAd->CommonCfg.BfNDPARate = 76;
				else
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"BfNDPARate %s invalid\n", tmpbuf);

				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"BfNDPARate = %d\n", pAd->CommonCfg.BfNDPARate);
			}
		}

#endif /* TXBF_SUPPORT */

#ifdef DOT11_HE_AX
	read_mu_param_from_file(pAd, tmpbuf, pBuffer);
	read_he_dyn_smps_from_file(pAd, tmpbuf, pBuffer);
	read_ul_mu_data_param_from_file(pAd, tmpbuf, pBuffer);
	read_er_su_param_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT11_HE_AX */

		/*IEEE80211H*/
		if (RTMPGetKeyParameter("IEEE80211H", tmpbuf, 10, pBuffer, TRUE)) {
			struct wifi_dev *wdev = NULL;
			UCHAR Index, ch_band = CMD_CH_BAND_24G;

			for (Index = 0; Index < WDEV_NUM_MAX; Index++) {
				wdev = pAd->wdev_list[Index];
				if (wdev) {
					ch_band = wlan_config_get_ch_band(wdev);
					break;
				}
			}

			if (ch_band != CMD_CH_BAND_24G) { /* 2.4G use SeamlessCSA to config */
				if (os_str_tol(tmpbuf, 0, 10) != 0) /*Enable*/
					pAd->CommonCfg.bIEEE80211H = TRUE;
				else { /*Disable*/
					pAd->CommonCfg.bIEEE80211H = FALSE;
#ifdef BACKGROUND_SCAN_SUPPORT
					pAd->BgndScanCtrl.DfsZeroWaitSupport = FALSE;
#endif
#ifdef MT_DFS_SUPPORT
					pAd->CommonCfg.DfsParameter.bDfsEnable = FALSE;
#ifdef MT_BAND4_DFS_SUPPORT /*302502*/
					pAd->CommonCfg.DfsParameter.band4DfsEnable = FALSE;
#endif
					UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"Disable DFS/Zero wait=%d/%d\n",
						 IS_SUPPORT_MT_DFS(pAd),
						 IS_SUPPORT_MT_ZEROWAIT_DFS(pAd));
#endif
				}
			} else
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_NOTICE,
					"2.4G.\n");

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_NOTICE,
				"IEEE80211H=%d\n", pAd->CommonCfg.bIEEE80211H);
		}

		/*RDRegion*/
		if (RTMPGetKeyParameter("RDRegion", tmpbuf, 128, pBuffer, TRUE)) {
			if ((strncmp(tmpbuf, "JAP_W53", 7) == 0) || (strncmp(tmpbuf, "jap_w53", 7) == 0)) {
				pAd->CommonCfg.RDDurRegion = JAP_W53;
				/*pRadarDetect->DfsSessionTime = 15;*/
			} else if ((strncmp(tmpbuf, "JAP_W56", 7) == 0) || (strncmp(tmpbuf, "jap_w56", 7) == 0)) {
				pAd->CommonCfg.RDDurRegion = JAP_W56;
				/*pRadarDetect->DfsSessionTime = 13;*/
			} else if ((strncmp(tmpbuf, "JAP", 3) == 0) || (strncmp(tmpbuf, "jap", 3) == 0)) {
				pAd->CommonCfg.RDDurRegion = JAP;
				/*pRadarDetect->DfsSessionTime = 5;*/
			} else  if ((strncmp(tmpbuf, "FCC", 3) == 0) || (strncmp(tmpbuf, "fcc", 3) == 0)) {
				pAd->CommonCfg.RDDurRegion = FCC;
				/*pRadarDetect->DfsSessionTime = 5;*/
			} else if ((strncmp(tmpbuf, "CE", 2) == 0) || (strncmp(tmpbuf, "ce", 2) == 0)) {
				pAd->CommonCfg.RDDurRegion = CE;
				/*pRadarDetect->DfsSessionTime = 13;*/
			} else if ((strncmp(tmpbuf, "KR", 2) == 0) || (strncmp(tmpbuf, "kr", 2) == 0)) {
				pAd->CommonCfg.RDDurRegion = KR;
				/*pRadarDetect->DfsSessionTime = 13;*/
			} else if ((strncmp(tmpbuf, "BAND4", 5) == 0) || (strncmp(tmpbuf, "band4", 5) == 0)) {
				pAd->CommonCfg.RDDurRegion = BAND4;
				/*pRadarDetect->DfsSessionTime = 13;*/
			} else {
				pAd->CommonCfg.RDDurRegion = CE;
				/*pRadarDetect->DfsSessionTime = 13;*/
			}

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"RDRegion=%d\n", pAd->CommonCfg.RDDurRegion);
		} else {
			pAd->CommonCfg.RDDurRegion = CE;
			/*pRadarDetect->DfsSessionTime = 13;*/
		}

		/* CACMemoEn */
		if (RTMPGetKeyParameter("CACMemoEn", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->CommonCfg.DfsParameter.CACMemoEn = FALSE;
			if (os_str_tol(tmpbuf, 0, 10) != 0)
				pAd->CommonCfg.DfsParameter.CACMemoEn = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_NOTICE,
				"CACMemoEn=%d\n", pAd->CommonCfg.DfsParameter.CACMemoEn);
		}

		/* DFS Pre CAC */
		if (RTMPGetKeyParameter("DfsPreCac", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->CommonCfg.DfsParameter.bPreCacEn = FALSE;
			if (os_str_tol(tmpbuf, 0, 10) != 0)
				pAd->CommonCfg.DfsParameter.bPreCacEn = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_NOTICE,
				"DfsPreCac=%d\n", pAd->CommonCfg.DfsParameter.bPreCacEn);
		}

#ifdef SYSTEM_LOG_SUPPORT

		/*WirelessEvent*/
		if (RTMPGetKeyParameter("WirelessEvent", tmpbuf, 10, pBuffer, TRUE)) {
			BOOLEAN FlgIsWEntSup = FALSE;

			if (os_str_tol(tmpbuf, 0, 10) != 0)
				FlgIsWEntSup = TRUE;

			RtmpOsWlanEventSet(pAd, &pAd->CommonCfg.bWirelessEvent, FlgIsWEntSup);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"WirelessEvent=%d\n", pAd->CommonCfg.bWirelessEvent);
		}

#endif /* SYSTEM_LOG_SUPPORT */
#ifdef WDS_SUPPORT
		rtmp_read_wds_from_file(pAd, tmpbuf, pBuffer);
#endif /* WDS_SUPPORT */
		/*Security Parameters */
		ReadSecurityParameterFromFile(pAd, tmpbuf, pBuffer);
#ifdef MBO_SUPPORT
		ReadMboParameterFromFile(pAd, tmpbuf, pBuffer);
#endif /* MBO_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
		ReadMapParameterFromFile(pAd, tmpbuf, pBuffer);
#endif /* MAP_SUPPORT */
#ifdef QOS_R1
		if (RTMPGetKeyParameter("QoSEnable", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) != 0)
				pAd->bQoSEnable = os_str_tol(tmpbuf, 0, 10);
			else
				pAd->bQoSEnable = 0;	/* disable*/
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"QoSEnable=%d\n", pAd->bQoSEnable);

#ifdef DSCP_PRI_SUPPORT
			if (IS_QOS_ENABLE(pAd))
				QoS_Init_DSCP2UP_Mapping(pAd);
#endif
		}
#ifdef QOS_R2
		if (RTMPGetKeyParameter("QoSMgmtCapa", tmpbuf, 10, pBuffer, TRUE)) {
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.bQoSMCapability
					= os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"%s, QoSMgmtCapa=%d\n",
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.if_dev->name,
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.bQoSMCapability);
			}
		}
#ifdef MAP_R5
		if (RTMPGetKeyParameter("DSCPPolicyEnable", tmpbuf, 10, pBuffer, TRUE)) {
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].bDSCPPolicyEnable
					= os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"%s, bDSCPPolicyEnable=%d\n",
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.if_dev->name,
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].bDSCPPolicyEnable);
			}
		}
#endif /* MAP_R5 */
#endif /* QOS_R2 */

#ifdef MSCS_PROPRIETARY
		if (RTMPGetKeyParameter("DABSgroupkeybitmap", tmpbuf, 16, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 16) != 0) {
				*((ULONG *)&pAd->keybitmap[2]) = (ULONG)os_str_tol(tmpbuf, 0, 16);
				}
				if (pAd->keybitmap[2] == 0 && pAd->keybitmap[3] == 0)
					pAd->keybitmap[2] = GENERAL_KEY;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"DABSkeybitmap=%lu\n", *((ULONG *)&pAd->keybitmap[2]));
		}
		if (RTMPGetKeyParameter("DABSvendorkeybitmap", tmpbuf, 16, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 16) != 0) {
				*((ULONG *)&pAd->keybitmap[0]) = (ULONG)os_str_tol(tmpbuf, 0, 16);
				}
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"DABSkeybitmap=%lu\n", *((ULONG *)&pAd->keybitmap[0]));
		}
#endif	/* MSCS_PROPRIETARY */
#endif /* QOS_R1 */
#ifdef DSCP_PRI_SUPPORT
		/*read custom DSCP mapping,only after QOS DSCP init
		* As QOS DSCP init resets the mapping table*/
		read_dscp_pri_param(pAd, tmpbuf, pBuffer);
#endif /*DSCP_PRI_SUPPORT*/
#ifdef DPP_SUPPORT
		if (RTMPGetKeyParameter("DppEnable", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) != 0)
				pAd->bDppEnable = os_str_tol(tmpbuf, 0, 10);
			else
				pAd->bDppEnable = 0;	/* disable*/
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"DppEnable=%d\n", pAd->bDppEnable);
		}
#endif /* DPP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/*Access Control List*/
			rtmp_read_acl_parms_from_file(pAd, tmpbuf, pBuffer);
#ifdef APCLI_SUPPORT
			rtmp_read_ap_client_from_file(pAd, tmpbuf, pBuffer);
#endif /* APCLI_SUPPORT */
#ifdef IGMP_SNOOP_SUPPORT
			/* Igmp Snooping information*/
			rtmp_read_igmp_snoop_from_file(pAd, tmpbuf, pBuffer);
#endif /* IGMP_SNOOP_SUPPORT */
#ifdef IDS_SUPPORT
			rtmp_read_ids_from_file(pAd, tmpbuf, pBuffer);
#endif /* IDS_SUPPORT */
#ifdef MWDS
			rtmp_read_MWDS_from_file(pAd, tmpbuf, pBuffer);
#endif /* MWDS */
#ifdef MAC_REPEATER_SUPPORT

			if (RTMPGetKeyParameter("MACRepeaterEn", tmpbuf, 10, pBuffer, FALSE)) {
				BOOLEAN bEnable = FALSE;
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (os_str_tol(macptr, 0, 10) != 0)
						bEnable = TRUE;
					else
						bEnable = FALSE;

					AsicSetReptFuncEnable(pAd, bEnable, i);
					pAd->ApCfg.bMACRepeaterEn_precfg = bEnable;
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"MACRepeaterEn=%d Band=%d\n",
						pAd->ApCfg.bMACRepeaterEn_precfg, i);
				}
				/* Disable DFS zero wait support */
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)

				if (pAd->ApCfg.bMACRepeaterEn_precfg) {
					pAd->BgndScanCtrl.DfsZeroWaitSupport = FALSE;
					UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"\x1b[31m:Disable DfsZeroWait\x1b[m\n");
				}

#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */
			}

			if (RTMPGetKeyParameter("MACRepeaterOuiMode", tmpbuf, 10, pBuffer, FALSE)) {
				INT OuiMode = os_str_tol(tmpbuf, 0, 10);

				if (OuiMode == CASUALLY_DEFINE_MAC_ADDR)
					pAd->ApCfg.MACRepeaterOuiMode = CASUALLY_DEFINE_MAC_ADDR;
				else if (OuiMode == VENDOR_DEFINED_MAC_ADDR_OUI)
					pAd->ApCfg.MACRepeaterOuiMode = VENDOR_DEFINED_MAC_ADDR_OUI; /* customer specific */
				else
					pAd->ApCfg.MACRepeaterOuiMode = FOLLOW_CLI_LINK_MAC_ADDR_OUI; /* use Ap-Client first 3 bytes MAC assress (default) */

				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"MACRepeaterOuiMode=%d\n", pAd->ApCfg.MACRepeaterOuiMode);
			}

#endif /* MAC_REPEATER_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

		if (RTMPGetKeyParameter("SE_OFF", tmpbuf, 25, pBuffer, TRUE)) {
			ULONG SeOff = os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.bSeOff = SeOff > 0 ? TRUE : FALSE;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "SE_OFF=%d\n",
					 pAd->CommonCfg.bSeOff);
		}
		if (RTMPGetKeyParameter("BcnDataRate", tmpbuf, 25, pBuffer, TRUE)) {
			ULONG lvalue = 0;

			if (kstrtol(tmpbuf, 16, &lvalue))
				MTWF_PRINT("%s: Error input BcnDataRate(=%s)\n",
					__func__, tmpbuf);
			else
				pAd->CommonCfg.BcnDataRate = lvalue;

			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BcnDataRate=%d\n",
				pAd->CommonCfg.BcnDataRate);
		}

		if (RTMPGetKeyParameter("AntennaIndex", tmpbuf, 25, pBuffer, TRUE)) {
			ULONG antenna_index = simple_strtol(tmpbuf, 0, 10);

			if (antenna_index > 28)
				antenna_index = 0;

			if (antenna_index == 24 || antenna_index == 25)
				antenna_index = 0;

			pAd->CommonCfg.ucAntennaIndex = antenna_index;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"antenna_index=%d\n",
				pAd->CommonCfg.ucAntennaIndex);
		}

#ifdef DOT11_N_SUPPORT
		read_ht_param_from_file(pAd, tmpbuf, pBuffer);
#ifdef DOT11_VHT_AC
		read_vht_param_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
		read_he_param_from_file(pAd, tmpbuf, pBuffer);
#endif
#endif /* DOT11_N_SUPPORT */

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		if (RTMPGetKeyParameter("OldChannel_Dev1", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldChannelCfg(pAd, tmpbuf);
		if (RTMPGetKeyParameter("OldChannel_Dev2", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldChannelCfg(pAd, tmpbuf);

		if (RTMPGetKeyParameter("OldHTBW_Dev1", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldBWCfg(pAd, tmpbuf, FALSE);
		if (RTMPGetKeyParameter("OldHTBW_Dev2", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldBWCfg(pAd, tmpbuf, FALSE);

		if (RTMPGetKeyParameter("OldVHTBW_Dev1", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldBWCfg(pAd, tmpbuf, TRUE);
		if (RTMPGetKeyParameter("OldVHTBW_Dev2", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldBWCfg(pAd, tmpbuf, TRUE);

		if (RTMPGetKeyParameter("Ch144Support", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) == 1)
				pAd->CommonCfg.bCh144Enabled = TRUE;
			else
				pAd->CommonCfg.bCh144Enabled = FALSE;
		}
#endif

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef WSC_AP_SUPPORT
			RTMP_STRING tok_str[19] = {0};

			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				ret = snprintf(tok_str, sizeof(tok_str), "WscDefaultSSID%d", i + 1);
				if (os_snprintf_error(sizeof(tok_str), ret))
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"snprintf error!\n");

				if (RTMPGetKeyParameter(tok_str, tmpbuf, 33, pBuffer, FALSE)) {
					NdisZeroMemory(&pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscDefaultSsid, sizeof(NDIS_802_11_SSID));
					NdisMoveMemory(pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscDefaultSsid.Ssid, tmpbuf, strlen(tmpbuf));
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscDefaultSsid.SsidLength = strlen(tmpbuf);
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							 "WscDefaultSSID[%d]=%s\n", i, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscDefaultSsid.Ssid);
				}
			}

			/*WscConfMode*/
			if (RTMPGetKeyParameter("WscConfMode", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					INT WscConfMode = os_str_tol(macptr, 0, 10);

					if (i >= pAd->ApCfg.BssidNum)
						break;

					if (WscConfMode > 0 && WscConfMode < 8)
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscConfMode = WscConfMode;
					else
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscConfMode = WSC_DISABLE;

					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							 "I/F(ra%d) WscConfMode=%d\n", i, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscConfMode);
				}
			}

			/*WscConfStatus*/
			if (RTMPGetKeyParameter("WscConfStatus", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscConfStatus = (INT) os_str_tol(macptr, 0, 10);
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							 "I/F(ra%d) WscConfStatus=%d\n", i, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscConfStatus);
				}
			}

			/*WscConfMethods*/
			if (RTMPGetKeyParameter("WscConfMethods", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscConfigMethods = (USHORT)os_str_tol(macptr, 0, 16);
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							 "I/F(ra%d) WscConfMethods=0x%x\n", i, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscConfigMethods);
				}
			}

			/*WscKeyASCII (0:Hex, 1:ASCII(random length), others: ASCII length, default 8)*/
			if (RTMPGetKeyParameter("WscKeyASCII", tmpbuf, 10, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					INT Value;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					Value = (INT) os_str_tol(tmpbuf, 0, 10);

					if (Value == 0 || Value == 1)
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscKeyASCII = Value;
					else if (Value >= 8 && Value <= 63)
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscKeyASCII = Value;
					else
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscKeyASCII = 8;

					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_WARN,
						"WscKeyASCII=%d\n",
						pAd->ApCfg.MBSSID[i].wdev.WscControl.WscKeyASCII);
				}
			}

			if (RTMPGetKeyParameter("WscSecurityMode", tmpbuf, 50, pBuffer, TRUE)) {
				for (i = 0; i < pAd->ApCfg.BssidNum; i++)
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscSecurityMode = WPAPSKTKIP;

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					INT tmpMode = 0;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					tmpMode = (INT) os_str_tol(macptr, 0, 10);

					if (tmpMode <= WPAPSKTKIP)
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscSecurityMode = tmpMode;

					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"I/F(ra%d) WscSecurityMode=%d\n",
						i, pAd->ApCfg.MBSSID[i].wdev.WscSecurityMode);
				}
			}

			/* WCNTest*/
			if (RTMPGetKeyParameter("WCNTest", tmpbuf, 10, pBuffer, TRUE)) {
				BOOLEAN	bEn = FALSE;

				if ((strncmp(tmpbuf, "0", 1) == 0))
					bEn = FALSE;
				else
					bEn = TRUE;

				for (i = 0; i < pAd->ApCfg.BssidNum; i++)
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.bWCNTest = bEn;

				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"WCNTest=%d\n", bEn);
			}

			/*WSC UUID Str*/
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				PWSC_CTRL	pWpsCtrl = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl;

				ret = snprintf(tok_str, sizeof(tok_str), "WSC_UUID_Str%d", i + 1);
				if (os_snprintf_error(sizeof(tok_str), ret))
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"snprintf error!\n");

				if (RTMPGetKeyParameter(tok_str, tmpbuf, 40, pBuffer, FALSE)) {
					NdisMoveMemory(&pWpsCtrl->Wsc_Uuid_Str[0], tmpbuf, strlen(tmpbuf));
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"UUID_Str[%d]=%s\n", i + 1, pWpsCtrl->Wsc_Uuid_Str);
				}
			}

			/*WSC UUID Hex*/
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				PWSC_CTRL	pWpsCtrl = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl;

				ret = snprintf(tok_str, sizeof(tok_str), "WSC_UUID_E%d", i + 1);
				if (os_snprintf_error(sizeof(tok_str), ret))
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"snprintf error!\n");

				if (RTMPGetKeyParameter(tok_str, tmpbuf, 40, pBuffer, FALSE)) {
					AtoH(tmpbuf, &pWpsCtrl->Wsc_Uuid_E[0], UUID_LEN_HEX);
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"Wsc_Uuid_E[%d]", i + 1);
					hex_dump("", &pWpsCtrl->Wsc_Uuid_E[0], UUID_LEN_HEX);
				}
			}

			/* WSC AutoTrigger Disable */
			if (RTMPGetKeyParameter("WscAutoTriggerDisable", tmpbuf, 10, pBuffer, TRUE)) {
				BOOLEAN	bEn = FALSE;

				if ((strncmp(tmpbuf, "0", 1) == 0))
					bEn = FALSE;
				else
					bEn = TRUE;

				for (i = 0; i < pAd->ApCfg.BssidNum; i++)
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.bWscAutoTriggerDisable = bEn;

				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"bWscAutoTriggerDisable=%d\n", bEn);
			}
#endif /* WSC_AP_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
			if (RTMPGetKeyParameter("RoamingEnhance", tmpbuf, 32, pBuffer, TRUE))
				pAd->ApCfg.bRoamingEnhance = (simple_strtol(tmpbuf, 0, 10) > 0)?TRUE:FALSE;
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			/*PSMode*/
			if (RTMPGetKeyParameter("PSMode", tmpbuf, MAX_PARAM_BUFFER_SIZE, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr != NULL) && (i < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), i++) {
					if (pAd->StaCfg[i].BssType == BSS_INFRA) {
						if ((strcmp(tmpbuf, "MAX_PSP") == 0) || (strcmp(tmpbuf, "max_psp") == 0)) {
							/*
								do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange()
								to exclude certain situations
							*/
							/*	MlmeSetPsm(pAd, PWR_SAVE);*/
							OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

							if (pAd->StaCfg[i].bWindowsACCAMEnable == FALSE)
								pAd->StaCfg[i].WindowsPowerMode = Ndis802_11PowerModeMAX_PSP;

							pAd->StaCfg[i].WindowsBatteryPowerMode = Ndis802_11PowerModeMAX_PSP;
							pAd->StaCfg[i].DefaultListenCount = 5;
						} else if ((strcmp(tmpbuf, "Fast_PSP") == 0) || (strcmp(tmpbuf, "fast_psp") == 0)
								   || (strcmp(tmpbuf, "FAST_PSP") == 0)) {
							/* do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange()*/
							/* to exclude certain situations.*/
							OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

							if (pAd->StaCfg[i].bWindowsACCAMEnable == FALSE)
								pAd->StaCfg[i].WindowsPowerMode = Ndis802_11PowerModeFast_PSP;

							pAd->StaCfg[i].WindowsBatteryPowerMode = Ndis802_11PowerModeFast_PSP;
							pAd->StaCfg[i].DefaultListenCount = 3;
						} else if ((strcmp(tmpbuf, "Legacy_PSP") == 0) || (strcmp(tmpbuf, "legacy_psp") == 0)
								   || (strcmp(tmpbuf, "LEGACY_PSP") == 0)) {
							/* do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange()*/
							/* to exclude certain situations.*/
							OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

							if (pAd->StaCfg[i].bWindowsACCAMEnable == FALSE)
								pAd->StaCfg[i].WindowsPowerMode = Ndis802_11PowerModeLegacy_PSP;

							pAd->StaCfg[i].WindowsBatteryPowerMode = Ndis802_11PowerModeLegacy_PSP;
							pAd->StaCfg[i].DefaultListenCount = 3;
						} else {
							/*Default Ndis802_11PowerModeCAM*/
							/* clear PSM bit immediately*/
							RTMP_SET_PSM_BIT(pAd, &pAd->StaCfg[i], PWR_ACTIVE);
							OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

							if (pAd->StaCfg[i].bWindowsACCAMEnable == FALSE)
								pAd->StaCfg[i].WindowsPowerMode = Ndis802_11PowerModeCAM;

							pAd->StaCfg[i].WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
						}

						MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"pAd->StaCfg[%d]::PSMode=%ld\n",  i,
							 pAd->StaCfg[i].WindowsPowerMode);
					}
				}
			}

			/* AutoRoaming by RSSI*/
			if (RTMPGetKeyParameter("AutoRoaming", tmpbuf, 32, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) == 0)
					pAd->StaCfg[0].bAutoRoaming = FALSE;
				else
					pAd->StaCfg[0].bAutoRoaming = TRUE;

				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"AutoRoaming=%d\n", pAd->StaCfg[0].bAutoRoaming);
			}

			/* RoamThreshold*/
			if (RTMPGetKeyParameter("RoamThreshold", tmpbuf, 32, pBuffer, TRUE)) {
				long lInfo = os_str_tol(tmpbuf, 0, 10);

				if (lInfo > 90 || lInfo < 60)
					pAd->StaCfg[0].dBmToRoam = -70;
				else
					pAd->StaCfg[0].dBmToRoam = (CHAR)(-1) * lInfo;

				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"RoamThreshold=%d  dBm\n", pAd->StaCfg[0].dBmToRoam);
			}

			if (RTMPGetKeyParameter("TGnWifiTest", tmpbuf, 10, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) == 0)
					pAd->StaCfg[0].bTGnWifiTest = FALSE;
				else
					pAd->StaCfg[0].bTGnWifiTest = TRUE;

				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"TGnWifiTest=%d\n", pAd->StaCfg[0].bTGnWifiTest);
			}

#ifdef ETH_CONVERT_SUPPORT

			/* Ethernet Converter Operation Mode.*/
			if (RTMPGetKeyParameter("EthConvertMode", tmpbuf, 32, pBuffer, TRUE)) {
				Set_EthConvertMode_Proc(pAd, tmpbuf);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"EthConvertMode=%d\n", pAd->EthConvert.ECMode);
			}

			/* Ethernet Converter Operation Mode.*/
			if (RTMPGetKeyParameter("EthCloneMac", tmpbuf, 32, pBuffer, TRUE)) {
				Set_EthCloneMac_Proc(pAd, tmpbuf);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"EthCloneMac="MACSTR"\n",
						 MAC2STR(pAd->EthConvert.EthCloneMac));
			}

#endif /* ETH_CONVERT_SUPPORT */

			/* Beacon Lost Time*/
			if (RTMPGetKeyParameter("BeaconLostTime", tmpbuf, 32, pBuffer, TRUE)) {
				ULONG lInfo = (ULONG)os_str_tol(tmpbuf, 0, 10);

				if ((lInfo != 0) && (lInfo <= 60))
					pAd->StaCfg[0].BeaconLostTime = (lInfo * OS_HZ);

				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"BeaconLostTime=%ld\n", pAd->StaCfg[0].BeaconLostTime);
			}

			/* Auto Connet Setting if no SSID			*/
			if (RTMPGetKeyParameter("AutoConnect", tmpbuf, 32, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) == 0)
					pAd->StaCfg[0].bAutoConnectIfNoSSID = FALSE;
				else
					pAd->StaCfg[0].bAutoConnectIfNoSSID = TRUE;
			}

#ifdef DOT11R_FT_SUPPORT

			/* FtSupport*/
			if (RTMPGetKeyParameter("FtSupport", tmpbuf, 32, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) == 0)
					pAd->StaCfg[0].Dot11RCommInfo.bFtSupport = FALSE;
				else
					pAd->StaCfg[0].Dot11RCommInfo.bFtSupport = TRUE;

				;
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"bFtSupport=%d\n",
					pAd->StaCfg[0].Dot11RCommInfo.bFtSupport);
			}

#endif /* DOT11R_FT_SUPPORT */

			/* FastConnect*/
			if (RTMPGetKeyParameter("FastConnect", tmpbuf, 32, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) == 0)
					pAd->StaCfg[0].bFastConnect = FALSE;
				else
					pAd->StaCfg[0].bFastConnect = TRUE;

				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"FastConnect=%d\n", pAd->StaCfg[0].bFastConnect);
			}
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef BT_APCLI_SUPPORT
		if (RTMPGetKeyParameter("BTApCliAutoBWSupport", tmpbuf, 128, pBuffer, TRUE)) {
			INT BT_APCLI_Auto_BW_Support = 0;

			BT_APCLI_Auto_BW_Support = os_str_tol(tmpbuf, 0, 10);
			pAd->ApCfg.ApCliAutoBWBTSupport = BT_APCLI_Auto_BW_Support;
		}
#endif
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	if (RTMPGetKeyParameter("ApCliSyncAutoBandSupport", tmpbuf, 128, pBuffer, TRUE)) {
		UCHAR policy = 0, value = 0;

		for (policy = 0, macptr = rstrtok(tmpbuf, ";"); (policy <= BAND_MAX_POLICY && macptr);
		macptr = rstrtok(NULL, ";"), policy++) {
			value = (UCHAR)os_str_tol(macptr, 0, 10);
			if (value == 0)
				continue;

			switch (policy) {
			case SAME_BAND_SYNC:
				SET_APCLI_SYNC_BAND_VALID(pAd, policy);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"[%s] Policy %d Band Valid %d\n",
					policy, IS_APCLI_SYNC_BAND_VALID(pAd, policy));
				break;

			case DEAUTH_PEERS:
				SET_APCLI_SYNC_PEER_DEAUTH_VALID(pAd, TRUE);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"[%s] Policy %d Deauth Valid %d\n",
					policy, IS_APCLI_SYNC_PEER_DEAUTH_VALID(pAd));
				break;

			case DIFF_BAND_SYNC:
			case BAND_MAX_POLICY:
			default:
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"Invalid Band Policy %d\n", policy);
				break;
			}
		}
	}

	if (RTMPGetKeyParameter("ApCliSyncAutoBWSupport", tmpbuf, 128, pBuffer, TRUE)) {
		UCHAR policy = 0, value = 0;
		POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
		struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

		if (IS_APCLI_SYNC_BAND_VALID(pAd, SAME_BAND_SYNC) || IS_APCLI_SYNC_BAND_VALID(pAd, DIFF_BAND_SYNC)) {
			for (policy = 0, macptr = rstrtok(tmpbuf, ";"); (policy <= BW_MAX_POLICY && macptr); macptr = rstrtok(NULL, ";"), policy++) {
				value = (UCHAR)os_str_tol(macptr, 0, 10);
				if (value == 0)
					continue;

				switch (policy) {
				case HT_2040_UP_ENBL:
				case HT_4020_DOWN_ENBL:
					SET_APCLI_AUTO_BW_HT_VALID(pAd, policy);
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"Policy %d HT Valid %d\n",
						policy, IS_SYNC_BW_POLICY_VALID(pAd, TRUE, policy));
					break;

				case VHT_80_2040_DOWN_ENBL:
				case VHT_2040_80_UP_ENBL:
					if (wlan_config_get_vht_bw(wdev) > VHT_BW_2040) {
						SET_APCLI_AUTO_BW_VHT_VALID(pAd, (policy-VHT_POLICY_OFFSET));
						MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							"Policy %d VHT Valid %d\n",
							policy,
							IS_SYNC_BW_POLICY_VALID(pAd,
								ALSE, (policy-VHT_POLICY_OFFSET)));

					} else if (policy == VHT_2040_80_UP_ENBL) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"Policy VHT 20/40->80 InValid Curret Cap %d\n", wlan_config_get_vht_bw(wdev));
					} else if (policy == VHT_80_2040_DOWN_ENBL) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"Policy VHT 80->20/40 InValid Curret Cap %d\n", wlan_config_get_vht_bw(wdev));
					}
					break;

				case VHT_160_2040_DOWN_ENBL:
				case VHT_160_80_DOWN_ENBL:
				case VHT_2040_160_UP_ENBL:
				case VHT_80_160_UP_ENBL:
				case BW_MAX_POLICY:
				default:
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"Invalid BW Policy  %d\n", policy);
					break;
				}
			}

			if (IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd)) {
				/* Disable 40 MHz Intolerance & BSS Coex Enable Fields */
				pAd->CommonCfg.bBssCoexEnable = FALSE;
				pAd->CommonCfg.bForty_Mhz_Intolerant = FALSE;
			} else {
				/* Disable Feature as Policy not updated */
				SET_APCLI_SYNC_BAND_FEATURE_DISABLE(pAd, POLICY_DISABLE);
				SET_APCLI_SYNC_PEER_DEAUTH_VALID(pAd, POLICY_DISABLE);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"Disable Auto Band Policy %d\n",
					IS_V10_AUTO_BAND_FEATURE_ENBL(pAd));
			}
		}
	}
#endif
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
			read_eaprate_param_from_file(pAd, tmpbuf, pBuffer);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

#ifdef MCAST_RATE_SPECIFIC

			/* McastPhyMode*/
			if (RTMPGetKeyParameter("McastPhyMode", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					UCHAR PhyMode = 0;
					union _HTTRANSMIT_SETTING *transmit;
					struct wifi_dev *wdev = NULL;
					UCHAR ht_bw = 0;
					struct _MAC_TABLE_ENTRY *entry;

					if (i >= pAd->ApCfg.BssidNum)
						break;
					wdev = &pAd->ApCfg.MBSSID[i].wdev;
					ht_bw = wlan_config_get_ht_bw(wdev);
					PhyMode = os_str_tol(macptr, 0, 10);
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
					transmit  = (wdev->channel > 14) ?
							 (&wdev->rate.MCastPhyMode_5G) : (&wdev->rate.MCastPhyMode);
#else
					transmit  = &wdev->rate.mcastphymode;
#endif
					transmit->field.BW = ht_bw;

					switch (PhyMode) {
					case MCAST_DISABLE: /* disable */
						entry = entry_get(pAd, MCAST_WCID);
						NdisMoveMemory(transmit,
							&entry->HTPhyMode,
							sizeof(union _HTTRANSMIT_SETTING));

						if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G) {
							transmit->field.MODE = MODE_CCK;
							transmit->field.BW =  BW_20;
							transmit->field.MCS = RATE_1;
						} else {
							transmit->field.MODE = MODE_OFDM;
							transmit->field.BW =  BW_20;
							transmit->field.MCS = OfdmRateToRxwiMCS[RATE_6];
						}

						break;

					case MCAST_CCK:	/* CCK*/
						if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G) {
							transmit->field.MODE = MODE_CCK;
							transmit->field.BW =  BW_20;
						} else {
							MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"Could not set CCK mode for 5G band.\n");
							transmit->field.MODE = MODE_OFDM;
							transmit->field.BW =  BW_20;
							/* pTransmit->field.MCS = OfdmRateToRxwiMCS[RATE_6]; */
						}

						break;

					case MCAST_OFDM:	/* OFDM*/
						transmit->field.MODE = MODE_OFDM;
						transmit->field.BW =  BW_20;
						break;

					default:
						MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"Unknown Multicast PhyMode %d.\n", PhyMode);
						MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"Set the default mode, MCAST_OFDM!\n");
						transmit->field.MODE = MODE_OFDM;
						transmit->field.BW =  BW_20;
						break;
					}
				}
			}
			/* McastMcs*/
			if (RTMPGetKeyParameter("McastMcs", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					union _HTTRANSMIT_SETTING *transmit;
					UCHAR Mcs = 0;
					struct wifi_dev *wdev;

					if (i >= pAd->ApCfg.BssidNum)
						break;
					wdev = &pAd->ApCfg.MBSSID[i].wdev;
					Mcs = os_str_tol(macptr, 0, 10);
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
					transmit  = (wdev->channel > 14) ?
						(&wdev->rate.MCastPhyMode_5G) : (&wdev->rate.MCastPhyMode);
#else
					transmit  = &wdev->rate.mcastphymode;
#endif
					switch (transmit->field.MODE) {
					case MODE_CCK:
						if (wlan_config_get_ch_band(wdev) != CMD_CH_BAND_24G
							|| Mcs > 3) {
							MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE,
								DBG_LVL_ERROR,
								"CCK mode: err band(%d) 5/6G, error MCS(%d)!\n",
								wlan_config_get_ch_band(wdev), Mcs);
							break;
						}

						if (pAd->CommonCfg.TxPreamble == Rt802_11PreambleShort) {
							if (Mcs == 0) {
								Mcs = Mcs + 1;
								MTWF_DBG(pAd,
									DBG_CAT_CFG, CATCFG_PROFILE,
									DBG_LVL_ERROR,
										"Invalid MCS(CCK-1M) value when short preamble is enabled!\n");
							}
							Mcs |= 0x4;
						}
						transmit->field.MCS = Mcs;

						break;

					case MODE_OFDM:
						if (Mcs > 7)
							MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE,
								DBG_LVL_ERROR,
								"MCS must in range from 0 ~ 7 for OFDM Mode.\n");
						else
							transmit->field.MCS = Mcs;

						break;

					default:
						transmit->field.MCS = Mcs;
						break;
					}
				}
			}
#endif /* MCAST_RATE_SPECIFIC */
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef WSC_INCLUDED
		rtmp_read_wsc_user_parms_from_file(pAd, tmpbuf, pBuffer);

		/* Wsc4digitPinCode = TRUE use 4-digit Pin code, otherwise 8-digit Pin code */
		if (RTMPGetKeyParameter("Wsc4digitPinCode", tmpbuf, 32, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					if (os_str_tol(macptr, 0, 10) != 0)	/* Enable */
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscEnrollee4digitPinCode = TRUE;
					else /* Disable */
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscEnrollee4digitPinCode = FALSE;

					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
							 "I/F(ra%d) Wsc4digitPinCode=%d\n", i, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscEnrollee4digitPinCode);
				}
			}
#endif /* CONFIG_AP_SUPPORT // */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (os_str_tol(tmpbuf, 0, 10) != 0)	/* Enable */
					pAd->StaCfg[0].wdev.WscControl.WscEnrollee4digitPinCode = TRUE;
				else /* Disable */
					pAd->StaCfg[0].wdev.WscControl.WscEnrollee4digitPinCode = FALSE;

				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						 "Wsc4digitPinCode=%d\n", pAd->StaCfg[0].wdev.WscControl.WscEnrollee4digitPinCode);
			}
#endif /* CONFIG_STA_SUPPORT // */
		}

		if (RTMPGetKeyParameter("WscVendorPinCode", tmpbuf, 256, pBuffer, TRUE)) {
			PWSC_CTRL pWscContrl = NULL;
			int bSetOk;
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			{
#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
					if (i >= pAd->ApCfg.BssidNum)
						break;
					pWscContrl = &pAd->ApCfg.MBSSID[i].wdev.WscControl;
				}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
					if (i >= MAX_MULTI_STA)
						break;
					pWscContrl = &pAd->StaCfg[0].wdev.WscControl;
				}
#endif /* CONFIG_STA_SUPPORT */

				bSetOk = RT_CfgSetWscPinCode(pAd, macptr, pWscContrl);

				if (bSetOk)
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"WscVendorPinCode= (%d)\n", bSetOk);
				else
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						"WscVendorPinCode: invalid pin code(%s)\n",
						macptr);
			}
		}

#ifdef WSC_V2_SUPPORT

		if (RTMPGetKeyParameter("WscV2Support", tmpbuf, 32, pBuffer, TRUE)) {
			UCHAR			bEnable;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					bEnable = (UCHAR)os_str_tol(macptr, 0, 10);
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscV2Info.bEnableWpsV2 = bEnable;
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"I/F(ra%d) WscV2Support=%d\n", i, bEnable);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				bEnable = (UCHAR)os_str_tol(tmpbuf, 0, 10);
				pAd->StaCfg[0].wdev.WscControl.WscV2Info.bEnableWpsV2 = bEnable;
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"%s - WscV2Support= (%d)\n", __func__, bEnable);
			}
#endif /* CONFIG_STA_SUPPORT */
		}

#endif /* WSC_V2_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT
		FT_rtmp_read_parameters_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT11R_FT_SUPPORT */
#ifdef OCE_SUPPORT
		Oce_read_parameters_from_file(pAd, tmpbuf, pBuffer);
#endif /* OCE_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11K_RRM_SUPPORT
		RRM_ReadParametersFromFile(pAd, tmpbuf, pBuffer);
#endif /* DOT11K_RRM_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef SINGLE_SKU

		if (RTMPGetKeyParameter("AntGain", tmpbuf, 10, pBuffer, TRUE)) {
			UCHAR AntGain = os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.AntGain = AntGain;
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"AntGain=%d\n", pAd->CommonCfg.AntGain);
		}

		if (RTMPGetKeyParameter("BandedgeDelta", tmpbuf, 10, pBuffer, TRUE)) {
			UCHAR Bandedge = os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.BandedgeDelta = Bandedge;
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BandedgeDelta=%d\n", pAd->CommonCfg.BandedgeDelta);
		}

#endif /* SINGLE_SKU */
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)

		/* set GPIO pin for wake-up signal */
		if (RTMPGetKeyParameter("WOW_GPIO", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_GPIO(pAd, tmpbuf);

		/* set WOW enable/disable */
		if (RTMPGetKeyParameter("WOW_Enable", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Enable(pAd, tmpbuf);

		/* set delay time for WOW really enable */
		if (RTMPGetKeyParameter("WOW_Delay", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Delay(pAd, tmpbuf);

		/* set GPIO pulse hold time */
		if (RTMPGetKeyParameter("WOW_Hold", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Hold(pAd, tmpbuf);

		/* set wakeup signal type */
		if (RTMPGetKeyParameter("WOW_InBand", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_InBand(pAd, tmpbuf);

		/* set wakeup interface */
		if (RTMPGetKeyParameter("WOW_Interface", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Interface(pAd, tmpbuf);

		/* set if down interface */
		if (RTMPGetKeyParameter("WOW_IfDown_Support", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_IfDown_Support(pAd, tmpbuf);

		/* set GPIO High Low */
		if (RTMPGetKeyParameter("WOW_GPIOHighLow", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_GPIOHighLow(pAd, tmpbuf);

#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT) */
#ifdef MICROWAVE_OVEN_SUPPORT

		if (RTMPGetKeyParameter("MO_FalseCCATh", tmpbuf, 10, pBuffer, TRUE))
			Set_MO_FalseCCATh_Proc(pAd, tmpbuf);

#endif /* MICROWAVE_OVEN_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#ifdef SNIFFER_SUPPORT

		if (RTMPGetKeyParameter("SnifferType", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->sniffer_ctl.sniffer_type = os_str_tol(tmpbuf, 0, 10);
			set_sniffer_mode(pAd->StaCfg[0].wdev.if_dev, pAd->sniffer_ctl.sniffer_type);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"SnifferType = %d\n", pAd->sniffer_ctl.sniffer_type);
		}

#endif /* SNIFFER_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

		if (RTMPGetKeyParameter("PS_RETRIEVE", tmpbuf, 10, pBuffer, TRUE)) {
			long PS_RETRIEVE;

			PS_RETRIEVE = os_str_tol(tmpbuf, 0, 10);
			pAd->bPS_Retrieve = PS_RETRIEVE;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"PS_RETRIEVE = %lx\n", PS_RETRIEVE);
		}

#ifdef FW_DUMP_SUPPORT

		if (RTMPGetKeyParameter("FWDump_Path", tmpbuf, 10, pBuffer, TRUE))
			set_fwdump_path(pAd, tmpbuf);

		if (RTMPGetKeyParameter("FWDump_MaxSize", tmpbuf, 10, pBuffer, TRUE))
			set_fwdump_max_size(pAd, tmpbuf);

#endif
#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)

		if (RTMPGetKeyParameter("IcapMode", tmpbuf, 10, pBuffer, TRUE)) {
			UINT8 ICapMode; /* 0 : Normal Mode; 1 : Internal Capture; 2 : Wifi Spectrum */

			ICapMode = simple_strtol(tmpbuf, 0, 10);
			pAd->ICapMode = ICapMode;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"ICapMode = %d\n", ICapMode);
		}

#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */
#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		rtmp_read_vow_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* CONFIG_AP_SUPPORT */
#endif /* VOW_SUPPORT */
#ifdef FW_LOG_DUMP
		rtmp_read_fw_log_dump_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* FW_LOG_DUMP */
		rtmp_read_cp_parms_from_file(pAd, tmpbuf, pBuffer);
		rtmp_read_multi_cli_nums_eap_th_parms_from_file(pAd, tmpbuf, pBuffer);
#ifdef MGMT_TXPWR_CTRL
		rtmp_read_mgmt_pwr_parms_from_file(pAd, tmpbuf, pBuffer);
#endif
		rtmp_read_retry_parms_from_file(pAd, tmpbuf, pBuffer);
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11V_MBSSID_SUPPORT
	if (RTMPGetKeyParameter("Dot11vMbssidExt", tmpbuf, 256, pBuffer, TRUE) && (strlen(tmpbuf) > 4))
		rtmp_read_dot11v_mbssid_cfg_from_file_ext(pAd, tmpbuf);
	else if (RTMPGetKeyParameter("Dot11vMbssid", tmpbuf, 100, pBuffer, TRUE))
		rtmp_read_dot11v_mbssid_cfg_from_file(pAd, tmpbuf);
#endif /* DOT11V_MBSSID_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_6G_SUPPORT
	rtmp_read_6g_discov_cfg_from_file(pAd, tmpbuf, pBuffer);
#endif

#ifdef DOT11_EHT_BE
	read_ap_eht_config_from_file(pAd, tmpbuf, pBuffer);
	read_sta_eht_config_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT11_EHT_BE */

#ifdef CFG_SUPPORT_FALCON_MURU
	if (RTMPGetKeyParameter("TamArbOpMode", tmpbuf, 32, pBuffer, TRUE)) {
		pAd->CommonCfg.TamArbOpMode = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"TamArbOpMode = %ld\n", pAd->CommonCfg.TamArbOpMode);
	}

	if (RTMPGetKeyParameter("HE_PpduFmt", tmpbuf, 32, pBuffer, TRUE)) {
		pAd->CommonCfg.HE_PpduFmt = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"HE_PpduFmt = %ld\n", pAd->CommonCfg.HE_PpduFmt);
	}

	if (RTMPGetKeyParameter("HE_OfdmaSchType", tmpbuf, 32, pBuffer, TRUE)) {
		pAd->CommonCfg.HE_OfdmaSchType = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"HE_OfdmaSchType = %ld\n", pAd->CommonCfg.HE_OfdmaSchType);
	}

	if (RTMPGetKeyParameter("HE_OfdmaUserNum", tmpbuf, 32, pBuffer, TRUE)) {
		pAd->CommonCfg.HE_OfdmaUserNum = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"HE_OfdmaUserNum = %ld\n", pAd->CommonCfg.HE_OfdmaUserNum);
	}

	if (RTMPGetKeyParameter("HE_TrigPadding", tmpbuf, 32, pBuffer, TRUE)) {
		pAd->CommonCfg.HE_TrigPadding = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"HE_TrigPadding = %ld\n", pAd->CommonCfg.HE_TrigPadding);
	}
#endif
#ifdef CFG_SUPPORT_FALCON_SR
	/* Spatial Reuse  */
	if (RTMPGetKeyParameter("SREnable", tmpbuf, 32, pBuffer, TRUE))
		SrProfileSREnable(pAd, tmpbuf);
	if (RTMPGetKeyParameter("SRMode", tmpbuf, 32, pBuffer, TRUE))
		SrProfileSRMode(pAd, tmpbuf);
	if (RTMPGetKeyParameter("SRSDEnable", tmpbuf, 32, pBuffer, TRUE))
		SrProfileSRSDEnable(pAd, tmpbuf);
	if (RTMPGetKeyParameter("SRDPDEnable", tmpbuf, 32, pBuffer, TRUE))
		SrProfileSRDPDEnable(pAd, tmpbuf);
#endif /* CFG_SUPPORT_FALCON_SR */

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_HE_AX
	if (RTMPGetKeyParameter("BSSColorValue", tmpbuf, 32, pBuffer, TRUE))
		bss_color_profile_enable(pAd, tmpbuf);
#endif /* DOT11_HE_AX */
#endif

#ifdef CFG_SUPPORT_FALCON_PP
	/* Preamble puncture */
	if (RTMPGetKeyParameter("PPEnable", tmpbuf, 32, pBuffer, TRUE))
		pp_profile_pp_en(pAd, tmpbuf);
#endif /* CFG_SUPPORT_FALCON_PP */

#ifdef DOT11_EHT_BE
		/*PP blacklist by CountryCode*/
		if (RTMPGetKeyParameter("PPBlackList", tmpbuf, 256, pBuffer, TRUE)) {
			for (i = 0, tmpptr = rstrtok(tmpbuf, ";"); tmpptr; tmpptr = rstrtok(NULL, ";"), i++) {
				if (i >= MAX_PP_BLACK_LIST_LEN)
					break;
				rtmp_set_pp_black_list(pAd, tmpptr, i);
			}
			for (i = 0; i < MAX_PP_BLACK_LIST_LEN; i++) {
				if (NdisCmpMemory(pAd->CommonCfg.pp_bl[i].CountryCode, pAd->CommonCfg.CountryCode, 2) == 0) {
					pAd->CommonCfg.pp_bl_valid = TRUE;
					break;
				}
			}
#ifdef CONFIG_STA_SUPPORT
			for (i = 0; i < MAX_PP_BLACK_LIST_LEN; i++) {
				if (NdisCmpMemory(pAd->StaCfg[0].pp_bl[i].CountryCode, pAd->CommonCfg.CountryCode, 2) == 0) {
					pAd->StaCfg[0].pp_bl_valid = TRUE;
					break;
				}
			}
#endif /* CONFIG_STA_SUPPORT */
		}
#endif /* DOT11_EHT_BE */

#ifdef ANTENNA_CONTROL_SUPPORT
		rtmp_read_ant_ctrl_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* ANTENNA_CONTROL_SUPPORT */
#ifdef ANDLINK_FEATURE_SUPPORT
	rtmp_read_andlink_parms_from_file(pAd, tmpbuf, pBuffer);
#endif/*ANDLINK_FEATURE_SUPPORT*/
#ifdef ACK_CTS_TIMEOUT_SUPPORT
	rtmp_read_ackcts_timeout_parms_from_file(pAd, tmpbuf, pBuffer);
#endif
#ifdef MLME_MULTI_QUEUE_SUPPORT
	rtmp_read_mlme_multiqueue_parms_from_file(pAd, tmpbuf, pBuffer);
#endif
	rtmp_read_quick_channel_switch_parms_from_file(pAd, tmpbuf, pBuffer);
#ifdef PEAK_ENHANCE
	read_peak_adjust_cfg_from_file(pAd, tmpbuf, pBuffer);
#endif /* PEAK_ENHANCE */

	if (RTMPGetKeyParameter("Wifi6GCap", tmpbuf, 10, pBuffer, TRUE)) {
		/* 0 : not support; 1 : WiFi 6E support */
		pAd->Wifi6gCap = simple_strtol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"Wifi6gCap = %d\n", pAd->Wifi6gCap);
	}

	if (RTMPGetKeyParameter("LPIEnable", tmpbuf, 10, pBuffer, TRUE)) {
		long long_val = 0;

		if (kstrtol(tmpbuf, 10, &long_val))
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"error kstrtol\n");
		else
			pAd->CommonCfg.LpiEn = long_val;

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"LpiEn = %d\n", pAd->CommonCfg.LpiEn);
	}
#ifdef MLR_SUPPORT
	if (RTMPGetKeyParameter("MLREnable", tmpbuf, 32, pBuffer, TRUE)) {
		/* 0 : not support; 1 : MLR support */
		pAd->CommonCfg.bMLREnable = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"MLREnable = %d\n", pAd->CommonCfg.bMLREnable);
	}

	if (RTMPGetKeyParameter("MLRVersion", tmpbuf, 32, pBuffer, TRUE)) {
		/* 2 : MLR v2 support */
		if (os_str_tol(tmpbuf, 0, 10) == 2)
			pAd->CommonCfg.MLRVersion = MLR_MODE_MLR_V2;
		else
			pAd->CommonCfg.MLRVersion = MLR_MODE_MLR_V1;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"MLRVersion = %d\n", pAd->CommonCfg.MLRVersion);
	}
#endif /* MLR_SUPPORT */
#ifdef DFS_SLAVE_SUPPORT
	if (RTMPGetKeyParameter("DfsSlaveEn", tmpbuf, 10, pBuffer, TRUE)) {
		pAd->slave_ctrl.enable = os_str_tol(tmpbuf, 0, 10) > 0 ? 1 : 0;
		if (pAd->slave_ctrl.enable) {
			pAd->slave_ctrl.disable_beacon = TRUE;
			pAd->slave_ctrl.disconnect_sta = TRUE;
		}
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
			"[DFS-SLAVE][%s] fDfsSlaveEn:%d\n", __func__, pAd->slave_ctrl.enable);
	}
	if (RTMPGetKeyParameter("SlaveSkipStaDisc", tmpbuf, 10, pBuffer, TRUE)) {
		if (os_str_tol(tmpbuf, 0, 10) > 0) {
			pAd->slave_ctrl.disconnect_sta = FALSE;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
			"[DFS-SLAVE][%s] Skip sta disconnect on BH link down\n", __func__);
		}
	}
#endif /* DFS_SLAVE_SUPPORT */
#ifdef WIFI_MD_COEX_SUPPORT
	rtmp_read_coex_param_from_file(pAd, tmpbuf, pBuffer);
#endif
	if (RTMPGetKeyParameter("BarRetry", tmpbuf, PER_BSS_SIZE_2(pAd), pBuffer, TRUE)) {
		UINT8 limit_cnt;

		if (os_str_tol(tmpbuf, 0, 10)) {
			limit_cnt = os_str_tol(tmpbuf, 0, 10);
			pAd->BarRetryLimitCnt = limit_cnt;
		}
	}
	if (RTMPGetKeyParameter("Single_RNR", tmpbuf, PER_BSS_SIZE_2(pAd), pBuffer, TRUE)) {
		u8 single_rnr;

		if (os_str_tol(tmpbuf, 0, 10)) {
			single_rnr = os_str_tol(tmpbuf, 0, 10);
			pAd->CommonCfg.single_rnr = single_rnr;
		}
	}

	/*Channel Priority*/
	if (RTMPGetKeyParameter("Channel_priority", tmpbuf, 300, pBuffer, TRUE)) {
		/*Get the channel priority segment*/
		/*data struct: Channel_priority=36-48:1,52-64:3,10*/
		/*not defined means priority 0, priority 0 channel won't be selected,*/
		/*priority:0~14*/
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"[ch prio] begin parsing ch priority :%s\n", tmpbuf);

		ParseChPriority(pAd, tmpbuf);
	}

	/*Forbid_BW40*/
	if (RTMPGetKeyParameter("Forbid_BW40", tmpbuf, 30, pBuffer, TRUE)) {

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"[Forbid BW40] begin parsing Forbid_BW40 :%s\n", tmpbuf);

		ParseForbidBW40(pAd, tmpbuf);
	}

	} while (0);

	os_free_mem(tmpbuf);
	return NDIS_STATUS_SUCCESS;
}

#ifdef WSC_INCLUDED
void rtmp_read_wsc_user_parms(
	PWSC_CTRL pWscControl,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	INT		i = 0;
	RTMP_STRING *macptr = NULL;
	ULONG len;

	if (RTMPGetKeyParameter("WscManufacturer", tmpbuf, WSC_MANUFACTURE_LEN, buffer, TRUE))
	{
		len = strlen(tmpbuf);
		if (len > WSC_MANUFACTURE_LEN) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
				 "input parameter[WscManufacturer] incorrect\n");
			return;
		}

		NdisZeroMemory(pWscControl->RegData.SelfInfo.Manufacturer, WSC_MANUFACTURE_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.Manufacturer, tmpbuf, len);

		if (pWscControl->RegData.SelfInfo.Manufacturer[0] != 0x00)
			RTMP_SET_FLAG(pWscControl, 0x01);
	}

	/*WSC_User_ModelName*/
	if (RTMPGetKeyParameter("WscModelName", tmpbuf, WSC_MODELNAME_LEN, buffer, TRUE))
	{
		len = strlen(tmpbuf);
		if (len > WSC_MODELNAME_LEN) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
				 "input parameter[WscModelName] incorrect\n");
			return;
		}

		NdisZeroMemory(pWscControl->RegData.SelfInfo.ModelName, WSC_MODELNAME_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.ModelName, tmpbuf, len);

		if (pWscControl->RegData.SelfInfo.ModelName[0] != 0x00)
			RTMP_SET_FLAG(pWscControl, 0x02);
	}

	/*WSC_User_DeviceName*/
	if (RTMPGetKeyParameter("WscDeviceName", tmpbuf, 256, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
		{
			if (pWscControl->WpsApBand == i)
			{
				len = strlen(macptr);
				if (len > WSC_DEVICENAME_LEN) {
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						 "input parameter[WscDeviceName] incorrect\n"
						 );
					return;
				}

				NdisZeroMemory(pWscControl->RegData.SelfInfo.DeviceName, WSC_DEVICENAME_LEN);
				NdisMoveMemory(pWscControl->RegData.SelfInfo.DeviceName, macptr, len);

				if (pWscControl->RegData.SelfInfo.DeviceName[0] != 0x00)
					RTMP_SET_FLAG(pWscControl, 0x04);

				break;
			}
		}
	}

	/*WSC_User_ModelNumber*/
	if (RTMPGetKeyParameter("WscModelNumber", tmpbuf, WSC_MODELNUNBER_LEN, buffer, TRUE))
	{
		len = strlen(tmpbuf);
		if (len > WSC_MODELNUNBER_LEN) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
				 "input parameter[WscModelNumber] incorrect\n");
			return;
		}

		NdisZeroMemory(pWscControl->RegData.SelfInfo.ModelNumber, WSC_MODELNUNBER_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.ModelNumber, tmpbuf, len);

		if (pWscControl->RegData.SelfInfo.ModelNumber[0] != 0x00)
			RTMP_SET_FLAG(pWscControl, 0x08);
	}

	/*WSC_User_SerialNumber*/
	if (RTMPGetKeyParameter("WscSerialNumber", tmpbuf, WSC_SERIALNUNBER_LEN, buffer, TRUE))
	{
		len = strlen(tmpbuf);
		if (len > WSC_SERIALNUNBER_LEN) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
				 "input parameter[WscSerialNumber] incorrect\n");
			return;
		}

		NdisZeroMemory(pWscControl->RegData.SelfInfo.SerialNumber, WSC_SERIALNUNBER_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.SerialNumber, tmpbuf, len);

		if (pWscControl->RegData.SelfInfo.SerialNumber[0] != 0x00)
			RTMP_SET_FLAG(pWscControl, 0x10);
	}
}

void rtmp_read_wsc_user_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	PWSC_CTRL           pWscControl;
#ifdef WSC_AP_SUPPORT
	int i = 0;

	for (i = 0; i < MAX_MBSSID_NUM(pAd); i++) {
		pWscControl = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl;
		pWscControl->WpsApBand = PF_TO_BSS_IDX(pAd, i);
		rtmp_read_wsc_user_parms(pWscControl, tmpbuf, buffer);
	}

#ifdef APCLI_SUPPORT
	pWscControl = &pAd->StaCfg[0].wdev.WscControl;
	rtmp_read_wsc_user_parms(pWscControl, tmpbuf, buffer);
#endif /* APCLI_SUPPORT */
#endif /* WSC_AP_SUPPORT */
#ifdef WSC_STA_SUPPORT
	pWscControl = &pAd->StaCfg[0].wdev.WscControl;
	rtmp_read_wsc_user_parms(pWscControl, tmpbuf, buffer);
#endif /* WSC_STA_SUPPORT */
}
#endif/*WSC_INCLUDED*/

void rtmp_read_cp_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "begin -->\n");
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	/* for enable/disable */
	if (RTMPGetKeyParameter("CP_SUPPORT", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->cp_support =  os_str_tol(tmpbuf, 0, 10);
		pAd->cp_support_off_by_qos_map = TRUE;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"CP_SUPPORT --> %d\n", pAd->cp_support);
	}
}

void rtmp_read_multi_cli_nums_eap_th_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "begin -->\n");
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	/* for enable/disable */
	if (RTMPGetKeyParameter("MCLI_NUMS_EAP_TH", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->multi_cli_nums_eap_th =  os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"MCLI_NUMS_EAP_TH --> %d\n", pAd->multi_cli_nums_eap_th);
	}
}

void rtmp_read_retry_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	UINT32		rv = 0;
	INT		i = 0, j = 0;
	CHAR		*ptok = NULL;
	CHAR		buf[8] = {0};
	UINT		qidx = TxQ_IDX_AC33 + 1, limit = 32, band_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "begin -->\n");
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("RetryLimit", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ":"); ptok && (i < WMM_NUM);
			ptok = rstrtok(NULL, ":"), i++) {
			qidx = TxQ_IDX_AC33 + 1;
			limit = 32;
			band_idx = 0;
			memset(buf, 0, 8);

			for (j = 0; j < 8; j++) {
				buf[j] = toupper(*(ptok+j));
			}

			if (strncmp(buf, "RTS", 3) == 0) {
				band_idx = 0;
				rv = sscanf(buf, "RTS%u-%u", &band_idx, &limit);
				if (!rv) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_WARN,
							"Format Error!\n");
					return;
				}
				pAd->rts_retrylimit = limit;
			}

			if (strncmp(buf, "AC", 2) == 0) {
				UINT ac = 0;
				rv = sscanf(buf, "AC%u-%u", &ac, &limit);
				if (!rv) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_WARN,
							"Format Error!\n");
					return;
				}
				qidx = (ac/10)*4 + ac%10;
				pAd->retrylimit[qidx] = limit;
			}

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"(band=%u qidx=%u,retrylimit=%u)\n", band_idx, qidx, limit);
		}
	}
	if (RTMPGetKeyParameter("RetryLimitWcid", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		long value;

		if (kstrtol(tmpbuf, 10, &value))
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_NOTICE,
				"Band%d: RetryLimitWcid error input(=%s)\n",
				band_idx,
				tmpbuf);
		else {
			if (value < hc_get_chip_wtbl_max_num(pAd))
				pAd->retrylimit_wcid = (UINT16)value;
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_NOTICE,
				"Band%d: RetryLimitWcid = %d\n",
				band_idx, pAd->retrylimit_wcid);
		}
	}
}

#ifdef CONFIG_6G_AFC_SUPPORT
VOID rtmp_read_afc_device_type_param(RTMP_ADAPTER *pAd, RTMP_STRING *buffer)
{
	RTMP_STRING *ptr = NULL;
	long DeviceType = 0;

	ptr = rstrtok(buffer, ";");

	if (ptr) {
		if (kstrtol(ptr, 10, &DeviceType))
			MTWF_PRINT("%s: Error input (=%s)\n", __func__, ptr);
		else {
			if (DeviceType >= AFC_MAX_DEVICE_TYPE)
				MTWF_PRINT("Invalid afc device parameter,\n"
				" enter 1 for standard device else 0!\n");
			else
				pAd->CommonCfg.AfcDeviceType = (UINT8)DeviceType;
		}
	} else
		MTWF_PRINT("%s: Error input !!\n", __func__);

}

VOID rtmp_read_afc_dup_en_param(RTMP_ADAPTER *pAd, RTMP_STRING *buffer)
{
	RTMP_STRING *ptr;
	long DupEn = 0;

	ptr = rstrtok(buffer, ";");
	if (ptr) {
		if (kstrtol(ptr, 10, &DupEn))
			MTWF_PRINT("%s: Error input (=%s)\n", __func__, ptr);
		else
			pAd->CommonCfg.AfcSpBwDup = (UINT8)DupEn;
	} else
		MTWF_PRINT("%s: Error input !!\n", __func__);
}

VOID rtmp_read_afc_spectrum_type_param(RTMP_ADAPTER *pAd, RTMP_STRING *buffer)
{
	RTMP_STRING *ptr = NULL;
	long SpectrumType = 0;

	ptr = rstrtok(buffer, ";");

	if (ptr) {
		if (kstrtol(ptr, 10, &SpectrumType))
			MTWF_PRINT("%s: Error input (=%s)\n", __func__, ptr);
		else
			pAd->CommonCfg.AfcSpectrumType = (UINT8)SpectrumType;
	} else
		MTWF_PRINT("%s: Error input !!\n", __func__);
}

VOID rtmp_read_afc_freq_range_param(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf)
{
	RTMP_STRING *macptr = NULL;
	INT i = 0, j = 0;
	long NumOfFreqRange = 0;
	long Frequency = 0;
	long ret;

	for (i = 0, macptr = rstrtok(tmpbuf, ":"); macptr; macptr = rstrtok(NULL, ":"), i++) {
		if (i == 0) {
			if (kstrtol(macptr, 10, &NumOfFreqRange) == 0) {
				afc_update_freq_num(pAd, NumOfFreqRange);
				continue;
			} else {
				MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
				return;
			}

		}

		ret = kstrtol(macptr, 10, &Frequency);
		if (ret == 0)
			afc_update_freq_range(i, j, (UINT16)Frequency);
		else {
			MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
			return;
		}
		if (i % 2 == 0)
			j++;
	}
}

VOID rtmp_read_acs_after_afc_param(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf)
{

	RTMP_STRING *ptr = NULL;
	long Enable = 0;

	ptr = rstrtok(tmpbuf, ";");

	if (ptr) {
		if (kstrtol(ptr, 10, &Enable))
			MTWF_PRINT("%s: Error input (=%s)\n", __func__, ptr);
		else
			pAd->CommonCfg.AcsAfterAfc = (UINT8)Enable;
	} else
		MTWF_PRINT("%s: Error input !!\n", __func__);
}

VOID rtmp_read_afc_opclass_131_param(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf)
{
	RTMP_STRING *macptr = NULL;
	INT i = 0, count = 0;
	long Enable = 0;
	long Channel = 0;
	long ret;

	for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		if (i == 0) {
			if (kstrtol(macptr, 10, &Enable) == 0) {
				afc_update_op_class_en(AFC_OP_CLASS_131, (UINT8)Enable);
				continue;
			} else {
				MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
				return;
			}
		}

		ret = kstrtol(macptr, 10, &Channel);
		if (ret == 0) {
			afc_update_op_class_channel(AFC_OP_CLASS_131, (UINT8)count, (UINT8)Channel);
			count++;
		} else {
			MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
			return;
		}
	}
	afc_update_op_class_channel_count(AFC_OP_CLASS_131, (UINT8)count);
}

VOID rtmp_read_afc_opclass_132_param(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf)
{
	RTMP_STRING *macptr = NULL;
	INT i = 0, count = 0;
	long Enable = 0;
	long Channel = 0;
	long ret;

	for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		if (i == 0) {
			if (kstrtol(macptr, 10, &Enable) == 0) {
				afc_update_op_class_en(AFC_OP_CLASS_132, (UINT8)Enable);
				continue;
			} else {
				MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
				return;
			}
		}

		ret = kstrtol(macptr, 10, &Channel);
		if (ret == 0) {
			afc_update_op_class_channel(AFC_OP_CLASS_132, (UINT8)count, (UINT8)Channel);
			count++;
		} else {
			MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
			return;
		}

	}
	afc_update_op_class_channel_count(AFC_OP_CLASS_132, (UINT8)count);
}

VOID rtmp_read_afc_opclass_133_param(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf)
{
	RTMP_STRING *macptr = NULL;
	INT i = 0, count = 0;
	long Enable = 0;
	long Channel = 0;
	long ret;

	for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		if (i == 0) {
			if (kstrtol(macptr, 10, &Enable) == 0) {
				afc_update_op_class_en(AFC_OP_CLASS_133, (UINT8)Enable);
				continue;
			} else {
				MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
				return;
			}
		}

		ret = kstrtol(macptr, 10, &Channel);
		if (ret == 0) {
			afc_update_op_class_channel(AFC_OP_CLASS_133, (UINT8)count, (UINT8)Channel);
			count++;
		} else {
			MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
			return;
		}

	}
	afc_update_op_class_channel_count(AFC_OP_CLASS_133, (UINT8)count);
}

VOID rtmp_read_afc_opclass_134_param(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf)
{
	RTMP_STRING *macptr = NULL;
	INT i = 0, count = 0;
	long Enable = 0;
	long Channel = 0;
	long ret;

	for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		if (i == 0) {
			if (kstrtol(macptr, 10, &Enable) == 0) {
				afc_update_op_class_en(AFC_OP_CLASS_134, (UINT8)Enable);
				continue;
			} else {
				MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
				return;
			}
		}

		ret = kstrtol(macptr, 10, &Channel);
		if (ret == 0) {
			afc_update_op_class_channel(AFC_OP_CLASS_134, (UINT8)count, (UINT8)Channel);
			count++;
		} else {
			MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
			return;
		}
	}
	afc_update_op_class_channel_count(AFC_OP_CLASS_134, (UINT8)count);
}

VOID rtmp_read_afc_opclass_135_param(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf)
{
	RTMP_STRING *macptr = NULL;
	INT i = 0, count = 0;
	long Enable = 0;
	long Channel = 0;
	long ret;

	for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		if (i == 0) {
			if (kstrtol(macptr, 10, &Enable) == 0) {
				afc_update_op_class_en(AFC_OP_CLASS_135, (UINT8)Enable);
				continue;
			} else {
				MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
				return;
			}
		}

		ret = kstrtol(macptr, 10, &Channel);
		if (ret == 0) {
			afc_update_op_class_channel(AFC_OP_CLASS_135, (UINT8)count, (UINT8)Channel);
			count++;
		} else {
			MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
			return;
		}
	}
	afc_update_op_class_channel_count(AFC_OP_CLASS_135, (UINT8)count);
}

VOID rtmp_read_afc_opclass_136_param(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf)
{
	RTMP_STRING *macptr = NULL;
	INT i = 0, count = 0;
	long Enable = 0;
	long Channel = 0;
	long ret;

	for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		if (i == 0) {
			if (kstrtol(macptr, 10, &Enable) == 0) {
				afc_update_op_class_en(AFC_OP_CLASS_136, (UINT8)Enable);
				continue;
			} else {
				MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
				return;
			}
		}

		ret = kstrtol(macptr, 10, &Channel);
		if (ret  == 0) {
			afc_update_op_class_channel(AFC_OP_CLASS_136, (UINT8)count, (UINT8)Channel);
			count++;
		} else {
			MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
			return;
		}
	}
	afc_update_op_class_channel_count(AFC_OP_CLASS_136, (UINT8)count);
}

VOID rtmp_read_afc_opclass_137_param(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf)
{
	RTMP_STRING *macptr = NULL;
	INT i = 0, count = 0;
	long Enable = 0;
	long Channel = 0;
	long ret;

	for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		if (i == 0) {
			if (kstrtol(macptr, 10, &Enable) == 0) {
				afc_update_op_class_en(AFC_OP_CLASS_137, (UINT8)Enable);
				continue;
			} else {
				MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
				return;
			}
		}

		ret = kstrtol(macptr, 10, &Channel);
		if (ret == 0) {
			afc_update_op_class_channel(AFC_OP_CLASS_137, (UINT8)count, (UINT8)Channel);
			count++;
		} else {
			MTWF_PRINT("%s: Error input (=%s)\n", __func__, macptr);
			return;
		}
	}
	afc_update_op_class_channel_count(AFC_OP_CLASS_137, (UINT8)count);
}

VOID rtmp_read_afc_DGgain_offset_param(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf)
{
	RTMP_STRING *ptr = NULL;
	long Enable = 0;

	ptr = rstrtok(tmpbuf, ";");

	if (ptr) {
		if (kstrtol(ptr, 10, &Enable))
			MTWF_PRINT("%s: Error input (=%s)\n", __func__, ptr);
		else
			pAd->CommonCfg.AfcDGgainOffset = (INT8)Enable;
	} else
		MTWF_PRINT("%s: Error input !!\n", __func__);
}

#endif /*CONFIG_6G_AFC_SUPPORT*/

#ifdef PEAK_ENHANCE
void read_peak_adjust_cfg_from_file(
	IN PRTMP_ADAPTER ad,
	IN char *tmpbuf,
	IN char *pBuffer)
{
	long lvalue;
	UINT8 band_idx = hc_get_hw_band_idx(ad);
	struct peak_enhance_ctrl *ctrl = &ad->CommonCfg.peek_enhance;

	if (band_idx >= CFG_WIFI_RAM_BAND_NUM)
		return;

	if (RTMPGetKeyParameter("EnAdjust", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 16, &lvalue))
			MTWF_PRINT("%s: Error input EnAdjust(=%s)\n", __func__, tmpbuf);
		else {
			ctrl->enable_adjust = lvalue;
			MTWF_PRINT("%s:[band%d] enable_adjust=0x%x\n",
				__func__, band_idx, ctrl->enable_adjust);
		}
	}

	if (RTMPGetKeyParameter("TxCountTh", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 10, &lvalue))
			MTWF_PRINT("%s: Error input TxCountTh(=%s)\n", __func__, tmpbuf);
		else {
			ctrl->tx_count_th[band_idx] = lvalue;
			MTWF_PRINT("%s:[band%d] tx_count_th=%d\n",
				__func__, band_idx, ctrl->tx_count_th[band_idx]);
		}
	}

	if (RTMPGetKeyParameter("TxPktSizeTh", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 10, &lvalue))
			MTWF_PRINT("%s: Error input TxPktSizeTh(=%s)\n", __func__, tmpbuf);
		else {
			ctrl->avg_tx_pkt_len = lvalue;
			MTWF_PRINT("%s:[band%d] avg_tx_pkt_len=%d\n",
				__func__, band_idx, ctrl->avg_tx_pkt_len);
		}
	}
}
#endif /* PEAK_ENHANCE */

NDIS_STATUS rtmp_read_physical_device_parms(
	void *ph_dev_obj,
	RTMP_STRING *pBuffer)
{
	struct physical_device *ph_dev = (struct physical_device *)ph_dev_obj;
	RTMP_STRING *tmpbuf;
	long lvalue = 0;
	int i, ret;
	char name[L2PROFILE_PATH_LEN] = {0};
	struct _RTMP_CHIP_CAP *chip_cap = PD_GET_CHIP_CAP_PTR(ph_dev);

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (tmpbuf == NULL)
		return NDIS_STATUS_FAILURE;

	/* Read profile path */
	for (i = 0; i < CFG_WIFI_RAM_BAND_NUM; i++) {
		ret = snprintf(&name[0], L2PROFILE_PATH_LEN, "BN%d_profile_path", i);
		if (os_snprintf_error(L2PROFILE_PATH_LEN, ret) != 0) {
			MTWF_PRINT("%s(%d):snprintf error, skip profile path setting. check!\n",
				__func__, __LINE__);
			break;
		}
		if (RTMPGetKeyParameter(&name[0], tmpbuf, L2PROFILE_PATH_LEN, pBuffer, TRUE)) {
			if (ph_dev->profile_path[i])
				os_free_mem(ph_dev->profile_path[i]);
			os_alloc_mem(NULL, (UCHAR **)&ph_dev->profile_path[i], L2PROFILE_PATH_LEN);
			if (ph_dev->profile_path[i])
				ret = snprintf(ph_dev->profile_path[i],
					L2PROFILE_PATH_LEN, "%s", tmpbuf);
			if (os_snprintf_error(L2PROFILE_PATH_LEN, ret) != 0) {
				MTWF_PRINT(
					"%s(%d):snprintf error, skip profile path setting. check!\n",
					__func__, __LINE__);
				break;
			}
			MTWF_PRINT("BN%d_profile_path: %s\n", i, ph_dev->profile_path[i]);
		}
	}

	/* E2pAccessMode */
	if (RTMPGetKeyParameter("E2pAccessMode", tmpbuf, 25, pBuffer, TRUE)) {
		UCHAR value = (UCHAR) os_str_tol(tmpbuf, 0, 10);

		PD_SET_E2P_ACCESS_MODE(ph_dev, value);
		MTWF_PRINT("E2pAccessMode=%d\n", ph_dev->E2pAccessMode);
	}

#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
	glb_npu_en_node = 1;
	if (RTMPGetKeyParameter("NPU_EN", tmpbuf, 25, pBuffer, TRUE)) {
		UCHAR NPU_EN = (UCHAR) os_str_tol(tmpbuf, 0, 10);

		MTWF_PRINT("NPU_EN=%d\n", NPU_EN);
		glb_npu_en_node = NPU_EN;
	}
#endif

	if (RTMPGetKeyParameter("MacAddress", tmpbuf, 25, pBuffer, TRUE)) {
		INT i, mac_len;
		UCHAR Address[MAC_ADDR_LEN]; /* User changed MAC address */

		/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
		mac_len = strlen(tmpbuf);
		if (mac_len != 17)
			MTWF_PRINT("invalid length (%d)\n", mac_len);
		else if (strcmp(tmpbuf, "00:00:00:00:00:00") == 0)
			MTWF_PRINT("invalid mac setting\n");
		else {
			for (i = 0; i < MAC_ADDR_LEN; i++) {
				AtoH((tmpbuf + (i*3)), &Address[i], 1);
			}
			PD_SET_ADMIN_ADDRESS(ph_dev, &Address);
		}
	}

	if (RTMPGetKeyParameter("MacAddress1", tmpbuf, 25, pBuffer, TRUE)) {
		INT i, mac_len;
		UCHAR Address[MAC_ADDR_LEN]; /* User changed MAC address */

		/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
		mac_len = strlen(tmpbuf);
		if (mac_len != 17)
			MTWF_PRINT("invalid length (%d)\n", mac_len);
		else if (strcmp(tmpbuf, "00:00:00:00:00:00") == 0)
			MTWF_PRINT("invalid mac setting\n");
		else {
			for (i = 0; i < MAC_ADDR_LEN; i++)
				AtoH((tmpbuf + (i*3)), &Address[i], 1);

			PD_SET_ADMIN_ADDRESS1(ph_dev, &Address);
		}
	}

	if (RTMPGetKeyParameter("MacAddress2", tmpbuf, 25, pBuffer, TRUE)) {
		INT i, mac_len;
		UCHAR Address[MAC_ADDR_LEN]; /* User changed MAC address */

		/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
		mac_len = strlen(tmpbuf);
		if (mac_len != 17)
			MTWF_PRINT("invalid length (%d)\n", mac_len);
		else if (strcmp(tmpbuf, "00:00:00:00:00:00") == 0)
			MTWF_PRINT("invalid mac setting\n");
		else {
			for (i = 0; i < MAC_ADDR_LEN; i++)
				AtoH((tmpbuf + (i*3)), &Address[i], 1);

			PD_SET_ADMIN_ADDRESS2(ph_dev, &Address);
		}
	}


	/*
	 * Device local max tx power
	 * Range -64 dBm to 63 dBm with a 0.5 dB step.
	 */
	if (RTMPGetKeyParameter("LocalMaxTxPwrBW20", tmpbuf, 25, pBuffer, TRUE)) {
		u8 value = (u8) os_str_tol(tmpbuf, 0, 10);

		ph_dev->txpwr_cfg.local_max_txpwr_bw20 = value;
		MTWF_PRINT("LocalMaxTxPwrBW20=%d\n", ph_dev->txpwr_cfg.local_max_txpwr_bw20);
	}
	if (RTMPGetKeyParameter("LocalMaxTxPwrBW40", tmpbuf, 25, pBuffer, TRUE)) {
		u8 value = (u8) os_str_tol(tmpbuf, 0, 10);

		ph_dev->txpwr_cfg.local_max_txpwr_bw40 = value;
		MTWF_PRINT("LocalMaxTxPwrBW40=%d\n", ph_dev->txpwr_cfg.local_max_txpwr_bw40);
	}
	if (RTMPGetKeyParameter("LocalMaxTxPwrBW80", tmpbuf, 25, pBuffer, TRUE)) {
		u8 value = (u8) os_str_tol(tmpbuf, 0, 10);

		ph_dev->txpwr_cfg.local_max_txpwr_bw80 = value;
		MTWF_PRINT("LocalMaxTxPwrBW80=%d\n", ph_dev->txpwr_cfg.local_max_txpwr_bw80);
	}
	if (RTMPGetKeyParameter("LocalMaxTxPwrBW160", tmpbuf, 25, pBuffer, TRUE)) {
		u8 value = (u8) os_str_tol(tmpbuf, 0, 10);

		ph_dev->txpwr_cfg.local_max_txpwr_bw160 = value;
		MTWF_PRINT("LocalMaxTxPwrBW160=%d\n", ph_dev->txpwr_cfg.local_max_txpwr_bw160);
	}
	if (RTMPGetKeyParameter("LocalMaxTxPwrBW320", tmpbuf, 25, pBuffer, TRUE)) {
		u8 value = (u8) os_str_tol(tmpbuf, 0, 10);

		ph_dev->txpwr_cfg.local_max_txpwr_bw320 = value;
		MTWF_PRINT("LocalMaxTxPwrBW320=%d\n", ph_dev->txpwr_cfg.local_max_txpwr_bw320);
	}

	/*WHNAT*/
#ifdef WHNAT_SUPPORT
	if (RTMPGetKeyParameter("WHNAT", tmpbuf, 10, pBuffer, TRUE)) {
		ph_dev->whnat_en = (u8)os_str_tol(tmpbuf, 0, 10);
		MTWF_PRINT("WHNAT=%d\n", ph_dev->whnat_en);
	}
#endif /*WHNAT_SUPPORT*/
#ifdef PEAK_ENHANCE
	if (RTMPGetKeyParameter("RxEnhanceEn", tmpbuf, 10, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 10, &lvalue))
			MTWF_PRINT("%s: Error input RxEnhanceEn(=%s)\n", __func__, tmpbuf);
		else {
			if (lvalue == 0)
				ph_dev->peak_rx_ctrl.enable = FALSE;
			else
				ph_dev->peak_rx_ctrl.enable = TRUE;
			MTWF_PRINT("%s: RxEnhanceEn=%d\n", __func__, ph_dev->peak_rx_ctrl.enable);
		}
	}
#endif /* PEAK_ENHANCE */

#ifdef CONFIG_3_WIRE_SUPPORT
	if (RTMPGetKeyParameter("ThreeWireFunctionEnable", tmpbuf, 10, pBuffer, TRUE)) {
		ph_dev->threeWire_function_enable = (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_PRINT("%s: ThreeWireFunctionEnable=%d\n", __func__, ph_dev->threeWire_function_enable);
	}
#endif

#ifdef CFG_RED_SUPPORT
	read_red_parms_from_file(ph_dev, tmpbuf, pBuffer);
#endif /* CFG_RED_SUPPORT */

	/* SW Band Number */
	if (RTMPGetKeyParameter("BandNum", tmpbuf, 25, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 10, &lvalue))
			MTWF_PRINT("%s: Error input BandNum(=%s)\n", __func__, tmpbuf);
		else {
			if (!lvalue || lvalue >= ph_dev->chip_cap.hw_band_num)
				MTWF_PRINT("%s: Invalid BandNum(=%ld)\n", __func__, lvalue);
			else
				PD_SET_BAND_NUM(ph_dev, lvalue);
			MTWF_PRINT("%s: band_num=%d\n", __func__, PD_GET_BAND_NUM(ph_dev));
		}
	}

	/* Enabled Band Define */
	if (RTMPGetKeyParameter("BandEnabled", tmpbuf, 25, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 16, &lvalue))
			MTWF_PRINT("%s: Error input BandEnabled(=%s)\n", __func__, tmpbuf);
		else {
			PD_SET_BAND_EN(ph_dev, (u8)lvalue);
			MTWF_PRINT("%s: BandEnabled=0x%x\n", __func__, PD_GET_BAND_EN(ph_dev));
		}
	}

#ifdef DOT11_EHT_BE
	/* MLO STR BitMap */
	if (RTMPGetKeyParameter("MloStrBitMap", tmpbuf, 25, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 16, &lvalue))
			MTWF_PRINT("%s: Error input MloStrBitMap(=%s)\n", __func__, tmpbuf);
		else {
			PD_SET_MLO_CONFIG_OP_SET(ph_dev, TRUE);
			PD_SET_MLO_STR_BITMAP(ph_dev, (u8)lvalue);
			MTWF_PRINT("%s: MloStrBitMap=0x%x\n",
				__func__, PD_GET_MLO_STR_BITMAP(ph_dev));
		}
	}
	/* MLO Sync Tx Enable */
	if (RTMPGetKeyParameter("MloSyncTx", tmpbuf, 25, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 10, &lvalue))
			MTWF_PRINT("%s: Error input MloSyncTx(=%s)\n", __func__, tmpbuf);
		else {
			PD_SET_MLO_CONFIG_OP_SET(ph_dev, TRUE);
			PD_SET_MLO_SYNC_TX_ENABLE(ph_dev, (u8)lvalue);
			MTWF_PRINT("%s: MloSyncTx=0x%x\n",
				__func__, PD_GET_MLO_SYNC_TX_ENABLE(ph_dev));
		}
	}
	/* MLO V1 */
	if (RTMPGetKeyParameter("MloV1", tmpbuf, 25, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 10, &lvalue))
			MTWF_PRINT("%s: Error input MloV1(=%s)\n", __func__, tmpbuf);
		else {
			if (lvalue == 1)
				PD_MTK_MLO_V1_ENABLE(ph_dev);
			else
				PD_MTK_MLO_V1_DISABLE(ph_dev);

			MTWF_PRINT("%s: MloV1=0x%x\n",
				__func__, PD_GET_MLO_V1_ENABLE(ph_dev));
		}
	}
#endif /* DOT11_EHT_BE */

	/* Duplicate Wtbl Number */
	if (RTMPGetKeyParameter("WtblDupNum", tmpbuf, 25, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 10, &lvalue))
			MTWF_PRINT("%s: Error input WtblDupNum(=%s)\n",
				__func__, tmpbuf);
		else {
			chip_cap->WtblDupNum = lvalue;
			MTWF_PRINT("%s: WtblDupNum=%d\n",
				__func__, chip_cap->WtblDupNum);

		}
	}

	if (RTMPGetKeyParameter("TestModeEn", tmpbuf, 10, pBuffer, TRUE)) {
		/* 0 : NormalMode; 1 : TestMode */
		ph_dev->TestModeEn = (u8)os_str_tol(tmpbuf, 0, 10);
		MTWF_PRINT("TestModeEn=%d\n", ph_dev->TestModeEn);

	}

#ifdef MAC_ADDR_ADJACENT_CHK
	/* Force MAC Addr Adj */
	if (RTMPGetKeyParameter("MacAddrAdj", tmpbuf, 32, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 10, &lvalue))
			MTWF_PRINT("%s: Error input MacAddrAdj(=%s)\n",
				__func__, tmpbuf);
		else {
			if (lvalue == 0)
				PD_SET_MAC_ADDR_ADJ(ph_dev, FALSE);
			else
				PD_SET_MAC_ADDR_ADJ(ph_dev, TRUE);
			MTWF_PRINT("%s: MacAddrAdj=0x%x\n",
				__func__, PD_GET_MAC_ADDR_ADJ(ph_dev));
		}
	}
#endif /* MAC_ADDR_ADJACENT_CHK */

#ifdef DOT11V_MBSSID_SUPPORT
	/* Duplicate 11v bcn/probe rsp in host instead of wm */
	if (RTMPGetKeyParameter("Dot11vHostDupBcn", tmpbuf, 32, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 10, &lvalue))
			MTWF_PRINT("%s: Error input Dot11vHostDupBcn(=%s)\n",
				__func__, tmpbuf);
		else {
			if (lvalue == 0)
				PD_SET_11V_BCN_DUP(ph_dev, FALSE);
			else
				PD_SET_11V_BCN_DUP(ph_dev, TRUE);
			MTWF_PRINT("%s: Dot11vHostDupBcn=0x%x\n",
				__func__, PD_GET_11V_BCN_DUP(ph_dev));
		}
	}
#endif /* DOT11V_MBSSID_SUPPORT */

#ifdef SW_CONNECT_SUPPORT
	if (RTMPGetKeyParameter("SwStaEnable", tmpbuf, 25, pBuffer, TRUE)) {
		/* 0 : off; 1 : on */
		if (kstrtol(tmpbuf, 10, &lvalue))
			MTWF_PRINT("%s: Error input SwStaEnable(=%s)\n", __func__, tmpbuf);
		else {
			if (lvalue == 1)
				ph_dev->bSwSta = TRUE;
			else
				ph_dev->bSwSta = FALSE;

			MTWF_PRINT("SwStaEnable=%s\n", (ph_dev->bSwSta == TRUE) ? "TRUE" : "FALSE");
		}
	}

#endif /* SW_CONNECT_SUPPORT */


	/* RRO step one timeout */
	if (RTMPGetKeyParameter("RroStepOneTimeout", tmpbuf, 25, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 10, &lvalue))
			MTWF_PRINT("%s: Error input RRO_STEP_ONE_TIMEOUT(=%s)\n",
				__func__, tmpbuf);
		else {
			if (lvalue < 32 || lvalue > 16384)
				MTWF_PRINT("%s: Error input RRO_STEP_ONE_TIMEOUT(=%s)\n",
					__func__, tmpbuf);
			else {
				chip_cap->step_one_timeout = lvalue;
				PD_SET_RRO_TIMEOUT_PROFILE_SET(ph_dev, TRUE);
			}
			MTWF_PRINT("%s: step_one_timeout=%d\n",
				__func__, chip_cap->step_one_timeout);

		}
	}
	/* RRO flush all timeout */
	if (RTMPGetKeyParameter("RroFlushAllTimeout", tmpbuf, 25, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 10, &lvalue))
			MTWF_PRINT("%s: Error input RRO_FLUSH_ALL_TIMEOUT(=%s)\n",
				__func__, tmpbuf);
		else {
			if (lvalue < 32 || lvalue > 32768)
				MTWF_PRINT("%s: Error input RRO_FLUSH_ALL_TIMEOUT(=%s)\n",
					__func__, tmpbuf);
			else {
				chip_cap->flush_all_timeout = lvalue;
				PD_SET_RRO_TIMEOUT_PROFILE_SET(ph_dev, TRUE);
			}
			MTWF_PRINT("%s: flush_all_timeout=%d\n",
				__func__, chip_cap->flush_all_timeout);

		}
	}
	/* RRO RSS RING MAPPING 0 */
	if (RTMPGetKeyParameter("RSSMAP0", tmpbuf, 25, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 16, &lvalue))
			MTWF_PRINT("%s: Error input RSSMAP0(=%s)\n", __func__, tmpbuf);
		else {
			chip_cap->rss_map.map0 = lvalue;
			MTWF_PRINT("%s: RSSMAP0=0x%x\n",
				__func__, chip_cap->rss_map.map0);
		}
	}
	/* RRO RSS RING MAPPING 1 */
	if (RTMPGetKeyParameter("RSSMAP1", tmpbuf, 25, pBuffer, TRUE)) {
		if (kstrtol(tmpbuf, 16, &lvalue))
			MTWF_PRINT("%s: Error input RSSMAP1(=%s)\n", __func__, tmpbuf);
		else {
			chip_cap->rss_map.map1 = lvalue;
			MTWF_PRINT("%s: RSSMAP1=0x%x\n",
				__func__, chip_cap->rss_map.map1);
		}
	}

	/* EDCCARegion */
	if (RTMPGetKeyParameter("EDCCARegion", tmpbuf, 10, pBuffer, TRUE)) {
		ph_dev->edcca_region = (u8)os_str_tol(tmpbuf, 0, 10);
		MTWF_PRINT("%s: EDCCARegion:%d\n", __func__, ph_dev->edcca_region);
	}

	os_free_mem(tmpbuf);
	return NDIS_STATUS_SUCCESS;
}

#ifdef FW_LOG_DUMP
void rtmp_read_fw_log_dump_parms_from_file(RTMP_ADAPTER *pAd, CHAR *tmpbuf, CHAR *buffer)
{
	UINT32 ip_addr;

	if (RTMPGetKeyParameter("fwlogserverip", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		if (rtinet_aton(tmpbuf, &ip_addr)) {
			pAd->physical_dev->fw_log_ctrl.fw_log_server_ip = ip_addr;
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"fw_log_server_ip=%s(%x)\n",
				tmpbuf, pAd->physical_dev->fw_log_ctrl.fw_log_server_ip);
		}
	}

	if (RTMPGetKeyParameter("fwlogservermac", tmpbuf, 25, buffer, TRUE)) {
		INT	i, mac_len;

		mac_len = strlen(tmpbuf);

		if (mac_len != 17) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"invalid length (%d)\n", mac_len);
			return;
		}

		if (strcmp(tmpbuf, "00:00:00:00:00:00") == 0) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"invalid mac setting\n");
			return;
		}

		for (i = 0; i < MAC_ADDR_LEN; i++) {
			AtoH(tmpbuf, &pAd->physical_dev->fw_log_ctrl.fw_log_server_mac[i], 1);
			tmpbuf = tmpbuf + 3;
		}
	}

	if (RTMPGetKeyParameter("fwlog", tmpbuf, 10, buffer, TRUE)) {
		RTMP_STRING *arg = pAd->physical_dev->fw_log_ctrl.fw_log_arg;
		INT	arg_len;

		arg_len = strlen(tmpbuf);
		if (arg_len)
			strlcpy(arg, tmpbuf, sizeof(arg_len) + 1);
	}
}

#endif /* FW_LOG_DUMP */

NDIS_STATUS	RTMPReadParametersHook(RTMP_ADAPTER *pAd)
{
	RTMP_STRING *src;
	INT retval = NDIS_STATUS_FAILURE;
	UINT32 buf_size;
	RTMP_STRING *buffer = NULL;

	src = get_dev_l2profile(pAd);

	if (!src || *src == '\0') {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			"l2profile not declared!\n");
		return retval;
	}

	os_load_code_from_bin(pAd, (unsigned char **)&buffer, src, &buf_size);
	if (buffer == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			"File %s read failed!\n", src);
		return retval;
	}

	retval = RTMPSetProfileParameters(pAd, buffer);


	os_free_mem(buffer);

	return retval;
}

/*Used for pre-read user configuration before system ready*/
NDIS_STATUS	RTMPPreReadProfile(RTMP_ADAPTER *pAd)
{
	RTMP_STRING *src;
	INT retval = NDIS_STATUS_FAILURE;
	UINT32 buf_size;
	RTMP_STRING *buffer = NULL;

	src = get_dev_l2profile(pAd);

	if (!src || *src == '\0') {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			"l2profile not declared!\n");
		return retval;
	}

	os_load_code_from_bin(pAd, (unsigned char **)&buffer, src, &buf_size);
	if (buffer == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			"File %s read failed!\n", src);
		return retval;
	}

	retval = RTMPSetPreProfileParameters(pAd, buffer);

	os_free_mem(buffer);

	return retval;
}

#ifdef WDS_SUPPORT
VOID AP_WDS_KeyNameMakeUp(
	IN	RTMP_STRING *pKey,
	IN	UINT32						KeyMaxSize,
	IN	INT							KeyId)
{
	INT ret;

	ret = snprintf(pKey, KeyMaxSize, "Wds%dKey", KeyId);
	if (os_snprintf_error(KeyMaxSize, ret) != 0)
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			"%s: snprintf error\n", __func__);
}
#endif /* WDS_SUPPORT */

/*Used for pre-read user configuration before system ready*/
NDIS_STATUS load_physical_device_profile(void *ph_dev_obj)
{
	RTMP_STRING *src = NULL;
	INT retval = NDIS_STATUS_FAILURE;
	UINT32 buf_size = MAX_INI_BUFFER_SIZE;
	RTMP_STRING *buffer = NULL;

	if (!ph_dev_obj)
		return retval;

	src = get_dev_profile_path(ph_dev_obj);

	if (!src || *src == '\0') {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			"device profile not declared!\n");
		return NDIS_STATUS_FAILURE;
	}

	os_load_code_from_bin(NULL, (unsigned char **)&buffer, src, &buf_size);
	if (buffer == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
			"File %s read failed!\n", src);
		return NDIS_STATUS_FAILURE;
	}

	rtmp_read_physical_device_parms(ph_dev_obj, buffer);
	retval = NDIS_STATUS_SUCCESS;

	os_free_mem(buffer);

	return retval;
}

#endif /* !CONFIG_PROFILE_OFF */

