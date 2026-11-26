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
	sec_profile.c

	Abstract:

*/
#include "rt_config.h"

#ifndef CONFIG_PROFILE_OFF

VOID ReadWPAParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
#ifdef CONFIG_AP_SUPPORT
	INT apidx = 0;
	RTMP_STRING tok_str[16];
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	INT staidx = 0;
#endif /* CONFIG_STA_SUPPORT */
	RTMP_STRING *macptr;
	INT ret;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		/*
			In WPA-WPA2 mix mode, it provides a more flexible cipher combination.
			- WPA-AES and WPA2-TKIP
			- WPA-AES and WPA2-TKIPAES
			- WPA-TKIP and WPA2-AES
			- WPA-TKIP and WPA2-TKIPAES
			- WPA-TKIPAES and WPA2-AES
			- WPA-TKIPAES and WPA2-TKIP
			- WPA-TKIPAES and WPA2-TKIPAES (default)
		*/
		if (RTMPGetKeyParameter("WpaMixPairCipher", tmpbuf, 128, pBuffer, TRUE)) {
			struct _SECURITY_CONFIG *pSecConfig = NULL;

			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), apidx++) {
				pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
				SetWdevAuthMode(pSecConfig, macptr);
				SetWdevEncrypMode(pSecConfig, macptr);
			}
		}

		/* GroupKey Rekey Method*/
		if (RTMPGetKeyParameter("RekeyMethod", tmpbuf, PER_BSS_SIZE_8(pAd), pBuffer, TRUE)) {
			struct _SECURITY_CONFIG *pSecConfig = NULL;
#ifndef RT_CFG80211_SUPPORT
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), apidx++) {
				pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
				pSecConfig->GroupReKeyMethod = SEC_GROUP_REKEY_DISABLE;

				if (rtstrcasecmp(macptr, "TIME") == TRUE)
					pSecConfig->GroupReKeyMethod = SEC_GROUP_REKEY_TIME;
				else if (rtstrcasecmp(macptr, "PKT") == TRUE)
					pSecConfig->GroupReKeyMethod = SEC_GROUP_REKEY_PACKET;
				else if (rtstrcasecmp(macptr, "DISCONNECT") == TRUE)
					pSecConfig->GroupReKeyMethod = SEC_GROUP_REKEY_DISCONNECT;

				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
					"I/F(%s%d) GroupKey ReKeyMethod=%x\n",
					INF_MBSSID_DEV_NAME, apidx, pSecConfig->GroupReKeyMethod);
			}
#endif /*#ifndef RT_CFG80211_SUPPORT*/
			/* Apply to remaining MBSS*/
			if (apidx == 1) {
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
#ifndef RT_CFG80211_SUPPORT
					pSecConfig->GroupReKeyMethod = pAd->ApCfg.MBSSID[0].wdev.SecConfig.GroupReKeyMethod;
					MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
						"I/F(%s%d) GroupKey ReKeyMethod=%x\n",
						INF_MBSSID_DEV_NAME, apidx, pSecConfig->GroupReKeyMethod);
#endif /*RT_CFG80211_SUPPORT*/
				}
			}
		}

		/* GroupKey RekeyInterval */
		if (RTMPGetKeyParameter("RekeyInterval", tmpbuf, PER_BSS_SIZE_8(pAd), pBuffer, TRUE)) {
			struct _SECURITY_CONFIG *pSecConfig = NULL;

			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), apidx++) {
				ULONG value_interval;

				pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
				value_interval = os_str_tol(macptr, 0, 10);
#ifndef RT_CFG80211_SUPPORT
				if ((value_interval >= 10) && (value_interval < MAX_GROUP_REKEY_INTERVAL))
					pSecConfig->GroupReKeyInterval = value_interval;
				else /*Default*/
					pSecConfig->GroupReKeyInterval = DEFAULT_GROUP_REKEY_INTERVAL;

				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
					"I/F(%s%d) GroupKey ReKeyInterval=%ld seconds\n",
					INF_MBSSID_DEV_NAME, apidx, pSecConfig->GroupReKeyInterval);
#endif /*RT_CFG80211_SUPPORT*/
			}

			/* Apply to remaining MBSS*/
			if (apidx == 1) {
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
#ifndef RT_CFG80211_SUPPORT
					pSecConfig->GroupReKeyInterval = pAd->ApCfg.MBSSID[0].wdev.SecConfig.GroupReKeyInterval;
					MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
						"I/F(%s%d) ReKeyInterval=%ld\n",
						INF_MBSSID_DEV_NAME, apidx, pSecConfig->GroupReKeyInterval);
#endif /*RT_CFG80211_SUPPORT*/
				}
			}
		}

#ifndef RT_CFG80211_SUPPORT
		/*PMKCachePeriod*/
		if (RTMPGetKeyParameter("PMKCachePeriod", tmpbuf, 255, pBuffer, TRUE)) {
			struct _SECURITY_CONFIG *pSecConfig = NULL;

			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), apidx++) {
				ULONG value_interval;

				pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
				value_interval = os_str_tol(macptr, 0, 10) * 60 * OS_HZ;
				pSecConfig->PMKCachePeriod = value_interval;

				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
					"I/F(%s%d) PMKCachePeriod=%ld\n",
					INF_MBSSID_DEV_NAME, apidx, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].PMKCachePeriod);
			}

			/* Apply to remaining MBSS*/
			if (apidx == 1) {
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
					pSecConfig->PMKCachePeriod = pAd->ApCfg.MBSSID[0].wdev.SecConfig.PMKCachePeriod;
					MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
						"I/F(%s%d) PMKCachePeriod=%ld\n",
						INF_MBSSID_DEV_NAME, apidx, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].PMKCachePeriod);
				}
			}
		}
#endif /*RT_CFG80211_SUPPORT*/
		/*WPAPSK_KEY*/
		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			struct _SECURITY_CONFIG *pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;

			ret = snprintf(tok_str, sizeof(tok_str), "WPAPSK%d", apidx + 1);
			if (os_snprintf_error(sizeof(tok_str), ret)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA,
					DBG_LVL_ERROR, "snprintf error!\n");
				return;
			}

			if (RTMPGetKeyParameter(tok_str, tmpbuf, 65, pBuffer, FALSE)) {
				ULONG len = strlen(tmpbuf);

				if (len < 65) {
					os_move_mem(pSecConfig->PSK, tmpbuf, len);
					pSecConfig->PSK[len] = '\0';
				} else
					pSecConfig->PSK[0] = '\0';
#ifdef WSC_AP_SUPPORT
					NdisZeroMemory(pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.WscControl.WpaPsk, 64);
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.WscControl.WpaPskLen = 0;
					if ((len >= 8) && (len <= 64)) {
						NdisMoveMemory(pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.WscControl.WpaPsk,
									tmpbuf, len);
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.WscControl.WpaPskLen = len;
					}
#endif /* WSC_AP_SUPPORT */

			}
		}

		/* If not match, search WPAPSK */
		if (RTMPGetKeyParameter("WPAPSK", tmpbuf, 512, pBuffer, FALSE)) {
			ULONG len;

			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), apidx++) {
				struct _SECURITY_CONFIG *pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;

				len = strlen(macptr);

				if (len < 65) {
					os_move_mem(pSecConfig->PSK, macptr, len);
					pSecConfig->PSK[len] = '\0';
				} else
					pSecConfig->PSK[0] = '\0';
#ifdef WSC_AP_SUPPORT
					NdisZeroMemory(pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.WscControl.WpaPsk, 64);
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.WscControl.WpaPskLen = 0;
					if ((len >= 8) && (len <= 64)) {
						NdisMoveMemory(pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.WscControl.WpaPsk,
									tmpbuf, len);
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.WscControl.WpaPskLen = len;
					}
#endif /* WSC_AP_SUPPORT */

			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (RTMPGetKeyParameter("WPAPSK", tmpbuf, 512, pBuffer, FALSE)) {
			for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
				struct _SECURITY_CONFIG *pSecConfig = &pAd->StaCfg[staidx].wdev.SecConfig;

				os_move_mem(pSecConfig->PSK, tmpbuf, strlen(tmpbuf));
				pSecConfig->PSK[strlen(tmpbuf)] = '\0';
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */
}

#if (defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)) && !defined(RT_CFG80211_SUPPORT)

VOID insert_pwd_id(struct _SECURITY_CONFIG *sec_cfg, struct pwd_id_list *pwd_id_list_head, RTMP_STRING *arg)
{
	RTMP_STRING *pwdid;
	RTMP_STRING *pwd;
	ULONG len = 0;
	struct pwd_id_list *pwd_id_ins = NULL;

	if (arg == NULL)
		return;

	pwd = rstrtok(arg, ":");
	pwdid = rstrtok(NULL, ":");

	if (pwd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "null pwd\n");
		return;
	}
	if (pwdid == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "null pwdid\n");
		os_zero_mem(sec_cfg->PSK, sizeof(sec_cfg->PSK));
		os_move_mem(sec_cfg->PSK, pwd, strlen(pwd));
		sec_cfg->sae_cap.pwd_id_only = FALSE;
		return;
	}

	os_alloc_mem(NULL, (UCHAR **)&pwd_id_ins, sizeof(struct pwd_id_list));
	if (pwd_id_ins == NULL)
		return;
	os_zero_mem(pwd_id_ins, sizeof(struct pwd_id_list));

	len = strlen(pwd);
	os_move_mem(pwd_id_ins->pwd, pwd, len);
	pwd_id_ins->pwd[len] = '\0';

	len = strlen(pwdid);
	os_move_mem(pwd_id_ins->pwd_id, pwdid, len);
	pwd_id_ins->pwd_id[len] = '\0';

	DlListAddTail(&pwd_id_list_head->list, &pwd_id_ins->list);
	sec_cfg->pwd_id_cnt++;
}

