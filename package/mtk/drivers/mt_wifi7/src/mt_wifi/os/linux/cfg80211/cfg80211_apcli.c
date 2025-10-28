#ifdef MTK_LICENSE
/****************************************************************************
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
 ***************************************************************************/
#endif /* MTK_LICENSE */
/****************************************************************************

	Abstract:

	All related CFG80211 P2P function body.

	History:

***************************************************************************/
#ifdef APCLI_CFG80211_SUPPORT
#include "rt_config.h"

INT CFG80211_FindStaIdxByNetDevice(RTMP_ADAPTER *pAd, PNET_DEV pNetDev)
{
	USHORT index = 0;
	BOOLEAN found = FALSE;
	PSTA_ADMIN_CONFIG pStaCfg;
	struct wifi_dev *wdev;
	UINT32 max_num_sta = pAd->MaxMSTANum;

	max_num_sta = min(pAd->ApCfg.ApCliNum, (UCHAR)MAX_APCLI_NUM);

#ifndef IWCOMMAND_CFG80211_SUPPORT
	if (pAd->flg_msta_init != FALSE) {
#endif /* IWCOMMAND_CFG80211_SUPPORT */
		for (index = 0; index < max_num_sta; index++) {
			pStaCfg = &pAd->StaCfg[index];
			wdev = &pStaCfg->wdev;
			if (pNetDev == wdev->if_dev) {
				found = TRUE;
				break;
			}
		}
#ifndef IWCOMMAND_CFG80211_SUPPORT
	}
#endif /* IWCOMMAND_CFG80211_SUPPORT */

	return (found) ? index : WDEV_NOT_FOUND;
}


VOID CFG80211DRV_SetApCliAssocIe(VOID *pAdOrg, PNET_DEV pNetDev, VOID *pData, UINT ie_len)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	struct wifi_dev *wdev;
	STA_ADMIN_CONFIG *apcli_entry;
	INT ifIndex;
	hex_dump("APCLI=", pData, ie_len);

	ifIndex = CFG80211_FindStaIdxByNetDevice(pAd, pNetDev);
	if (ifIndex == WDEV_NOT_FOUND) {
		printk("ifIndex for net device not found Returning \n");
		return;
	}
	apcli_entry = &pAd->StaCfg[ifIndex];
	wdev = &apcli_entry->wdev;

	if (ie_len > 0) {
		if (apcli_entry->wpa_supplicant_info.pWpaAssocIe) {
			os_free_mem(apcli_entry->wpa_supplicant_info.pWpaAssocIe);
			apcli_entry->wpa_supplicant_info.pWpaAssocIe = NULL;
		}

		os_alloc_mem(NULL, (UCHAR **)&apcli_entry->wpa_supplicant_info.pWpaAssocIe, ie_len);
		if (apcli_entry->wpa_supplicant_info.pWpaAssocIe) {
			apcli_entry->wpa_supplicant_info.WpaAssocIeLen = ie_len;
			os_move_mem(apcli_entry->wpa_supplicant_info.pWpaAssocIe, pData, apcli_entry->wpa_supplicant_info.WpaAssocIeLen);
		} else
			apcli_entry->wpa_supplicant_info.WpaAssocIeLen = 0;
	} else {
		if (apcli_entry->wpa_supplicant_info.pWpaAssocIe) {
			os_free_mem(apcli_entry->wpa_supplicant_info.pWpaAssocIe);
			apcli_entry->wpa_supplicant_info.pWpaAssocIe = NULL;
		}
		apcli_entry->wpa_supplicant_info.WpaAssocIeLen = 0;
	}
}


