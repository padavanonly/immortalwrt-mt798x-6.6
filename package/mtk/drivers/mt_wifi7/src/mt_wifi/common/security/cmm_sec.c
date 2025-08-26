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
	cmm_sec.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/
#include "rt_config.h"

#ifdef RT_CFG80211_SUPPORT
struct authmode_2_str {
	enum mtk_vendor_attr_authmode auth_mode;
	char authmode_str[128];
};

struct authmode_2_str auth_map[] = {
	{NL80211_AUTH_OPEN, "OPEN"},
	{NL80211_AUTH_SHARED, "SHARED"},
	{NL80211_AUTH_WEPAUTO, "WEPAUTO"},
	{NL80211_AUTH_WPA, "WPA"},
	{NL80211_AUTH_WPAPSK, "WPAPSK"},
	{NL80211_AUTH_WPANONE, "WPANONE"},
	{NL80211_AUTH_WPA2, "WPA2"},
	{NL80211_AUTH_WPA2MIX, "WPA2MIX"},
	{NL80211_AUTH_WPA2PSK, "WPA2PSK"},
	{NL80211_AUTH_WPA3, "WPA3"},
	{NL80211_AUTH_WPA3_192, "WPA3-192"},
	{NL80211_AUTH_WPA3PSK, "WPA3PSK"},
	{NL80211_AUTH_WPA2PSKWPA3PSK, "WPA2PSKWPA3PSK"},
	{NL80211_AUTH_WPA2PSKMIXWPA3PSK, "WPA2PSKMIXWPA3PSK"},
	{NL80211_AUTH_WPA1WPA2, "WPA1WPA2"},
	{NL80211_AUTH_WPAPSKWPA2PSK, "WPAPSKWPA2PSK"},
	{NL80211_AUTH_WPA_AES_WPA2_TKIPAES, "WPA_AES_WPA2_TKIPAES"},
	{NL80211_AUTH_WPA_AES_WPA2_TKIP, "WPA_AES_WPA2_TKIP"},
	{NL80211_AUTH_WPA_TKIP_WPA2_AES, "WPA_TKIP_WPA2_AES"},
	{NL80211_AUTH_WPA_TKIP_WPA2_TKIPAES, "WPA_TKIP_WPA2_TKIPAES"},
	{NL80211_AUTH_WPA_TKIPAES_WPA2_AES, "WPA_TKIPAES_WPA2_AES"},
	{NL80211_AUTH_WPA_TKIPAES_WPA2_TKIPAES, "WPA_TKIPAES_WPA2_TKIPAES"},
	{NL80211_AUTH_WPA_TKIPAES_WPA2_TKIP, "WPA_TKIPAES_WPA2_TKIP"},
	{NL80211_AUTH_OWE, "OWE"},
	{NL80211_AUTH_FILS_SHA256, "FILS_SHA256"},
	{NL80211_AUTH_FILS_SHA384, "FILS_SHA384"},
	{NL80211_AUTH_WAICERT, "WAICERT"},
	{NL80211_AUTH_WAIPSK, "WAIPSK"},
	{NL80211_AUTH_DPP, "DPP"},
	{NL80211_AUTH_DPPWPA2PSK, "DPPWPA2PSK"},
	{NL80211_AUTH_DPPWPA3PSK, "DPPWPA3PSK"},
	{NL80211_AUTH_DPPWPA3PSKWPA2PSK, "DPPWPA3PSKWPA2PSK"},
	{NL80211_AUTH_WPA2_ENT_OSEN, "WPA2-Ent-OSEN"}
};

struct encryptype_2_str {
	enum mtk_vendor_attr_encryptype type;
	char encryptype_str[128];
};

struct encryptype_2_str encryptype_map[] = {
	{NL80211_ENCRYPTYPE_NONE, "NONE"},
	{NL80211_ENCRYPTYPE_WEP, "WEP"},
	{NL80211_ENCRYPTYPE_TKIP, "TKIP"},
	{NL80211_ENCRYPTYPE_AES, "AES"},
	{NL80211_ENCRYPTYPE_CCMP128, "CCMP128"},
	{NL80211_ENCRYPTYPE_CCMP256, "CCMP256"},
	{NL80211_ENCRYPTYPE_GCMP128, "GCMP128"},
	{NL80211_ENCRYPTYPE_GCMP256, "GCMP256"},
	{NL80211_ENCRYPTYPE_TKIPAES, "TKIPAES"},
	{NL80211_ENCRYPTYPE_TKIPCCMP128, "TKIPCCMP128"},
	{NL80211_ENCRYPTYPE_WPA_AES_WPA2_TKIPAES, "WPA_AES_WPA2_TKIPAES"},
	{NL80211_ENCRYPTYPE_WPA_AES_WPA2_TKIP, "WPA_AES_WPA2_TKIP"},
	{NL80211_ENCRYPTYPE_WPA_TKIP_WPA2_AES, "WPA_TKIP_WPA2_AES"},
	{NL80211_ENCRYPTYPE_WPA_TKIP_WPA2_TKIPAES, "WPA_TKIP_WPA2_TKIPAES"},
	{NL80211_ENCRYPTYPE_WPA_TKIPAES_WPA2_AES, "WPA_TKIPAES_WPA2_AES"},
	{NL80211_ENCRYPTYPE_WPA_TKIPAES_WPA2_TKIPAES, "WPA_TKIPAES_WPA2_TKIPAES"},
	{NL80211_ENCRYPTYPE_WPA_TKIPAES_WPA2_TKIP, "WPA_TKIPAES_WPA2_TKIP"},
	{NL80211_ENCRYPTYPE_SMS4, "SMS4"}
};

INT Set_SecDefaultKeyID(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR KeyIdx)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
			"pSecConfig == NULL\n");
		return -EFAULT;
	}

	if ((KeyIdx >= 1) && (KeyIdx <= 4))
		pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
	else
		return -EINVAL;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
		"==> DefaultKeyId=%d\n",
		pSecConfig->PairwiseKeyId);
	return 0;
}

INT Set_SecWEPKey(
	IN PRTMP_ADAPTER pAd,
	IN struct wep_key_param *wep_key)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;
	SEC_KEY_INFO *pWebKey;

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WEP, DBG_LVL_ERROR, "pSecConfig == NULL\n");
		return -EFAULT;
	}

	if (wep_key->key_idx >= SEC_KEY_NUM)
		return -EINVAL;

	pWebKey = &pSecConfig->WepKey[wep_key->key_idx];

	if (wep_key->key_len == 5 || wep_key->key_len == 13 || wep_key->key_len == 16) {
		NdisZeroMemory(pWebKey, sizeof(SEC_KEY_INFO));
		pWebKey->KeyLen = wep_key->key_len;
		NdisMoveMemory(pWebKey->Key, wep_key->key, wep_key->key_len);
	} else {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WEP, DBG_LVL_ERROR,
			"invalid wep key(len=%d)\n",
			wep_key->key_len);
	}

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WEP, DBG_LVL_ERROR, "KeyID=%d\n",
			 wep_key->key_idx);
	return 0;
}

INT	Set_SecWPAPSK(
	IN PRTMP_ADAPTER pAd,
	IN char *passphrase)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
	INT i;
	struct _NDIS_AP_802_11_PMKID *pmkid_cache;

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"pSecConfig == NULL, arg=%s\n", passphrase);
		return -EFAULT;
	}

	pmkid_cache = PD_GET_PMKID_PTR(pAd->physical_dev);
	if (strlen(passphrase) < 65) {
		if (strlen(passphrase) != strlen(pSecConfig->PSK)
			|| !RTMPEqualMemory(passphrase, pSecConfig->PSK, strlen(passphrase))
			) {

			OS_SEM_LOCK(&pmkid_cache->pmkid_lock);
			for (i = 0; i < MAX_PMKID_COUNT; i++) {
				if ((pmkid_cache->BSSIDInfo[i].Valid == TRUE)
					&& (MAC_ADDR_EQUAL(&pmkid_cache->BSSIDInfo[i].own_mac, pMbss->wdev.bssid))) {
					pmkid_cache->BSSIDInfo[i].Valid = FALSE;
					MTWF_PRINT("%s():Modify PSK and clear PMKID (idx %d)from (mbssidx %d)\n", __func__, i, pMbss->mbss_idx);
				}
			}
			OS_SEM_UNLOCK(&pmkid_cache->pmkid_lock);
		}
#ifdef CONFIG_STA_SUPPORT
#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI) {
		UCHAR sta_idx = pObj->ioctl_if;
		BOOLEAN is_psk_same = 0;
		UCHAR i = 0;

		for (i = 0; i < (LEN_PSK + 1); i++) {
			if (pSecConfig->PSK[i] != passphrase[i]) {
				is_psk_same = 0;
				break;
			}

			if (pSecConfig->PSK[i] == '\0') {
				is_psk_same = 1;
				break;
			}
		}

		if (!is_psk_same) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
				"Delete pmk cache on password change\n");

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)

			sta_delete_psk_pmkid_cache_all(pAd, sta_idx);
#endif
		}
	}
#endif /* APCLI_SUPPORT */
#endif
		os_move_mem(pSecConfig->PSK, passphrase, strlen(passphrase));
		pSecConfig->PSK[strlen(passphrase)] = '\0';
	} else
		pSecConfig->PSK[0] = '\0';

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "PSK = %s\n",
			 passphrase);
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		WSC_CTRL *pWscControl = NULL;

		if ((pObj->ioctl_if_type == INT_MAIN || pObj->ioctl_if_type == INT_MBSSID)) {
			UCHAR apidx = pObj->ioctl_if;

			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
		}

#ifdef APCLI_SUPPORT
		else if (pObj->ioctl_if_type == INT_APCLI) {
			UCHAR    apcli_idx = pObj->ioctl_if;

			pWscControl = &pAd->StaCfg[apcli_idx].wdev.WscControl;
		}

#endif /* APCLI_SUPPORT */

		if (pWscControl) {
			NdisZeroMemory(pWscControl->WpaPsk, 64);
			pWscControl->WpaPskLen = 0;
			pWscControl->WpaPskLen = strlen(passphrase);
			NdisMoveMemory(pWscControl->WpaPsk, passphrase, pWscControl->WpaPskLen);
		}
	}
#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	return 0;
}
#endif

