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
	sec.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/

#ifndef	__SEC_H__
#define	__SEC_H__

#include "security/sec_cmm.h"
#include "security/crypt_md5.h"
#include "security/crypt_sha2.h"
#include "security/crypt_hmac.h"
#include "security/crypt_aes.h"
#include "security/crypt_arc4.h"
#include "security/wpa.h"

#ifdef DOT11W_PMF_SUPPORT
#include "security/pmf.h"
#endif /* DOT11W_PMF_SUPPORT */


#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
#include "security/sae_cmm.h"
#include "security/sae.h"
#include "security/owe_cmm.h"
#endif /* DOT11_SAE_SUPPORT */

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
#include "security/ecc.h"
#endif

#ifdef RT_CFG80211_SUPPORT
#include "mtk_vendor_nl80211.h"
#endif

extern UCHAR wpa3_test_ctrl; /* for wpa3 certifcation ap testbed specfic cmd */
extern UCHAR wpa3_test_ctrl2; /* for wpa3 certifcation ap testbed specfic cmd */
extern const UCHAR wfa_oui[3];
/*========================================
	The prototype is defined in cmm_sec.c
  ========================================*/
#ifdef RT_CFG80211_SUPPORT
int SetWdevEncrypModeByNlAttr(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN enum mtk_vendor_attr_encryptype type);

int SetWdevAuthModeByNlAttr(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN enum mtk_vendor_attr_authmode auth_mode);

INT Set_SecDefaultKeyID(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR KeyIdx);

INT Set_SecWEPKey(
	IN PRTMP_ADAPTER pAd,
	IN struct wep_key_param *wep_key);

INT	Set_SecWPAPSK(
	IN PRTMP_ADAPTER pAd,
	IN char *passphrase);

#endif /* RT_CFG80211_SUPPORT */
VOID SetWdevAuthMode(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN	RTMP_STRING * arg);

VOID SetWdevEncrypMode(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN RTMP_STRING * arg);

INT Set_SecAuthMode_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);

INT Set_SecEncrypType_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);

INT Set_SecDefaultKeyID_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);

INT Set_SecWPAPSK_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);

INT Set_SecWEPKey_Proc(
	IN PRTMP_ADAPTER pAd,
	IN CHAR KeyId,
	IN RTMP_STRING * arg);

INT Set_SecKey1_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg);

INT Set_SecKey2_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg);

INT Set_SecKey3_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg);

INT Set_SecKey4_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg);

UINT8 SecHWCipherSuitMapping(
	IN UINT32 encryMode);

INT ParseWebKey(
	IN  struct _SECURITY_CONFIG *pSecConfig,
	IN  RTMP_STRING * buffer,
	IN  INT KeyIdx,
	IN  INT Keylength);

#if defined(DOT1X_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
INT SetWdevOwnIPAddr(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN RTMP_STRING * arg);

VOID ReadRadiusParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * tmpbuf,
	IN RTMP_STRING * pBuffer);