VOID CFG80211_LostApInform(VOID *pAdOrg)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PNET_DEV pNetDev = pAd->StaCfg[0].wdev.if_dev;
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
	/*ULONG *cur_state = &pAd->ApCfg.ApCliTab[MAIN_MBSSID].CtrlCurrState;
	ULONG *cur_state = &pAd->StaCfg[0].CtrlCurrState;*/

	MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_NOTICE, "Lost Ap info.\n");

	pAd->cfg80211_ctrl.FlgCfg80211Connecting = FALSE;
	pAd->StaCfg[0].ApcliInfStat.Enable = FALSE;
	cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);

	if (pNetDev) {
		/*if (p80211CB->pCfg80211_Wdev->sme_state == CFG80211_SME_CONNECTED) {*/
		if (STA_STATUS_TEST_FLAG(&pAd->StaCfg[0], fSTA_STATUS_MEDIA_STATE_CONNECTED)) {
			cfg80211_disconnected(pNetDev, 0, NULL, 0, FALSE, GFP_KERNEL);
		} else if (pAd->StaCfg[0].wdev.cntl_machine.CurrState >= CNTL_WAIT_AUTH) {
			MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_NOTICE,
				"cntl_machine.CurrState = %lu.\n", pAd->StaCfg[0].wdev.cntl_machine.CurrState);
			/* cfg80211_connect_result(pNetDev, NULL, NULL, 0, NULL, 0,
				WLAN_STATUS_UNSPECIFIED_FAILURE, GFP_KERNEL); */
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_ERROR,
			"80211> BUG CFG80211_LostAPInform, BUT NetDevice not exist.\n");
	}
}

VOID CFG80211DRV_ApClientKeyAdd(VOID *pAdOrg, VOID *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_KEY *pKeyInfo;
	PSTA_ADMIN_CONFIG pApCliEntry;
	MAC_TABLE_ENTRY *pMacEntry = (MAC_TABLE_ENTRY *)NULL;
#ifdef DOT11_EHT_BE
	MAC_TABLE_ENTRY *pmlo_entry = NULL;
	struct mld_entry_t *mld_entry = NULL;
#endif
	STA_TR_ENTRY *tr_entry = NULL;
	struct wifi_dev *wdev;
	INT ifIndex;
	INT BssIdx;
	INT idx = 0;
#if defined(CONFIG_MAP_SUPPORT) && defined(DOT11_EHT_BE)
	struct mld_dev *mld = NULL;
	uint8_t i = 0;
	MAC_TABLE_ENTRY *pCurEntry = NULL;
	struct wifi_dev *pCurwdev = NULL;
	uint8_t need_send_event = TRUE;
#endif

	pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;
	ifIndex = CFG80211_FindStaIdxByNetDevice(pAd, pKeyInfo->pNetDev);
	if (ifIndex == WDEV_NOT_FOUND) {
		printk("Could not add key as Ifindex for netdevice not found \n");
		return;
	}
	BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + MAIN_MBSSID;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	wdev = &pApCliEntry->wdev;
#ifdef CONFIG_MAP_SUPPORT
	mld = wdev->mld_dev;
#endif
	pMacEntry = entry_get(pAd, pApCliEntry->MacTabWCID);
	if (pMacEntry && pMacEntry->wcid)
		tr_entry = tr_entry_get(pAd, pMacEntry->wcid);
	else {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
			"pMacEntry or wcid of pMacEntry not exist.\n");
		return;
	}

#ifdef DOT11_EHT_BE
	if (pKeyInfo->bPairwise == FALSE) {
		if ((pKeyInfo->KeyType == RT_CMD_80211_KEY_WPA)
				|| (pKeyInfo->KeyType == RT_CMD_80211_KEY_AES_CMAC)
				|| (pKeyInfo->KeyType == RT_CMD_80211_KEY_AES_GMAC256)
				|| (pKeyInfo->KeyType == RT_CMD_80211_KEY_AES_GMAC128)
				|| (pKeyInfo->KeyType == RT_CMD_80211_KEY_AES_CMAC256)) {
			if (pMacEntry->mlo.mlo_en && pAd->CommonCfg.mlo_preset_link_id < MLD_LINK_MAX) {
				mt_rcu_read_lock();
				mld_entry = rcu_dereference(pMacEntry->mld_entry);
				if (!mld_entry) {
					MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
					"(%s)[%d]: ERROR, mld_entry=NULL\n", __func__, __LINE__);
					mt_rcu_read_unlock();
					return;
				}
				for (idx = 0; idx < MLD_LINK_MAX; idx++) {
					pmlo_entry = mld_entry->link_entry[idx];
					if (pmlo_entry && pmlo_entry->mlo.link_info.link_id == pAd->CommonCfg.mlo_preset_link_id) {
						MTWF_DBG(pAd,
							DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_NOTICE,
							"mlo_preset_link_id: %d. pmlo_entry(%p),mac_addr"MACSTR"idx(%d),ucLinkId:%d\n",
							pAd->CommonCfg.mlo_preset_link_id, pmlo_entry, MAC2STR(pmlo_entry->Addr),
							idx, pmlo_entry->mlo.link_info.link_id);
						pMacEntry = pmlo_entry;
						wdev = pmlo_entry->wdev;
						tr_entry = tr_entry_get(pAd, pMacEntry->wcid);
						break;
					}
				}
				mt_rcu_read_unlock();
			}
		}
	}