VOID SetWdevAuthMode(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN RTMP_STRING * arg)
{
	UINT32 AKMMap = 0;

	CLEAR_SEC_AKM(AKMMap);
#ifdef CONFIG_HOTSPOT_R3
	pSecConfig->bIsWPA2EntOSEN = FALSE;
#endif

	if (rtstrcasecmp(arg, "OPEN") == TRUE)
		SET_AKM_OPEN(AKMMap);
	else if (rtstrcasecmp(arg, "SHARED") == TRUE)
		SET_AKM_SHARED(AKMMap);
	else if (rtstrcasecmp(arg, "WEPAUTO") == TRUE) {
		SET_AKM_OPEN(AKMMap);
		SET_AKM_AUTOSWITCH(AKMMap);
	} else if (rtstrcasecmp(arg, "WPA") == TRUE)
		SET_AKM_WPA1(AKMMap);
	else if (rtstrcasecmp(arg, "WPAPSK") == TRUE)
		SET_AKM_WPA1PSK(AKMMap);
	else if (rtstrcasecmp(arg, "WPANONE") == TRUE)
		SET_AKM_WPANONE(AKMMap);
	else if (rtstrcasecmp(arg, "WPA2") == TRUE)
		SET_AKM_WPA2(AKMMap);
	else if (rtstrcasecmp(arg, "WPA2MIX") == TRUE) {
		SET_AKM_WPA2(AKMMap);
		SET_AKM_WPA2_SHA256(AKMMap);
	} else if (rtstrcasecmp(arg, "WPA2PSK") == TRUE)
		SET_AKM_WPA2PSK(AKMMap);
	else if (rtstrcasecmp(arg, "WPA3") == TRUE) {
		/* WPA3 code flow is same as WPA2, the usage of SEC_AKM_WPA3 is to force pmf on */
		SET_AKM_WPA2(AKMMap);
		SET_AKM_WPA3(AKMMap);
	}
#ifdef DOT11_SUITEB_SUPPORT
	else if (rtstrcasecmp(arg, "WPA3-192") == TRUE)
		SET_AKM_WPA3_192BIT(AKMMap);
#endif
#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
	else if (rtstrcasecmp(arg, "WPA3PSK") == TRUE)
		SET_AKM_SAE_SHA256(AKMMap);
	else if (rtstrcasecmp(arg, "WPA3PSK_EXT") == TRUE) {
		SET_AKM_SAE_EXT(AKMMap);
	} else if (rtstrcasecmp(arg, "WPA2PSKWPA3PSK") == TRUE) {
		SET_AKM_SAE_SHA256(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
	} else if (rtstrcasecmp(arg, "WPA2PSKMIXWPA3PSK") == TRUE) {
		SET_AKM_SAE_SHA256(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
		SET_AKM_WPA2PSK_SHA256(AKMMap);
	}
#endif /* DOT11_SAE_SUPPORT */
	else if (rtstrcasecmp(arg, "WPA1WPA2") == TRUE) {
		SET_AKM_WPA1(AKMMap);
		SET_AKM_WPA2(AKMMap);
	} else if (rtstrcasecmp(arg, "WPAPSKWPA2PSK") == TRUE) {
		SET_AKM_WPA1PSK(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
	} else if ((rtstrcasecmp(arg, "WPA_AES_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_AES_WPA2_TKIP") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIP_WPA2_AES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIP_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_AES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_TKIP") == TRUE)) {
		SET_AKM_WPA1PSK(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
	} else if (rtstrcasecmp(arg, "OWE") == TRUE) {
		SET_AKM_OWE(AKMMap);
	}
#ifdef OCE_FILS_SUPPORT
	else if (rtstrcasecmp(arg, "FILS_SHA256") == TRUE) {
		SET_AKM_WPA2(AKMMap);
		SET_AKM_FILS_SHA256(AKMMap);
	} else if (rtstrcasecmp(arg, "FILS_SHA384") == TRUE) {
		SET_AKM_WPA2(AKMMap);
		SET_AKM_FILS_SHA384(AKMMap);
	}
#endif /* OCE_FILS_SUPPORT */
	else if (rtstrcasecmp(arg, "DPP") == TRUE) {
		SET_AKM_DPP(AKMMap);
	} else if (rtstrcasecmp(arg, "DPPWPA2PSK") == TRUE) {
		SET_AKM_DPP(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
	} else if (rtstrcasecmp(arg, "DPPWPA3PSK") == TRUE) {
		SET_AKM_DPP(AKMMap);
		SET_AKM_SAE_SHA256(AKMMap);
	} else if (rtstrcasecmp(arg, "DPPWPA3PSKWPA2PSK") == TRUE) {
		SET_AKM_DPP(AKMMap);
		SET_AKM_SAE_SHA256(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
#ifdef MAP_R6
	} else if (rtstrcasecmp(arg, "SAE-EXTDPPWPA3PSKWPA2PSK") == TRUE) {
		SET_AKM_DPP(AKMMap);
		SET_AKM_SAE_SHA256(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
		SET_AKM_SAE_EXT(AKMMap);
	} else if (rtstrcasecmp(arg, "SAE-EXTDPPWPA2PSK") == TRUE) {
		SET_AKM_DPP(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
		SET_AKM_SAE_EXT(AKMMap);
	} else if (rtstrcasecmp(arg, "SAE-EXTDPPWPA3PSK") == TRUE) {
		SET_AKM_DPP(AKMMap);
		SET_AKM_SAE_SHA256(AKMMap);
		SET_AKM_SAE_EXT(AKMMap);
	} else if (rtstrcasecmp(arg, "SAE-EXTWPA2PSK") == TRUE) {
		SET_AKM_DPP(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
	} else if (rtstrcasecmp(arg, "SAE-EXTWPA3PSK") == TRUE) {
		SET_AKM_SAE_SHA256(AKMMap);
		SET_AKM_SAE_EXT(AKMMap);
	} else if (rtstrcasecmp(arg, "SAE-EXTDPP") == TRUE) {
		SET_AKM_DPP(AKMMap);
		SET_AKM_SAE_EXT(AKMMap);
	} else if (rtstrcasecmp(arg, "SAE-EXT") == TRUE) {
		SET_AKM_SAE_EXT(AKMMap);
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_DEBUG, "Only AKM24 Supported\n");
#endif
	}

#ifdef CONFIG_HOTSPOT_R3
	else if (rtstrcasecmp(arg, "WPA2-Ent-OSEN") == TRUE) {
		pSecConfig->bIsWPA2EntOSEN = TRUE;
		SET_AKM_WPA2(AKMMap);
		SET_AKM_OSEN(AKMMap);
	}
#endif
	else {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
			"Not support (AuthMode=%s, len=%d)\n",
				 arg, (int) strlen(arg));
	}

	if (AKMMap != 0x0)
		pSecConfig->AKMMap = AKMMap;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
		"AuthMode=0x%x\n",
		pSecConfig->AKMMap);
}


VOID SetWdevEncrypMode(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN RTMP_STRING * arg)
{
	UINT Cipher = 0;

	if (rtstrcasecmp(arg, "NONE") == TRUE)
		SET_CIPHER_NONE(Cipher);
	else if (rtstrcasecmp(arg, "WEP") == TRUE)
		SET_CIPHER_WEP(Cipher);
	else if (rtstrcasecmp(arg, "TKIP") == TRUE)
		SET_CIPHER_TKIP(Cipher);
	else if ((rtstrcasecmp(arg, "AES") == TRUE) || (rtstrcasecmp(arg, "CCMP128") == TRUE))
		SET_CIPHER_CCMP128(Cipher);
	else if (rtstrcasecmp(arg, "CCMP256") == TRUE)
		SET_CIPHER_CCMP256(Cipher);
	else if (rtstrcasecmp(arg, "GCMP128") == TRUE)
		SET_CIPHER_GCMP128(Cipher);
	else if (rtstrcasecmp(arg, "GCMP256") == TRUE)
		SET_CIPHER_GCMP256(Cipher);
	else if ((rtstrcasecmp(arg, "TKIPAES") == TRUE) || (rtstrcasecmp(arg, "TKIPCCMP128") == TRUE)) {
		SET_CIPHER_TKIP(Cipher);
		SET_CIPHER_CCMP128(Cipher);
	} else if ((rtstrcasecmp(arg, "WPA_AES_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_AES_WPA2_TKIP") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIP_WPA2_AES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIP_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_AES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_TKIP") == TRUE)) {
		SET_CIPHER_TKIP(Cipher);
		SET_CIPHER_CCMP128(Cipher);
	}

	else {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
			"Not support (EncrypType=%s, len=%d)\n",
			arg, (int) strlen(arg));
	}

	if (Cipher != 0x0) {
		pSecConfig->PairwiseCipher = Cipher;
		CLEAR_GROUP_CIPHER(pSecConfig);
	}

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
		"PairwiseCipher=0x%x\n",
		GET_PAIRWISE_CIPHER(pSecConfig));
}

#ifdef RT_CFG80211_SUPPORT
int SetWdevAuthModeByNlAttr(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN enum mtk_vendor_attr_authmode auth_mode)
{
	if (auth_mode >= NL80211_AUTH_WPA2_ENT_OSEN)
		return -EINVAL;

	SetWdevAuthMode(pSecConfig, auth_map[auth_mode].authmode_str);

	return 0;
}

int SetWdevEncrypModeByNlAttr(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN enum mtk_vendor_attr_encryptype type)
{
	if (type >= NL80211_ENCRYPTYPE_SMS4)
		return -EINVAL;

	SetWdevEncrypMode(pSecConfig, encryptype_map[type].encryptype_str);

	return 0;
}
#endif

INT Set_SecAuthMode_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
			"pSecConfig == NULL, arg=%s\n", arg);
		return FALSE;
	}

	SetWdevAuthMode(pSecConfig, arg);
	return TRUE;
}

INT Set_SecEncrypType_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
			"pSecConfig == NULL, arg=%s\n", arg);
		return FALSE;
	}

	SetWdevEncrypMode(pSecConfig, arg);
	return TRUE;
}

INT Set_SecDefaultKeyID_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;
	ULONG KeyIdx;

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
			"pSecConfig == NULL, arg=%s\n", arg);
		return FALSE;
	}

	KeyIdx = os_str_tol(arg, 0, 10);

	if ((KeyIdx >= 1) && (KeyIdx <= 4))
		pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
	else
		return FALSE;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
		"==> DefaultKeyId=%d\n",
		pSecConfig->PairwiseKeyId);
	return TRUE;
}


INT	Set_SecWPAPSK_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
	INT i;
	struct _NDIS_AP_802_11_PMKID *pmkid_cache;

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"pSecConfig == NULL, arg=%s\n",
			arg);
		return FALSE;
	}

	if (strlen(arg) < 65) {
		if (strlen(arg) != strlen(pSecConfig->PSK)
			|| !RTMPEqualMemory(arg, pSecConfig->PSK, strlen(arg))
			) {

			pmkid_cache = PD_GET_PMKID_PTR(pAd->physical_dev);
			OS_SEM_LOCK(&pmkid_cache->pmkid_lock);
			for (i = 0; i < MAX_PMKID_COUNT; i++) {
				if ((pmkid_cache->BSSIDInfo[i].Valid == TRUE)
					&& (MAC_ADDR_EQUAL(&pmkid_cache->BSSIDInfo[i].own_mac, pMbss->wdev.bssid))) {
					pmkid_cache->BSSIDInfo[i].Valid = FALSE;
					MTWF_PRINT("%s():Modify PSK and clear PMKID (idx %d)from (mbssidx %d)\n", __func__, i, pMbss->mbss_idx);
				}
			}
			OS_SEM_UNLOCK(&pmkid_cache->pmkid_lock);
		}
#ifdef CONFIG_STA_SUPPORT
#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI) {
		UCHAR sta_idx = pObj->ioctl_if;
		BOOLEAN is_psk_same = 0;
		UCHAR i = 0;

		 for (i = 0; i < (LEN_PSK + 1); i++) {
				if (pSecConfig->PSK[i] != arg[i]) {
					is_psk_same = 0;
					break;
				}

				if (pSecConfig->PSK[i] == '\0') {
					is_psk_same = 1;
					break;
				}
		}

		if (!is_psk_same) {
			/*PSK has changed we need to clear store apcli pmk cache for AKM's that use PSK*/
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
						"Delete pmk cache on password change\n");

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)

			sta_delete_psk_pmkid_cache_all(pAd, sta_idx);