static VOID rm_new_line(
	RTMP_STRING * str)
{
	UINT32 i;
	UINT32 j;

	for (i = 0, j = 0; i < strlen(str); i++) {
		if (str[i] == '\n')
			continue;
		str[j++] = str[i];
	}
	str[j] = '\0';
}

static VOID read_sae_parma_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
	INT i = 0, ret;
	struct _SECURITY_CONFIG *sec_cfg = NULL;
	RTMP_STRING *macptr;

	if (RTMPGetKeyParameter("PweMethod", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR pwe_method = 0;

				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (macptr)
					pwe_method = os_str_tol(macptr, 0, 10);

				if (pwe_method > MAX_PWE_METHOD) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
						"pwe method should not be %d",
						 pwe_method);
				}
				sec_cfg->sae_cap.gen_pwe_method = pwe_method;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR pwe_method = 0;

				sec_cfg = &pAd->StaCfg[i].wdev.SecConfig;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (pwe_method > MAX_PWE_METHOD) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
						"pwe method should not be %d",
						 pwe_method);
				}
				sec_cfg->sae_cap.gen_pwe_method = pwe_method;
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				i = 0;
				while (i < MAX_MBSSID_NUM(pAd))
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_cap.gen_pwe_method = PWE_MIXED;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				i = 0;
				while (i < MAX_MULTI_STA)
					pAd->StaCfg[i++].wdev.SecConfig.sae_cap.gen_pwe_method = PWE_MIXED;
			}
#endif /* CONFIG_STA_SUPPORT */
		}

	if (RTMPGetKeyParameter("PWDIDR", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR pwd_id_only = 0;
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (macptr)
					pwd_id_only = os_str_tol(macptr, 0, 10);
				sec_cfg->sae_cap.pwd_id_only = (pwd_id_only) ? TRUE : FALSE;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR pwd_id_only = 0;
				sec_cfg = &pAd->StaCfg[i].wdev.SecConfig;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (macptr)
					pwd_id_only = os_str_tol(macptr, 0, 10);
				sec_cfg->sae_cap.pwd_id_only = (pwd_id_only) ? TRUE : FALSE;
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				i = 0;
				while (i < MAX_MBSSID_NUM(pAd))
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_cap.pwd_id_only = FALSE;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				i = 0;
				while (i < MAX_MULTI_STA)
					pAd->StaCfg[i++].wdev.SecConfig.sae_cap.pwd_id_only = FALSE;
			}
#endif /* CONFIG_STA_SUPPORT */
		}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		RTMP_STRING *macptr2;
		RTMP_STRING tok_str[16];

		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			ret = snprintf(tok_str, sizeof(tok_str), "PWDID%d", i + 1);
			if (os_snprintf_error(sizeof(tok_str), ret)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE,
					DBG_LVL_ERROR, "snprintf error!\n");
				return;
			}

			if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				DlListInit(&sec_cfg->pwd_id_list_head.list);
				macptr = tmpbuf;
				do {
					macptr = rstrtok(macptr, ";");
					macptr2 = rstrtok(NULL, "\0");

					insert_pwd_id(sec_cfg, &sec_cfg->pwd_id_list_head, macptr);
					macptr = macptr2;
				} while (macptr);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	if (RTMPGetKeyParameter("SAEPK", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR sae_pk_en = 0;

				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (macptr)
					sae_pk_en = os_str_tol(macptr, 0, 10);

				if (sae_pk_en >= MAX_SAE_PK_EN) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
						"sae_pk_en should not be %d",
						 sae_pk_en);
				}
				sec_cfg->sae_cap.sae_pk_en = sae_pk_en;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR sae_pk_en = 0;

				sec_cfg = &pAd->StaCfg[i].wdev.SecConfig;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (sae_pk_en >= MAX_SAE_PK_EN) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
						"sae_pk_en should not be %d",
						 sae_pk_en);
				}
				sec_cfg->sae_cap.sae_pk_en = sae_pk_en;
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			i = 0;
			while (i < MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_cap.sae_pk_en = SAE_PK_DISABLE;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			i = 0;
			while (i < MAX_MULTI_STA)
				pAd->StaCfg[i++].wdev.SecConfig.sae_cap.sae_pk_en = SAE_PK_DISABLE;
		}
#endif /* CONFIG_STA_SUPPORT */
	}

	if (RTMPGetKeyParameter("SAEPKInputMode", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR key_input_mode = 0;

				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (macptr)
					key_input_mode = os_str_tol(macptr, 0, 10);

				if (key_input_mode >= MAX_SAE_PK_KEY_INPUT_MODE) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
						"key_input_mode should not be %d",
						 key_input_mode);
				}
				sec_cfg->sae_cap.key_input_mode = key_input_mode;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			i = 0;
			while (i < MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_cap.key_input_mode = SAE_PK_KEY_INPUT_MODE_HEX;
		}
