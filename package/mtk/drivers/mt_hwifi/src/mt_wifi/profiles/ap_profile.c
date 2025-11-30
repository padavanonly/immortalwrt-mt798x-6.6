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
	ap_profile.c

	Abstract:

*/
#include "rt_config.h"

#ifndef CONFIG_PROFILE_OFF

#ifdef CONFIG_AP_SUPPORT
void rtmp_read_acl_parms_from_file(
	RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING tok_str[32], *macptr;
	INT			i = 0, j = 0, idx;
	UCHAR		macAddress[MAC_ADDR_LEN];
	BOOLEAN		isDuplicate = FALSE;
	int ret;
	struct _BSS_STRUCT *mbss;

	memset(macAddress, 0, MAC_ADDR_LEN);

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		mbss = &pAd->ApCfg.MBSSID[idx];
		os_zero_mem(&mbss->AccessControlList, sizeof(RT_802_11_ACL));
		/* AccessPolicyX*/
		ret = snprintf(tok_str, sizeof(tok_str), "AccessPolicy%d", idx);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE,
				DBG_LVL_ERROR, "snprintf error!\n");
			return;
		}
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 10, buffer, TRUE)) {
			switch (os_str_tol(tmpbuf, 0, 10)) {
			case 1: /* Allow All, and the AccessControlList is positive now.*/
				mbss->AccessControlList.Policy = 1;
				break;

			case 2: /* Reject All, and the AccessControlList is negative now.*/
				mbss->AccessControlList.Policy = 2;
				break;

			case 0: /* Disable, don't care the AccessControlList.*/
			default:
				mbss->AccessControlList.Policy = 0;
				break;
			}

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"%s=%ld\n", tok_str,
				mbss->AccessControlList.Policy);
		}

		/* AccessControlListX*/
		ret = snprintf(tok_str, sizeof(tok_str), "AccessControlList%d", idx);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE,
					DBG_LVL_ERROR, "snprintf error!\n");
			return;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr
				&& (mbss->AccessControlList.Num <= MAX_NUM_OF_ACL_LIST);
				macptr = rstrtok(NULL, ";"), i++) {
				/* Mac address acceptable format 01:02:03:04:05:06 length 17*/
				if (strlen(macptr) != 17)
					continue;

				ASSERT(mbss->AccessControlList.Num <= MAX_NUM_OF_ACL_LIST);

				for (j = 0; j < MAC_ADDR_LEN; j++) {
					AtoH(macptr, &macAddress[j], 1);
					macptr = macptr + 3;
				}

				if (mbss->AccessControlList.Num == MAX_NUM_OF_ACL_LIST) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_WARN,
						"The ACL is full!\n");
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_WARN,
						"The last entry of ACL is "MACSTR"\n",
						MAC2STR(macAddress));
					break;
				}
				isDuplicate = FALSE;
				for (j = 0; j < mbss->AccessControlList.Num; j++) {
					if (memcmp(mbss->AccessControlList.Entry[j].Addr,
						&macAddress, 6) == 0) {
						isDuplicate = TRUE;
						MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_WARN,
							"You have added an entry before :\n");
						MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_WARN,
							"The duplicate entry is "MACSTR"\n",
							MAC2STR(macAddress));
					}
				}
				if (!isDuplicate) {
					ULONG acl_idx = 0;

					mbss->AccessControlList.Num++;
					acl_idx = mbss->AccessControlList.Num - 1;
					NdisMoveMemory(
						mbss->AccessControlList.Entry[acl_idx].Addr,
						macAddress, MAC_ADDR_LEN);
				}
			}

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"%s=Get %ld Mac Address\n", tok_str,
				mbss->AccessControlList.Num);
		}
	}
}

/*
    ========================================================================

    Routine Description:
	In kernel mode read parameters from file

    Arguments:
	src                     the location of the file.
	dest                        put the parameters to the destination.
	Length                  size to read.

    Return Value:
	None

    Note:

    ========================================================================
*/
static void rtmp_read_ap_edca_from_file(
	RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr, *edcaptr, tok_str[16];
	INT	i = 0, j = 0;
	EDCA_PARM *pEdca;
	RTMP_STRING *ptmpStr[6];
	struct wifi_dev *wdev = NULL;
	UCHAR ack_policy[WMM_NUM_OF_AC] = {0};
	int ret;

	for (j = 0; j < WMM_NUM; j++) {
		ret = snprintf(tok_str, sizeof(tok_str), "APEdca%d", j);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE,
				DBG_LVL_ERROR, "snprintf error!\n");
			return;
		}
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "%s\n", tok_str);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE)) {
			pEdca = &pAd->CommonCfg.APEdcaParm[j];

			for (i = 0, edcaptr = rstrtok(tmpbuf, ";"); edcaptr && (i < 6);
				edcaptr = rstrtok(NULL, ";"), i++)
				ptmpStr[i] = edcaptr;

			if (i != 6) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"Input parameter incorrect\n");
				return;
			}

			/*APValid*/
			edcaptr = ptmpStr[0];

			if (edcaptr) {
				pEdca->bValid = (UCHAR) os_str_tol(edcaptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"Valid=%d\n", pEdca->bValid);
			}

			/*APAifsn*/
			edcaptr = ptmpStr[1];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ",");
					macptr && (i < WMM_NUM_OF_AC);
					macptr = rstrtok(NULL, ","), i++) {
					pEdca->Aifsn[i] = (UCHAR) os_str_tol(macptr, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"APAifsn[%d]=%d\n", i, pEdca->Aifsn[i]);
				}
			}

			/*APCwmin*/
			edcaptr = ptmpStr[2];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ",");
					macptr && (i < WMM_NUM_OF_AC);
					macptr = rstrtok(NULL, ","), i++) {
					pEdca->Cwmin[i] = (UCHAR) os_str_tol(macptr, 0, 10);
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"APCwmin[%d]=%d\n", i, pEdca->Cwmin[i]);
				}
			}

			/*APCwmax*/
			edcaptr = ptmpStr[3];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ",");
					macptr && (i < WMM_NUM_OF_AC);
					macptr = rstrtok(NULL, ","), i++) {
					pEdca->Cwmax[i] = (UCHAR) os_str_tol(macptr, 0, 10);
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"APCwmax[%d]=%d\n", i, pEdca->Cwmax[i]);
				}
			}

			/*APTxop*/
			edcaptr = ptmpStr[4];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ",");
					macptr && (i < WMM_NUM_OF_AC);
					macptr = rstrtok(NULL, ","), i++) {
					pEdca->Txop[i] = (USHORT) os_str_tol(macptr, 0, 10);
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"APTxop[%d]=%d\n", i, pEdca->Txop[i]);
				}
			}

			/*APACM*/
			edcaptr = ptmpStr[5];
			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ",");
					macptr && (i < WMM_NUM_OF_AC);
					macptr = rstrtok(NULL, ","), i++) {
					pEdca->bACM[i] = (BOOLEAN) os_str_tol(macptr, 0, 10);
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"APACM[%d]=%d\n", i, pEdca->bACM[i]);
				}
			}
		}
	}

	/*AckPolicy*/
	for (i = 0 ; i < pAd->ApCfg.BssidNum; i++) {
		ret = snprintf(tok_str, sizeof(tok_str), "APAckPolicy%d", i);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE,
				DBG_LVL_ERROR, "snprintf error!\n");
			return;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE)) {
			wdev = &pAd->ApCfg.MBSSID[i].wdev;

			for (j = 0, edcaptr = rstrtok(tmpbuf, ";"); edcaptr && (i < WMM_NUM_OF_AC);
				edcaptr = rstrtok(NULL, ";"), j++)
				ack_policy[j] = (USHORT) simple_strtol(edcaptr, 0, 10);

			wlan_config_set_ack_policy(wdev, ack_policy);
		}
	}
}