#endif
		}
	}
#endif /* APCLI_SUPPORT */
#endif
		os_move_mem(pSecConfig->PSK, arg, strlen(arg));
		pSecConfig->PSK[strlen(arg)] = '\0';
	} else
		pSecConfig->PSK[0] = '\0';

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "PSK = %s\n", arg);
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		WSC_CTRL *pWscControl = NULL;

		if ((pObj->ioctl_if_type == INT_MAIN || pObj->ioctl_if_type == INT_MBSSID)) {
			UCHAR apidx = pObj->ioctl_if;

			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
		}

#ifdef APCLI_SUPPORT
		else if (pObj->ioctl_if_type == INT_APCLI) {
			UCHAR    apcli_idx = pObj->ioctl_if;

			pWscControl = &pAd->StaCfg[apcli_idx].wdev.WscControl;
		}

#endif /* APCLI_SUPPORT */

		if (pWscControl) {
			NdisZeroMemory(pWscControl->WpaPsk, 64);
			pWscControl->WpaPskLen = 0;
			pWscControl->WpaPskLen = strlen(arg);
			NdisMoveMemory(pWscControl->WpaPsk, arg, pWscControl->WpaPskLen);
		}
	}
#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}

INT Set_SecWEPKey_Proc(
	IN PRTMP_ADAPTER pAd,
	IN CHAR KeyId,
	IN RTMP_STRING * arg)
{
	INT retVal = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WEP, DBG_LVL_ERROR,
			"pSecConfig == NULL, arg=%s\n",
			arg);
		return FALSE;
	}

	retVal = ParseWebKey(pSecConfig, arg, KeyId, 0);
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WEP, DBG_LVL_INFO, "KeyID=%d, key=%s\n",
			 KeyId, arg);
	return retVal;
}


INT Set_SecKey1_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg)
{
	return Set_SecWEPKey_Proc(pAd, 0, arg);
}

INT Set_SecKey2_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg)
{
	return Set_SecWEPKey_Proc(pAd, 1, arg);
}

INT Set_SecKey3_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg)
{
	return Set_SecWEPKey_Proc(pAd, 2, arg);
}

INT Set_SecKey4_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg)
{
	return Set_SecWEPKey_Proc(pAd, 3, arg);
}


RTMP_STRING *GetAuthModeStr(
	IN UINT32 authMode)
{
	if (IS_AKM_OPEN(authMode))
		return "OPEN";
	else if (IS_AKM_SHARED(authMode))
		return "SHARED";
	else if (IS_AKM_AUTOSWITCH(authMode))
		return "WEPAUTO";
	else if (IS_AKM_WPANONE(authMode))
		return "WPANONE";
	else if (IS_AKM_FT_WPA2PSK(authMode) && IS_AKM_FT_SAE_SHA256(authMode))
		return "FT-WPA2PSKWPA3PSK";
	else if (IS_AKM_WPA1(authMode) && IS_AKM_WPA2(authMode))
		return "WPA1WPA2";
	else if (IS_AKM_WPA1PSK(authMode) && IS_AKM_WPA2PSK(authMode))
		return "WPAPSKWPA2PSK";
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_SAE_SHA256(authMode) && IS_AKM_DPP(authMode))
		return "DPPWPA3PSKWPA2PSK";
	else if (IS_AKM_SAE_SHA256(authMode) && IS_AKM_DPP(authMode))
		return "DPPWPA3PSK";
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_DPP(authMode))
		return "DPPWPA2PSK";
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_SAE_SHA256(authMode) && IS_AKM_SAE_EXT(authMode))
		return "WPA2PSKWPA3PSKWPA3PSK-EXT";
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_SAE_SHA256(authMode))
		return "WPA2PSKWPA3PSK";
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_WPA2PSK_SHA256(authMode) && IS_AKM_SAE_SHA256(authMode))
		return "WPA2PSKMIXWPA3PSK";
	else if (IS_AKM_FT_SAE_SHA256(authMode))
		return "FT-SAE";
	else if (IS_AKM_FT_SAE_EXT(authMode))
		return "FT-SAE-EXT";
	else if (IS_AKM_SAE_SHA256(authMode))
		return "WPA3PSK";
	else if (IS_AKM_SAE_EXT(authMode))
		return "WPA3PSK-EXT";
	else if (IS_AKM_WPA1(authMode))
		return "WPA";
	else if (IS_AKM_WPA1PSK(authMode))
		return "WPAPSK";
	else if (IS_AKM_FT_WPA2(authMode))
		return "FT-WPA2";
	else if (IS_AKM_FT_WPA2PSK(authMode))
		return "FT-WPA2PSK";
	else if (IS_AKM_WPA3(authMode)) /* WPA3 will be always accompanied by WPA2, so it should put before the WPA2 */
		return "WPA3";
	else if (IS_AKM_WPA2(authMode))
		return "WPA2";
	else if (IS_AKM_WPA2(authMode) && IS_AKM_WPA2_SHA256(authMode))
		return "WPA2MIX";
	else if (IS_AKM_WPA2PSK(authMode))
		return "WPA2PSK";
	else if (IS_AKM_WPA3_192BIT(authMode))
		return "WPA3-192";
	else if (IS_AKM_OWE(authMode))
		return "OWE";
	else if (IS_AKM_DPP(authMode))
		return "DPP";
	else
		return "UNKNOW";
}

RTMP_STRING *GetEncryModeStr(
	IN UINT32 encryMode)
{
	if (IS_CIPHER_NONE(encryMode))
		return "NONE";
	else if (IS_CIPHER_WEP(encryMode))
		return "WEP";
	else if (IS_CIPHER_TKIP(encryMode) && IS_CIPHER_CCMP128(encryMode))
		return "TKIPAES";
	else if (IS_CIPHER_TKIP(encryMode))
		return "TKIP";
	else if (IS_CIPHER_CCMP128(encryMode) && IS_CIPHER_GCMP256(encryMode))
		return "AES_GCMP256";
	else if (IS_CIPHER_CCMP128(encryMode))
		return "AES";
	else if (IS_CIPHER_CCMP256(encryMode))
		return "CCMP256";
	else if (IS_CIPHER_GCMP128(encryMode))
		return "GCMP128";
	else if (IS_CIPHER_GCMP256(encryMode))
		return "GCMP256";
	else if (IS_CIPHER_BIP_CMAC128(encryMode))
		return "BIP-CMAC128";
	else if (IS_CIPHER_BIP_CMAC256(encryMode))
		return "BIP-CMAC256";
	else if (IS_CIPHER_BIP_GMAC128(encryMode))
		return "BIP-GMAC128";
	else if (IS_CIPHER_BIP_GMAC256(encryMode))
		return "BIP-GMAC256";
	else
		return "UNKNOW";
}