#endif /* CONFIG_AP_SUPPORT */
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		RTMP_STRING tok_str[19];
		RTMP_STRING head_str[] = "-----BEGIN EC PRIVATE KEY-----";
		RTMP_STRING tail_str[] = "-----END EC PRIVATE KEY-----";

		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			ret = snprintf(tok_str, sizeof(tok_str), "SAEPKKeyFilePath%d", i + 1);
			if (os_snprintf_error(sizeof(tok_str), ret)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE,
					DBG_LVL_ERROR, "snprintf error!\n");
				return;
			}

			if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
				RTMP_OS_FD_EXT srcf;
				INT retval;
				ULONG buf_size = 500;
				RTMP_STRING *buffer = NULL;
				RTMP_STRING *ptr = NULL;
				UCHAR is_found = FALSE;
				UCHAR out[130] = {0};
				UINT32 out_len;

				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;

				if (sec_cfg->sae_cap.key_input_mode != SAE_PK_KEY_INPUT_MODE_FILE_EC_PRIVATE)
					continue;

				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "Open file \"%s\"\n", tmpbuf);

				os_alloc_mem(pAd, (UCHAR **)&buffer, buf_size);

				if (!buffer)
					continue;

				os_file_open(tmpbuf, (RTMP_OS_FD_EXT *)&srcf, O_RDONLY, 0);

				if (srcf.Status)
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, "Open file \"%s\" failed!\n", tmpbuf);
				else {
					retval = os_file_read(srcf, buffer, buf_size - 1);

					if (retval > 0) {
						ptr = rtstrstr(buffer, tail_str);

						if (NdisEqualMemory(buffer, head_str, strlen(head_str)) &&
							ptr != NULL) {
							ptr[0] = '\0';
							ptr = buffer + strlen(head_str);
							rm_new_line(ptr);
							is_found = TRUE;
						}

					if (is_found && !sae_pk_pem_decode(&sec_cfg->sae_pk, ptr, out, &out_len))
								sec_cfg->sae_cap.key_input_mode = SAE_PK_KEY_INPUT_MODE_HEX;

						retval = NDIS_STATUS_SUCCESS;
					} else {
						sec_cfg->sae_cap.key_input_mode = SAE_PK_KEY_INPUT_MODE_HEX;
						MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, "Read file \"%s\" failed(errCode=%d)!\n", tmpbuf, retval);
					}

					if (os_file_close(srcf) != 0) {
						retval = NDIS_STATUS_FAILURE;
						MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, "Close file \"%s\" failed(errCode=%d)!\n", tmpbuf, retval);
					}
				}

				os_free_mem(buffer);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		RTMP_STRING tok_str[16];

		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			ret = snprintf(tok_str, sizeof(tok_str), "SAEPKKey%d", i + 1);
			if (os_snprintf_error(sizeof(tok_str), ret)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE,
					DBG_LVL_ERROR, "snprintf error!\n");
				return;
			}

			if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
				UCHAR pri_key_bin[68];
				UINT32 pri_key_len = sizeof(pri_key_bin);

				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;

				if (sec_cfg->sae_cap.key_input_mode != SAE_PK_KEY_INPUT_MODE_HEX)
					continue;

				str_to_bin(tmpbuf, pri_key_bin, &pri_key_len);

				if (pri_key_len != 0) {
					os_alloc_mem(NULL, (UCHAR **) &sec_cfg->sae_pk.fixed_pri_key, pri_key_len);
					if (sec_cfg->sae_pk.fixed_pri_key == NULL)
						return;
					os_move_mem(sec_cfg->sae_pk.fixed_pri_key, pri_key_bin, pri_key_len);
					sec_cfg->sae_pk.fixed_pri_key_len = pri_key_len;
				}

				hex_dump_with_cat_and_lvl("SAEPKKey", pri_key_bin, pri_key_len, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		RTMP_STRING tok_str[16];

		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			ret = snprintf(tok_str, sizeof(tok_str), "SAEPKStartM%d", i + 1);
			if (os_snprintf_error(sizeof(tok_str), ret)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE,
					DBG_LVL_ERROR, "snprintf error!\n");
				return;
			}

			if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
				UCHAR modifier[SAE_PK_MODIFIER_BYTES_LEN];
				UINT32 modifier_len = SAE_PK_MODIFIER_BYTES_LEN;

				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;

				str_to_bin(tmpbuf, modifier, &modifier_len);

				if (modifier_len == SAE_PK_MODIFIER_BYTES_LEN) {
					os_alloc_mem(NULL, (UCHAR **) &sec_cfg->sae_pk.fixed_start_modifier, SAE_PK_MODIFIER_BYTES_LEN);
					if (sec_cfg->sae_pk.fixed_start_modifier == NULL)
						return;
					os_move_mem(sec_cfg->sae_pk.fixed_start_modifier, modifier, SAE_PK_MODIFIER_BYTES_LEN);
				}


				hex_dump_with_cat_and_lvl("SAEPKStartM", modifier, modifier_len, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	if (RTMPGetKeyParameter("SAEPKSec", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				sec_cfg->sae_pk.sec = os_str_tol(macptr, 0, 10);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			i = 0;
			while (i < MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_pk.sec = SAE_PK_AUTO_GEN_DEF_SEC;
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	if (RTMPGetKeyParameter("SAEPKLambda", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				sec_cfg->sae_pk.lambda = os_str_tol(macptr, 0, 10);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			i = 0;
			while (i < MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_pk.lambda = SAE_PK_AUTO_GEN_DEF_LAMBDA;
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	if (RTMPGetKeyParameter("SAEPKGroup", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;

				if (sec_cfg->sae_cap.key_input_mode != SAE_PK_KEY_INPUT_MODE_HEX)
					continue;

				sec_cfg->sae_pk.group_id = os_str_tol(macptr, 0, 10);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			i = 0;
			while (i < MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_pk.group_id = SAE_DEFAULT_GROUP;
		}
#endif /* CONFIG_AP_SUPPORT */
	}
	/* test or testbed behavior only */
	if (RTMPGetKeyParameter("SAEPKCfg", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR sae_pk_test = 0;

				sae_pk_test = os_str_tol(macptr, 0, 16);
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				sec_cfg->sae_pk.sae_pk_test_ctrl = sae_pk_test;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			i = 0;
			while (i < MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_pk.sae_pk_test_ctrl = 0;
		}
#endif /* CONFIG_AP_SUPPORT */
	}
}
#endif

#if defined(DOT1X_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
VOID ReadRadiusParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING tok_str[16], *macptr;
	UINT32 ip_addr;
	INT i = 0, ret;
	BOOLEAN bUsePrevFormat = FALSE;
	UINT offset;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
#ifdef CONFIG_AP_SUPPORT
	INT apidx;
#endif /* CONFIG_AP_SUPPORT */
#ifdef RADIUS_ACCOUNTING_SUPPORT
	BOOLEAN				bAcctUsePrevFormat = FALSE;
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
	PUCHAR count;
	UCHAR srv_idx = 0;
	os_alloc_mem(NULL, (UCHAR **)&count, MAX_MBSSID_NUM(pAd));
	if (count == NULL)
		return;
	os_zero_mem(count, MAX_MBSSID_NUM(pAd));

	/* own_ip_addr*/
	if (RTMPGetKeyParameter("own_ip_addr", tmpbuf, 32, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
					"I/F(%s%d) ==> ",
					INF_MBSSID_DEV_NAME, apidx);
				SetWdevOwnIPAddr(pSecConfig, macptr);
			}

			/* Apply to remaining MBSS*/
			if (apidx >= 1) {
				/*
				* own_ip_addr is global setting , don't need to merge in dbdc multi profile,
				* in this point, let all bss set the same own_ip_addr for safe
				*/
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
					pSecConfig->own_ip_addr = pAd->ApCfg.MBSSID[0].wdev.SecConfig.own_ip_addr;
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	if (RTMPGetKeyParameter("own_radius_port", tmpbuf, 32, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";");
				 (macptr && apidx < pAd->ApCfg.BssidNum);
				 macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;

				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
						"I/F(%s%d) ==> ", INF_MBSSID_DEV_NAME, apidx);


				pSecConfig->own_radius_port = simple_strtol(macptr, 0, 10);
			}

			/* Apply to remaining MBSS*/
			if (apidx >= 1) {
				/*
				*	own_radius_port is global setting , let all bss set the same own_radius_port
				*/
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
					pSecConfig->own_radius_port =
						pAd->ApCfg.MBSSID[0].wdev.SecConfig.own_radius_port;
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* session_timeout_interval*/
	if (RTMPGetKeyParameter("session_timeout_interval", tmpbuf, 32, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;
				pSecConfig->session_timeout_interval = os_str_tol(macptr, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
					"I/F(%s%d) ==> session_timeout_interval=%d\n",
					INF_MBSSID_DEV_NAME, apidx, pSecConfig->session_timeout_interval);
			}

			/* Apply to remaining MBSS*/
			if (apidx == 1) {
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
					pSecConfig->session_timeout_interval = pAd->ApCfg.MBSSID[0].wdev.SecConfig.session_timeout_interval;
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
						"I/F(%s%d) ==> session_timeout_interval=%d\n",
						INF_MBSSID_DEV_NAME, apidx, pSecConfig->session_timeout_interval);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* quiet_interval */
	if (RTMPGetKeyParameter("quiet_interval", tmpbuf, 32, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;
				pSecConfig->quiet_interval = os_str_tol(macptr, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
					"I/F(%s%d) ==> quiet_interval=%d\n",
					INF_MBSSID_DEV_NAME, apidx, pSecConfig->quiet_interval);
			}

			/* Apply to remaining MBSS*/
			if (apidx == 1) {
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
					pSecConfig->quiet_interval = pAd->ApCfg.MBSSID[0].wdev.SecConfig.quiet_interval;
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
						"I/F(%s%d) ==> quiet_interval=%d\n",
						INF_MBSSID_DEV_NAME, apidx, pSecConfig->quiet_interval);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* EAPifname*/
	if (RTMPGetKeyParameter("EAPifname", tmpbuf, 256, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;

				if (strlen(macptr) > 0 && strlen(macptr) <= IFNAMSIZ) {
					pSecConfig->EAPifname_len = strlen(macptr);
					NdisMoveMemory(pSecConfig->EAPifname, macptr, strlen(macptr));
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
						"I/F(%s%d) ==> EAPifname=%s, len=%d\n",
						INF_MBSSID_DEV_NAME, apidx, pSecConfig->EAPifname, pSecConfig->EAPifname_len);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* PreAuthifname*/
	if (RTMPGetKeyParameter("PreAuthifname", tmpbuf, 256, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;

				if (strlen(macptr) > 0 && strlen(macptr) <= IFNAMSIZ) {
					pSecConfig->PreAuthifname_len = strlen(macptr);
					NdisMoveMemory(pSecConfig->PreAuthifname, macptr, strlen(macptr));
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
						"I/F(%s%d) ==> PreAuthifname=%s, len=%d\n",
						INF_MBSSID_DEV_NAME, apidx, pSecConfig->PreAuthifname, pSecConfig->PreAuthifname_len);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* PreAuth */
	if (RTMPGetKeyParameter("PreAuth", tmpbuf, 256, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;

				if (os_str_tol(macptr, 0, 10) != 0)  /*Enable*/
					pSecConfig->PreAuth = TRUE;
				else /*Disable*/
					pSecConfig->PreAuth = FALSE;

				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
					"I/F(%s%d) ==> PreAuth=%d\n",
					INF_MBSSID_DEV_NAME, apidx, pSecConfig->PreAuth);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* IEEE8021X */
	if (RTMPGetKeyParameter("IEEE8021X", tmpbuf, 256, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;

				if (os_str_tol(macptr, 0, 10) != 0)  /*Enable*/
					pSecConfig->IEEE8021X = TRUE;
				else /*Disable*/
					pSecConfig->IEEE8021X = FALSE;

				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
					"IF(%s%d) ==> IEEE8021X=%d\n",
					INF_MBSSID_DEV_NAME, apidx, pSecConfig->IEEE8021X);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

#ifdef RADIUS_ACCOUNTING_SUPPORT

	/*radius_request_cui
	if(RTMPGetKeyParameter("radius_request_cui", tmpbuf, 32, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			if (i >= pAd->ApCfg.BssidNum)
				break;

			if(os_str_tol(macptr, 0, 10) != 0)
				pAd->ApCfg.MBSSID[i].radius_request_cui = TRUE;
			else
				pAd->ApCfg.MBSSID[i].radius_request_cui = FALSE;

			DBGPRINT(RT_DEBUG_ERROR, ("IF(ra%d), radius_request_cui=%d\n", i, pAd->ApCfg.MBSSID[i].radius_request_cui));
		}
	}*/
	/*radius_acct_authentic*/
	if (RTMPGetKeyParameter("radius_acct_authentic", tmpbuf, 32, pBuffer, TRUE)) {
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;
				pSecConfig->radius_acct_authentic = os_str_tol(macptr, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
					"IF(%s%d) ==> radius_acct_authentic=%d\n",
					INF_MBSSID_DEV_NAME, apidx, pSecConfig->radius_acct_authentic);
			}
		}
	}

	/*acct_interim_interval*/
	if (RTMPGetKeyParameter("acct_interim_interval", tmpbuf, 32, pBuffer, TRUE)) {
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;
				pSecConfig->acct_interim_interval = os_str_tol(macptr, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
					"IF(%s%d) ==> acct_interim_interval=%d\n",
					INF_MBSSID_DEV_NAME, apidx, pSecConfig->acct_interim_interval);
			}
		}
	}

	/*acct_enable*/
	if (RTMPGetKeyParameter("acct_enable", tmpbuf, 32, pBuffer, TRUE)) {
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;
				pSecConfig->acct_enable = os_str_tol(macptr, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
					"IF(%s%d) ==> acct_enable=%d\n",
					INF_MBSSID_DEV_NAME, apidx, pSecConfig->acct_enable);
			}
		}
	}

#endif	/* RADIUS_ACCOUNTING_SUPPORT */
	/* RADIUS_Server */
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Server", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
			pSecConfig = &wdev->SecConfig;
			if (rtinet_aton(macptr, &ip_addr) && (pSecConfig->radius_srv_num < MAX_RADIUS_SRV_NUM)) {
				pSecConfig->radius_srv_info[pSecConfig->radius_srv_num].radius_ip = ip_addr;
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
					"IF(%s%d) ==> radius_ip(seq-%d)=%s\n",
					INF_MBSSID_DEV_NAME, apidx, pSecConfig->radius_srv_num, macptr);
				pSecConfig->radius_srv_num++;
			}
		}
	}
	/* RADIUS_Port */
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Port", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
			pSecConfig = &wdev->SecConfig;
			if (count[apidx] < pSecConfig->radius_srv_num) {
				srv_idx = count[apidx];
				pSecConfig->radius_srv_info[srv_idx].radius_port = (UINT32) os_str_tol(macptr, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
					"IF(%s%d) ==> radius_port(seq-%d)=%d\n",
					INF_MBSSID_DEV_NAME, apidx, 0, pSecConfig->radius_srv_info[0].radius_port);
				count[apidx]++;
			}
		}
	}
	os_free_mem(count);

	/* RADIUS_Key  */
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Key", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		if (strlen(tmpbuf) > pAd->ApCfg.BssidNum)
			bUsePrevFormat = TRUE;

		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
			pSecConfig = &wdev->SecConfig;

			if (strlen(macptr) > 0) {
				RADIUS_SRV_INFO *p_radius_srv_info = &pSecConfig->radius_srv_info[0];

				p_radius_srv_info->radius_key_len = strlen(macptr) > 64 ? 64 : strlen(macptr);
				NdisMoveMemory(p_radius_srv_info->radius_key, macptr, p_radius_srv_info->radius_key_len);
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
					"IF(%s%d) ==> radius_key(seq-%d)=%s, len=%d\n",
					INF_MBSSID_DEV_NAME, apidx, 0, macptr, p_radius_srv_info->radius_key_len);
			}
		}
	}

	if (!bUsePrevFormat) {
		for (i = 0; i < MAX_MBSSID_NUM(pAd); i++) {
			ret = snprintf(tok_str, sizeof(tok_str), "RADIUS_Key%d", i + 1);
			if (os_snprintf_error(sizeof(tok_str), ret)) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X,
					DBG_LVL_ERROR, "snprintf error!\n");
				return;
			}
			offset = 0;
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			pSecConfig = &wdev->SecConfig;
			srv_idx = 0;

			while (RTMPGetKeyParameterWithOffset(tok_str, tmpbuf, &offset, 128, pBuffer, FALSE)) {
				if (strlen(tmpbuf) > 0) {
					if (srv_idx < pSecConfig->radius_srv_num) {
						RADIUS_SRV_INFO *p_radius_srv_info = &pSecConfig->radius_srv_info[srv_idx];

						p_radius_srv_info->radius_key_len = strlen(tmpbuf) > 64 ? 64 : strlen(tmpbuf);
						NdisMoveMemory(p_radius_srv_info->radius_key, tmpbuf, p_radius_srv_info->radius_key_len);
						MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO, "IF(%s%d) ==> radius_key(seq-%d)=%s, len=%d\n",
							INF_MBSSID_DEV_NAME, i, 0, p_radius_srv_info->radius_key, p_radius_srv_info->radius_key_len);
						srv_idx++;
					}
				}
			}
		}
	}

	/* NasIdX, X indicate the interface index(1~8) */
	for (i = 0; i < MAX_MBSSID_NUM(pAd); i++) {
		wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
		pSecConfig = &wdev->SecConfig;
		ret = snprintf(tok_str, sizeof(tok_str), "NasId%d", i + 1);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X,
				DBG_LVL_ERROR, "snprintf error!\n");
			return;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 33, pBuffer, FALSE)) {
			if (strlen(tmpbuf) > 0) {
				pSecConfig->NasIdLen = strlen(tmpbuf) > IFNAMSIZ ? IFNAMSIZ : strlen(tmpbuf);
				NdisMoveMemory(pSecConfig->NasId, tmpbuf, pSecConfig->NasIdLen);
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
					"IF(%s%d) ==> NAS-ID=%s, len=%d\n",
					INF_MBSSID_DEV_NAME, i, pSecConfig->NasId, pSecConfig->NasIdLen);
			}
		}
	}