static void rtmp_read_bss_edca_from_file(
	RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr, *edcaptr, tok_str[16];
	UINT i = 0, j = 0;
	RTMP_STRING *ptmpStr[6];
	struct _EDCA_PARM *pBssEdca = NULL;
	int ret;

	for (j = 0; j < pAd->ApCfg.BssidNum; j++) {
		ret = snprintf(tok_str, sizeof(tok_str), "BSSEdca%d", j);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE,
				DBG_LVL_ERROR, "snprintf error!\n");
			return;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE)) {
			pBssEdca = wlan_config_get_ht_edca(&pAd->ApCfg.MBSSID[j].wdev);

			if (!pBssEdca) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"BSS[%d]: Invalid pBssEdca\n", j);
				return;
			}

			for (i = 0, edcaptr = rstrtok(tmpbuf, ";"); edcaptr;
				edcaptr = rstrtok(NULL, ";"), i++) {
				if (i >= (sizeof(ptmpStr) / sizeof(ptmpStr[0]))) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
						 "Input parameter[Edca] incorrect\n"
						 );
					return;
				}
				ptmpStr[i] = edcaptr;
			}
			if (i != 5) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"Input parameter incorrect\n");
				return;
			}

			/*BSSAifsn*/
			edcaptr = ptmpStr[0];
			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr;
					macptr = rstrtok(NULL, ","), i++) {
					if (i >= WMM_NUM_OF_AC) {
						MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"Input parameter[Aifsn] incorrect\n");
						return;
					}

					pBssEdca->Aifsn[i] = (UCHAR) simple_strtol(macptr, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"BSSAifsn[%d]=%d\n", i, pBssEdca->Aifsn[i]);
				}
			}

			/*BSSCwmin*/
			edcaptr = ptmpStr[1];
			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr;
					macptr = rstrtok(NULL, ","), i++) {
					if (i >= WMM_NUM_OF_AC) {
						MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							 "Input parameter[Cwmin] incorrect\n");
						return;
					}
					pBssEdca->Cwmin[i] = (UCHAR) simple_strtol(macptr, 0, 10);
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"BSSCwmin[%d]=%d\n", i, pBssEdca->Cwmin[i]);
				}
			}

			/*BSSCwmax*/
			edcaptr = ptmpStr[2];
			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr;
					macptr = rstrtok(NULL, ","), i++) {
					if (i >= WMM_NUM_OF_AC) {
						MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							 "Input parameter[Cwmax] incorrect\n");
						return;
					}
					pBssEdca->Cwmax[i] = (UCHAR) simple_strtol(macptr, 0, 10);
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"BSSCwmax[%d]=%d\n", i, pBssEdca->Cwmax[i]);
				}
			}

			/*BSSTxop*/
			edcaptr = ptmpStr[3];
			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr;
					macptr = rstrtok(NULL, ","), i++) {
					if (i >= WMM_NUM_OF_AC) {
						MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							 "Input parameter[Txop] incorrect\n");
						return;
					}
					pBssEdca->Txop[i] = (USHORT) simple_strtol(macptr, 0, 10);
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"BSSTxop[%d]=%d\n", i, pBssEdca->Txop[i]);
				}
			}

			/*BSSACM*/
			edcaptr = ptmpStr[4];
			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr;
					macptr = rstrtok(NULL, ","), i++) {
					if (i >= WMM_NUM_OF_AC) {
						MTWF_DBG(pAd,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							 "Input parameter[ACM] incorrect\n");
						return;
					}
					pBssEdca->bACM[i] = (BOOLEAN) simple_strtol(macptr, 0, 10);
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						"BSSACM[%d]=%d\n", i, pBssEdca->bACM[i]);
				}
			}
		}
	}
}


void rtmp_read_ap_wmm_parms_from_file(
	RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr;
	INT	i = 0, j = 0;
	struct _EDCA_PARM *pBssEdca = NULL;
	struct _BSS_STRUCT *mbss;

	/*WmmCapable*/
	if (RTMPGetKeyParameter("WmmCapable", tmpbuf, 32, buffer, TRUE)) {
		BOOLEAN bEnableWmm = FALSE;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
			macptr = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;
			mbss = &pAd->ApCfg.MBSSID[i];
			if (os_str_tol(macptr, 0, 10) != 0) {
				mbss->wdev.bWmmCapable = TRUE;
				bEnableWmm = TRUE;
			} else
				mbss->wdev.bWmmCapable = FALSE;

			if (bEnableWmm) {
				pAd->CommonCfg.APEdcaParm[0].bValid = TRUE;

				/* Apply BSS[0] setting to all as default */
				if (i == 0)
					wlan_config_set_edca_valid_all(&pAd->wpf, TRUE);
				else
					wlan_config_set_edca_valid(&mbss->wdev, TRUE);
			} else {
				pAd->CommonCfg.APEdcaParm[0].bValid = FALSE;

				/* Apply BSS[0] setting to all as default */
				if (i == 0)
					wlan_config_set_edca_valid_all(&pAd->wpf, FALSE);
				else
					wlan_config_set_edca_valid(&mbss->wdev, FALSE);
			}

			mbss->bWmmCapableOrg = mbss->wdev.bWmmCapable;
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"I/F(ra%d) WmmCapable=%d\n", i,
				mbss->wdev.bWmmCapable);
		}
	}

	/*New WMM Parameter*/
	rtmp_read_ap_edca_from_file(pAd, tmpbuf, buffer);

	/*DLSCapable*/
	if (RTMPGetKeyParameter("DLSCapable", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;
			mbss = &pAd->ApCfg.MBSSID[i];
			if (os_str_tol(macptr, 0, 10) != 0) /*Enable*/
				mbss->bDLSCapable = TRUE;
			else /*Disable*/
				mbss->bDLSCapable = FALSE;

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"I/F(ra%d) DLSCapable=%d\n", i,
				mbss->bDLSCapable);
		}
	}

	/*APAifsn*/
	if (RTMPGetKeyParameter("APAifsn", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < WMM_NUM_OF_AC);
			macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.APEdcaParm[0].Aifsn[i] = (UCHAR) os_str_tol(macptr, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"APAifsn[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Aifsn[i]);
		}
	}

	/*APCwmin*/
	if (RTMPGetKeyParameter("APCwmin", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < WMM_NUM_OF_AC);
			macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.APEdcaParm[0].Cwmin[i] = (UCHAR) os_str_tol(macptr, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"APCwmin[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Cwmin[i]);
		}
	}

	/*APCwmax*/
	if (RTMPGetKeyParameter("APCwmax", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < WMM_NUM_OF_AC);
			macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.APEdcaParm[0].Cwmax[i] = (UCHAR) os_str_tol(macptr, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"APCwmax[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Cwmax[i]);
		}
	}

	/*APTxop*/
	if (RTMPGetKeyParameter("APTxop", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < WMM_NUM_OF_AC);
			macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.APEdcaParm[0].Txop[i] = (USHORT) os_str_tol(macptr, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"APTxop[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Txop[i]);
		}
	}

	/*APACM*/
	if (RTMPGetKeyParameter("APACM", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < WMM_NUM_OF_AC);
			macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.APEdcaParm[0].bACM[i] = (BOOLEAN) os_str_tol(macptr, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"APACM[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].bACM[i]);
		}
	}

	/* Apply default (BSS) WMM Parameter */
	for (j = 0; j < pAd->ApCfg.BssidNum; j++) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "BSS[%d]:\n", j);
		pBssEdca = wlan_config_get_ht_edca(&pAd->ApCfg.MBSSID[j].wdev);

		if (!pBssEdca)
			continue;

		/*BSSAifsn*/
		if (RTMPGetKeyParameter("BSSAifsn", tmpbuf, 32, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < WMM_NUM_OF_AC);
				macptr = rstrtok(NULL, ";"), i++) {
				pBssEdca->Aifsn[i] = (UCHAR) simple_strtol(macptr, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"BSSAifsn[%d]=%d\n", i, pBssEdca->Aifsn[i]);
			}
		}

		/*BSSCwmin*/
		if (RTMPGetKeyParameter("BSSCwmin", tmpbuf, 32, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < WMM_NUM_OF_AC);
				macptr = rstrtok(NULL, ";"), i++) {
				pBssEdca->Cwmin[i] = (UCHAR) simple_strtol(macptr, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"BSSCwmin[%d]=%d\n", i, pBssEdca->Cwmin[i]);
			}
		}

		/*BSSCwmax*/
		if (RTMPGetKeyParameter("BSSCwmax", tmpbuf, 32, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < WMM_NUM_OF_AC);
				macptr = rstrtok(NULL, ";"), i++) {
				pBssEdca->Cwmax[i] = (UCHAR) simple_strtol(macptr, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"BSSCwmax[%d]=%d\n", i, pBssEdca->Cwmax[i]);
			}
		}

		/*BSSTxop*/
		if (RTMPGetKeyParameter("BSSTxop", tmpbuf, 32, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < WMM_NUM_OF_AC);
				macptr = rstrtok(NULL, ";"), i++) {
				pBssEdca->Txop[i] = (USHORT) simple_strtol(macptr, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"BSSTxop[%d]=%d\n", i, pBssEdca->Txop[i]);
			}
		}

		/*BSSACM*/
		if (RTMPGetKeyParameter("BSSACM", tmpbuf, 32, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < WMM_NUM_OF_AC);
				macptr = rstrtok(NULL, ";"), i++) {
				pBssEdca->bACM[i] = (BOOLEAN) simple_strtol(macptr, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"BSSACM[%d]=%d\n", i, pBssEdca->bACM[i]);
			}
		}
	}

	/*Apply new (BSS) WMM Parameter*/
	rtmp_read_bss_edca_from_file(pAd, tmpbuf, buffer);


	/*AckPolicy*/
	if (RTMPGetKeyParameter("AckPolicy", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < WMM_NUM_OF_AC);
			macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.AckPolicy[i] = (UCHAR) os_str_tol(macptr, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"AckPolicy[%d]=%d\n", i, pAd->CommonCfg.AckPolicy[i]);
		}

		wlan_config_set_ack_policy_all(&pAd->wpf, pAd->CommonCfg.AckPolicy);
	}