UINT32 SecAuthModeOldToNew(
	IN USHORT authMode)
{
	UINT32 AKMMap = 0;

	switch (authMode) {
	case Ndis802_11AuthModeOpen:
		SET_AKM_OPEN(AKMMap);
		break;

	case Ndis802_11AuthModeShared:
		SET_AKM_SHARED(AKMMap);
		break;

	case Ndis802_11AuthModeAutoSwitch:
		SET_AKM_AUTOSWITCH(AKMMap);
		break;

	case Ndis802_11AuthModeWPA:
		SET_AKM_WPA1(AKMMap);
		break;

	case Ndis802_11AuthModeWPAPSK:
		SET_AKM_WPA1PSK(AKMMap);
		break;

	case Ndis802_11AuthModeWPANone:
		SET_AKM_WPANONE(AKMMap);
		break;

	case Ndis802_11AuthModeWPA2:
		SET_AKM_WPA2(AKMMap);
		break;

	case Ndis802_11AuthModeWPA2PSK:
		SET_AKM_WPA2PSK(AKMMap);
		break;

	case Ndis802_11AuthModeWPA1WPA2:
		SET_AKM_WPA1(AKMMap);
		SET_AKM_WPA2(AKMMap);
		break;

	case Ndis802_11AuthModeWPA1PSKWPA2PSK:
		SET_AKM_WPA1PSK(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
		break;
	}

	return AKMMap;
}


UINT32 SecEncryModeOldToNew(
	IN USHORT encryMode)
{
	UINT32 EncryType = 0;

	switch (encryMode) {
	case Ndis802_11WEPDisabled:
		SET_CIPHER_NONE(EncryType);
		break;

	case Ndis802_11WEPEnabled:
		SET_CIPHER_WEP(EncryType);
		break;

	case Ndis802_11TKIPEnable:
		SET_CIPHER_TKIP(EncryType);
		break;

	case Ndis802_11AESEnable:
		SET_CIPHER_CCMP128(EncryType);
		break;

	case Ndis802_11TKIPAESMix:
		SET_CIPHER_TKIP(EncryType);
		SET_CIPHER_CCMP128(EncryType);
		break;
	}

	return EncryType;
}


USHORT SecAuthModeNewToOld(
	IN UINT32 authMode)
{
	if (IS_AKM_OPEN(authMode))
		return Ndis802_11AuthModeOpen;
	else if (IS_AKM_SHARED(authMode))
		return Ndis802_11AuthModeShared;
	else if (IS_AKM_AUTOSWITCH(authMode))
		return Ndis802_11AuthModeAutoSwitch;
	else if (IS_AKM_WPANONE(authMode))
		return Ndis802_11AuthModeWPANone;
	else if (IS_AKM_WPA1(authMode) && IS_AKM_WPA2(authMode))
		return Ndis802_11AuthModeWPA1WPA2;
	else if (IS_AKM_WPA1PSK(authMode) && IS_AKM_WPA2PSK(authMode))
		return Ndis802_11AuthModeWPA1PSKWPA2PSK;
	else if (IS_AKM_WPA1(authMode))
		return Ndis802_11AuthModeWPA;
	else if (IS_AKM_WPA1PSK(authMode))
		return Ndis802_11AuthModeWPAPSK;
	else if (IS_AKM_WPA2(authMode))
		return Ndis802_11AuthModeWPA2;
	else if (IS_AKM_WPA2PSK(authMode))
		return Ndis802_11AuthModeWPA2PSK;

	else
		return Ndis802_11AuthModeOpen;
}


USHORT SecEncryModeNewToOld(
	IN UINT32 encryMode)
{
	if (IS_CIPHER_NONE(encryMode))
		return Ndis802_11WEPDisabled;
	else if (IS_CIPHER_WEP(encryMode))
		return Ndis802_11WEPEnabled;
	else if (IS_CIPHER_TKIP(encryMode))
		return Ndis802_11TKIPEnable;
	else if (IS_CIPHER_CCMP128(encryMode))
		return Ndis802_11AESEnable;
	else if (IS_CIPHER_TKIP(encryMode) && IS_CIPHER_CCMP128(encryMode))
		return Ndis802_11TKIPAESMix;

	else
		return Ndis802_11WEPDisabled;
}


UINT8 SecHWCipherSuitMapping(
	IN UINT32 encryMode)
{
	if (IS_CIPHER_NONE(encryMode))
		return CIPHER_SUIT_NONE;
	else if (IS_CIPHER_WEP(encryMode))
		return CIPHER_SUIT_WEP_40;
	else if (IS_CIPHER_TKIP(encryMode))
		return CIPHER_SUIT_TKIP_W_MIC;
	else if (IS_CIPHER_CCMP128(encryMode))
		return CIPHER_SUIT_CCMP_W_MIC;
	else if (IS_CIPHER_CCMP256(encryMode))
		return CIPHER_SUIT_CCMP_256;
	else if (IS_CIPHER_GCMP128(encryMode))
		return CIPHER_SUIT_GCMP_128;
	else if (IS_CIPHER_GCMP256(encryMode))
		return CIPHER_SUIT_GCMP_256;

	else
		return CIPHER_SUIT_NONE;
}


INT ParseWebKey(
	IN  struct _SECURITY_CONFIG *pSecConfig,
	IN  RTMP_STRING *buffer,
	IN  INT KeyIdx,
	IN  INT Keylength)
{
	UINT32 KeyLen = Keylength;
	SEC_KEY_INFO *pWebKey = &pSecConfig->WepKey[KeyIdx];
	UINT32 i = 0;

	if (KeyLen == 0)
		KeyLen = strlen(buffer);

	switch (KeyLen) {
	case 5: /*wep 40 Ascii type*/
	case 13: /*wep 104 Ascii type*/
	case 16: /*wep 128 Ascii type*/
		NdisZeroMemory(pWebKey, sizeof(SEC_KEY_INFO));
		pWebKey->KeyLen = KeyLen;
		NdisMoveMemory(pWebKey->Key, buffer, KeyLen);
		break;

	case 10: /*wep 40 Hex type*/
	case 26: /*wep 104 Hex type*/
	case 32: /*wep 128 Hex type*/
		for (i = 0; i < KeyLen; i++) {
			if (!isxdigit(*(buffer + i)))
				return FALSE;  /*Not Hex value;*/
		}

		NdisZeroMemory(pWebKey, sizeof(SEC_KEY_INFO));
		pWebKey->KeyLen = KeyLen / 2;
		AtoH(buffer, pWebKey->Key, pWebKey->KeyLen);
		pWebKey->Key[pWebKey->KeyLen] = '\0';
		break;

	default: /*Invalid argument */
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
			"(keyIdx=%d):Invalid argument (arg=%s)\n",
			KeyIdx, buffer);
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "(KeyIdx=%d, Alg=0x%x)\n",
			 KeyIdx, pSecConfig->PairwiseCipher);
	return TRUE;
}


#if defined(DOT1X_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
INT SetWdevOwnIPAddr(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN RTMP_STRING *arg)
{
	UINT32 ip_addr;

	if (rtinet_aton(arg, &ip_addr)) {
		pSecConfig->own_ip_addr = ip_addr;
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO, "own_ip_addr=%s(%x)\n", arg, pSecConfig->own_ip_addr);
	}

	return TRUE;
}

#ifdef CONFIG_AP_SUPPORT
VOID Dot1xIoctlQueryRadiusConf(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	UCHAR apidx, srv_idx, keyidx, KeyLen = 0;
	UCHAR *mpool;
	PDOT1X_CMM_CONF pConf;
	struct _SECURITY_CONFIG *pSecConfigMain = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR main_apidx = (UCHAR) pObj->ioctl_if;
	UCHAR last_apidx = pAd->ApCfg.BssidNum - 1;


	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO, "==>\n");

	if ((main_apidx > pAd->ApCfg.BssidNum - 1)
		|| (main_apidx > last_apidx)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_ERROR,
			"Invalid MBSSID index(%d)!\n",
			main_apidx);
		return;
	}

	pSecConfigMain = &pAd->ApCfg.MBSSID[main_apidx].wdev.SecConfig;
	/* Allocate memory */
	os_alloc_mem(NULL, (PUCHAR *)&mpool, sizeof(DOT1X_CMM_CONF));

	if (mpool == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_ERROR,
			"!!!out of resource!!!\n");
		return;
	}

	NdisZeroMemory(mpool, sizeof(DOT1X_CMM_CONF));
	pConf = (PDOT1X_CMM_CONF)mpool;
	/* get MBSS number */
	pConf->mbss_num = (last_apidx - main_apidx + 1);
	/* get own ip address */
	pConf->own_ip_addr = pSecConfigMain->own_ip_addr;
	/* get own radius port */
	pConf->own_radius_port = pSecConfigMain->own_radius_port;
	/* get retry interval */
	pConf->retry_interval = pSecConfigMain->retry_interval;
	/* get session timeout interval */
	pConf->session_timeout_interval = pSecConfigMain->session_timeout_interval;
	/* Get the quiet interval */
	pConf->quiet_interval = pSecConfigMain->quiet_interval;

	for (apidx = main_apidx; apidx <= last_apidx; apidx++) {
		struct _SECURITY_CONFIG *pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
		UCHAR apidx_locate = apidx - main_apidx;
		PDOT1X_BSS_INFO p1xBssInfo = &pConf->Dot1xBssInfo[apidx_locate];
#ifdef RADIUS_ACCOUNTING_SUPPORT
		PACCT_BSS_INFO pAcctBssInfo = &pConf->AcctBssInfo[apidx_locate];

		pAcctBssInfo->radius_srv_num = pSecConfig->radius_acct_srv_num;
#endif /* RADIUS_ACCOUNTING_SUPPORT */
		p1xBssInfo->radius_srv_num = pSecConfig->radius_srv_num;

		/* prepare radius ip, port and key */
		for (srv_idx = 0; srv_idx < pSecConfig->radius_srv_num; srv_idx++) {
			if (pSecConfig->radius_srv_info[srv_idx].radius_ip != 0) {
				p1xBssInfo->radius_srv_info[srv_idx].radius_ip = pSecConfig->radius_srv_info[srv_idx].radius_ip;
				p1xBssInfo->radius_srv_info[srv_idx].radius_port = pSecConfig->radius_srv_info[srv_idx].radius_port;
				p1xBssInfo->radius_srv_info[srv_idx].radius_key_len = pSecConfig->radius_srv_info[srv_idx].radius_key_len;

				if (pSecConfig->radius_srv_info[srv_idx].radius_key_len > 0) {
					if (pSecConfig->radius_srv_info[srv_idx].radius_key_len > ARRAY_SIZE(p1xBssInfo->radius_srv_info[srv_idx].radius_key))
						pSecConfig->radius_srv_info[srv_idx].radius_key_len = ARRAY_SIZE(p1xBssInfo->radius_srv_info[srv_idx].radius_key);
					NdisMoveMemory(p1xBssInfo->radius_srv_info[srv_idx].radius_key,
								   pSecConfig->radius_srv_info[srv_idx].radius_key,
								   pSecConfig->radius_srv_info[srv_idx].radius_key_len);
				}
			}
		}

#ifdef RADIUS_ACCOUNTING_SUPPORT

		/* prepare accounting radius ip, port and key */
		for (srv_idx = 0; srv_idx < pSecConfig->radius_acct_srv_num; srv_idx++) {
			if (pSecConfig->radius_acct_srv_info[srv_idx].radius_ip != 0) {
				pAcctBssInfo->radius_srv_info[srv_idx].radius_ip = pSecConfig->radius_acct_srv_info[srv_idx].radius_ip;
				pAcctBssInfo->radius_srv_info[srv_idx].radius_port = pSecConfig->radius_acct_srv_info[srv_idx].radius_port;
				pAcctBssInfo->radius_srv_info[srv_idx].radius_key_len = pSecConfig->radius_acct_srv_info[srv_idx].radius_key_len;

				if (pSecConfig->radius_acct_srv_info[srv_idx].radius_key_len > 0) {
					if (pSecConfig->radius_acct_srv_info[srv_idx].radius_key_len > ARRAY_SIZE(pAcctBssInfo->radius_srv_info[srv_idx].radius_key))
						pSecConfig->radius_acct_srv_info[srv_idx].radius_key_len = ARRAY_SIZE(pAcctBssInfo->radius_srv_info[srv_idx].radius_key);

					NdisMoveMemory(pAcctBssInfo->radius_srv_info[srv_idx].radius_key,
								   pSecConfig->radius_acct_srv_info[srv_idx].radius_key,
								   pSecConfig->radius_acct_srv_info[srv_idx].radius_key_len);
				}
			}
		}

#endif /* RADIUS_ACCOUNTING_SUPPORT */
		p1xBssInfo->ieee8021xWEP = (pSecConfig->IEEE8021X) ? 1 : 0;

		if (p1xBssInfo->ieee8021xWEP) {
			/* Default Key index, length and material */
			keyidx = pSecConfig->PairwiseKeyId;
			p1xBssInfo->key_index = keyidx;
			/* Determine if the key is valid. */
			KeyLen = pSecConfig->WepKey[keyidx].KeyLen;

			if (KeyLen == 5 || KeyLen == 13) {
				p1xBssInfo->key_length = KeyLen;
				NdisMoveMemory(p1xBssInfo->key_material, pSecConfig->WepKey[keyidx].Key, KeyLen);
			}
		}

		/* Get NAS-ID per BSS */
		if (pSecConfig->NasIdLen > 0) {
			p1xBssInfo->nasId_len = pSecConfig->NasIdLen;
			NdisMoveMemory(p1xBssInfo->nasId, pSecConfig->NasId, pSecConfig->NasIdLen);
		}

		/* get EAPifname */
		if (pSecConfig->EAPifname_len > 0) {
			pConf->EAPifname_len[apidx_locate] = pSecConfig->EAPifname_len;
			NdisMoveMemory(pConf->EAPifname[apidx_locate], pSecConfig->EAPifname, pSecConfig->EAPifname_len);
		}

		/* get PreAuthifname */
		if (pSecConfig->PreAuthifname_len > 0) {
			pConf->PreAuthifname_len[apidx_locate] = pSecConfig->PreAuthifname_len;
			NdisMoveMemory(pConf->PreAuthifname[apidx_locate], pSecConfig->PreAuthifname, pSecConfig->PreAuthifname_len);
		}

#ifdef RADIUS_ACCOUNTING_SUPPORT
		/* pAcctBssInfo->radius_request_cui = (pSecConfig->radius_request_cui) ? 1 : 0; */
		pAcctBssInfo->radius_acct_authentic = pSecConfig->radius_acct_authentic;
		pAcctBssInfo->acct_interim_interval = pSecConfig->acct_interim_interval;
		pAcctBssInfo->acct_enable = pSecConfig->acct_enable;
#endif /* RADIUS_ACCOUNTING_SUPPORT */
#ifdef RADIUS_MAC_ACL_SUPPORT
		/* Radius MAC Auth Config */
		pConf->RadiusAclEnable[apidx_locate] = pSecConfig->RadiusMacAuthCache.Policy;
		/* Radius MAC Auth Cache Timeout in 1XDaemon */
		pConf->AclCacheTimeout[apidx_locate] = pSecConfig->RadiusMacAuthCacheTimeout;
#endif /* RADIUS_MAC_ACL_SUPPORT */
	}

	wrq->u.data.length = sizeof(DOT1X_CMM_CONF);

	if (copy_to_user(wrq->u.data.pointer, pConf, wrq->u.data.length))
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_ERROR, "copy_to_user() fail\n");

	os_free_mem(mpool);
}