#ifdef RADIUS_ACCOUNTING_SUPPORT
	/* RADIUS_Acct_Server*/
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Acct_Server", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
			pSecConfig = &wdev->SecConfig;

			if (rtinet_aton(macptr, &ip_addr) && (pSecConfig->radius_acct_srv_num < MAX_RADIUS_SRV_NUM)) {
				pSecConfig->radius_acct_srv_info[pSecConfig->radius_acct_srv_num].radius_ip = ip_addr;
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
					"IF(%s%d) ==> radius_acct_ip(seq-%d)=%s\n",
					INF_MBSSID_DEV_NAME, apidx, pSecConfig->radius_acct_srv_num, macptr);
				pSecConfig->radius_acct_srv_num++;
			}
		}
	}

	/* RADIUS_Acct_Port*/
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Acct_Port", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
			pSecConfig = &wdev->SecConfig;
			pSecConfig->radius_acct_srv_info[0].radius_port = (UINT32) os_str_tol(macptr, 0, 10);	/* TODO: idx */
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
				"IF(%s%d) ==> radius_acct_port(seq-%d)=%d\n",
				INF_MBSSID_DEV_NAME, apidx, 0, pSecConfig->radius_acct_srv_info[0].radius_port);
		}
	}

	/* RADIUS_Key*/
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Acct_Key", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		if (strlen(tmpbuf) > 0)
			bAcctUsePrevFormat = TRUE;

		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
			pSecConfig = &wdev->SecConfig;

			if (strlen(macptr) > 0) {
				RADIUS_SRV_INFO *p_radius_srv_info = &pSecConfig->radius_acct_srv_info[0];	/* TODO: idx */

				p_radius_srv_info->radius_key_len = strlen(macptr) > 64 ? 64 : strlen(macptr);
				NdisMoveMemory(p_radius_srv_info->radius_key, macptr, p_radius_srv_info->radius_key_len);
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
					"IF(%s%d) ==> radius_acct_key(seq-%d)=%s, len=%d\n",
					INF_MBSSID_DEV_NAME, apidx, 0, macptr, p_radius_srv_info->radius_key_len);
			}
		}
	}

	if (!bAcctUsePrevFormat) {
		for (i = 0; i < MAX_MBSSID_NUM(pAd); i++) {
			snprintf(tok_str, sizeof(tok_str), "RADIUS_Acct_Key%d", i + 1);
			offset = 0;
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			pSecConfig = &wdev->SecConfig;

			while (RTMPGetKeyParameterWithOffset(tok_str, tmpbuf, &offset, 128, pBuffer, FALSE)) {
				if (strlen(tmpbuf) > 0) {
					RADIUS_SRV_INFO *p_radius_srv_info = &pSecConfig->radius_acct_srv_info[0];		/* TODO: idx */

					p_radius_srv_info->radius_key_len = strlen(tmpbuf) > 64 ? 64 : strlen(tmpbuf);
					NdisMoveMemory(p_radius_srv_info->radius_key, tmpbuf, p_radius_srv_info->radius_key_len);
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
						"IF(%s%d) ==> radius_acct_key(seq-%d)=%s, len=%d\n",
						INF_MBSSID_DEV_NAME, i, 0, p_radius_srv_info->radius_key, p_radius_srv_info->radius_key_len);
				}
			}
		}
	}

#endif	/* RADIUS_ACCOUNTING_SUPPORT */
}
#endif /* DOT1X_SUPPORT */