#endif/*DOT11_EHT_BE*/

	if (!tr_entry) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
			"tr_entry not exist\n");
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
		"CFG Debug --->\n");
	if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40 || pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP104) {
		if (pKeyInfo->bPairwise == FALSE) {
			struct _ASIC_SEC_INFO *info = NULL;
			int i = 0;
			if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40)
				SET_CIPHER_WEP40(wdev->SecConfig.GroupCipher);
			else
				SET_CIPHER_WEP104(wdev->SecConfig.GroupCipher);

			NdisCopyMemory(&pAd->cfg80211_ctrl.WepKeyInfoBackup, pKeyInfo, sizeof(CMD_RTPRIV_IOCTL_80211_KEY));
			wdev->SecConfig.WepKey[pKeyInfo->KeyId].KeyLen = pKeyInfo->KeyLen;
			os_move_mem(wdev->SecConfig.WepKey[pKeyInfo->KeyId].Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
			wdev->SecConfig.GroupKeyId = pKeyInfo->KeyId;
			os_move_mem(wdev->SecConfig.GTK, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
			/* Set key material to Asic */
			os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
			if (info == NULL) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
						"Fail to allocate memory!\n");
				return;
			} else {
				os_zero_mem(info, sizeof(ASIC_SEC_INFO));
				info->Operation = SEC_ASIC_ADD_GROUP_KEY;
				info->Direction = SEC_ASIC_KEY_RX;
				info->Wcid = wdev->bss_info_argument.bmc_wlan_idx;
				info->BssIndex = pMacEntry->func_tb_idx;
				/* Info.BssIndex = wdev->wdev_idx; */
				info->Cipher = wdev->SecConfig.GroupCipher;
				info->KeyIdx = pKeyInfo->KeyId;
				os_move_mem(&info->PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
				os_move_mem(info->Key.Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
				info->Key.KeyLen = pKeyInfo->KeyLen;

				for (i = 0; i < pKeyInfo->KeyLen; i++)
					MTWF_PRINT("%02x ", pKeyInfo->KeyBuf[i]);
				MTWF_PRINT("\n");
				HW_ADDREMOVE_KEYTABLE(pAd, info);
				os_free_mem(info);
			}
		}
		{
			struct _ASIC_SEC_INFO *info = NULL;
			int i = 0;
			pMacEntry->SecConfig.PairwiseKeyId = pKeyInfo->KeyId;
			if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40)
				SET_CIPHER_WEP40(pMacEntry->SecConfig.PairwiseCipher);
			else
				SET_CIPHER_WEP104(pMacEntry->SecConfig.PairwiseCipher);

			/* Set key material to Asic */
			os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
			if (info == NULL) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
						"Fail to allocate memory!\n");
				return;
			} else {
				os_zero_mem(info, sizeof(ASIC_SEC_INFO));
				info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
				info->Direction = SEC_ASIC_KEY_BOTH;
				info->Wcid = pMacEntry->wcid;
				info->BssIndex = pMacEntry->func_tb_idx;
				/* Info.BssIndex = wdev->wdev_idx; */
				info->KeyIdx = pMacEntry->SecConfig.PairwiseKeyId;
				info->Cipher = pMacEntry->SecConfig.PairwiseCipher;
				info->KeyIdx = pMacEntry->SecConfig.PairwiseKeyId;
				os_move_mem(info->Key.Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
				os_move_mem(&info->PeerAddr[0], pMacEntry->Addr, MAC_ADDR_LEN);
				info->Key.KeyLen = pKeyInfo->KeyLen;
				for (i = 0; i < pKeyInfo->KeyLen; i++)
					MTWF_PRINT("%02x ", pKeyInfo->KeyBuf[i]);
				MTWF_PRINT("\n");
				HW_ADDREMOVE_KEYTABLE(pAd, info);
				os_free_mem(info);
			}
		}
	} else if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WPA) {
		if (pKeyInfo->bPairwise == FALSE) {
			struct _ASIC_SEC_INFO *info = NULL;

			if ((pKeyInfo->KeyId != 1) && (pKeyInfo->KeyId != 2))
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
					"Invalid GTK Key ID\n");
			else
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
					"GTK pKeyInfo->KeyId=%d\n", pKeyInfo->KeyId);

			NdisCopyMemory(&pMacEntry->SecConfig.GTK, pKeyInfo->KeyBuf, MAX_LEN_GTK);

			CLEAR_GROUP_CIPHER(&pMacEntry->SecConfig);

			switch (pKeyInfo->cipher) {
			case Ndis802_11GCMP256Enable:
				SET_CIPHER_GCMP256(pMacEntry->SecConfig.GroupCipher);
				break;

			case Ndis802_11AESEnable:
				SET_CIPHER_CCMP128(pMacEntry->SecConfig.GroupCipher);
				break;

			case Ndis802_11TKIPEnable:
				SET_CIPHER_TKIP(pMacEntry->SecConfig.GroupCipher);
				break;

			case Ndis802_11GCMP128Enable:
				SET_CIPHER_GCMP128(pMacEntry->SecConfig.GroupCipher);
				break;

			case Ndis802_11CCMP256Enable:
				SET_CIPHER_CCMP256(pMacEntry->SecConfig.GroupCipher);
				break;
			}

			/* Set key material to Asic */
			os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
			if (info) {
				os_zero_mem(info, sizeof(ASIC_SEC_INFO));
				info->Operation = SEC_ASIC_ADD_GROUP_KEY;
				info->Direction = SEC_ASIC_KEY_RX;
				info->BssIndex = pMacEntry->func_tb_idx;
				info->Cipher = pMacEntry->SecConfig.GroupCipher;
				info->KeyIdx = (UINT8)(pKeyInfo->KeyId & 0x0fff);
				pMacEntry->SecConfig.GroupKeyId = info->KeyIdx;
				if (!pMacEntry->IsRekeyGTK) {
					info->Wcid = wdev->bss_info_argument.bmc_wlan_idx;
					info->Wcid2 = 0;
					pMacEntry->IsRekeyGTK = TRUE;
				} else {
					info->Wcid = wdev->bss_info_argument.bmc_wlan_idx2;
					info->Wcid2 = wdev->bss_info_argument.bmc_wlan_idx;
				}
				os_move_mem(&info->PeerAddr[0], pMacEntry->Addr, MAC_ADDR_LEN);
				/* Install Shared key */
				os_move_mem(info->Key.Key, &pMacEntry->SecConfig.GTK, LEN_MAX_GTK);
#ifdef CONFIG_MAP_SUPPORT
				if (IS_MAP_ENABLE(pAd) && tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
					need_send_event = FALSE;
#endif
				if (IS_AKM_WPA_CAPABILITY(pMacEntry->SecConfig.AKMMap)) {
					/* set 802.1x port control */
					tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
					pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
					WifiSysUpdatePortSecur(pAd, pMacEntry, NULL);
				}
				HOST_DBG(pAd, "ApClientKeyAdd GTK, wcid=%d, cipher=0x%x, operation=%d\n", info->Wcid, info->Cipher, IS_REMOVEKEY_OPERATION(info));
				HOST_HEXDUMP(pAd, "GTK", (UCHAR *)info->Key.Key, LEN_MAX_GTK);
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
					"ApClientKeyAdd GTK, wcid=%d, cipher=0x%x, operation=%d\n", info->Wcid, info->Cipher, IS_REMOVEKEY_OPERATION(info));
				WPAInstallKey(pAd, info, TRUE, TRUE);
				wdev->SecConfig.Handshake.GTKState = REKEY_ESTABLISHED;
				os_free_mem(info);
			} else
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
					"struct alloc fail\n");
		} else {
			if (pMacEntry) {
				struct _ASIC_SEC_INFO *info = NULL;
				NdisCopyMemory(&pMacEntry->SecConfig.PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);

				CLEAR_PAIRWISE_CIPHER(&pMacEntry->SecConfig);

				switch (pKeyInfo->cipher) {
				case Ndis802_11GCMP256Enable:
					SET_CIPHER_GCMP256(pMacEntry->SecConfig.PairwiseCipher);
					break;

				case Ndis802_11AESEnable:
					SET_CIPHER_CCMP128(pMacEntry->SecConfig.PairwiseCipher);
					break;

				case Ndis802_11TKIPEnable:
					SET_CIPHER_TKIP(pMacEntry->SecConfig.PairwiseCipher);
					break;

				case Ndis802_11GCMP128Enable:
					SET_CIPHER_GCMP128(pMacEntry->SecConfig.PairwiseCipher);
					break;

				case Ndis802_11CCMP256Enable:
					SET_CIPHER_CCMP256(pMacEntry->SecConfig.PairwiseCipher);
					break;
				}

				/* Set key material to Asic */
				os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
				if (info) {
					os_zero_mem(info, sizeof(ASIC_SEC_INFO));
					info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
					info->Direction = SEC_ASIC_KEY_BOTH;
					info->Wcid = pMacEntry->wcid;
					info->BssIndex = pMacEntry->func_tb_idx;
					info->Cipher = pMacEntry->SecConfig.PairwiseCipher;
					/*pEntry->SecConfig.PairwiseKeyId;*/
					info->KeyIdx = (UINT8)(pKeyInfo->KeyId & 0x0fff);
					os_move_mem(&info->PeerAddr[0], pMacEntry->Addr, MAC_ADDR_LEN);
					os_move_mem(info->Key.Key, pKeyInfo->KeyBuf, (LEN_TK + LEN_TK2));
					HOST_DBG(pAd, "ApClientKeyAdd PTK, wcid=%d, cipher=0x%x, operation=%d\n", info->Wcid, info->Cipher, IS_REMOVEKEY_OPERATION(info));
					HOST_HEXDUMP(pAd, "PTK", (UCHAR *)info->Key.Key, LEN_TK + LEN_TK2);
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
						"ApClientKeyAdd PTK, wcid=%d, cipher=0x%x, operation=%d\n", info->Wcid, info->Cipher, IS_REMOVEKEY_OPERATION(info));
#ifdef DOT11_EHT_BE
					if (pMacEntry->mlo.mlo_en) {
						mlo_install_key(pMacEntry, info, TRUE, TRUE);
						mlo_update_port_secure(pMacEntry, NULL);
					} else
#endif
					WPAInstallKey(pAd, info, TRUE, TRUE);

					os_free_mem(info);
				} else
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
						"struct alloc fail\n");
			} else
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
						"CFG: Set Security. (PAIRWISE) But pEntry NULL\n");
		}
	}