VOID Dot1xIoctlRadiusData(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	UCHAR *pPkt;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO, "IF(ra%d)\n", pObj->ioctl_if);

	if (pObj->ioctl_if > pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_ERROR,
			"Invalid MBSSID index(%d)!\n",
			pObj->ioctl_if);
		return;
	}

	os_alloc_mem(pAd, (UCHAR **)&pPkt, wrq->u.data.length);
	if (pPkt) {
		if (copy_from_user(pPkt, wrq->u.data.pointer, wrq->u.data.length) == 0) {
			pSecConfig = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig;

			if (IS_AKM_1X(pSecConfig->AKMMap)
					|| (pSecConfig->IEEE8021X == TRUE))
				WpaSend(pAd, (PUCHAR)pPkt, wrq->u.data.length);
		}
		os_free_mem(pPkt);
	}
}


/*
    ==========================================================================
    Description:
		UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID Dot1xIoctlAddWPAKey(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	NDIS_AP_802_11_KEY	*pKey;
	ULONG				KeyIdx;
	MAC_TABLE_ENTRY		*pEntry;
	UCHAR				apidx;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	apidx =	(UCHAR) pObj->ioctl_if;
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO, "IF(ra%d)\n", apidx);
	os_alloc_mem(pAd, (UCHAR **)&pKey, wrq->u.data.length);
	if (pKey == NULL)
		return;

	if (copy_from_user(pKey, wrq->u.data.pointer, wrq->u.data.length)) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO,
			"copy from user failed\n");
		os_free_mem(pKey);
		return;
	}

	pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (IS_AKM_1X(pSecConfig->AKMMap)) {
		if ((pKey->KeyLength == 32) || (pKey->KeyLength == 64)) {
			UCHAR key_len = LEN_PMK;

			pEntry = MacTableLookup(pAd, pKey->addr);

			if (pEntry != NULL) {
				INT k_offset = 0;
#ifdef DOT11R_FT_SUPPORT

				/* The key shall be the second 256 bits of the MSK. */
				if (IS_FT_RSN_STA(pEntry) && pKey->KeyLength == 64)
					k_offset = 32;

#endif /* DOT11R_FT_SUPPORT */
				if (IS_AKM_WPA3_192BIT(pSecConfig->AKMMap) && (pKey->KeyLength == 64))
					key_len = LEN_PMK_SHA384;


#ifdef OCE_FILS_SUPPORT
				if (IS_AKM_FILS_SHA384(pEntry->SecConfig.AKMMap) && pKey->KeyLength == 64)
					key_len = LEN_PMK_SHA384;
#endif /* OCE_FILS_SUPPORT */

				NdisMoveMemory(pSecConfig->PMK, pKey->KeyMaterial + k_offset, key_len);
				hex_dump("PMK", pSecConfig->PMK, key_len);
			}
		}
	} else {	/* Old WEP stuff */
		ASIC_SEC_INFO Info = {0};

		if (pKey->KeyLength > 16) {
			os_free_mem(pKey);
			return;
		}
		KeyIdx = pKey->KeyIndex & 0x0fffffff;

		if (KeyIdx < 4) {
			/* For Group key setting */
			if (pKey->KeyIndex & 0x80000000) {
				UINT16 Wcid;
				/* Default key for tx (shared key) */
				pSecConfig->GroupKeyId = (UCHAR) KeyIdx;

				/* set key material and key length */
				if (pKey->KeyLength > 16) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_ERROR, "-IF(ra%d) : Key length too long %d\n", apidx, pKey->KeyLength);
					pKey->KeyLength = 16;
				}

				pSecConfig->WepKey[KeyIdx].KeyLen = (UCHAR) pKey->KeyLength;
				NdisMoveMemory(pSecConfig->WepKey[KeyIdx].Key, &pKey->KeyMaterial, pKey->KeyLength);

				/* Set Ciper type */
				if (pKey->KeyLength == 5)
					SET_CIPHER_WEP40(pSecConfig->GroupCipher);
				else
					SET_CIPHER_WEP104(pSecConfig->GroupCipher);

				/* Get a specific WCID to record this MBSS key attribute */
				GET_GroupKey_WCID(wdev, Wcid);
				/* Set key material to Asic */
				os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
				Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
				Info.Direction = SEC_ASIC_KEY_TX;
				Info.Wcid = Wcid;
				Info.BssIndex = apidx;
				Info.Cipher = pSecConfig->GroupCipher;
				Info.KeyIdx = pSecConfig->GroupKeyId;
				os_move_mem(&Info.PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
				os_move_mem(&Info.Key, &pSecConfig->WepKey[Info.KeyIdx], sizeof(SEC_KEY_INFO));
				HW_ADDREMOVE_KEYTABLE(pAd, &Info);
			} else { /* For Pairwise key setting */
				STA_TR_ENTRY *tr_entry = NULL;

				pEntry = MacTableLookup(pAd, pKey->addr);

				if (pEntry) {
					pSecConfig = &pEntry->SecConfig;
					pSecConfig->PairwiseKeyId = (UCHAR) KeyIdx;
					/* set key material and key length */
					pSecConfig->WepKey[KeyIdx].KeyLen = (UCHAR) pKey->KeyLength;
					NdisMoveMemory(pSecConfig->WepKey[KeyIdx].Key, &pKey->KeyMaterial, pKey->KeyLength);

					/* Set Ciper type */
					if (pKey->KeyLength == 5)
						SET_CIPHER_WEP40(pSecConfig->PairwiseCipher);
					else
						SET_CIPHER_WEP104(pSecConfig->PairwiseCipher);

					/* Set key material to Asic */
					os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
					Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
					Info.Direction = SEC_ASIC_KEY_BOTH;
					Info.Wcid = pEntry->wcid;
					Info.BssIndex = pEntry->func_tb_idx;
					Info.Cipher = pSecConfig->PairwiseCipher;
					Info.KeyIdx = pSecConfig->PairwiseKeyId;
					os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
					os_move_mem(&Info.Key, &pSecConfig->WepKey[Info.KeyIdx], sizeof(SEC_KEY_INFO));
					/* HW_ADDREMOVE_KEYTABLE(pAd, &Info); */
					/* open 802.1x port control and privacy filter */
					tr_entry = tr_entry_get(pAd, pEntry->wcid);
					tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
					WifiSysUpdatePortSecur(pAd, pEntry, &Info);
				}
			}
		}
	}
	os_free_mem(pKey);
}


/*
    ==========================================================================
    Description:
		UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID Dot1xIoctlStaticWepCopy(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	MAC_TABLE_ENTRY  *pEntry;
	UCHAR MacAddr[MAC_ADDR_LEN] = {0};
	UCHAR apidx;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	apidx =	(UCHAR) pObj->ioctl_if;
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_INFO, "RTMPIoctlStaticWepCopy-IF(ra%d)\n", apidx);

	if (wrq->u.data.length != sizeof(MacAddr)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_ERROR, "RTMPIoctlStaticWepCopy: the length isn't match (%d)\n", wrq->u.data.length);
		return;
	} else {
		UINT32 len;

		len = copy_from_user(&MacAddr, wrq->u.data.pointer, wrq->u.data.length);
		pEntry = MacTableLookup(pAd, MacAddr);

		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_ERROR,
				"RTMPIoctlStaticWepCopy: the mac address isn't match\n");
			return;
		} else {
			struct _SECURITY_CONFIG *pSecConfigEnrty = NULL;
			struct _SECURITY_CONFIG *pSecConfigProfile = NULL;
			STA_TR_ENTRY *tr_entry = NULL;
			ASIC_SEC_INFO Info = {0};

#ifdef OCE_FILS_SUPPORT
			if (IS_AKM_FILS(pEntry->SecConfig.AKMMap)) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_DOT1X, DBG_LVL_ERROR,
					"RTMPIoctlStaticWepCopy: skip for FILS\n");
				return;
			}
#endif /* OCE_FILS_SUPPORT */

			pSecConfigProfile = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
			pSecConfigEnrty = &pEntry->SecConfig;
			pSecConfigEnrty->PairwiseKeyId = pSecConfigProfile->PairwiseKeyId;
			pSecConfigEnrty->PairwiseCipher = pSecConfigProfile->PairwiseCipher;
			os_move_mem(&pSecConfigEnrty->WepKey, &pSecConfigProfile->WepKey, sizeof(SEC_KEY_INFO) * SEC_KEY_NUM);
			/* Set key material to Asic */
			os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
			Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
			Info.Direction = SEC_ASIC_KEY_BOTH;
			Info.Wcid = pEntry->wcid;
			Info.BssIndex = pEntry->func_tb_idx;
			Info.Cipher = pEntry->SecConfig.PairwiseCipher;
			Info.KeyIdx = pEntry->SecConfig.PairwiseKeyId;
			os_move_mem(&Info.Key, &pEntry->SecConfig.WepKey[pEntry->SecConfig.PairwiseKeyId], sizeof(SEC_KEY_INFO));
			os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
			/* HW_ADDREMOVE_KEYTABLE(pAd, &Info); */
			/* open 802.1x port control and privacy filter */
			tr_entry = tr_entry_get(pAd, pEntry->wcid);
			tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
			pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
			WifiSysUpdatePortSecur(pAd, pEntry, &Info);
		}
	}

	return;
}
#endif /* CONFIG_AP_SUPPORT */
#endif /* #if defined(DOT1X_SUPPORT) && !defined(RT_CFG80211_SUPPORT) */