#ifdef APCLI_SUPPORT
VOID ReadApcliSecParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr;
	INT i, idx, ret;
	INT apcli_idx;
	RTMP_STRING tok_str[16];

	struct _SECURITY_CONFIG *pSecConfig = NULL;

	/*ApCliAuthMode*/
	if (RTMPGetKeyParameter("ApCliAuthMode", tmpbuf, 255, pBuffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->MSTANum); macptr = rstrtok(NULL, ";"), i++) {
			pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;
			MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "I/F(apcli%d) ==> ", i);
			SetWdevAuthMode(pSecConfig, macptr);
		}
	}

	/*ApCliEncrypType*/
	if (RTMPGetKeyParameter("ApCliEncrypType", tmpbuf, 255, pBuffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->MSTANum); macptr = rstrtok(NULL, ";"), i++) {
			pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;
			MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "I/F(apcli%d) ==> ", i);
			SetWdevEncrypMode(pSecConfig, macptr);
		}
	}

	for (apcli_idx = 0; apcli_idx < pAd->MSTANum; apcli_idx++) {
		pSecConfig = &pAd->StaCfg[apcli_idx].wdev.SecConfig;

		if (apcli_idx == 0) {
			ret = snprintf(tok_str, sizeof(tok_str), "ApCliWPAPSK");
			if (os_snprintf_error(sizeof(tok_str), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI,
					DBG_LVL_ERROR, "snprintf error!\n");
				return;
			}
		} else {
			ret = snprintf(tok_str, sizeof(tok_str), "ApCliWPAPSK%d", apcli_idx);
			if (os_snprintf_error(sizeof(tok_str), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI,
					DBG_LVL_ERROR, "snprintf error!\n");
				return;
			}
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 65, pBuffer, FALSE)) {
			if (strlen(tmpbuf) < 65) {
				os_move_mem(pSecConfig->PSK, tmpbuf, strlen(tmpbuf));
				pSecConfig->PSK[strlen(tmpbuf)] = '\0';
			} else
				pSecConfig->PSK[0] = '\0';
		}
	}

	/*ApCliDefaultKeyID*/
	if (RTMPGetKeyParameter("ApCliDefaultKeyID", tmpbuf, 255, pBuffer, TRUE)) {
		ULONG KeyIdx = 0;
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->MSTANum); macptr = rstrtok(NULL, ";"), i++) {
			pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;
			KeyIdx = os_str_tol(macptr, 0, 10);

			if ((KeyIdx >= 1) && (KeyIdx <= 4))
				pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
			else
				pSecConfig->PairwiseKeyId = 0;

			MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
				"I/F(apcli%d)) ==> DefaultKeyId=%d\n",
				i, pSecConfig->PairwiseKeyId);
		}
	}

	/*ApCliKeyXType, ApCliKeyXStr*/
	for (idx = 0; idx < SEC_SHARE_KEY_NUM; idx++) {
		RTMP_STRING tok_str[16];

		ret = snprintf(tok_str, sizeof(tok_str), "ApCliKey%dType", idx + 1);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI,
				DBG_LVL_ERROR, "snprintf error!\n");
			return;
		}

		/*ApCliKey1Type*/
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, pBuffer, TRUE)) {
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (i = 0; i < pAd->MSTANum; i++) {
					if (i == 0)
						ret = snprintf(tok_str, sizeof(tok_str), "ApCliKey%dStr", idx + 1);
					else
						ret = snprintf(tok_str, sizeof(tok_str), "ApCliKey%dStr%d", idx + 1, i);
					if (os_snprintf_error(sizeof(tok_str), ret)) {
						MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI,
							DBG_LVL_ERROR, "snprintf error!\n");
						return;
					}

					if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, pBuffer, FALSE)) {
						pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;
						ParseWebKey(pSecConfig, tmpbuf, idx, 0);
					}
				}
			}
		}
	}

#ifdef DOT11_SAE_SUPPORT
	/*ApCliPweMethod*/
	if (RTMPGetKeyParameter("ApCliPweMethod", tmpbuf, 255, pBuffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->MSTANum); macptr = rstrtok(NULL, ";"), i++) {
			pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(apcli%d) ==> ", i);
			pSecConfig->sae_cap.gen_pwe_method = os_str_tol(macptr, 0, 10);
		}
	} else {
		for (i = 0; i < pAd->MSTANum; i++)
			pAd->StaCfg[i].wdev.SecConfig.sae_cap.gen_pwe_method = PWE_MIXED;
	}

	/*ApCliSAEPK*/
	if (RTMPGetKeyParameter("ApCliSAEPK", tmpbuf, 255, pBuffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->MSTANum); macptr = rstrtok(NULL, ";"), i++) {
			pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(apcli%d) ==> ", i);
			pSecConfig->sae_cap.sae_pk_en = os_str_tol(macptr, 0, 10);
		}
	} else {
		for (i = 0; i < pAd->MSTANum; i++)
			pAd->StaCfg[i].wdev.SecConfig.sae_cap.sae_pk_en = SAE_PK_DISABLE;
	}

	/*ApCliSAEGroup*/
	if (RTMPGetKeyParameter("ApCliSAEGroup", tmpbuf, 255, pBuffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->MSTANum); macptr = rstrtok(NULL, ";"), i++) {
			UCHAR group;

			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(apcli%d) ==> ", i);
			group = os_str_tol(macptr, 0, 10);
			if (group != 19 && group != 20 && group != 21)
				group = SAE_DEFAULT_GROUP;
			pAd->StaCfg[i].sae_cfg_group = group;
		}
	} else {
		for (i = 0; i < pAd->MSTANum; i++)
			pAd->StaCfg[i].sae_cfg_group = SAE_DEFAULT_GROUP;
	}
#endif
	/* ApCliTransDisableSupported */
	if (RTMPGetKeyParameter("ApCliTransDisableSupported", tmpbuf, 255, pBuffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->MSTANum); macptr = rstrtok(NULL, ";"), i++) {

			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(apcli%d) ==>", i);

			pAd->StaCfg[i].ApCliTransDisableSupported = os_str_tol(macptr, 0, 10);

			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
				"i:%d ApCliTransDisableSupported :%d\n",
				i, pAd->StaCfg[i].ApCliTransDisableSupported);

			if (pAd->StaCfg[i].ApCliTransDisableSupported)
				NdisZeroMemory(&(pAd->StaCfg[i].ApCli_tti_bitmap), sizeof(struct transition_disable_bitmap));
		}
	} else {
		for (i = 0; i < pAd->MSTANum; i++)
			pAd->StaCfg[i].ApCliTransDisableSupported = 0;
	}
	/* ApCliOCVSupport */
	if (RTMPGetKeyParameter("ApCliOCVSupport", tmpbuf, 255, pBuffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;
		struct _SECURITY_CONFIG *pSecConfig = NULL;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->MSTANum); macptr = rstrtok(NULL, ";"), i++) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(apcli%d) ==>", i);
			pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;

			if (pSecConfig) {
				pSecConfig->apcli_ocv_support = os_str_tol(macptr, 0, 10);

				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
					"i:%d ApCliOCVSupport :%d\n",
					i, pSecConfig->apcli_ocv_support);
			}
		}
	} else {
		for (i = 0; i < pAd->MSTANum; i++)
			pAd->StaCfg[i].wdev.SecConfig.apcli_ocv_support = 0;
	}
}
#endif /* APCLI_SUPPORT */

#ifdef WDS_SUPPORT
VOID ReadWDSSecParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr;
	INT i, idx, ret;
	BOOLEAN	bUsePrevFormat = FALSE;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	/* WDS direct insert Key to Asic, not need do 4-way */
	/* WdsEncrypType */
	if (RTMPGetKeyParameter("WdsEncrypType", tmpbuf, 255, pBuffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			pSecConfig = &pAd->WdsTab.WdsEntry[i].wdev.SecConfig;
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "I/F(wds%d) ==> ", i);
			SetWdevEncrypMode(pSecConfig, macptr);
		}
	}

	/*WdsKey*/
	if (RTMPGetKeyParameter("WdsKey", tmpbuf, 255, pBuffer, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			pSecConfig = &pAd->WdsTab.WdsEntry[i].wdev.SecConfig;

			if ((strlen(macptr) > 0) && (strlen(macptr) < 65)) {
				os_move_mem(pSecConfig->PSK, macptr, strlen(macptr));
				pSecConfig->PSK[strlen(macptr)] = '\0';
				bUsePrevFormat = TRUE;
			} else
				pSecConfig->PSK[0] = '\0';
		}
	}

	/*WdsDefaultKeyID*/
	if (RTMPGetKeyParameter("WdsDefaultKeyID", tmpbuf, 255, pBuffer, TRUE)) {
		ULONG KeyIdx = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			if (i == pAd->WdsTab.wds_num)
				break;
			pSecConfig = &pAd->WdsTab.WdsEntry[i].wdev.SecConfig;
			KeyIdx = os_str_tol(macptr, 0, 10);

			if ((KeyIdx >= 1) && (KeyIdx <= 4))
				pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
			else
				pSecConfig->PairwiseKeyId = 0;

			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
				"I/F(wds%d)) ==> DefaultKeyId=%d\n",
				i, pSecConfig->PairwiseKeyId);
		}
	}

	/*WdsXKey */
	if (bUsePrevFormat == FALSE) {
		i = 0;
		for (idx = 0; idx < MAX_WDS_ENTRY; idx++) {
			RTMP_STRING tok_str[16];

			ret = snprintf(tok_str, sizeof(tok_str), "Wds%dKey", idx);
			if (os_snprintf_error(sizeof(tok_str), ret)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY,
					DBG_LVL_ERROR, "snprintf error!\n");
				return;
			}
			if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, pBuffer, FALSE)) {
				for (macptr = rstrtok(tmpbuf, ";"); macptr && (i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
					if (i == pAd->WdsTab.wds_num)
						break;
					pSecConfig = &pAd->WdsTab.WdsEntry[i].wdev.SecConfig;
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_DEBUG,
						"I/F(wds%d)) ==> Key string=%s\n", i, macptr);
							if (IS_CIPHER_WEP(pSecConfig->PairwiseCipher))
								ParseWebKey(pSecConfig, macptr, pSecConfig->PairwiseKeyId, 0);
					else if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher)
							 || IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher)
							 || IS_CIPHER_CCMP256(pSecConfig->PairwiseCipher)
							 || IS_CIPHER_GCMP128(pSecConfig->PairwiseCipher)
							 || IS_CIPHER_GCMP256(pSecConfig->PairwiseCipher)) {
						if (strlen(macptr) < 65) {
							os_move_mem(pSecConfig->PSK, macptr, strlen(macptr));
							pSecConfig->PSK[strlen(macptr)] = '\0';
						} else
							pSecConfig->PSK[0] = '\0';
					}
				}
			}
		}
	}
}
#endif /* WDS_SUPPORT */