#ifdef UAPSD_SUPPORT
	/*UAPSDCapable*/
	if (RTMPGetKeyParameter("UAPSDCapable", tmpbuf, 64, buffer, TRUE) ||
		RTMPGetKeyParameter("APSDCapable", tmpbuf, 64, buffer, TRUE)) { /* backward compatible with old SDK */

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (VALID_MBSS(pAd, i)) {
				pAd->ApCfg.MBSSID[i].wdev.UapsdInfo.bAPSDCapable = \
						(UCHAR) os_str_tol(macptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"UAPSDCapable[%d]=%d\n", i,
					pAd->ApCfg.MBSSID[i].wdev.UapsdInfo.bAPSDCapable);
			} else
				break;
		}

		if (i == 1) {
			/*
				Old format in UAPSD settings: only 1 parameter
				i.e. UAPSD for all BSS is enabled or disabled.
			*/
			for (i = 1; VALID_MBSS(pAd, i); i++) {
				pAd->ApCfg.MBSSID[i].wdev.UapsdInfo.bAPSDCapable =
					pAd->ApCfg.MBSSID[0].wdev.UapsdInfo.bAPSDCapable;
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"UAPSDCapable[%d]=%d\n", i,
					pAd->ApCfg.MBSSID[i].wdev.UapsdInfo.bAPSDCapable);
			}
		}

#ifdef APCLI_SUPPORT

		if (pAd->ApCfg.FlgApCliIsUapsdInfoUpdated == FALSE) {
			/*
				Backward:
				All UAPSD for AP Client interface is same as MBSS0
				when we can not find "ApCliUAPSDCapable".
				When we find "ApCliUAPSDCapable" hereafter, we will over-write.
			*/
			for (i = 0; i < MAX_APCLI_NUM; i++) {
				pAd->StaCfg[i].wdev.UapsdInfo.bAPSDCapable = \
						pAd->ApCfg.MBSSID[0].wdev.UapsdInfo.bAPSDCapable;
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
					"default ApCliUAPSDCapable[%d]=%d\n",
						 i, pAd->StaCfg[i].wdev.UapsdInfo.bAPSDCapable);
			}
		}

#endif /* APCLI_SUPPORT */
	}

#endif /* UAPSD_SUPPORT */
}

#ifdef BAND_STEERING
void BndStrgSetProfileParam(
	struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *pBuffer)
{
	INT	i = 0;
	RTMP_STRING *macptr = NULL;

	if (RTMPGetKeyParameter("BandSteering", tmpbuf, 10, pBuffer, TRUE)) {
		pAd->ApCfg.BandSteering = (UCHAR) simple_strtol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BND_STRG, DBG_LVL_INFO,
			"BandSteering=%d\n", pAd->ApCfg.BandSteering);
	}

	if (RTMPGetKeyParameter("BndStrgBssIdx", tmpbuf, 50, pBuffer, TRUE)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BND_STRG, DBG_LVL_INFO,
			"BndStrgBssIdx=%s\n", tmpbuf);

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && (i < MAX_BEACON_NUM);
			macptr = rstrtok(NULL, ";"), i++)
			pAd->ApCfg.BndStrgBssIdx[i] = simple_strtoul(macptr, 0, 10);

		if (i == 0)
			pAd->ApCfg.BndStrgBssIdx[MAIN_MBSSID] = 1;
	} else
		pAd->ApCfg.BndStrgBssIdx[MAIN_MBSSID] = 1;
}
#endif /* BAND_STEERING */

#ifdef IDS_SUPPORT

VOID rtmp_read_ids_from_file(
	IN  PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	/*IdsEnable */
	if (RTMPGetKeyParameter("IdsEnable", tmpbuf, 10, buffer, TRUE)) {
		if (os_str_tol(tmpbuf, 0, 10) == 1)
			pAd->ApCfg.IdsEnable = TRUE;
		else
			pAd->ApCfg.IdsEnable = FALSE;

		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_IDS, DBG_LVL_INFO,
			"IDS is %s\n", pAd->ApCfg.IdsEnable ? "enabled" : "disabled");
	}

	/*AuthFloodThreshold */
	if (RTMPGetKeyParameter("AuthFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.AuthFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_IDS, DBG_LVL_INFO,
			"AuthFloodThreshold = %d\n", pAd->ApCfg.AuthFloodThreshold);
	}

	/*AssocReqFloodThreshold */
	if (RTMPGetKeyParameter("AssocReqFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.AssocReqFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_IDS, DBG_LVL_INFO,
			"AssocReqFloodThreshold = %d\n", pAd->ApCfg.AssocReqFloodThreshold);
	}

	/*ReassocReqFloodThreshold */
	if (RTMPGetKeyParameter("ReassocReqFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.ReassocReqFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_IDS, DBG_LVL_INFO,
			"ReassocReqFloodThreshold = %d\n", pAd->ApCfg.ReassocReqFloodThreshold);
	}

	/*ProbeReqFloodThreshold */
	if (RTMPGetKeyParameter("ProbeReqFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.ProbeReqFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_IDS, DBG_LVL_INFO,
			"ProbeReqFloodThreshold = %d\n", pAd->ApCfg.ProbeReqFloodThreshold);
	}

	/*DisassocFloodThreshold */
	if (RTMPGetKeyParameter("DisassocFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.DisassocFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_IDS, DBG_LVL_INFO,
			"DisassocFloodThreshold = %d\n", pAd->ApCfg.DisassocFloodThreshold);
	}

	/*DeauthFloodThreshold */
	if (RTMPGetKeyParameter("DeauthFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.DeauthFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_IDS, DBG_LVL_INFO,
			"DeauthFloodThreshold = %d\n", pAd->ApCfg.DeauthFloodThreshold);
	}

	/*EapReqFloodThreshold */
	if (RTMPGetKeyParameter("EapReqFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.EapReqFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_IDS, DBG_LVL_INFO,
			"EapReqFloodThreshold = %d\n", pAd->ApCfg.EapReqFloodThreshold);
	}

	/* DataFloodThreshold  */
	if (RTMPGetKeyParameter("DataFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.DataFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_IDS, DBG_LVL_INFO,
			"DataFloodThreshold = %d\n", pAd->ApCfg.DataFloodThreshold);
	}
}

#endif /* IDS_SUPPORT */