#ifdef APCLI_SUPPORT
INT Set_ApCli_Trans_Disable_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj;
	UCHAR *pTransDisable = NULL;
	UCHAR TransDisable = 0;
	UINT32 staidx = 0;

	if (strlen(arg) == 0)
	return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
			"pObj->ioctl_if is invalid value\n");
		return FALSE;
	}

	staidx = pObj->ioctl_if;
	pTransDisable = &pAd->StaCfg[staidx].ApCliTransDisableSupported;

	TransDisable = os_str_tol(arg, 0, 10);

	*pTransDisable = (UCHAR) TransDisable;
	NdisZeroMemory(&(pAd->StaCfg[staidx].ApCli_tti_bitmap), sizeof(struct transition_disable_bitmap));
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
		"[SAE] ApCliTransdisable is=%d\n",
		TransDisable);

	return TRUE;
}

#endif /* APCLI_SUPPORT */

#if (defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)) && !defined(RT_CFG80211_SUPPORT)
UCHAR str_to_bin(
	IN RTMP_STRING *str,
	OUT UCHAR *bin,
	INOUT UINT32 *bin_sz)
{
	UINT32 i = 0;
	UCHAR v = 0;
	UINT32 len = 0;
	UINT32 max_len = *bin_sz;

	for (i = 0; str[i] != '\0' && len <= *bin_sz; i++) {
		if (str[i] >= 'a')
			v += str[i] - 'a' + 10;
		else
			v += str[i] - '0';

		if (i % 2 == 0)
			v <<= 4;
		else {
			bin[i / 2] = v;
			len++;
			v = 0;
		}
	}

	*bin_sz = len;

	return (len == max_len && str[i] != '\0') ? FALSE : TRUE;
}

VOID sae_pwd_id_deinit(IN PRTMP_ADAPTER pAd)
{
#ifdef CONFIG_AP_SUPPORT
	UINT i = 0;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		struct _SECURITY_CONFIG *sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
		struct pwd_id_list *pwd_id_ins = &sec_cfg->pwd_id_list_head;
		struct pwd_id_list *pwd_id_tmp = NULL;

		if (sec_cfg->pwd_id_cnt == 0)
			continue;
		while (DlListEmpty(&pwd_id_ins->list)) {
			pwd_id_tmp = DlListFirst(&pwd_id_ins->list, struct pwd_id_list, list);
			if (pwd_id_tmp != NULL) {/*ALPS05331068*/
				DlListDel(&pwd_id_tmp->list);
				os_free_mem(pwd_id_tmp);
			}
		}

		sec_cfg->pwd_id_cnt = 0;
	}
#endif /* CONFIG_AP_SUPPORT */
}
#endif

INT32 fill_wtbl_key_info_struc(
	IN struct _ASIC_SEC_INFO *pInfo,
	OUT CMD_WTBL_SECURITY_KEY_T * rWtblSecurityKey)
{
	if (IS_REMOVEKEY_OPERATION(pInfo)) {
		rWtblSecurityKey->ucAddRemove = CMD_SEC_KEY_REMOVE_KEY_OP;
		rWtblSecurityKey->ucKeyLen = sizeof(rWtblSecurityKey->aucKeyMaterial);
	} else {   /* Add Key */
		SEC_KEY_INFO *pSecKey = &pInfo->Key;

		rWtblSecurityKey->ucAddRemove = CMD_SEC_KEY_ADD_KEY_OP;
		rWtblSecurityKey->ucKeyId = pInfo->KeyIdx;

		if (pSecKey->KeyLen > sizeof(rWtblSecurityKey->aucKeyMaterial)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
				"KeyLen is larger than the aucKeyMaterial\n");
			return NDIS_STATUS_FAILURE;
		} else
			rWtblSecurityKey->ucKeyLen = pSecKey->KeyLen;

		os_move_mem(rWtblSecurityKey->aucKeyMaterial, pSecKey->Key, rWtblSecurityKey->ucKeyLen);

		if (IS_CIPHER_WEP(pInfo->Cipher)) {
			if (rWtblSecurityKey->ucKeyLen == LEN_WEP40)
				rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_WEP_40;
			else if (rWtblSecurityKey->ucKeyLen == LEN_WEP104)
				rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_WEP_104;
			else if (rWtblSecurityKey->ucKeyLen == LEN_WEP128)
				rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_WEP_128;
		} else if (IS_CIPHER_TKIP(pInfo->Cipher)) {
			rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_TKIP_W_MIC;
			os_move_mem(&rWtblSecurityKey->aucKeyMaterial[16], pSecKey->RxMic, LEN_TKIP_MIC);
			os_move_mem(&rWtblSecurityKey->aucKeyMaterial[24], pSecKey->TxMic, LEN_TKIP_MIC);
		} else if (IS_CIPHER_CCMP128(pInfo->Cipher))
			rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_CCMP_W_MIC;
		else if (IS_CIPHER_CCMP256(pInfo->Cipher))
			rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_CCMP_256;
		else if (IS_CIPHER_GCMP128(pInfo->Cipher))
			rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_GCMP_128;
		else if (IS_CIPHER_GCMP256(pInfo->Cipher))
			rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_GCMP_256;

		else {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
				"Not support Cipher[0x%x]\n",
				pInfo->Cipher);
			return NDIS_STATUS_FAILURE;
		}

		if ((pInfo->Direction == SEC_ASIC_KEY_TX)
			|| (pInfo->Direction == SEC_ASIC_KEY_BOTH)) {
			rWtblSecurityKey->ucRkv = 0;
			rWtblSecurityKey->ucIkv = 0;
		}

		if ((pInfo->Direction == SEC_ASIC_KEY_RX)
			|| (pInfo->Direction == SEC_ASIC_KEY_BOTH)) {
			rWtblSecurityKey->ucRkv = 1;

			if (IS_CIPHER_BIP_CMAC128(pInfo->Cipher)
				|| ((IS_CIPHER_CCMP128(pInfo->Cipher)) && (rWtblSecurityKey->ucKeyLen == 32)))
				rWtblSecurityKey->ucIkv = 1;
		}
	}

	return NDIS_STATUS_SUCCESS;
}


#ifdef WIFI_UNIFIED_COMMAND
INT32 fill_uni_cmd_wtbl_key_info_struc_v2(
	IN void *hdev_ctrl,
	IN struct _ASIC_SEC_INFO *pInfo,
	OUT P_CMD_WTBL_SECURITY_KEY_V2_T rWtblSecurityKey)
{
	rWtblSecurityKey->ucEntryCount = 0;
	if (IS_REMOVEKEY_OPERATION(pInfo)) {
		rWtblSecurityKey->ucAddRemove = CMD_SEC_KEY_REMOVE_KEY_OP;
	} else if (pInfo->bigtk_key_len) { /* Add BIGTK */
		struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(hdev_ctrl);
		UINT8 *buf = rWtblSecurityKey->aucBuffer;
		UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T *wtbl_bip = (UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T *)buf;

		rWtblSecurityKey->ucAddRemove = CMD_SEC_KEY_ADD_KEY_OP;
		wtbl_bip->ucKeyIdx = pInfo->bigtk_key_idx;
		wtbl_bip->ucKeyLength = pInfo->bigtk_key_len;
		wtbl_bip->ucSubLength = sizeof(UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T);
		os_move_mem(&wtbl_bip->aucBipn, &pInfo->tx_tsc, LEN_WPA_TSC);
		wtbl_bip->ucBcnProtMode = chip_cap->bcn_prot_sup;
#ifdef SOFT_BIP_GMAC
		if (IS_CIPHER_BIP_GMAC128(pInfo->Cipher) || IS_CIPHER_BIP_GMAC256(pInfo->Cipher))
			wtbl_bip->ucBcnProtMode = BCN_PROT_EN_SW_MODE;
#endif
		if (IS_CIPHER_BIP_CMAC128(pInfo->Cipher))
			wtbl_bip->ucCipherId = SEC_CIPHER_ID_BCN_PROT_CMAC_128;
		else if (IS_CIPHER_BIP_CMAC256(pInfo->Cipher))
			wtbl_bip->ucCipherId = SEC_CIPHER_ID_BCN_PROT_CMAC_256;
		else if (IS_CIPHER_BIP_GMAC128(pInfo->Cipher))
			wtbl_bip->ucCipherId = SEC_CIPHER_ID_BCN_PROT_GMAC_128;
		else if (IS_CIPHER_BIP_GMAC256(pInfo->Cipher))
			wtbl_bip->ucCipherId = SEC_CIPHER_ID_BCN_PROT_GMAC_256;
		else {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
				"Not support BIGTK[0x%x]\n", pInfo->Cipher);
			return NDIS_STATUS_FAILURE;
		}
		os_move_mem(wtbl_bip->aucKeyMaterial, pInfo->bigtk, pInfo->bigtk_key_len);

		wtbl_bip->u2WlanIndex = cpu2le16(pInfo->Wcid);
		wtbl_bip->ucMgmtProtection = 0;
		wtbl_bip->fgNeedRsp = 0;

		rWtblSecurityKey->u2RekeyWlanIdx2 = cpu2le16(pInfo->Wcid2);
		rWtblSecurityKey->ucEntryCount++;
		buf += wtbl_bip->ucSubLength;

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
			"Install BIGTK, u2WlanIndex = %d, u2RekeyWlanIdx2 = %d, ucCipherId = %d, ucKeyIdx = %d\n",
			wtbl_bip->u2WlanIndex, rWtblSecurityKey->u2RekeyWlanIdx2, wtbl_bip->ucCipherId, wtbl_bip->ucKeyIdx);
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
			"BcnProtMode = %d\n", wtbl_bip->ucBcnProtMode);
	} else if (IS_CIPHER_BIP_CMAC128(pInfo->Cipher)
			|| IS_CIPHER_BIP_CMAC256(pInfo->Cipher)
			|| IS_CIPHER_BIP_GMAC128(pInfo->Cipher)
			|| IS_CIPHER_BIP_GMAC256(pInfo->Cipher)) { /* Add IGTK */
		UINT8 *buf = rWtblSecurityKey->aucBuffer;
		UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T *wtbl_bip = (UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T *)buf;

#ifdef SOFT_BIP_GMAC
		if (IS_CIPHER_BIP_GMAC128(pInfo->Cipher) || IS_CIPHER_BIP_GMAC256(pInfo->Cipher)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_NOTICE,
				"HW does not support BIP-GMAC for IGTK and return!\n");
			return NDIS_STATUS_FAILURE;
		}