#ifdef DOT11W_PMF_SUPPORT
/*
========================================================================
Routine Description:
    Protection Management Frame Capable
    Protection Management Frame Required

Arguments:

Return Value:

Note:
RSNA policy selection in a ESS: IEEE P802.11w Table 8-1a
RSNA policy selection in an IBSS: IEEE P802.11w Table 8-1b
========================================================================
*/
void rtmp_read_pmf_parameters_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * tmpbuf,
	IN RTMP_STRING * pBuffer)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		INT apidx;
		POS_COOKIE pObj;
		RTMP_STRING *macptr;

		pObj = (POS_COOKIE) pAd->OS_Cookie;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_MFPC = FALSE;
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_MFPR = FALSE;
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_PMFSHA256 = FALSE;
		}

		/* Protection Management Frame Capable */
		if (RTMPGetKeyParameter("PMFMFPC", tmpbuf, PER_BSS_SIZE_2(pAd), pBuffer, TRUE)) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				pObj->ioctl_if = apidx;
				Set_PMFMFPC_Proc(pAd, macptr);
			}
		}

		/* Protection Management Frame Required */
		if (RTMPGetKeyParameter("PMFMFPR", tmpbuf, PER_BSS_SIZE_2(pAd), pBuffer, TRUE)) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				pObj->ioctl_if = apidx;
				Set_PMFMFPR_Proc(pAd, macptr);
			}
		}

		if (RTMPGetKeyParameter("PMFSHA256", tmpbuf, PER_BSS_SIZE_2(pAd), pBuffer, TRUE)) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				pObj->ioctl_if = apidx;
				Set_PMFSHA256_Proc(pAd, macptr);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		INT staidx;
		POS_COOKIE pObj;
		RTMP_STRING *macptr;

		pObj = (POS_COOKIE) pAd->OS_Cookie;

		for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
			pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg.Desired_MFPC = FALSE;
			pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg.Desired_MFPR = FALSE;
			pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg.Desired_PMFSHA256 = FALSE;
		}

		/* Protection Management Frame Capable */
		if (RTMPGetKeyParameter("PMFMFPC", tmpbuf, 32, pBuffer, TRUE)) {
			for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
				pObj->ioctl_if = staidx;
				Set_PMFMFPC_Proc(pAd, macptr);
			}
		}

		/* Protection Management Frame Required */
		if (RTMPGetKeyParameter("PMFMFPR", tmpbuf, 32, pBuffer, TRUE)) {
			for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
				pObj->ioctl_if = staidx;
				Set_PMFMFPR_Proc(pAd, macptr);
			}
		}

		if (RTMPGetKeyParameter("PMFSHA256", tmpbuf, 32, pBuffer, TRUE)) {
			for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
				pObj->ioctl_if = staidx;
				Set_PMFSHA256_Proc(pAd, macptr);
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */
}
#endif /* DOT11W_PMF_SUPPORT */

#ifdef BCN_PROTECTION_SUPPORT
VOID read_bcn_prot_parma_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * tmpbuf,
	IN RTMP_STRING * pBuffer)
{
#ifdef CONFIG_AP_SUPPORT
	INT i = 0;
	struct _SECURITY_CONFIG *sec_cfg = NULL;
	RTMP_STRING *macptr;
#endif

	if (RTMPGetKeyParameter("BcnProt", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR bcn_prot = 0;
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO,
					"I/F(%s%d) ==> ", INF_MBSSID_DEV_NAME, i);
				if (macptr)
					bcn_prot = os_str_tol(macptr, 0, 10);
				sec_cfg->bcn_prot_cfg.desired_bcn_prot_en = (bcn_prot) ? TRUE : FALSE;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				i = 0;
				while (i < MAX_MBSSID_NUM(pAd))
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.bcn_prot_cfg.desired_bcn_prot_en = FALSE;
			}
#endif /* CONFIG_AP_SUPPORT */
		}
}
#endif /* BCN_PROTECTION_SUPPORT */


static VOID read_ocv_parma_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
#ifdef CONFIG_AP_SUPPORT
	INT i = 0;
	struct _SECURITY_CONFIG *sec_cfg = NULL;
	RTMP_STRING *macptr;
#endif /* CONFIG_AP_SUPPORT */

	if (RTMPGetKeyParameter("OCVSupport", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR ocv_support = 0;

				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (macptr)
					ocv_support = os_str_tol(macptr, 0, 10);

				sec_cfg->ocv_support = ocv_support;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				i = 0;
				while (i < MAX_MBSSID_NUM(pAd))
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.ocv_support = 0;
			}
#endif /* CONFIG_AP_SUPPORT */
	}
}