#ifdef WDS_SUPPORT
VOID rtmp_read_wds_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr;
	INT			i = 0, j;
	UCHAR		op_mode, macAddress[MAC_ADDR_LEN];
	PRT_802_11_WDS_ENTRY pWdsEntry;
	struct wifi_dev *wdev, *main_wdev = NULL;
	long value = 0;

	if (buffer == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "invalid buffer\n");
		return;
	}

	/*Wds Number */
	if (RTMPGetKeyParameter("WdsNum", tmpbuf, 10, buffer, TRUE)) {
		if (kstrtol(tmpbuf, 10, &value))
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
				"invalid input(=%s)\n", tmpbuf);
		else
			pAd->WdsTab.wds_num = value;
	}

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "WDS Number: band[%d]=%d\n",
		hc_get_hw_band_idx(pAd),
		pAd->WdsTab.wds_num);

	/*WdsEnable */
	if (RTMPGetKeyParameter("WdsEnable", tmpbuf, 10, buffer, TRUE)) {
		UCHAR wds_idx = 0;
		RT_802_11_WDS_ENTRY *pWdsEntry, *src_entry;
		UINT32 start_idx = 0;
		UINT32 end_idx = MAX_WDS_ENTRY;

		if (kstrtol(tmpbuf, 10, &value))
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
				"invalid input(=%s)\n", tmpbuf);
		else {
			switch (value) {
			case WDS_BRIDGE_MODE:
				/*
				 * Bridge mode
				 * DisAllow association(stop Beacon generation and Probe Req.
				 */
				pAd->WdsTab.Mode = WDS_BRIDGE_MODE;
				break;
			case WDS_RESTRICT_MODE:
			case WDS_REPEATER_MODE:
				/* Repeater mode */
				pAd->WdsTab.Mode = WDS_REPEATER_MODE;
				break;
			case WDS_LAZY_MODE:
				/*
				 * Lazy mode
				 * Auto learn wds entry by same SSID, channel, security policy
				 */
				for (wds_idx = start_idx; wds_idx < end_idx; wds_idx++) {
					pWdsEntry = &pAd->WdsTab.WdsEntry[wds_idx];

					if (WDS_ENTRY_IS_ASSIGNED(pWdsEntry->flag))
						WdsEntryDel(pAd, pWdsEntry->PeerWdsAddr);

					/*
					 * When Lazy mode is enabled,
					 * the all wds-link shall share the same encryption type
					 * and key material
					 */
					if (wds_idx > start_idx) {
						src_entry = &pAd->WdsTab.WdsEntry[start_idx];
						os_move_mem(pWdsEntry->wdev.SecConfig.WepKey,
							src_entry->wdev.SecConfig.WepKey,
							sizeof(pWdsEntry->wdev.SecConfig.WepKey));
						os_move_mem(pWdsEntry->wdev.SecConfig.PSK,
							src_entry->wdev.SecConfig.PSK,
							sizeof(pWdsEntry->wdev.SecConfig.PSK));
						pWdsEntry->wdev.SecConfig.AKMMap =
							src_entry->wdev.SecConfig.AKMMap;
						pWdsEntry->wdev.SecConfig.PairwiseCipher =
							src_entry->wdev.SecConfig.PairwiseCipher;
						pWdsEntry->wdev.SecConfig.PairwiseKeyId =
							src_entry->wdev.SecConfig.PairwiseKeyId;
					}
				}

				pAd->WdsTab.Mode = WDS_LAZY_MODE;
				pAd->WdsTab.wds_num = MAX_WDS_ENTRY;
				break;
			case WDS_DISABLE_MODE:	/* Disable mode */
			default:
				for (wds_idx = start_idx; wds_idx < end_idx; wds_idx++) {
					pWdsEntry = &pAd->WdsTab.WdsEntry[wds_idx];

					if (WDS_ENTRY_IS_ASSIGNED(pWdsEntry->flag)
						&& (HcGetBandByWdev(&pWdsEntry->wdev) == i))
						WdsEntryDel(pAd, pWdsEntry->PeerWdsAddr);
				}
				pAd->WdsTab.Mode = WDS_DISABLE_MODE;
				pAd->WdsTab.wds_num = 0;
				break;
			}
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
				"band%d WDS-Enable mode=%d\n", i, pAd->WdsTab.Mode);
		}
	}

	/*WdsList */
	if (RTMPGetKeyParameter("WdsList", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY);
			macptr = rstrtok(NULL, ";"), i++) {
			/*Mac address acceptable format 01:02:03:04:05:06 length 17 */
			if (strlen(macptr) != 17)
				continue;

			if (strcmp(macptr, "00:00:00:00:00:00") == 0)
				continue;

			for (j = 0; j < MAC_ADDR_LEN; j++) {
				AtoH(macptr, &macAddress[j], 1);
				macptr = macptr+3;
			}
			WdsEntryAlloc(pAd, macAddress);
			if (pAd->WdsTab.Mode != WDS_DISABLE_MODE
				&& pAd->WdsTab.Mode != WDS_LAZY_MODE)
				pAd->WdsTab.wds_num++;
		}
	}

	/*WdsPhyMode */
	if (RTMPGetKeyParameter("WdsPhyMode", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY);
			macptr = rstrtok(NULL, ";"), i++) {
			if (i == pAd->WdsTab.wds_num)
				break;
			pWdsEntry = &pAd->WdsTab.WdsEntry[i];

			main_wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

			if (rtstrcasecmp(macptr, "CCK") == TRUE)
				op_mode = MODE_CCK;
			else if (rtstrcasecmp(macptr, "OFDM") == TRUE)
				op_mode = MODE_OFDM;

#ifdef DOT11_N_SUPPORT
			else if (rtstrcasecmp(macptr, "HTMIX") == TRUE)
				op_mode = MODE_HTMIX;
			else if (rtstrcasecmp(macptr, "GREENFIELD") == TRUE)
				op_mode = MODE_HTGREENFIELD;

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
			else if (rtstrcasecmp(macptr, "VHT") == TRUE)
				op_mode = MODE_VHT;

#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
			else if (rtstrcasecmp(macptr, "HE") == TRUE)
				op_mode = MODE_HE;
#endif
			else
				op_mode = MODE_UNKNOWN;

			pWdsEntry->phy_mode = get_wds_phymode(main_wdev, op_mode);
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
				"If/wds%d - PeerPhyMode=0x%x\n", i, pWdsEntry->phy_mode);
		}
	}

	/* WdsTxMode */
	if (RTMPGetKeyParameter("WdsTxMode", tmpbuf, 25, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY);
			macptr = rstrtok(NULL, ";"), i++) {
			if (i == pAd->WdsTab.wds_num)
				break;
			wdev = &pAd->WdsTab.WdsEntry[i].wdev;
			wdev->DesiredTransmitSetting.field.FixedTxMode =
				RT_CfgSetFixedTxPhyMode(macptr);
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
				"I/F(wds%d) Tx Mode = %d\n", i,
				wdev->DesiredTransmitSetting.field.FixedTxMode);
		}
	}

	/* WdsTxMcs */
	if (RTMPGetKeyParameter("WdsTxMcs", tmpbuf, 50, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY);
			macptr = rstrtok(NULL, ";"), i++) {
			if (i == pAd->WdsTab.wds_num)
				break;
			wdev = &pAd->WdsTab.WdsEntry[i].wdev;
			wdev->DesiredTransmitSetting.field.MCS =
				RT_CfgSetTxMCSProc(macptr, &wdev->bAutoTxRateSwitch);

			if (wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO)
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
					"I/F(wds%d) Tx MCS = AUTO\n", i);
			else {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
					"I/F(wds%d) Tx MCS = %d\n", i,
					wdev->DesiredTransmitSetting.field.MCS);
			}
		}
	}

#ifdef WDS_VLAN_SUPPORT
	/* WdsVlan */
	if (RTMPGetKeyParameter("WDS_VLANID", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY);
			macptr = rstrtok(NULL, ";"), i++) {
			if (i == pAd->WdsTab.wds_num)
				break;
			pAd->WdsTab.WdsEntry[i].wdev.VLAN_VID = os_str_tol(macptr, 0, 10);
			pAd->WdsTab.WdsEntry[i].wdev.VLAN_Priority = 0;
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
				"If/wds%d - WdsVlanId=%d\n", i,
				pAd->WdsTab.WdsEntry[i].wdev.VLAN_VID);
		}
	}

#endif /* WDS_VLAN_SUPPORT */
}

#endif /* WDS_SUPPORT */

#ifdef DOT11_EHT_BE
static void rtmp_read_mld_group_idx_from_file(
	PRTMP_ADAPTER pAd, RTMP_STRING *Buffer)
{
	UINT32 i;
	CHAR *macptr;
	u8 mld_grp;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr;
			macptr = rstrtok(NULL, ";"), i++) {
		mld_grp = os_str_tol(macptr, 0, 10);

		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pMbss = &pAd->ApCfg.MBSSID[i];
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			if ((mld_grp < BMGR_MAX_MLD_GRP_CNT) && (mld_grp != BMGR_INVALID_MLD_GRP)) {
				pMbss->mld_grp_idx = mld_grp;
				pMbss->mld_grp_by_cfg = TRUE;
				pMbss->unique_mld_addr_enable = TRUE;
                                if (mld_grp != 1) 
                                        wdev->ap_mlo_disable = TRUE;
                                else
                                        wdev->ap_mlo_disable = FALSE;
				MTWF_PRINT("BSS%d mld_grp_idx=%d AP_MLO_DISABLE =%d\n",
					i, pMbss->mld_grp_idx,wdev->ap_mlo_disable);
			} else
			{	
				wdev->ap_mlo_disable = TRUE;
				MTWF_PRINT("INVALID mld_grp_idx: %d (larger than max=%d)\n",
					mld_grp, MAX_MLD_GROUP_NUM);
			}
		}
	}
}

static void rtmp_reset_mld_group(
	PRTMP_ADAPTER pAd)
{
	UINT32 i;
	BSS_STRUCT *pMbss = NULL;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			pMbss = &pAd->ApCfg.MBSSID[i];
			pMbss->mld_grp_idx = 0;
			pMbss->mld_grp_by_cfg = FALSE;
			pMbss->mld_addr_by_cfg = FALSE;
			COPY_MAC_ADDR(pMbss->pf_mld_addr, ZERO_MAC_ADDR);
			pMbss->unique_mld_addr_enable = FALSE;
			COPY_MAC_ADDR(pMbss->multilink_mld_addr, ZERO_MAC_ADDR);
		}
	}
}

static void rtmp_read_mld_addr_from_file(
	IN PRTMP_ADAPTER pAd,
	IN char *tmpbuf,
	IN char *pBuffer)
{
	UINT32 i;
	BSS_STRUCT *pMbss = NULL;
	RTMP_STRING addr_key[20];
	INT ret;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			pMbss = &pAd->ApCfg.MBSSID[i];

			if (!pMbss->mld_grp_idx || !pMbss->mld_grp_by_cfg)
				continue;

			ret = snprintf(addr_key, sizeof(addr_key), "MldAddr%d", pMbss->mld_grp_idx);
			if (os_snprintf_error(sizeof(addr_key), ret)) {
				MTWF_PRINT("%s(): snprintf error!\n", __func__);
				return;
			}

			if (RTMPGetKeyParameter(addr_key, tmpbuf, 25, pBuffer, TRUE)) {
				INT j, mac_len;
				UCHAR Address[MAC_ADDR_LEN];

				/* Mac address acceptable format 01:02:03:04:05:06 (len=17) */
				mac_len = strlen(tmpbuf);
				if (mac_len != 17)
					MTWF_PRINT("%s(): invalid length (%d)\n", __func__, mac_len);
				else if (strcmp(tmpbuf, "00:00:00:00:00:00") == 0)
					MTWF_PRINT("%s(): invalid mac setting\n", __func__);
				else {
					for (j = 0; j < MAC_ADDR_LEN; j++)
						AtoH((tmpbuf + (j*3)), &Address[j], 1);

					/* MLD Mac address which registers to BSS Manager */
					COPY_MAC_ADDR(pMbss->pf_mld_addr, Address);
					pMbss->mld_addr_by_cfg = TRUE;

					MTWF_PRINT("BSS%d mld_grp_idx=%d (%pM)\n",
						i, pMbss->mld_grp_idx, pMbss->pf_mld_addr);
				}
			}
		}
	}
}