#endif /* SOFT_BIP_GMAC */

		rWtblSecurityKey->ucAddRemove = CMD_SEC_KEY_ADD_KEY_OP;
		wtbl_bip->ucKeyIdx = pInfo->igtk_key_idx;
		wtbl_bip->ucKeyLength = pInfo->IGTKKeyLen;
		wtbl_bip->ucSubLength = sizeof(UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T);
		if (IS_CIPHER_BIP_CMAC256(pInfo->Cipher))
			wtbl_bip->ucCipherId = SEC_CIPHER_ID_BIP_CMAC_256;
		else if (IS_CIPHER_BIP_GMAC128(pInfo->Cipher))
			wtbl_bip->ucCipherId = SEC_CIPHER_ID_BIP_GMAC_128;
		else if (IS_CIPHER_BIP_GMAC256(pInfo->Cipher))
			wtbl_bip->ucCipherId = SEC_CIPHER_ID_BIP_GMAC_256;
		else
			wtbl_bip->ucCipherId = SEC_CIPHER_ID_BIP_CMAC_128;
		os_move_mem(wtbl_bip->aucKeyMaterial, pInfo->IGTK, pInfo->IGTKKeyLen);

		wtbl_bip->u2WlanIndex = cpu2le16(pInfo->Wcid);
		wtbl_bip->ucMgmtProtection = 0;
		wtbl_bip->fgNeedRsp = 0;

		rWtblSecurityKey->u2RekeyWlanIdx2 = cpu2le16(pInfo->Wcid2);
		rWtblSecurityKey->ucEntryCount++;
		buf += wtbl_bip->ucSubLength;

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
			"Install IGTK, u2WlanIndex = %d, u2RekeyWlanIdx2 = %d, ucCipherId = %d, ucKeyIdx = %d\n",
			wtbl_bip->u2WlanIndex, rWtblSecurityKey->u2RekeyWlanIdx2, wtbl_bip->ucCipherId, wtbl_bip->ucKeyIdx);
	} else { /* Add PTK or GTK */
		SEC_KEY_INFO *pSecKey = &pInfo->Key;
		UINT8 *buf = rWtblSecurityKey->aucBuffer;
		UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T *wtbl_cipher = (UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T *)buf;

		rWtblSecurityKey->ucAddRemove = CMD_SEC_KEY_ADD_KEY_OP;
		wtbl_cipher->ucKeyIdx = pInfo->KeyIdx;
		if (pSecKey->KeyLen > sizeof(wtbl_cipher->aucKeyMaterial)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
				"KeyLen is larger than the aucKeyMaterial\n");
			return NDIS_STATUS_FAILURE;
		} else
			wtbl_cipher->ucKeyLength = pSecKey->KeyLen;
		wtbl_cipher->ucSubLength = sizeof(UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T);
		os_move_mem(wtbl_cipher->aucKeyMaterial, pSecKey->Key, wtbl_cipher->ucKeyLength);
		if (IS_CIPHER_WEP(pInfo->Cipher)) {
			if (pSecKey->KeyLen == LEN_WEP40)
				wtbl_cipher->ucCipherId = SEC_CIPHER_ID_WEP40;
			else if (pSecKey->KeyLen == LEN_WEP104)
				wtbl_cipher->ucCipherId = SEC_CIPHER_ID_WEP104;
			else if (pSecKey->KeyLen == LEN_WEP128)
				wtbl_cipher->ucCipherId = SEC_CIPHER_ID_WEP128;
		} else if (IS_CIPHER_TKIP(pInfo->Cipher)) {
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_TKIP;
			os_move_mem(&wtbl_cipher->aucKeyMaterial[16], pSecKey->RxMic, LEN_TKIP_MIC);
			os_move_mem(&wtbl_cipher->aucKeyMaterial[24], pSecKey->TxMic, LEN_TKIP_MIC);
		} else if (IS_CIPHER_CCMP128(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_CCMP128;
		else if (IS_CIPHER_CCMP256(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_CCMP256;
		else if (IS_CIPHER_GCMP128(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_GCMP128;
		else if (IS_CIPHER_GCMP256(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_GCMP256;
		else {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
				"Not support Cipher[0x%x]\n", pInfo->Cipher);
			return NDIS_STATUS_FAILURE;
		}

		wtbl_cipher->u2WlanIndex = cpu2le16(pInfo->Wcid);
		wtbl_cipher->ucMgmtProtection = 0;
		wtbl_cipher->fgNeedRsp = 0;

		rWtblSecurityKey->u2RekeyWlanIdx2 = cpu2le16(pInfo->Wcid2);
		rWtblSecurityKey->ucEntryCount++;
		buf += wtbl_cipher->ucSubLength;

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
			"Install PTK/GTK, u2WlanIndex = %d, u2RekeyWlanIdx2 = %d, ucCipherId = %d, ucKeyIdx = %d\n",
			wtbl_cipher->u2WlanIndex, rWtblSecurityKey->u2RekeyWlanIdx2, wtbl_cipher->ucCipherId, wtbl_cipher->ucKeyIdx);
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* WIFI_UNIFIED_COMMAND */

INT32 fill_wtbl_key_info_struc_v2(
	IN struct _ASIC_SEC_INFO *pInfo,
	OUT CMD_WTBL_SECURITY_KEY_V2_T * rWtblSecurityKey)
{
	rWtblSecurityKey->ucEntryCount = 0;
	if (IS_REMOVEKEY_OPERATION(pInfo)) {
		rWtblSecurityKey->ucAddRemove = CMD_SEC_KEY_REMOVE_KEY_OP;
	} else {   /* Add Key */
		SEC_KEY_INFO *pSecKey = &pInfo->Key;
		UINT8 *buf = rWtblSecurityKey->aucBuffer;
		CMD_WTBL_SEC_CIPHER_GENERAL_T *wtbl_cipher = (CMD_WTBL_SEC_CIPHER_GENERAL_T *)buf;
		UCHAR is_igtk_exist = FALSE;

		rWtblSecurityKey->ucAddRemove = CMD_SEC_KEY_ADD_KEY_OP;
		wtbl_cipher->ucKeyIdx = pInfo->KeyIdx;
		wtbl_cipher->ucSubLength = sizeof(CMD_WTBL_SEC_CIPHER_GENERAL_T);

		if (IS_CIPHER_BIP_CMAC128(pInfo->Cipher) ||
			(IS_CIPHER_CCMP128(pInfo->Cipher) && pSecKey->KeyLen == 32)) {
			is_igtk_exist = TRUE;
			pSecKey->KeyLen = LEN_CCMP128_TK;
		}

		if (pSecKey->KeyLen > sizeof(wtbl_cipher->aucKeyMaterial)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
				"KeyLen is larger than the aucKeyMaterial\n");
			return NDIS_STATUS_FAILURE;
		} else
			wtbl_cipher->ucKeyLength = pSecKey->KeyLen;

		os_move_mem(wtbl_cipher->aucKeyMaterial, pSecKey->Key, wtbl_cipher->ucKeyLength);


		if (IS_CIPHER_WEP(pInfo->Cipher)) {
			if (pSecKey->KeyLen == LEN_WEP40)
				wtbl_cipher->ucCipherId = SEC_CIPHER_ID_WEP40;
			else if (pSecKey->KeyLen == LEN_WEP104)
				wtbl_cipher->ucCipherId = SEC_CIPHER_ID_WEP104;
			else if (pSecKey->KeyLen == LEN_WEP128)
				wtbl_cipher->ucCipherId = SEC_CIPHER_ID_WEP128;
		} else if (IS_CIPHER_TKIP(pInfo->Cipher)) {
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_TKIP;
			os_move_mem(&wtbl_cipher->aucKeyMaterial[16], pSecKey->RxMic, LEN_TKIP_MIC);
			os_move_mem(&wtbl_cipher->aucKeyMaterial[24], pSecKey->TxMic, LEN_TKIP_MIC);
		} else if (IS_CIPHER_CCMP128(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_CCMP128;
		else if (IS_CIPHER_CCMP256(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_CCMP256;
		else if (IS_CIPHER_GCMP128(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_GCMP128;
		else if (IS_CIPHER_GCMP256(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_GCMP256;
		else {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
				" Not support Cipher[0x%x]\n",
					 pInfo->Cipher);
			return NDIS_STATUS_FAILURE;
		}

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
				"ucCipherId = %d, ucKeyIdx = %d\n", wtbl_cipher->ucCipherId, wtbl_cipher->ucKeyIdx);

		rWtblSecurityKey->ucEntryCount++;
		buf += wtbl_cipher->ucSubLength;

		if (is_igtk_exist) {
			CMD_WTBL_SEC_CIPHER_BIP_T *wtbl_bip = (CMD_WTBL_SEC_CIPHER_BIP_T *)buf;

			wtbl_bip->ucSubLength = sizeof(CMD_WTBL_SEC_CIPHER_BIP_T);
			wtbl_bip->ucCipherId = SEC_CIPHER_ID_BIP_CMAC_128;
			wtbl_bip->ucKeyLength = pInfo->IGTKKeyLen;
			wtbl_bip->ucKeyIdx = pInfo->igtk_key_idx;
			os_move_mem(wtbl_bip->aucKeyMaterial, pInfo->IGTK, pInfo->IGTKKeyLen);
			rWtblSecurityKey->ucEntryCount++;

			buf += wtbl_bip->ucSubLength;
		}

		if (pInfo->bigtk_key_len) {
			CMD_WTBL_SEC_CIPHER_BIP_T *wtbl_bip = (CMD_WTBL_SEC_CIPHER_BIP_T *)buf;

			wtbl_bip->ucSubLength = sizeof(CMD_WTBL_SEC_CIPHER_BIP_T);
			wtbl_bip->ucCipherId = SEC_CIPHER_ID_BIP_CMAC_128;
			wtbl_bip->ucKeyLength = pInfo->bigtk_key_len;
			wtbl_bip->ucKeyIdx = pInfo->bigtk_key_idx;
			os_move_mem(wtbl_bip->aucKeyMaterial, pInfo->bigtk, pInfo->bigtk_key_len);
			rWtblSecurityKey->ucEntryCount++;
		}
	}

	return NDIS_STATUS_SUCCESS;
}

VOID process_pmkid(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *entry,
	INT CacheIdx)
{
#ifndef RT_CFG80211_SUPPORT
	if (CacheIdx != INVALID_PMKID_IDX) {
		/* Enqueue a EAPOL-start message with the pEntry for WPAPSK State Machine */
		if ((entry->EnqueueEapolStartTimerRunning == EAPOL_START_DISABLE
			)
#ifdef WSC_AP_SUPPORT
			&& !entry->bWscCapable
#endif /* WSC_AP_SUPPORT */
			) {
			/* Enqueue a EAPOL-start message with the pEntry */
			entry->EnqueueEapolStartTimerRunning = EAPOL_START_PSK;
			entry->SecConfig.Handshake.WpaState = AS_INITPSK;
			os_move_mem(&entry->SecConfig.Handshake.AAddr,
				wdev->bssid,
				MAC_ADDR_LEN);
			os_move_mem(&entry->SecConfig.Handshake.SAddr,
				entry->Addr,
				MAC_ADDR_LEN);
			RTMPSetTimer(&entry->SecConfig.StartFor4WayTimer,
				ENQUEUE_EAPOL_START_TIMER);
		}

		store_pmkid_cache_in_sec_config(pAd, entry, CacheIdx);

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_INFO,
				"ASSOC - 2.PMKID matched and start key cache algorithm\n");
	} else {
		store_pmkid_cache_in_sec_config(pAd, entry, INVALID_PMKID_IDX);
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_ERROR,
			 "ASSOC - 2.PMKID not found\n");

		/* Enqueue a EAPOL-start message to trigger EAP SM */
		if (entry->EnqueueEapolStartTimerRunning == EAPOL_START_DISABLE
		) {
			entry->EnqueueEapolStartTimerRunning = EAPOL_START_1X;
			RTMPSetTimer(&entry->SecConfig.StartFor4WayTimer, ENQUEUE_EAPOL_START_TIMER);
		}
	}
#endif /*#ifndef RT_CFG80211_SUPPORT*/
}

VOID store_pmkid_cache_in_sec_config(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN INT32 cache_idx)
{
	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_ERROR,
			"pEntry is null\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_DEBUG, "EntryType = %d\n", pEntry->EntryType);

	if (cache_idx == INVALID_PMKID_IDX) {
		pEntry->SecConfig.pmkid = NULL;
		pEntry->SecConfig.pmk_cache = NULL;
	} else {
		if (IS_ENTRY_CLIENT(pEntry)) {
#ifdef CONFIG_AP_SUPPORT
			if ((cache_idx < 0) || (cache_idx >= MAX_PMKID_COUNT)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_ERROR,
					"cache idx(%d) error\n", cache_idx);
				return;
			}
			pEntry->SecConfig.pmkid =
				PD_GET_PMKID_PTR(pAd->physical_dev)->BSSIDInfo[cache_idx].PMKID;
			pEntry->SecConfig.pmk_cache =
				PD_GET_PMKID_PTR(pAd->physical_dev)->BSSIDInfo[cache_idx].PMK;
#endif
#ifdef CONFIG_STA_SUPPORT
			{
				PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);
				if ((cache_idx < 0) || (cache_idx >= PMKID_NO)) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_ERROR,
						"cache idx(%d) error\n", cache_idx);
					return;
				}
				if (pStaCfg) {
					pEntry->SecConfig.pmkid = pStaCfg->SavedPMK[cache_idx].PMKID;
					pEntry->SecConfig.pmk_cache = pStaCfg->SavedPMK[cache_idx].PMK;
				} else
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_ERROR,
							"pStaCfg is null\n");
			}
#endif
		}
	}
}

UCHAR is_pmkid_cache_in_sec_config(
	IN struct _SECURITY_CONFIG *pSecConfig)
{
	if (pSecConfig && pSecConfig->pmkid && pSecConfig->pmk_cache)
		return TRUE;
	else
		return FALSE;
}

/* input: wdev->SecConfig */
INT build_rsnxe_ie(
	IN struct wifi_dev *wdev,
	IN struct _SECURITY_CONFIG *sec_cfg,
	IN UCHAR *buf)
{
	INT extend_length = 0;
	UCHAR ie = IE_RSNXE;
	UCHAR ie_len = 1;
	UCHAR cap = 0;

	if (!wdev || !sec_cfg)
		return 0;

	/* remove it if any other authmode also use rsnxe */
	if (!IS_AKM_SAE(sec_cfg->AKMMap))
		return 0;

#ifdef DOT11_SAE_SUPPORT
	if (IS_AKM_SAE(sec_cfg->AKMMap) &&
		sec_cfg->sae_cap.gen_pwe_method != PWE_LOOPING_ONLY)
		cap |= (1 << IE_RSNXE_CAPAB_SAE_H2E);

	if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->sae_cap.sae_pk_en != SAE_PK_DISABLE)
		cap |= (1 << IE_RSNXE_CAPAB_SAE_PK);
#endif


	if (cap == 0)
		return 0;

	if (buf == NULL)
		return sizeof(ie) + sizeof(ie_len) + sizeof(cap);

	NdisMoveMemory(buf + extend_length, &ie, sizeof(ie));
	extend_length += sizeof(ie);
	NdisMoveMemory(buf + extend_length, &ie_len, sizeof(ie_len));
	extend_length += sizeof(ie_len);
	NdisMoveMemory(buf + extend_length, &cap, sizeof(cap));
	extend_length += sizeof(cap);

	return extend_length;
}


/* input: pEntry->SecConfig */
UINT parse_rsnxe_ie(
	IN struct _SECURITY_CONFIG *sec_cfg,
	IN UCHAR *rsnxe_ie,
	IN UCHAR rsnxe_ie_len,
	IN UCHAR need_copy)
{
#ifdef DOT11_SAE_SUPPORT
	if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->sae_conn_type) {
		if (rsnxe_ie[1] == 0 || !(rsnxe_ie[2] & (1 << IE_RSNXE_CAPAB_SAE_H2E))) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				"IE_RSNXE_CAPAB_SAE_H2E should not be 0\n");
			return MLME_UNSPECIFY_FAIL;
		}
	}

	if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->sae_conn_type == SAE_CONNECTION_TYPE_SAEPK) {
		if (rsnxe_ie[1] == 0 || !(rsnxe_ie[2] & (1 << IE_RSNXE_CAPAB_SAE_PK))) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				"IE_RSNXE_CAPAB_SAE_PK should not be 0\n");
			return MLME_UNSPECIFY_FAIL;
		}
	}

	if (need_copy) {
		NdisMoveMemory(sec_cfg->rsnxe_content, rsnxe_ie, rsnxe_ie_len);
		sec_cfg->rsnxe_len = rsnxe_ie_len;
	} else if (sec_cfg->rsnxe_len == 0) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				"fail due to sec_cfg->rsnxe_len == 0\n");
		return MLME_UNSPECIFY_FAIL;
	} else if (NdisCmpMemory(sec_cfg->rsnxe_content, rsnxe_ie, rsnxe_ie_len) != 0) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				"rsnxe compare fail\n");
		return MLME_UNSPECIFY_FAIL;
	}