#ifdef DOT11W_PMF_SUPPORT
	else if ((pKeyInfo->KeyType == RT_CMD_80211_KEY_AES_CMAC)
			|| (pKeyInfo->KeyType == RT_CMD_80211_KEY_AES_GMAC256)
			|| (pKeyInfo->KeyType == RT_CMD_80211_KEY_AES_GMAC128)
			|| (pKeyInfo->KeyType == RT_CMD_80211_KEY_AES_CMAC256)) {
		if (pKeyInfo->bPairwise == FALSE && (pKeyInfo->KeyId == 4 || pKeyInfo->KeyId == 5)) {
			struct _ASIC_SEC_INFO *info = NULL;
			NdisCopyMemory(&pMacEntry->SecConfig.PmfCfg.IGTK[pKeyInfo->KeyId - 4][0], pKeyInfo->KeyBuf, pKeyInfo->KeyLen);

			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
				"PMF IGTK pKeyInfo->KeyId=%d\n", pKeyInfo->KeyId);

			switch (pKeyInfo->KeyType) {
			case RT_CMD_80211_KEY_AES_CMAC:
				SET_CIPHER_BIP_CMAC128(wdev->SecConfig.PmfCfg.igtk_cipher);
				break;

			case RT_CMD_80211_KEY_AES_CMAC256:
				SET_CIPHER_BIP_CMAC256(wdev->SecConfig.PmfCfg.igtk_cipher);
				break;

			case RT_CMD_80211_KEY_AES_GMAC128:
				SET_CIPHER_BIP_GMAC128(wdev->SecConfig.PmfCfg.igtk_cipher);
				break;

			case RT_CMD_80211_KEY_AES_GMAC256:
				SET_CIPHER_BIP_GMAC256(wdev->SecConfig.PmfCfg.igtk_cipher);
				break;
			}

			/* Set key material to Asic */
			os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
			if (info) {
				os_zero_mem(info, sizeof(ASIC_SEC_INFO));
				info->Operation = SEC_ASIC_ADD_GROUP_KEY;
				info->Direction = SEC_ASIC_KEY_RX;
				info->BssIndex = pMacEntry->func_tb_idx;
				info->Cipher = pMacEntry->SecConfig.GroupCipher;
				info->Cipher |= wdev->SecConfig.PmfCfg.igtk_cipher;
				info->KeyIdx = pMacEntry->SecConfig.GroupKeyId;
				info->igtk_key_idx = pKeyInfo->KeyId;
				if (!pMacEntry->IsRekeyIGTK) {
					info->Wcid = wdev->bss_info_argument.bmc_wlan_idx;
					info->Wcid2 = 0;
					pMacEntry->IsRekeyIGTK = TRUE;
				} else {
					info->Wcid = wdev->bss_info_argument.bmc_wlan_idx2;
					info->Wcid2 = wdev->bss_info_argument.bmc_wlan_idx;
				}

				os_move_mem(&info->PeerAddr[0], pMacEntry->Addr, MAC_ADDR_LEN);
				os_move_mem(info->Key.Key, &pMacEntry->SecConfig.GTK, (LEN_TK + LEN_TK2));
				os_move_mem(info->IGTK, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
				info->IGTKKeyLen = LEN_BIP128_IGTK;
				if (IS_CIPHER_BIP_CMAC256(pMacEntry->SecConfig.PmfCfg.igtk_cipher)
						|| IS_CIPHER_BIP_GMAC256(pMacEntry->SecConfig.PmfCfg.igtk_cipher))
					info->IGTKKeyLen = LEN_BIP256_IGTK;
				/* Install Shared key */
				os_move_mem(&wdev->SecConfig.PmfCfg.IGTK[pKeyInfo->KeyId - 4][0], pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
				HOST_DBG(pAd, "ApClientKeyAdd IGTK, wcid=%d, cipher=0x%x, operation=%d\n", info->Wcid, info->Cipher, IS_REMOVEKEY_OPERATION(info));
				HOST_HEXDUMP(pAd, "IGTK", (UCHAR *)wdev->SecConfig.PmfCfg.IGTK[pKeyInfo->KeyId - 4], info->IGTKKeyLen);
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
					"ApClientKeyAdd IGTK, wcid=%d, cipher=0x%x, operation=%d\n", info->Wcid, info->Cipher, IS_REMOVEKEY_OPERATION(info));
#ifdef CONFIG_MAP_SUPPORT
				if (IS_MAP_ENABLE(pAd) && tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
					need_send_event = FALSE;
#endif
				if (IS_AKM_WPA_CAPABILITY(pMacEntry->SecConfig.AKMMap)) {
					/* set 802.1x port control */
					tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
					pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
					WifiSysUpdatePortSecur(pAd, pMacEntry, NULL);
				}
				WPAInstallKey(pAd, info, TRUE, TRUE);
				wdev->SecConfig.Handshake.GTKState = REKEY_ESTABLISHED;
				os_free_mem(info);
			} else {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
						"struct alloc fail\n");
			}
		}
#ifdef BCN_PROTECTION_SUPPORT
		else if (pKeyInfo->bPairwise == FALSE && (pKeyInfo->KeyId == 6 || pKeyInfo->KeyId == 7)) {
			struct _ASIC_SEC_INFO *info = NULL;

			NdisCopyMemory(&pMacEntry->SecConfig.bcn_prot_cfg.bigtk[pKeyInfo->KeyId - 6][0], pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
				"PMF BIGTK pKeyInfo->KeyId=%d\n", pKeyInfo->KeyId);

			switch (pKeyInfo->KeyType) {
			case RT_CMD_80211_KEY_AES_CMAC:
				SET_CIPHER_BIP_CMAC128(wdev->SecConfig.PmfCfg.igtk_cipher);
				break;

			case RT_CMD_80211_KEY_AES_CMAC256:
				SET_CIPHER_BIP_CMAC256(wdev->SecConfig.PmfCfg.igtk_cipher);
				break;

			case RT_CMD_80211_KEY_AES_GMAC128:
				SET_CIPHER_BIP_GMAC128(wdev->SecConfig.PmfCfg.igtk_cipher);
				break;

			case RT_CMD_80211_KEY_AES_GMAC256:
				SET_CIPHER_BIP_GMAC256(wdev->SecConfig.PmfCfg.igtk_cipher);
				break;
			}

			/* Set key material to Asic */
			os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
			if (info) {
				UCHAR tx_tsc[20];

				os_zero_mem(info, sizeof(ASIC_SEC_INFO));
				NdisZeroMemory(&tx_tsc, sizeof(tx_tsc));
				info->Operation = SEC_ASIC_ADD_GROUP_KEY;
				info->Direction = SEC_ASIC_KEY_RX;
				info->BssIndex = pMacEntry->func_tb_idx;
				info->Cipher = pMacEntry->SecConfig.GroupCipher;
				info->Cipher |= wdev->SecConfig.PmfCfg.igtk_cipher;
				info->KeyIdx = pMacEntry->SecConfig.GroupKeyId;
				info->bigtk_key_idx = pKeyInfo->KeyId;
				info->bigtk_key_len = LEN_BIP128_IGTK;
				if (IS_CIPHER_BIP_CMAC256(pMacEntry->SecConfig.PmfCfg.igtk_cipher)
						|| IS_CIPHER_BIP_GMAC256(pMacEntry->SecConfig.PmfCfg.igtk_cipher))
					info->bigtk_key_len = LEN_BIP256_IGTK;

				if (!pMacEntry->IsRekeyBIGTK) {
					info->Wcid = wdev->bss_info_argument.bmc_wlan_idx;
					info->Wcid2 = 0;
					pMacEntry->IsRekeyBIGTK = TRUE;
				} else {
					info->Wcid = wdev->bss_info_argument.bmc_wlan_idx2;
					info->Wcid2 = wdev->bss_info_argument.bmc_wlan_idx;
				}
				AsicGetTxTsc(pAd, wdev, TSC_TYPE_BIGTK_PN_MASK, tx_tsc);
				tx_tsc[0]++;
				os_move_mem(&info->tx_tsc, &tx_tsc, LEN_WPA_TSC);
				os_move_mem(&info->PeerAddr[0], pMacEntry->Addr, MAC_ADDR_LEN);
				os_move_mem(info->Key.Key, &pMacEntry->SecConfig.GTK, (LEN_TK + LEN_TK2));
				os_move_mem(info->bigtk, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
				/* Install Shared key */
				NdisZeroMemory(&wdev->bss_info_argument.bcn_prot_cfg, sizeof(wdev->bss_info_argument.bcn_prot_cfg));
				NdisZeroMemory(&wdev->SecConfig.bcn_prot_cfg, sizeof(wdev->SecConfig.bcn_prot_cfg));
				os_move_mem(&wdev->SecConfig.bcn_prot_cfg.bigtk[pKeyInfo->KeyId - 6][0], pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
				HOST_DBG(pAd, "ApClientKeyAdd BIGTK, wcid=%d, cipher=0x%x, operation=%d\n", info->Wcid, info->Cipher, IS_REMOVEKEY_OPERATION(info));
				HOST_HEXDUMP(pAd, "BIGTK", (UCHAR *)wdev->SecConfig.bcn_prot_cfg.bigtk[pKeyInfo->KeyId - 6], LEN_MAX_BIGTK);
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
					"ApClientKeyAdd BIGTK, wcid=%d, cipher=0x%x, operation=%d\n", info->Wcid, info->Cipher, IS_REMOVEKEY_OPERATION(info));
				wdev->SecConfig.bcn_prot_cfg.bigtk_cipher = info->Cipher;
				wdev->SecConfig.bcn_prot_cfg.bigtk_key_idx = pKeyInfo->KeyId;
				wdev->SecConfig.bcn_prot_cfg.bcn_prot_en = 1;
				NdisMoveMemory(&wdev->bss_info_argument.bcn_prot_cfg, &wdev->SecConfig.bcn_prot_cfg, sizeof(wdev->SecConfig.bcn_prot_cfg));
				WPAInstallKey(pAd, info, TRUE, TRUE);
				if (IS_AKM_WPA_CAPABILITY(pMacEntry->SecConfig.AKMMap)) {
					/* set 802.1x port control */
					tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
					pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
					WifiSysUpdatePortSecur(pAd, pMacEntry, NULL);
				}
				wdev->SecConfig.Handshake.GTKState = REKEY_ESTABLISHED;
				os_free_mem(info);
			} else
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
						"struct alloc fail\n");
		}
	}