static void rtmp_read_tid_mapping_from_file(
	PRTMP_ADAPTER pAd, RTMP_STRING *Buffer)
{
	UINT32 i;
	CHAR *macptr;
	u8 tid_mapping;
	BSS_STRUCT *pMbss = NULL;

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr;
			macptr = rstrtok(NULL, ";"), i++) {
		tid_mapping = os_str_tol(macptr, 0, 10);

		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pMbss = &pAd->ApCfg.MBSSID[i];
			pMbss->tid_mapping = tid_mapping;

			MTWF_PRINT("BSS%d TidMapping=0x%x\n",
				i, pMbss->tid_mapping);
		}
	}
}

void read_ap_eht_config_from_file(
	IN PRTMP_ADAPTER pAd,
	IN char *tmpbuf,
	IN char *pBuffer)
{
	char *macptr;
	UINT32 idx;
	long value;
	struct wifi_dev *wdev = NULL;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	if (RTMPGetKeyParameter("MldGroup", tmpbuf, 100, pBuffer, TRUE)) {
		rtmp_read_mld_group_idx_from_file(pAd, tmpbuf);
		rtmp_read_mld_addr_from_file(pAd, tmpbuf, pBuffer);
	} else
		rtmp_reset_mld_group(pAd);

	if (RTMPGetKeyParameter("TidMapping", tmpbuf, 100, pBuffer, TRUE))
		rtmp_read_tid_mapping_from_file(pAd, tmpbuf);

	if (RTMPGetKeyParameter("EHT_ApNsepPriAccess", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_nsep_priority_access(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_ApOmCtrl", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_eht_om_ctrl(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_ApTxopSharing", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_eht_txop_sharing_trigger(
					wdev, (u8)value);
			}
		}
	}
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef WIFI_TWT_SUPPORT
		if (RTMPGetKeyParameter("EHT_ApRestrictedTwt", tmpbuf,
				MAX_PARAMETER_LEN, pBuffer, TRUE)) {

			for (idx = 0, macptr = rstrtok(tmpbuf, ";");
				(macptr && idx < pAd->ApCfg.BssidNum);
				macptr = rstrtok(NULL, ";"), idx++) {
				wdev = &pAd->ApCfg.MBSSID[idx].wdev;
				if (kstrtol(macptr, 10, &value))
					MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
						__LINE__,
						band_idx,
						idx,
						macptr);
				else {
					MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
						__LINE__, band_idx, idx);
					wlan_config_set_eht_restricted_twt(
						wdev, (u8)value);
#ifdef DOT11_HE_AX
					/*
					 * shall set the Restricted TWT Support subfield
					 * in its transmitted EHT Capabilities element to 1
					 * and shall set the Broadcast TWT Support subfield
					 * in its transmitted HE Capabilities element to 1
					 */
					if (value > 0) {
						UINT8 he_twt_support;

						he_twt_support =
							wlan_config_get_he_twt_support(wdev);
						if (!TWT_SUPPORT_BTWT(he_twt_support))
							wlan_config_set_he_twt_support(wdev,
							(he_twt_support | TWT_PROFILE_SUPPORT_BTWT));
					}
#endif /* DOT11_HE_AX */
				}
			}
		}
	}