static VOID read_td_parma_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * tmpbuf,
	IN RTMP_STRING * pBuffer)
{
#ifdef CONFIG_AP_SUPPORT
	INT i = 0;
	struct _SECURITY_CONFIG *sec_cfg = NULL;
	RTMP_STRING *macptr;
#endif /* CONFIG_AP_SUPPORT */

	if (RTMPGetKeyParameter("TransitionDisable", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (macptr) {
					sec_cfg->td_value_fixed_en = TRUE;
					sec_cfg->td_value = os_str_tol(macptr, 0, 10);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				i = 0;
				while (i < MAX_MBSSID_NUM(pAd))
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.td_value_fixed_en = FALSE;
			}
#endif /* CONFIG_AP_SUPPORT */
	}
}

VOID ReadSecurityParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr;
#ifdef CONFIG_AP_SUPPORT
	INT apidx;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	INT staidx;
#endif /* CONFIG_STA_SUPPORT */
	INT idx, ret;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	/*AuthMode*/
	if (RTMPGetKeyParameter("AuthMode", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
					"I/F(%s%d) ==> ",
					INF_MBSSID_DEV_NAME, apidx);
				SetWdevAuthMode(pSecConfig, macptr);
				wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
				wdev = &pAd->StaCfg[staidx].wdev;
				pSecConfig = &wdev->SecConfig;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
					"I/F(%s%d) ==> ",
					INF_MBSSID_DEV_NAME, staidx);
				SetWdevAuthMode(pSecConfig, macptr);
				wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	}

	/*EncrypType*/
	if (RTMPGetKeyParameter("EncrypType", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
					"I/F(%s%d) ==> ",
					INF_MBSSID_DEV_NAME, apidx);
				SetWdevEncrypMode(pSecConfig, macptr);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
				pSecConfig = &pAd->StaCfg[staidx].wdev.SecConfig;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
					"I/F(%s%d) ==> ",
					INF_MBSSID_DEV_NAME, staidx);
				SetWdevEncrypMode(pSecConfig, macptr);
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	}

	/* Web DefaultKeyID */
	if (RTMPGetKeyParameter("DefaultKeyID", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
		ULONG KeyIdx = 0;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
				KeyIdx = os_str_tol(macptr, 0, 10);

				if ((KeyIdx >= 1) && (KeyIdx <= 4))
					pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
				else
					pSecConfig->PairwiseKeyId = 0;

				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
					"I/F(%s%d)) ==> DefaultKeyId=%d\n",
					INF_MBSSID_DEV_NAME, apidx, pSecConfig->PairwiseKeyId);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
				pSecConfig = &pAd->StaCfg[staidx].wdev.SecConfig;
				KeyIdx = os_str_tol(tmpbuf, 0, 10);

				if ((KeyIdx >= 1) && (KeyIdx <= 4))
					pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
				else
					pSecConfig->PairwiseKeyId = 0;

				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
					"I/F(%s%d)) ==> DefaultKeyId=%d\n",
					INF_MBSSID_DEV_NAME, staidx, pSecConfig->PairwiseKeyId);
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	}

	/* KeyType, KeyStr for WEP  */
	for (idx = 0; idx < 4; idx++) {
		INT i = 0;
		RTMP_STRING tok_str[16];
		ULONG KeyType[MAX_BEACON_NUM];

		ret = snprintf(tok_str, sizeof(tok_str), "Key%dType", idx + 1);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY,
				DBG_LVL_ERROR, "snprintf error!\n");
			return;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++)
				if (i < MAX_MBSSID_NUM(pAd))
					KeyType[i] = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				BOOLEAN bKeyxStryIsUsed = FALSE;

				for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					ret = snprintf(tok_str, sizeof(tok_str),
						"Key%dStr%d", idx + 1, i + 1);
					if (os_snprintf_error(sizeof(tok_str), ret)) {
						MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY,
							DBG_LVL_ERROR, "Line%d: snprintf error!\n",
							__LINE__);
						break;
					}
					if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
						pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
						ParseWebKey(pSecConfig, tmpbuf, idx, 0);

						if (bKeyxStryIsUsed == FALSE)
							bKeyxStryIsUsed = TRUE;
					}
				}

				if (bKeyxStryIsUsed == FALSE) {
					ret = snprintf(tok_str, sizeof(tok_str),
						"Key%dStr", idx + 1);

					if (os_snprintf_error(sizeof(tok_str), ret)) {
						MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY,
							DBG_LVL_ERROR, "Line%d: snprintf error!\n",
							__LINE__);
					} else if (RTMPGetKeyParameter(tok_str,
						tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
						if (pAd->ApCfg.BssidNum == 1) {
							pSecConfig = &pAd->ApCfg.MBSSID[BSS0].wdev.SecConfig;
							ParseWebKey(pSecConfig, tmpbuf, idx, 0);
						} else {
							/* Anyway, we still do the legacy dissection of the whole KeyxStr string.*/
							for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
								pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
								ParseWebKey(pSecConfig, macptr, idx, 0);
							}
						}
					}
				}
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				BOOLEAN bKeyxStryIsUsed = FALSE;

				for (staidx = 0; staidx < MAX_MULTI_STA; staidx++) {
					ret = snprintf(tok_str, sizeof(tok_str),
						"Key%dStr%d", idx + 1, i + 1);

					if (os_snprintf_error(sizeof(tok_str), ret)) {
						MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY,
							DBG_LVL_ERROR, "(line%d)snprintf error!\n",
							__LINE__);
					} else if (RTMPGetKeyParameter(tok_str, tmpbuf,
							MAX_PARAMETER_LEN, pBuffer, FALSE)) {
						pSecConfig = &pAd->StaCfg[staidx].wdev.SecConfig;
						ParseWebKey(pSecConfig, tmpbuf, idx, 0);

						if (bKeyxStryIsUsed == FALSE)
							bKeyxStryIsUsed = TRUE;
					}
				}

				if (bKeyxStryIsUsed == FALSE) {
					ret = snprintf(tok_str, sizeof(tok_str),
						"Key%dStr", idx + 1);

					if (os_snprintf_error(sizeof(tok_str), ret)) {
						MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY,
							DBG_LVL_ERROR, "(line%d)snprintf error!\n",
							__LINE__);
					} else if (RTMPGetKeyParameter(tok_str, tmpbuf,
							MAX_PARAMETER_LEN, pBuffer, FALSE)) {
						if (pAd->MSTANum == 1) {
							pSecConfig = &pAd->StaCfg[MAIN_MSTA_ID].wdev.SecConfig;
							ParseWebKey(pSecConfig, tmpbuf, idx, 0);
						} else {
							/* Anyway, we still do the legacy dissection of the whole KeyxStr string.*/
							for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
								pSecConfig = &pAd->StaCfg[staidx].wdev.SecConfig;
								ParseWebKey(pSecConfig, macptr, idx, 0);
							}
						}
					}
				}
			}
#endif /* CONFIG_STA_SUPPORT */
		}
	}

	ReadWPAParameterFromFile(pAd, tmpbuf, pBuffer);
#if defined(DOT11_SAE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
	read_sae_parma_from_file(pAd, tmpbuf, pBuffer);
#endif
#if defined(DOT1X_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
	ReadRadiusParameterFromFile(pAd, tmpbuf, pBuffer);
#endif /* DOT1X_SUPPORT */
#ifdef APCLI_SUPPORT
	ReadApcliSecParameterFromFile(pAd, tmpbuf, pBuffer);
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
	ReadWDSSecParameterFromFile(pAd, tmpbuf, pBuffer);
#endif /* WDS_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT
	rtmp_read_pmf_parameters_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT11W_PMF_SUPPORT */
#ifdef BCN_PROTECTION_SUPPORT
	read_bcn_prot_parma_from_file(pAd, tmpbuf, pBuffer);
#endif
	read_ocv_parma_from_file(pAd, tmpbuf, pBuffer);
	read_td_parma_from_file(pAd, tmpbuf, pBuffer);
}

#ifdef DOT11R_FT_SUPPORT
/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
void FT_rtmp_read_parameters_from_file(
	IN PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *pBuffer)
{
	INT Loop;

	/* FtSupport */
	if (RTMPGetKeyParameter("FtSupport", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
			macptr = rstrtok(NULL, ";"), Loop++) {
			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			if (os_str_tol(macptr, 0, 10) != 0) /*Enable */
				pAd->ApCfg.MBSSID[Loop].wdev.FtCfg.FtCapFlag.Dot11rFtEnable = TRUE;
			else /*Disable */
				pAd->ApCfg.MBSSID[Loop].wdev.FtCfg.FtCapFlag.Dot11rFtEnable = FALSE;

			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
				"I/F(ra%d) Dot11rFtEnable=%d\n",
				Loop,
				pAd->ApCfg.MBSSID[Loop].wdev.FtCfg.FtCapFlag.Dot11rFtEnable);
		}
	} else {
		Loop = 0;
		while (Loop < MAX_MBSSID_NUM(pAd))
			pAd->ApCfg.MBSSID[Loop++].wdev.FtCfg.FtCapFlag.Dot11rFtEnable = FALSE;
	}

	/* FtOnly */
	if (RTMPGetKeyParameter("FtOnly", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
			macptr = rstrtok(NULL, ";"), Loop++) {
			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			if (os_str_tol(macptr, 0, 10) != 0) /*Enable */
				pAd->ApCfg.MBSSID[Loop].wdev.SecConfig.ft_only = TRUE;
			else /*Disable */
				pAd->ApCfg.MBSSID[Loop].wdev.SecConfig.ft_only = FALSE;

			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
				"I/F(ra%d) ft_only=%d\n",
				Loop,
				pAd->ApCfg.MBSSID[Loop].wdev.SecConfig.ft_only);
		}
	} else {
		Loop = 0;
		while (Loop < MAX_MBSSID_NUM(pAd))
			pAd->ApCfg.MBSSID[Loop++].wdev.SecConfig.ft_only = FALSE;
	}

	/* FtRic */
	if (RTMPGetKeyParameter("FtRic", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
			macptr = rstrtok(NULL, ";"), Loop++) {
			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			if (os_str_tol(macptr, 0, 10) != 0) /*Enable */
				pAd->ApCfg.MBSSID[Loop].wdev.FtCfg.FtCapFlag.RsrReqCap = TRUE;
			else /*Disable */
				pAd->ApCfg.MBSSID[Loop].wdev.FtCfg.FtCapFlag.RsrReqCap = FALSE;

			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
				"I/F(ra%d) Dot11rFtRic=%d\n",
				Loop,
				pAd->ApCfg.MBSSID[Loop].wdev.FtCfg.FtCapFlag.RsrReqCap);
		}
	} else {
		Loop = 0;
		while (Loop < MAX_MBSSID_NUM(pAd))
			pAd->ApCfg.MBSSID[Loop++].wdev.FtCfg.FtCapFlag.RsrReqCap = FALSE;
	}

	/* FtOtd */
	if (RTMPGetKeyParameter("FtOtd", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
			macptr = rstrtok(NULL, ";"), Loop++) {
			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			if (os_str_tol(macptr, 0, 10) != 0) /*Enable */
				pAd->ApCfg.MBSSID[Loop].wdev.FtCfg.FtCapFlag.FtOverDs = TRUE;
			else /*Disable */
				pAd->ApCfg.MBSSID[Loop].wdev.FtCfg.FtCapFlag.FtOverDs = FALSE;

			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
				"I/F(ra%d) Dot11rFtOtd=%d\n",
				Loop,
				pAd->ApCfg.MBSSID[Loop].wdev.FtCfg.FtCapFlag.FtOverDs);
		}
	} else {
		Loop = 0;
		while (Loop < MAX_MBSSID_NUM(pAd))
			pAd->ApCfg.MBSSID[Loop++].wdev.FtCfg.FtCapFlag.FtOverDs = FALSE;
	}

	for (Loop = 0; Loop < MAX_MBSSID_NUM(pAd); Loop++) {
		RTMP_STRING tok_str[16];
		/*
			FtMdId:
			FtMdId shall be a value of two octets.
		*/
		NdisZeroMemory(tok_str, sizeof(tok_str));
		(void)snprintf(tok_str, sizeof(tok_str), "FtMdId%d", Loop + 1);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 32, pBuffer, FALSE)) {

			RTMP_STRING	 *value = NULL, *mode = NULL;
			PFT_CFG pFtCfg;

			pFtCfg = &pAd->ApCfg.MBSSID[Loop].wdev.FtCfg;

			value = rstrtok(tmpbuf, ":");
			if (value)
				mode = rstrtok(NULL, ":");

			if (mode && NdisCmpMemory(mode, "H", 1) == 0) {
				UINT16 hex_value = 0;

				hex_value = simple_strtol(value, 0, 16);
				NdisMoveMemory(pFtCfg->FtMdId, &hex_value, FT_MDID_LEN);

				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_ERROR,
					"Invalid interface number (%s)(%04x).\n",
					value, hex_value);
			} else

			{
				if (value && strlen(value) == FT_MDID_LEN) {
					NdisMoveMemory(pFtCfg->FtMdId, value, FT_MDID_LEN);
					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
						"FtMdid(%d)=%c%c\n",
						Loop, pFtCfg->FtMdId[0], pFtCfg->FtMdId[1]);
				} else {
					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
						"Invalid MdId=%s\n", value);
				}
			}
		}

		/*
			FtR0khId:
			FtR0khId shall be in string of 1 ~ 48 octets.
		*/
		NdisZeroMemory(tok_str, sizeof(tok_str));
		(void)snprintf(tok_str, sizeof(tok_str), "FtR0khId%d", Loop + 1);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, FT_ROKH_ID_LEN + 1, pBuffer, FALSE)) {
			if (strlen(tmpbuf) <= FT_ROKH_ID_LEN) {
				NdisMoveMemory(pAd->ApCfg.MBSSID[Loop].wdev.FtCfg.FtR0khId,
					tmpbuf, strlen(tmpbuf));
				pAd->ApCfg.MBSSID[Loop].wdev.FtCfg.FtR0khId[strlen(tmpbuf)] = '\0';
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
					"FtR0khId(%d)=%s\n", Loop,
					pAd->ApCfg.MBSSID[Loop].wdev.FtCfg.FtR0khId);
			} else {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
					"Invalid R0khId(%d)=%s Len=%d\n",
					Loop, tmpbuf, (INT)strlen(tmpbuf));
			}
		}
	}
}
#endif /* DOT11R_FT_SUPPORT */