#endif /* BCN_PROTECTION_SUPPORT */
#endif /* DOT11W_PMF_SUPPORT */
#ifdef MWDS
	if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
		MWDSAPCliPeerEnable(pAd, pApCliEntry, pMacEntry);
#endif
#ifdef CONFIG_STA_SUPPORT
#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
	pApCliEntry = &pAd->StaCfg[tr_entry->func_tb_idx];
	if (IS_MAP_ENABLE(pAd) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) && (need_send_event == TRUE))
		wapp_send_apcli_association_change(WAPP_APCLI_ASSOCIATED,
							pAd, pApCliEntry);
#endif /*WAPP_SUPPORT*/
#if  defined(CONFIG_MAP_SUPPORT)
	if (pMacEntry && (pMacEntry->func_tb_idx < MAX_APCLI_NUM) && tr_entry) {
		if ((tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) && (need_send_event == TRUE)) {
			MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					"APCLIENT MAP_ENABLE\n");
#ifdef A4_CONN
			if (IS_MAP_ENABLE(pAd)) {
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
				if (pMacEntry->mlo.mlo_en)
					map_a4_mlo_peer_enable(pAd, pApCliEntry, pMacEntry, FALSE);
				else
#endif
					map_a4_peer_enable(pAd, pMacEntry, FALSE);
			}
#endif
			map_send_bh_sta_wps_done_event(pAd, pMacEntry, FALSE);
#if defined(CONFIG_MAP_SUPPORT) && defined(DOT11_EHT_BE)
		for (i = 0; i < MLD_LINK_MAX; i++) {
			if (mld && mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
				pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
				pCurwdev = mld->mld_own_links[i].wdev;
				if (pCurEntry && pCurwdev->wdev_ops && pCurwdev->wdev_ops->linkup) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"%s set bh_ready for (%02x:%02x:%02x:%02x:%02x:%02x) pEntry(%p): wcid(%d)!!\n",
					__func__, PRINT_MAC(pCurwdev->if_addr), pCurEntry, pCurEntry->wcid);
					pCurwdev->bh_ready_sent = TRUE;
				}
			}
		}
#endif
		}
	}
#endif /*defined(CONFIG_MAP_SUPPORT) */
#ifdef WH_EVENT_NOTIFIER
	if (pMacEntry && tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
		EventHdlr pEventHdlrHook = NULL;

		pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_EXT_UPLINK_STAT);
		if (pEventHdlrHook && pMacEntry->wdev)
			pEventHdlrHook(pAd, pMacEntry, (UINT32)WHC_UPLINK_STAT_CONNECTED);
	}
#endif /* WH_EVENT_NOTIFIER */
#endif /* CONFIG_STA_SUPPORT */
}
#endif /* APCLI_CFG80211_SUPPORT */