#endif /* WIFI_TWT_SUPPORT */

	if (RTMPGetKeyParameter("EHT_ApScsTraffic", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_eht_scs_traffic(
					wdev, (u8)value);
			}
		}
	}

	if (RTMPGetKeyParameter("EHT_ApMsdPres", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_med_sync_pres(
					wdev, (u8)value);
			}
		}
	}

	if (RTMPGetKeyParameter("EHT_ApMsdDur", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_med_sync_dur(
					wdev, (u8)value);
			}
		}
	}

	if (RTMPGetKeyParameter("EHT_ApMsdOfdmEdThr", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_med_sync_ofdm_ed_thr(
					wdev, (u8)value);
			}
		}
	}

	if (RTMPGetKeyParameter("EHT_ApMsdMaxTxop", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_med_sync_max_txop(
					wdev, (u8)value);
			}
		}
	}

	if (RTMPGetKeyParameter("EHT_ApEmlsr_mr", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_emlsr_mr(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_ApEmlsr_mr_trans_to", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_trans_to(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_ApEmlsr_mr_OMN", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_eml_omn_en(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_ApBw", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {
		UCHAR IdBss = 0;
		struct wifi_dev *tdev = NULL;

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;

			if (kstrtol(macptr, 10, &value)) {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
				break;
			}

			MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
			wlan_config_set_eht_bw(
				wdev, (u8)value);

			/* for first time, update all same as main bss */
			if (idx == 0) {
				for (IdBss = 1; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
					tdev = &pAd->ApCfg.MBSSID[IdBss].wdev;
					if (tdev)
						wlan_config_set_eht_bw(tdev, (u8)value);
				}
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_ApTxNss", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_eht_tx_nss(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_ApRxNss", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_eht_rx_nss(
					wdev, (u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_MaxMcs", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_eht_max_mcs(
					wdev, ((u8)value) == 0 ? MCS_13:(u8)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_ApDisSubChBitMap", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 16, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_eht_dis_subch_bitmap(
					wdev, (u16)value);
			}
		}
	}
	if (RTMPGetKeyParameter("EHT_ApT2lmNegoSupport", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {

		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->ApCfg.BssidNum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - AP Band%d(MBSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_t2lm_nego_support(
					wdev, (u8)value);
			}
		}
	}
#ifdef CONFIG_STA_SUPPORT
	if (RTMPGetKeyParameter("EHT_ApcliT2lmNegoSupport", tmpbuf,
			MAX_PARAMETER_LEN, pBuffer, TRUE)) {
		for (idx = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && idx < pAd->MaxMSTANum);
			macptr = rstrtok(NULL, ";"), idx++) {
			wdev = &pAd->StaCfg[idx].wdev;
			if (kstrtol(macptr, 10, &value))
				MTWF_PRINT("Line.%d - Apcli Band%d(BSS%d): error input(=%s)\n",
					__LINE__,
					band_idx,
					idx,
					macptr);
			else {
				MTWF_PRINT("Line.%d - Apcli Band%d(BSS%d) ==> ",
					__LINE__, band_idx, idx);
				wlan_config_set_t2lm_nego_support(
					wdev, (u8)value);
			}
		}
	}
#endif
}

#endif /* DOT11_EHT_BE */

#ifdef VOW_SUPPORT
void rtmp_read_vow_parms_from_file(
	IN PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	UINT16		i = 0, j = 0;
	CHAR		*ptok = NULL;
	CHAR		*macptr;
	CHAR		*tmp = NULL;
	CHAR		*pwatf_string = NULL;
	UINT16	wtbl_max_num = hc_get_chip_wtbl_max_num(pAd);
	int ret;

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "%s: begin -->\n",
			 __func__);
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	/* for enable/disable */
	if (RTMPGetKeyParameter("VOW_BW_Ctrl", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.en_bw_ctrl =  os_str_tol(tmpbuf, 0, 10) != 0 ? TRUE : FALSE;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "VOW_BW_Ctrl --> %d\n",
				 pAd->vow_cfg.en_bw_ctrl);
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Airtime_Fairness_En", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.en_airtime_fairness =  os_str_tol(tmpbuf, 0, 10) != 0 ? TRUE : FALSE;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_Airtime_Fairness_En --> %d\n",
				 pAd->vow_cfg.en_airtime_fairness);
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Airtime_Ctrl_En", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {

			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].at_on =  os_str_tol(ptok, 0, 10) != 0 ? TRUE : FALSE;
			pAd->vow_bss_cfg[i].group_table_idx = i;
			pAd->bss_group.bw_group_idx[i] = i;
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BSS(%d) VOW_Airtime_Ctrl_En --> %d\n",
					 i, pAd->vow_bss_cfg[i].at_on);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Rate_Ctrl_En", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {

			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].bw_on =  os_str_tol(ptok, 0, 10) != 0 ? TRUE : FALSE;
			pAd->vow_bss_cfg[i].group_table_idx = i;
			pAd->bss_group.bw_group_idx[i] = i;
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BSS(%d) VOW_Rate_Ctrl_En --> %d\n",
					 i, pAd->vow_bss_cfg[i].bw_on);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_RX_En", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		pAd->vow_rx_time_cfg.rx_time_en =  os_str_tol(tmpbuf, 0, 10) != 0 ? TRUE : FALSE;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_RX_En --> %d\n",
				 pAd->vow_rx_time_cfg.rx_time_en);
	}

	/* for gorup setting */
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Min_Rate", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {

			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].min_rate =  (UINT16)os_str_tol(ptok, 0, 10);
			pAd->vow_bss_cfg[i].min_rate_token = vow_convert_rate_token(
									pAd, VOW_MIN, i);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BSS(%d) VOW_Group_Min_Rate --> %d\n",
					 i, pAd->vow_bss_cfg[i].min_rate);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Max_Rate", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {

			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].max_rate =  (UINT16)os_str_tol(ptok, 0, 10);
			pAd->vow_bss_cfg[i].max_rate_token = vow_convert_rate_token(
									pAd, VOW_MAX, i);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BSS(%d) VOW_Group_Max_Rate --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_rate);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Min_Ratio", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {

			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].min_airtime_ratio =  (UINT8)os_str_tol(ptok, 0, 10);
			pAd->vow_bss_cfg[i].min_airtime_token = vow_convert_airtime_token(
										pAd, VOW_MIN, i);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BSS(%d) VOW_Group_Min_Ratio --> %d\n",
					 i, pAd->vow_bss_cfg[i].min_airtime_ratio);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Max_Ratio", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {

			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].max_airtime_ratio =  (UINT8)os_str_tol(ptok, 0, 10);
			pAd->vow_bss_cfg[i].max_airtime_token = vow_convert_airtime_token(
										pAd, VOW_MAX, i);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BSS(%d) VOW_Group_Max_Ratio --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_airtime_ratio);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Refill_Period", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.refill_period =  (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_Refill_Period --> %d\n",
				 pAd->vow_cfg.refill_period);
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Min_Rate_Bucket_Size", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {

			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].min_ratebucket_size =  (UINT16)os_str_tol(ptok, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BSS(%d) VOW_Group_Min_Rate_Bucket_Size --> %d\n",
					 i, pAd->vow_bss_cfg[i].min_ratebucket_size);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Max_Rate_Bucket_Size", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;
			pAd->vow_bss_cfg[i].max_ratebucket_size =  (UINT16)os_str_tol(ptok, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BSS(%d) VOW_Group_Max_Rate_Bucket_Size --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_ratebucket_size);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Min_Airtime_Bucket_Size", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].min_airtimebucket_size = (UINT8)os_str_tol(ptok, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BSS(%d) VOW_Group_Min_Airtime_Bucket_Size --> %d\n",
					 i, pAd->vow_bss_cfg[i].min_airtimebucket_size);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Max_Airtime_Bucket_Size", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].max_airtimebucket_size = (UINT8)os_str_tol(ptok, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BSS(%d) VOW_Group_Max_Airtime_Bucket_Size --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_airtimebucket_size);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Backlog", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].max_backlog_size = (UINT16)os_str_tol(ptok, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BSS(%d) VOW_Group_Backlog --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_backlog_size);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Max_Wait_Time", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].max_wait_time = (UINT8)os_str_tol(ptok, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BSS(%d) VOW_Group_Max_Wait_Time --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_wait_time);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_DWRR_Quantum", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].dwrr_quantum =  (UINT8)os_str_tol(ptok, 0, 10);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"BSS(%d) VOW_Group_DWRR_Quantum --> %d\n",
					 i, pAd->vow_bss_cfg[i].dwrr_quantum);
		}
	}

	/* for stations */
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Sta_VO_DWRR_Quantum", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0; i < wtbl_max_num; i++)
			pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_VO] = (UINT8)os_str_tol(tmpbuf, 0, 10);

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_Sta_VO_DWRR_Quantum --> %d\n",
				 (UINT8)os_str_tol(tmpbuf, 0, 10));
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Sta_VI_DWRR_Quantum", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0; i < wtbl_max_num; i++)
			pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_VI] =
						(UINT8)os_str_tol(tmpbuf, 0, 10);

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_Sta_VI_DWRR_Quantum --> %d\n",
				 (UINT8)os_str_tol(tmpbuf, 0, 10));
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Sta_BE_DWRR_Quantum", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		for (i = 0; i < wtbl_max_num; i++)
			pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_BE] =
						(UINT8)os_str_tol(tmpbuf, 0, 10);

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_Sta_BE_DWRR_Quantum --> %d\n",
				 (UINT8)os_str_tol(tmpbuf, 0, 10));
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Sta_BK_DWRR_Quantum", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0; i < wtbl_max_num; i++)
			pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_BK] =
						(UINT8)os_str_tol(tmpbuf, 0, 10);

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_Sta_BK_DWRR_Quantum --> %d\n",
				 (UINT8)os_str_tol(tmpbuf, 0, 10));
	}

	/* for group/stations control */
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_WMM_Search_Rule_Band0", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.dbdc0_search_rule =  os_str_tol(tmpbuf, 0, 10) ? 1 : 0;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_WMM_Search_Rule_Band0 --> %d\n",
				 pAd->vow_cfg.dbdc0_search_rule);
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_WMM_Search_Rule_Band1", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.dbdc1_search_rule =  os_str_tol(tmpbuf, 0, 10) ? 1 : 0;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_WMM_Search_Rule_Band1 --> %d\n",
				 pAd->vow_cfg.dbdc1_search_rule);
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Sta_DWRR_Max_Wait_Time", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.sta_max_wait_time =  (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_Sta_DWRR_Max_Wait_Time --> %d\n",
				 pAd->vow_cfg.sta_max_wait_time);
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_DWRR_Max_Wait_Time", tmpbuf, 128, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.group_max_wait_time =  (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_Group_DWRR_Max_Wait_Time --> %d\n",
				 pAd->vow_cfg.group_max_wait_time);
	}

	/* Weigthed Airtime Fairness - Enable/Disable*/
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_WATF_Enable", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		pAd->vow_watf_en =  (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_WATF_Enable --> %d\n", pAd->vow_watf_en);
	}

	if (pAd->vow_watf_en) {
		/* Weigthed Airtime Fairness - Different DWRR quantum value*/
		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

		if (RTMPGetKeyParameter(
			"VOW_WATF_Q_LV0", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)
			&& (strlen(tmpbuf) > 0)) {
			pAd->vow_watf_q_lv0 = (UINT8)os_str_tol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[0] = pAd->vow_watf_q_lv0;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"VOW_WATF_Q_LV0 --> %d\n", pAd->vow_watf_q_lv0);
		} else
			pAd->vow_watf_q_lv0 = 4;

		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

		if (RTMPGetKeyParameter(
			"VOW_WATF_Q_LV1", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)
			&& (strlen(tmpbuf) > 0)) {
			pAd->vow_watf_q_lv1 = (UINT8)os_str_tol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[1] = pAd->vow_watf_q_lv1;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"VOW_WATF_Q_LV1 --> %d\n", pAd->vow_watf_q_lv1);
		} else
			pAd->vow_watf_q_lv1 = 8;

		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

		if (RTMPGetKeyParameter(
			"VOW_WATF_Q_LV2", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)
			&& (strlen(tmpbuf) > 0)) {
			pAd->vow_watf_q_lv2 = (UINT8)os_str_tol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[2] = pAd->vow_watf_q_lv2;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"VOW_WATF_Q_LV2 --> %d\n", pAd->vow_watf_q_lv2);
		} else
			pAd->vow_watf_q_lv2 = 12;

		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

		if (RTMPGetKeyParameter(
			"VOW_WATF_Q_LV3", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)
			&& (strlen(tmpbuf) > 0)) {
			pAd->vow_watf_q_lv3 = (UINT8)os_str_tol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[3] = pAd->vow_watf_q_lv3;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"VOW_WATF_Q_LV3 --> %d\n", pAd->vow_watf_q_lv3);
		} else
			pAd->vow_watf_q_lv3 = 16;

		/* Weigthed Airtime Fairness - Different DWRR quantum MAC address list*/
		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
		os_alloc_mem(NULL, (UCHAR **)&pwatf_string, 32);
		if(pwatf_string == NULL) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				 "%s: alloc failed, pwatf_string = NULL\n", __func__);
			return;
		}

		os_alloc_mem(NULL, (UCHAR **)&tmp, 32*34);
		if (tmp == NULL) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				 "%s: alloc failed, tmp = NULL\n", __func__);
			os_free_mem(pwatf_string);
			return;
		}
		os_zero_mem(tmp, 32*34);
		for (i = 0; i < VOW_WATF_LEVEL_NUM; i++) {
			ret = snprintf(pwatf_string, 32, "VOW_WATF_MAC_LV%d", i);
			if (os_snprintf_error(32, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE,
					DBG_LVL_ERROR, "snprintf error!\n");
				os_free_mem(pwatf_string);
				os_free_mem(tmp);
				return;
			}

			if (RTMPGetKeyParameter(
				pwatf_string, tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)
				&& (pAd->vow_watf_en)) {
				ULONG str_tem_len;
				UINT16 lv;

				for (j = 0, macptr = rstrtok(tmpbuf, ",");
					 macptr && (j < VOW_WATF_LEVEL_NUM);
					 macptr = rstrtok(NULL, ","), j++) {
					/* Mac address acceptable format 01:02:03:04:05:06 */
					if (strlen(macptr) != MAC_ADDR_STR_LEN)
						continue;
					str_tem_len = MAC_ADDR_STR_LEN + 2 + 2;
					lv = i;

					ret = snprintf(tmp, str_tem_len, "%d-%s", lv, macptr);
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
						    "tmp=%s, ret=%u, str_tem_len=%lu\n",
							 tmp, ret, str_tem_len);
					if (os_snprintf_error(str_tem_len, ret)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE,
							DBG_LVL_ERROR, "snprintf error!\n");
						os_free_mem(pwatf_string);
						os_free_mem(tmp);
						return;
					}

					if (!set_vow_watf_add_entry(pAd, tmp))
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE,
						DBG_LVL_ERROR,
						"set_vow_watf_add_entry fail, %s\n", tmp);
				}
			}
		}

		if (pwatf_string != NULL)
			os_free_mem(pwatf_string);

		if (tmp != NULL)
			os_free_mem(tmp);
	}

	/* fast round robin */
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (RTMPGetKeyParameter("VOW_STA_FRR_QUANTUM", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		pAd->vow_sta_frr_quantum =  (UINT8)simple_strtol(tmpbuf, 0, 10);

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_STA_FRR_QUANTUM --> %d\n", pAd->vow_sta_frr_quantum);
	}

	if (RTMPGetKeyParameter("VOW_Schedule_Type", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		if ((strcmp(tmpbuf, "HW") == 0)
			|| (strcmp(tmpbuf, "Hw") == 0)
			|| (strcmp(tmpbuf, "hw") == 0))
			pAd->vow_sch_cfg.sch_type = (UINT8)VOW_SCH_FOLLOW_HW;
		else
			pAd->vow_sch_cfg.sch_type = (UINT8)VOW_SCH_FOLLOW_ALGO;
		/* use mcli_sch_cfg.sch_type to record default setting */
		pAd->vow_cfg.mcli_sch_cfg.sch_type = pAd->vow_sch_cfg.sch_type;

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_Schedule_Type --> %u\n", pAd->vow_sch_cfg.sch_type);
	}

	if (RTMPGetKeyParameter("VOW_Schedule_Policy", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		if ((strcmp(tmpbuf, "WRR") == 0)
			|| (strcmp(tmpbuf, "Wrr") == 0)
			|| (strcmp(tmpbuf, "wrr") == 0))
			pAd->vow_sch_cfg.sch_policy = (UINT8)VOW_SCH_POL_WRR;
		else
			pAd->vow_sch_cfg.sch_policy = (UINT8)VOW_SCH_POL_SRR;
		/* use mcli_sch_cfg.sch_policy to record default setting */
		pAd->vow_cfg.mcli_sch_cfg.sch_policy = pAd->vow_sch_cfg.sch_policy;

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_Schedule_Policy --> %u\n", pAd->vow_sch_cfg.sch_policy);
	}

	if (RTMPGetKeyParameter(
		"VOW_Mcli_Schedule_Enable", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)
		&& (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.mcli_schedule_en =  (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"VOW_Mcli_Balance_Enable --> %d\n", pAd->vow_cfg.mcli_schedule_en);
	}

}
#endif /* VOW_SUPPORT */

#ifdef IGMP_SNOOP_SUPPORT
void rtmp_read_igmp_snoop_from_file(
	IN  PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	INT		i;
	RTMP_STRING	*macptr = NULL;
#ifdef IGMP_TVM_SUPPORT
	UINT32 TVMode = IGMP_TVM_MODE_DISABLE;
	RTMP_STRING *pTVMode = NULL;
	UINT32 IsTVModeEnable = IGMP_TVM_SWITCH_DISABLE;
	RTMP_STRING *pIgmpEnhancedEn = NULL;
#endif /* IGMP_TVM_SUPPORT */

	/*IgmpSnEnable */
	if (RTMPGetKeyParameter("IgmpSnEnable", tmpbuf, 128, buffer, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
			struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "I/F(%s%d) ==> ",
					 INF_MBSSID_DEV_NAME, i);
			if (macptr) {
				wdev->IgmpSnoopEnable = os_str_tol(macptr, 0, 10);
#ifdef IGMP_TVM_SUPPORT
				/* Just remove First Enable Parameter from arg */
				if (strsep(&macptr, "-")) {
					pIgmpEnhancedEn = strsep(&macptr, "-");
					if (pIgmpEnhancedEn) {
						IsTVModeEnable = os_str_toul(
							pIgmpEnhancedEn, 0, 10);

						pTVMode = strsep(&macptr, "-");
						if (pTVMode)
							TVMode = os_str_toul(pTVMode, 0, 10);
					}
				}
				wdev->IsTVModeEnable = IsTVModeEnable;
				wdev->TVModeType = TVMode;
				MTWF_PRINT(
					"%s: Enable = %u, IsTVModeEnable = %u, TVModeType = %u\n",
					wdev->IgmpSnoopEnable, wdev->IsTVModeEnable,
					wdev->TVModeType));
#endif /* IGMP_TVM_SUPPORT */


			}
		}
	}
}
#endif /* IGMP_SNOOP_SUPPORT */

#ifdef CONFIG_MAP_SUPPORT
INT ReadSRMeshUlModeParameterFromFile(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *pBuffer)
{
#ifdef CFG_SUPPORT_FALCON_SR
	if (RTMPGetKeyParameter("SRMeshUlMode", tmpbuf, 32, pBuffer, TRUE)) {
		/* parameter parsing */
		pAd->CommonCfg.SRMeshUlMode = (UCHAR) os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"[SRMeshUlMode] %d\n",
			pAd->CommonCfg.SRMeshUlMode);
	}
#endif /* CFG_SUPPORT_FALCON_SR */
	return TRUE;
}