#ifdef CONFIG_AP_SUPPORT
VOID Dot1xIoctlQueryRadiusConf(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID Dot1xIoctlRadiusData(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID Dot1xIoctlAddWPAKey(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID Dot1xIoctlStaticWepCopy(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT1X_SUPPORT */

#ifdef APCLI_SUPPORT
VOID ReadApcliSecParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * tmpbuf,
	IN RTMP_STRING * pBuffer);

INT Set_ApCli_Trans_Disable_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);
#endif /* APCLI_SUPPORT */

#if (defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)) && !defined(RT_CFG80211_SUPPORT)
UCHAR str_to_bin(
	IN RTMP_STRING *str,
	OUT UCHAR *bin,
	INOUT UINT32 *bin_sz);

VOID sae_pwd_id_deinit(
	IN PRTMP_ADAPTER pAd);
#endif

INT32 fill_wtbl_key_info_struc(
	IN struct _ASIC_SEC_INFO *pInfo,
	OUT CMD_WTBL_SECURITY_KEY_T * rWtblSecurityKey);

#ifdef WIFI_UNIFIED_COMMAND
INT32 fill_uni_cmd_wtbl_key_info_struc_v2(
	IN void *hdev_ctrl,
	IN struct _ASIC_SEC_INFO *pInfo,
	OUT P_CMD_WTBL_SECURITY_KEY_V2_T rWtblSecurityKey);
#endif /* WIFI_UNIFIED_COMMAND */

INT32 fill_wtbl_key_info_struc_v2(
	IN struct _ASIC_SEC_INFO *pInfo,
	OUT CMD_WTBL_SECURITY_KEY_V2_T * rWtblSecurityKey);


VOID store_pmkid_cache_in_sec_config(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN INT32 cache_idx);

VOID process_pmkid(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *entry,
	INT CacheIdx);

UCHAR is_pmkid_cache_in_sec_config(
	IN struct _SECURITY_CONFIG *pSecConfig);

INT build_rsnxe_ie(
	IN struct wifi_dev *wdev,
	IN struct _SECURITY_CONFIG *sec_cfg,
	IN UCHAR *buf);

UINT parse_rsnxe_ie(
	IN struct _SECURITY_CONFIG *sec_cfg,
	IN UCHAR *rsnxe_ie,
	IN UCHAR rsnxe_ie_len,
	IN UCHAR need_copy);

VOID build_oci_common_field(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN UCHAR oct_support,
	OUT UCHAR *buf,
	OUT ULONG *buf_len);

VOID build_oci_ie(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	OUT UCHAR *buf,
	OUT ULONG *buf_len);

UCHAR parse_oci_common_field(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN UCHAR oct_support,
	IN UCHAR *buf,
	IN ULONG buf_len);

UCHAR parse_oci_ie(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN UCHAR *buf,
	IN ULONG buf_len);


INT set_wpa3_test(
	IN RTMP_ADAPTER *ad, RTMP_STRING *arg);

INT set_transition_disable(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg);

#ifndef RT_CFG80211_SUPPORT
#ifdef DOT11_EHT_BE
u8 *build_mlo_link_kde(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN u8 link_id,
	IN u8 *buf,
	OUT ULONG * offset);

int parse_mlo_link_kde(
	IN u8 *buf,
	IN u32 buf_len,
	OUT u8 *link_id,
	OUT u8 **link_mac,
	OUT u8 **rsne,
	OUT u8 **rsnxe);

u8 *build_mlo_gtk_kde(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN u8 link_id,
	IN u8 *buf,
	OUT ULONG * offset);

#ifdef DOT11W_PMF_SUPPORT
u8 *build_mlo_igtk_kde(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN u8 link_id,
	IN u8 *buf,
	OUT ULONG * offset);
#endif

#ifdef BCN_PROTECTION_SUPPORT
u8 *build_mlo_bigtk_kde(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN u8 link_id,
	IN u8 *buf,
	OUT ULONG * offset);
#endif
#endif
#endif /* RT_CFG80211_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
/* ========================================
    The prototype is defined in ap/ap_sec.c
  ========================================*/
INT APSecInit(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev);

INT ap_sec_deinit(
	IN struct wifi_dev *wdev);

INT ap_key_table_init(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev);

INT ap_set_key_for_sta_rec(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev,
	IN STA_REC_CTRL_T * sta_rec);

#ifndef RT_CFG80211_SUPPORT
DECLARE_TIMER_FUNCTION(GroupRekeyExec);
VOID GroupRekeyExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif /* RT_CFG80211_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT
VOID ap_set_wait_sa_query_for_csa(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev);

VOID APPMFInit(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev);

DECLARE_TIMER_FUNCTION(csa_timeout_proc);
VOID csa_timeout_proc(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif

#ifndef RT_CFG80211_SUPPORT
VOID WPAGroupRekeyByWdev(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev);
#endif /* RT_CFG80211_SUPPORT */

VOID APStartRekeyTimer(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev);


VOID APStopRekeyTimer(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev);


VOID APReleaseRekeyTimer(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev);


INT Show_APSecurityInfo_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);

VOID CheckBMCPortSecured(
	IN RTMP_ADAPTER * pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN BOOLEAN isConnect);


#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
/* ========================================
    The prototype is defined in sta/sta_sec.c
  ========================================*/
INT Show_STASecurityInfo_Proc(
	IN PRTMP_ADAPTER pAd,
	OUT RTMP_STRING * pBuf,
	IN ULONG BufLen);

VOID PaserSecurityIE(
	IN struct wifi_dev *wdev,
	IN struct _MAC_TABLE_ENTRY *entry,
	IN BCN_IE_LIST * ie_list,
	IN USHORT * LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE,
	OUT USHORT * RsnCapability,
	OUT USHORT * IsSHA256);
#endif /* CONFIG_STA_SUPPORT */

#ifdef BCN_PROTECTION_SUPPORT
INT show_bcn_prot_dbg_info(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);
#endif /* BCN_PROTECTION_SUPPORT */


void sec_update_entry_rsn_ie(
	struct _MAC_TABLE_ENTRY *entry,
	struct _IE_lists *ie_list);

#endif