#ifdef OCE_SUPPORT

void Oce_read_parameters_from_file(
	IN PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *pBuffer)
{
	struct wifi_dev *wdev = NULL;
	OCE_CTRL *oceCtrl = NULL;
	INT Loop = 0;

	if (RTMPGetKeyParameter("OCE_SUPPORT", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
			macptr = rstrtok(NULL, ";"), Loop++) {

			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;


			if (os_str_tol(macptr, 0, 10) != 0)
				oceCtrl->bOceEnable = TRUE;
			else
				oceCtrl->bOceEnable = FALSE;

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_INFO,
				"I/F(ra%d) OCE_SUPPORT=%d\n", Loop, oceCtrl->bOceEnable);
		}
	}

	if (RTMPGetKeyParameter("OCE_FD_FRAME", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
			macptr = rstrtok(NULL, ";"), Loop++) {
			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;


			if (os_str_tol(macptr, 0, 10) != 0)
				oceCtrl->bFdFrameEnable = TRUE;
			else
				oceCtrl->bFdFrameEnable = FALSE;

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_INFO,
				"I/F(ra%d) OCE_FD_FRAME=%d\n", Loop, oceCtrl->bFdFrameEnable);
		}
	}

	if (RTMPGetKeyParameter("OCE_FILS_HLP", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
			macptr = rstrtok(NULL, ";"), Loop++) {

			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (os_str_tol(macptr, 0, 10) != 0) {
				oceCtrl->bOceFilsHlpEnable = TRUE;
				OCE_SET_CONTROL_FIELD(oceCtrl->OceCapIndication,
					HLP_ENABLED, OCE_HLP_ENABLED_MASK,
					OCE_HLP_ENABLED_OFFSET); /* HLP_ENABLED */
			} else
				oceCtrl->bOceFilsHlpEnable = FALSE;

			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_INFO,
				"I/F(ra%d) OCE_FILS_HLP=%d\n",
				Loop, oceCtrl->bOceEnable);
		}
	}


	if (RTMPGetKeyParameter("OCE_FILS_DhcpServer", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
			macptr = rstrtok(NULL, ";"), Loop++) {
			UINT32 ip_addr;

			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (rtinet_aton(macptr, &ip_addr)) {
				oceCtrl->FilsDhcpServerIp = ip_addr;
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_INFO,
					"OCE_FILS_DhcpServer=%s(%x)\n",
					macptr, oceCtrl->FilsDhcpServerIp);
			}
		}
	}

	if (RTMPGetKeyParameter("OCE_FILS_DhcpServerPort", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
			macptr = rstrtok(NULL, ";"), Loop++) {


			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;

			oceCtrl->FilsDhcpServerPort = os_str_tol(macptr, 0, 10);

			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_INFO,
				"I/F(ra%d) OCE_FILS_DhcpServerPort=%d\n",
				Loop, oceCtrl->FilsDhcpServerPort);
		}
	}

	if (RTMPGetKeyParameter("OCE_FILS_REALMS", tmpbuf, 256, pBuffer, TRUE)) {
		RTMP_STRING *macptr;
		UCHAR result[64] = {0}, i = 0;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
			macptr = rstrtok(NULL, ";"), Loop++) {

			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;

			for (i = 0 ; i < strlen(macptr); i++) {
				macptr[i] = tolower(macptr[i]);
			}
#ifndef RT_CFG80211_SUPPORT
			RT_SHA256(macptr, strlen(macptr), result);
#endif /* RT_CFG80211_SUPPORT */
			if (FILS_REALMS_HASH_LEN <= sizeof(oceCtrl->FilsRealmsHash)
				&& FILS_REALMS_HASH_LEN <= ARRAY_SIZE(result))
				NdisMoveMemory(
					&oceCtrl->FilsRealmsHash, result, FILS_REALMS_HASH_LEN);
			else {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_ERROR,
					"Length of FilsRealmsHash or result is invalid !\n");
				return;
			}

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_INFO,
				" OCE_FILS_Realms%d (%02x:%02x)\n", (Loop + 1),
				result[0], result[1]);

		}
	}

	if (RTMPGetKeyParameter("OCE_FILS_CACHE", tmpbuf, 256, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
			macptr = rstrtok(NULL, ";"), Loop++) {

			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (os_str_tol(macptr, 0, 10) != 0)
				oceCtrl->FilsCacheId = RandomByte(pAd) << 8 | RandomByte(pAd);
			else
				oceCtrl->FilsCacheId = 0;


			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_INFO,
				" OCE_FILS_CACHE%d (%04x)\n", (Loop + 1),
				oceCtrl->FilsCacheId);

		}
	}

	if (RTMPGetKeyParameter("OCE_RNR_SUPPORT", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
			macptr = rstrtok(NULL, ";"), Loop++) {
			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;

			oceCtrl->bApRnrCompleteEnable = (UINT8)os_str_tol(macptr, 0, 10) ? TRUE : FALSE;

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_INFO,
				"(bApRnrCompleteEnable[%d]=%d)\n", Loop,
				oceCtrl->bApRnrCompleteEnable);
		}
	}

	for (Loop = 0; Loop < MAX_MBSSID_NUM(pAd); Loop++) {
		RTMP_STRING tok_str[16];
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
		OCE_CTRL *oceCtrl = &wdev->OceCtrl;

		/*
			OCE_ASSOC_RssiThres:
		*/
		NdisZeroMemory(tok_str, sizeof(tok_str));
		(void)snprintf(tok_str, sizeof(tok_str), "OCE_ASSOC_RssiThres%d", Loop + 1);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 32, pBuffer, TRUE)) {
			INT8 rssi = os_str_tol(tmpbuf, 0, 10);

			oceCtrl->AssocRSSIThres = rssi;
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_INFO,
				"OCE_ASSOC_RssiThres%d (%d)\n",
				(Loop + 1), oceCtrl->AssocRSSIThres);
		}

		/*
			OCE_ASSOC_RetryDelay:
		*/
		NdisZeroMemory(tok_str, sizeof(tok_str));
		(void)snprintf(tok_str, sizeof(tok_str), "OCE_ASSOC_RetryDelay%d", Loop + 1);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 32, pBuffer, TRUE)) {
			UINT8 time = os_str_tol(tmpbuf, 0, 10);

			oceCtrl->AssocRetryDelay = time;
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_INFO,
				"OCE_ASSOC_RetryDelay%d (%d)\n",
				(Loop + 1), oceCtrl->AssocRSSIThres);
		}
	}
}

#endif /* OCE_SUPPORT */

#endif /* !CONFIG_PROFILE_OFF */