INT ReadMapParameterFromFile(
    PRTMP_ADAPTER pAd,
    RTMP_STRING *tmpbuf,
    RTMP_STRING *pBuffer)
{
	if (RTMPGetKeyParameter("MapMode", tmpbuf, 25, pBuffer, TRUE)) {
		pAd->MAPMode = (UCHAR) os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"MAP_MODE=%d\n", pAd->MAPMode);
	}
#ifdef EM_PLUS_SUPPORT
	if (RTMPGetKeyParameter("MapStatsDisable", tmpbuf, 25, pBuffer, TRUE)) {
		pAd->MapStatsDisable = (UCHAR) os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"MapStatsDisable=%d\n", pAd->MapStatsDisable);
	} else
		pAd->MapStatsDisable = 0;
#endif
#ifdef CONFIG_MAP_3ADDR_SUPPORT
	if (RTMPGetKeyParameter("MapAccept3Addr", tmpbuf, 25, pBuffer, TRUE)) {
		pAd->MapAccept3Addr = (UCHAR) os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"MapAccept3Addr=%d\n", pAd->MapAccept3Addr);
	}
#endif
	if (RTMPGetKeyParameter("MapBalance", tmpbuf, 25, pBuffer, TRUE)) {
		pAd->MapBalance = (UCHAR) os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"MAP_BALANCE=%u\n", pAd->MapBalance);
	}

	ReadSRMeshUlModeParameterFromFile(pAd, tmpbuf, pBuffer);

#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
	if (IS_MAP_TURNKEY_ENABLE(pAd))
		pAd->ApCfg.bRoamingEnhance = TRUE;
#endif
#endif
	if (IS_MAP_TURNKEY_ENABLE(pAd)) {
		int j;

		for (j = BSS0; j < WDEV_NUM_MAX; j++) {
			struct wifi_dev *wdev = pAd->wdev_list[j];
			if (wdev && wdev->wdev_type == WDEV_TYPE_AP)
				map_make_vend_ie(pAd, (UCHAR)wdev->func_idx);
			if (wdev && wdev->wdev_type == WDEV_TYPE_STA) {
				if (wdev->func_idx >= MAX_APCLI_NUM)
					continue;
				SetApCliEnableByWdev(pAd, wdev, FALSE);
			}
		}
	}

#ifdef CONFIG_RCSA_SUPPORT
	if (IS_MAP_TURNKEY_ENABLE(pAd))
		pAd->CommonCfg.DfsParameter.bRCSAEn = FALSE;
#endif
	return TRUE;
}
#endif /* CONFIG_MAP_SUPPORT */