#endif
	return MLME_SUCCESS;
}

/* not support oct now */
VOID build_oci_common_field(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN UCHAR oct_support,
	OUT UCHAR *buf,
	OUT ULONG *buf_len)
{
	buf[0] = get_regulatory_class(ad, wdev->channel, wdev->PhyMode, wdev); /* Operating Class */
	buf[1] = (wpa3_test_ctrl == 12) ? wpa3_test_ctrl2 : wdev->channel; /*wlan_operate_get_prim_ch(wdev)*/ /* Primary Channel Number */
	buf[2] = wlan_operate_get_cen_ch_2(wdev); /* Frequency Segment 1 Channel Number */

	*buf_len = 3;

	if (oct_support)
		; /* not support now */
}

VOID build_oci_ie(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	OUT UCHAR *buf,
	OUT ULONG *buf_len)
{
	ULONG len;

	if (wdev == NULL)
		return;

	buf[0] = IE_WLAN_EXTENSION;
	buf[2] = EID_EXT_OCI;

	build_oci_common_field(ad, wdev, TRUE, buf + 3, &len);
	buf[1] = len + 1;
	*buf_len = len + 3;

	hex_dump_with_cat_and_lvl("oci ie", buf,
		*buf_len, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_INFO); /* todo: change to trace*/
}

UCHAR parse_oci_common_field(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN UCHAR oct_support,
	IN UCHAR *buf,
	IN ULONG buf_len)
{
	UCHAR bw = wlan_operate_get_bw(wdev);
	UCHAR spacing;

	hex_dump_with_cat_and_lvl("peer oci", buf, buf_len, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_INFO);

	if (buf_len < 3) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			"invalid oci len(%lu)\n",
			buf_len);
		return FALSE;
	}

	if (!is_channel_in_channelset_by_reg_class(ad, buf[0], wdev->PhyMode, wdev->channel)) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			"operating class(%d) check fail, chanel isn't in channel list\n",
			buf[0]);
		return FALSE;
	}

	if (get_spacing_by_reg_class(ad, buf[0], wdev->PhyMode, &spacing) == FALSE) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			"get reg_class table fail(op class = %d)\n",
			buf[0]);
		return FALSE;
	}

	if (bw > spacing) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			"bandwidth mismatch (peer: %d, our: %d)\n",
			spacing, bw);
		return FALSE;
	}

	if (buf[1] != wdev->channel) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			"primary ch(%d) check fail, primary ch(%d)\n",
			buf[1], wdev->channel);
		return FALSE;
	}

	if (buf[2] != wlan_operate_get_cen_ch_2(wdev)) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			"second central ch(%d) check fail, second central ch(%d)\n",
			buf[2], wlan_operate_get_cen_ch_2(wdev));
		return FALSE;
	}

	if (!oct_support && buf_len > 3) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			"oci field len is wrong\n");
		return FALSE;
	}

	return TRUE;
}

UCHAR parse_oci_ie(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN UCHAR *buf,
	IN ULONG buf_len)
{

	if (buf_len < MIN_OCI_LEN) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
				"buf_len(%lu) is less than min oci len\n", buf_len);
		return FALSE;
	}

	if (buf[0] != IE_WLAN_EXTENSION ||
		buf[2] != EID_EXT_OCI) {
		hex_dump_with_cat_and_lvl("oci_ie fail, peer oci ie", buf,
			buf_len, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR);
		return FALSE;
	}

	return parse_oci_common_field(ad, wdev, TRUE, buf + 3, buf[1] - 1);
}

void sec_update_entry_rsn_ie(
	struct _MAC_TABLE_ENTRY *entry,
	struct _IE_lists *ie_list)
{
	UCHAR ie_len = 0;
	UCHAR *ie_ptr;
	BOOLEAN is_override_ie = FALSE;

	NdisZeroMemory(entry->RSN_IE, MAX_LEN_OF_RSNIE);
	entry->RSNIE_Len = 0;

#ifdef CFG_RSNO_SUPPORT
	if (ie_list->rsneo_ie_len) {
		ie_len = ie_list->rsneo_ie_len;
		ie_ptr = ie_list->rsneo_ie;
		is_override_ie = TRUE;
	} else
#endif /* CFG_RSNO_SUPPORT */
	if (ie_list->RSNIE_Len) {
		ie_len = ie_list->RSNIE_Len;
		ie_ptr = ie_list->RSN_IE;
	}
	if (ie_len) {
		if (is_override_ie) {
			entry->RSNIE_Len = ie_len - VENDOR_OUI_TYPE_LEN;
			ie_ptr += (2 + VENDOR_OUI_TYPE_LEN); /* 2: ID(1) + Len(1) */
			NdisMoveMemory(&entry->RSN_IE[2], ie_ptr, ie_len-(2 + VENDOR_OUI_TYPE_LEN));
			entry->RSN_IE[0] = EID_RSN;
			entry->RSN_IE[1] = entry->RSNIE_Len - 2;
			hex_dump_with_cat_and_lvl("New RSN IE",
				entry->RSN_IE, entry->RSNIE_Len,
				DBG_CAT_SEC, CATSEC_RSNO, DBG_LVL_INFO);
		} else {
			NdisMoveMemory(entry->RSN_IE, ie_ptr, ie_len);
			entry->RSNIE_Len = ie_len;
		}
	}
}

