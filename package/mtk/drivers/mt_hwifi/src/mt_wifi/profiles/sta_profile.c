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
/*
 ***************************************************************************
 ***************************************************************************

	Module Name:
	sta_profile.c

	Abstract:

*/
#include "rt_config.h"

#ifndef CONFIG_PROFILE_OFF

#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11_EHT_BE


void reset_sta_eth_mld_cfg_addr(PRTMP_ADAPTER pAd)
{
	u32 idx = 0;
	PSTA_ADMIN_CONFIG psta_cfg = NULL;

	/*clear the mld cfg infos*/
	for (idx = 0; idx < pAd->MaxMSTANum; idx++) {
		psta_cfg = &pAd->StaCfg[idx];
		psta_cfg->pf_mld_addr_en = FALSE;
		os_zero_mem(psta_cfg->pf_mld_addr, MAC_ADDR_LEN);
	}
}

void read_sta_eht_config_from_file(
	IN PRTMP_ADAPTER pAd,
	IN char *tmpbuf,
	IN char *pBuffer)
{
	char *macptr;
	UINT32 idx;
	long value;
	struct wifi_dev *wdev = NULL;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	if (RTMPGetKeyParameter("EHT_StaNsepPriAccess", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - Band%d(STA%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - Band%d(STA%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_nsep_priority_access(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_StaOmCtrl", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - Band%d(STA%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - Band%d(STA%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_eht_om_ctrl(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_StaTxopSharing", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - Band%d(STA%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - Band%d(STA%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_eht_txop_sharing_trigger(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_StaRestrictedTwt", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - Band%d(STA%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - Band%d(STA%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_eht_restricted_twt(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_StaScsTraffic", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - Band%d(STA%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - Band%d(STA%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_eht_scs_traffic(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_StaEmlsr_mr", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - Band%d(STA%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - Band%d(STA%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_emlsr_mr(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_StaEmlsr_padding_delay", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - Band%d(STA%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - Band%d(STA%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_emlsr_padding(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_StaEmlsr_trans_delay", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - Band%d(STA%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - Band%d(STA%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_emlsr_trans_delay(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_StaEmlsr_bitmap", tmpbuf,
		MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - Band%d(STA%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - Band%d(STA%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_emlsr_bitmap(
					wdev, (u16)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_StaLinkAntNum", tmpbuf,
		MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - Band%d(STA%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - Band%d(STA%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_emlsr_antnum(
					wdev, (u8)value);
			}
		}
	}

	if (RTMPGetKeyParameter("EHT_StaNstr_bitmap", tmpbuf,
		MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - Band%d(STA%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - Band%d(STA%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_nstr_bitmap(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_StaBw", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - Band%d(STA%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - Band%d(STA%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_eht_bw(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_StaTxNss", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - Band%d(STA%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - Band%d(STA%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_eht_tx_nss(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_StaRxNss", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - Band%d(STA%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - Band%d(STA%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_eht_rx_nss(
					wdev, (u8)value);
			}
		}
	}

	/* ApcliMloDisable */
	if (RTMPGetKeyParameter("ApcliMloDisable", tmpbuf, 32, pBuffer, FALSE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("I/F STA (%s%d): error input(=%s)\n",
					INF_MBSSID_DEV_NAME,
					idx,
					macptr);
			else {
				if (wdev) {
					wdev->apcli_mlo_diable = value;
#if defined(CONFIG_MAP_SUPPORT) && defined(MTK_HOSTAPD_SUPPORT)
					pAd->map_apcli_mlo_disable = value;
#endif
				}
				MTWF_PRINT("I/F STA (%s%d) ==> ApcliMloDisable = %d\n",
					INF_MBSSID_DEV_NAME, idx, wdev->apcli_mlo_diable);
			}
		}
	}

	/* ApcliMldAddr */
	reset_sta_eth_mld_cfg_addr(pAd);
	if (RTMPGetKeyParameter("ApcliMldAddr", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			int i = 0, mac_len = 0;
			u8 tmp_addr[MAC_ADDR_LEN] = {0};
			PSTA_ADMIN_CONFIG psta_cfg = &pAd->StaCfg[idx];

			/*init value*/
			os_zero_mem(psta_cfg->pf_mld_addr, MAC_ADDR_LEN);
			psta_cfg->pf_mld_addr_en = FALSE;

			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR, "macptr[%d]:(%s)\n", idx, macptr);
			/* Mac address acceptable format 01:02:03:04:05:06 (len=17)*/
			mac_len = strlen(macptr);
			if (mac_len != 17)
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR, "invalid length (%d)\n", mac_len);
			else if (strcmp(macptr, "00:00:00:00:00:00") == 0)
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"invalid mac setting: 00:00:00:00:00:00\n");
			else {
				for (i = 0; i < MAC_ADDR_LEN; i++)
					AtoH((tmpbuf + (i*3)), &tmp_addr[i], 1);
				COPY_MAC_ADDR(psta_cfg->pf_mld_addr, tmp_addr);
				psta_cfg->pf_mld_addr_en = TRUE;
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR, "mac val:("MACSTR")\n", MAC2STR(psta_cfg->pf_mld_addr));
			}
		}
	}

}

#endif /* DOT11_EHT_BE */
#endif /* CONFIG_STA_SUPPORT */

#endif /* !CONFIG_PROFILE_OFF */