#ifdef MBO_SUPPORT
MBO_ERR_CODE ReadMboParameterFromFile(
    PRTMP_ADAPTER pAd,
    RTMP_STRING *tmpbuf,
    RTMP_STRING *pBuffer)
{
	INT loop;
	RTMP_STRING *macptr;

	/* MboSupport */
	if (RTMPGetKeyParameter("MboSupport", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
		for (loop = 0, macptr = rstrtok(tmpbuf, ";");
				(macptr && loop < MAX_MBSSID_NUM(pAd));
					macptr = rstrtok(NULL, ";"), loop++) {
			UINT8 Enable;

			Enable = (UINT8)simple_strtol(macptr, 0, 10);
			pAd->ApCfg.MBSSID[loop].wdev.MboCtrl.bMboEnable =
				(Enable) ? TRUE : FALSE;
			pAd->ApCfg.MBSSID[loop].wdev.MboCtrl.MboCapIndication =
				(Enable) ? MBO_AP_CAP_CELLULAR_AWARE : MBO_AP_CAP_NOT_SUPPORT;

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MBO, DBG_LVL_INFO,
				"(bMboEnable[%d]=%d, MboCapIndication = 0x%02x)\n", loop,
				pAd->ApCfg.MBSSID[loop].wdev.MboCtrl.bMboEnable,
				pAd->ApCfg.MBSSID[loop].wdev.MboCtrl.MboCapIndication);

		}
	}

#ifdef CONFIG_STA_SUPPORT
	if (RTMPGetKeyParameter("MboSupport", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
		for (loop = 0, macptr = rstrtok(tmpbuf, ";");
				(macptr && loop < MAX_MULTI_STA);
					macptr = rstrtok(NULL, ";"), loop++) {
			UINT8 Enable;

			Enable = (UINT8)simple_strtol(macptr, 0, 10);
			pAd->StaCfg[loop].wdev.MboCtrl.bMboEnable =
				(Enable) ? TRUE : FALSE;

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MBO, DBG_LVL_INFO,
				" (bMboEnable[%d]=%d)\n", loop,
				pAd->StaCfg[loop].wdev.MboCtrl.bMboEnable);

		}
	}
#endif /* CONFIG_STA_SUPPORT */
	return MBO_SUCCESS;
}
#endif /* MBO_SUPPORT */

#ifdef MWDS
VOID rtmp_read_MWDS_from_file(
	IN  PRTMP_ADAPTER pAd,
	PSTRING tmpbuf,
	PSTRING buffer
)
{
	PSTRING tmpptr = NULL;
#ifdef CONFIG_AP_SUPPORT

	/* ApMWDS */
	if (RTMPGetKeyParameter("ApMWDS", tmpbuf, 256, buffer, TRUE)) {
		INT Value;
		UCHAR i = 0;

		for (i = 0, tmpptr = rstrtok(tmpbuf, ";"); tmpptr; tmpptr = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			Value = (INT) simple_strtol(tmpptr, 0, 10);

			pAd->ApCfg.MBSSID[i].wdev.bDefaultMwdsStatus = (Value == 0) ? FALSE : TRUE;
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"ApMWDS=%d\n", Value);
		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef APCLI_SUPPORT

	/* ApCliMWDS */
	if (RTMPGetKeyParameter("ApCliMWDS", tmpbuf, 256, buffer, TRUE)) {
		INT Value;
		UCHAR i = 0;

		for (i = 0, tmpptr = rstrtok(tmpbuf, ";"); tmpptr; tmpptr = rstrtok(NULL, ";"), i++) {
			if (i >= MAX_APCLI_NUM)
				break;

			Value = (INT) simple_strtol(tmpptr, 0, 10);

			pAd->StaCfg[i].wdev.bDefaultMwdsStatus = (Value == 0) ? FALSE : TRUE;
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"ApCliMWDS=%d\n", Value);
		}
	}

#endif /* APCLI_SUPPORT */
}
#endif /* MWDS */

#ifdef CFG_RED_SUPPORT
void read_red_parms_from_file(
	struct physical_device *ph_dev, char *tmpbuf, char *buffer)
{
	long value = 0;

	/* for enable/disable */
	if (RTMPGetKeyParameter("RED_Enable", tmpbuf, 25, buffer, TRUE)) {
		if (kstrtol(tmpbuf, 10, &value))
			MTWF_PRINT("%s: Error input(=%s)\n",
				__func__,
				tmpbuf);
		else {
			if (value)
				PD_SET_RED_ENABLE(ph_dev, TRUE);
			else
				PD_SET_RED_ENABLE(ph_dev, FALSE);
			MTWF_PRINT("%s: RED_Enable --> %d\n",
				__func__,
				PD_GET_RED_ENABLE(ph_dev));
		}
	}
}
#endif /* CFG_RED_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
void RRM_ReadParametersFromFile(
	IN PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	INT loop;
	RTMP_STRING *macptr;

	/* RRMEnable */
#ifdef CONFIG_AP_SUPPORT
	if (RTMPGetKeyParameter("RRMEnable", tmpbuf, 255, buffer, TRUE)) {
		for (loop = 0, macptr = rstrtok(tmpbuf, ";");
			 (macptr && loop < MAX_MBSSID_NUM(pAd));
			 macptr = rstrtok(NULL, ";"), loop++) {
			LONG Enable;

			Enable = os_str_tol(macptr, 0, 10);
			pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, loop)].wdev.RrmCfg.bDot11kRRMEnable =
				(Enable > 0) ? TRUE : FALSE;
			pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, loop)].wdev.RrmCfg.bDot11kRRMEnableSet = TRUE;
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO,
				"(bDot11kRRMEnable[%d]=%d)\n", loop,
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, loop)].wdev.RrmCfg.bDot11kRRMEnable);
		}
	} else {
		for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++)
			pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, loop)].wdev.RrmCfg.bDot11kRRMEnable = FALSE;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO,
			"pAd->MSTANum=%d\n", pAd->MSTANum);
	if (RTMPGetKeyParameter("RRMEnable", tmpbuf, 255, buffer, TRUE)) {
		for (loop = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && loop < pAd->MSTANum);
			macptr = rstrtok(NULL, ";"), loop++) {
			LONG Enable;

			Enable = os_str_tol(macptr, 0, 10);
			pAd->StaCfg[loop].wdev.RrmCfg.bDot11kRRMEnable =
				(Enable > 0) ? TRUE : FALSE;
			pAd->StaCfg[loop].wdev.RrmCfg.bDot11kRRMEnableSet = TRUE;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO,
				"(STA-bDot11kRRMEnable[%d]=%d)\n", loop,
				pAd->StaCfg[loop].wdev.RrmCfg.bDot11kRRMEnable);
		}
	} else {
		for (loop = 0; loop < pAd->MSTANum; loop++)
			pAd->StaCfg[loop].wdev.RrmCfg.bDot11kRRMEnable = FALSE;
	}
#endif /* CONFIG_STA_SUPPORT */

	if (RTMPGetKeyParameter("RegDomain", tmpbuf, 255, buffer, TRUE)) {
		if (!strncmp(tmpbuf, "Global", 6))
		    pAd->reg_domain = REG_GLOBAL;
		else
		    pAd->reg_domain = REG_LOCAL;

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO,
		"pAd->reg_domain = %u\n",
		pAd->reg_domain);
	}
	return;
}
#endif /* DOT11K_RRM_SUPPORT */

#ifdef CONFIG_DOT11V_WNM
void WNM_ReadParametersFromFile(
	IN PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	INT loop;
	RTMP_STRING *macptr;
	/* WNMEnable */
#ifdef CONFIG_AP_SUPPORT
	if (RTMPGetKeyParameter("WNMEnable", tmpbuf, 255, buffer, TRUE)) {
		for (loop = 0, macptr = rstrtok(tmpbuf, ";");
				(macptr && loop < MAX_MBSSID_NUM(pAd));
					macptr = rstrtok(NULL, ";"), loop++) {
			LONG Enable;
			Enable = simple_strtol(macptr, 0, 10);
			pAd->ApCfg.MBSSID[loop].WNMCtrl.WNMBTMEnable =
				(Enable > 0) ? TRUE : FALSE;
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO,
				"%s::(bDot11vWNMEnable[%d]=%d)\n",
				__FUNCTION__, loop,
				pAd->ApCfg.MBSSID[loop].WNMCtrl.WNMBTMEnable);
		}
	} else {
		for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++)
			pAd->ApCfg.MBSSID[loop].WNMCtrl.WNMBTMEnable = FALSE;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO,
			"pAd->MSTANum=%d\n", pAd->MSTANum);
	if (RTMPGetKeyParameter("WNMEnable", tmpbuf, 255, buffer, TRUE)) {
		for (loop = 0, macptr = rstrtok(tmpbuf, ";");
				(macptr && loop < pAd->MSTANum);
					macptr = rstrtok(NULL, ";"), loop++) {
			LONG Enable;

			Enable = simple_strtol(macptr, 0, 10);
			pAd->StaCfg[loop].WNMCtrl.WNMBTMEnable =
				(Enable > 0) ? TRUE : FALSE;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO,
				"%s::(STA-bDot11vWNMEnable[%d]=%d)\n",
				__FUNCTION__, loop,
				pAd->StaCfg[loop].WNMCtrl.WNMBTMEnable);
		}
	} else {
		for (loop = 0; loop < pAd->MSTANum; loop++)
			pAd->StaCfg[loop].WNMCtrl.WNMBTMEnable = FALSE;
	}
#endif /* CONFIG_STA_SUPPORT */
	return;
}
#endif /* CONFIG_DOT11V_WNM */

#endif /* CONFIG_AP_SUPPORT */
#endif /* !CONFIG_PROFILE_OFF */

