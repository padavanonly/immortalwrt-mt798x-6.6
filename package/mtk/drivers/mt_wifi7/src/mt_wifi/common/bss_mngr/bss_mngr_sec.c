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
 ***************************************************************************
*/

#include "rt_config.h"
#include "bss_mngr.h"

#ifdef DOT11_EHT_BE
#ifndef RT_CFG80211_SUPPORT
int build_mlo_kde(
	IN u8 data_type,
	IN struct _MAC_TABLE_ENTRY *entry,
	IN u8 is_all_link,
	IN u8 *buf,
	OUT ULONG * offset)
{
	u8 i = 0, link_index = 0;
	struct _MAC_TABLE_ENTRY *entry_ptr;
	u8 *pos = buf;
	struct query_mld_ap_basic basic = {0};
	struct mld_entry_t *mld_entry = NULL;
	struct _MAC_TABLE_ENTRY *tmp_entry_ptr[MLD_LINK_MAX];

	mt_rcu_read_lock();
	mld_entry = get_mld_entry_by_mac(entry->mlo.mld_addr);
	if (!mld_entry) {
		MTWF_DBG(entry->pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
			"mld_entry=NULL\n");
		mt_rcu_read_unlock();
		return -1;
	}
	for (link_index = 0; link_index < MLD_LINK_MAX; link_index++)
		tmp_entry_ptr[link_index] = mld_entry->link_entry[link_index];

	mt_rcu_read_unlock();

	if (is_all_link)
		entry_ptr = tmp_entry_ptr[i];
	else
		entry_ptr = entry;

	while (1) {
		if (!entry_ptr || !entry_ptr->wdev)
			goto next;

		if (entry_ptr->wdev->wdev_type == WDEV_TYPE_AP)
			bss_mngr_query_mld_ap_basic(entry_ptr->wdev, &basic);
		else if (entry_ptr->wdev->wdev_type == WDEV_TYPE_STA &&
			get_sta_mld_link_by_idx(entry_ptr->wdev->mld_dev, i) != NULL) {
			basic.link_id = i;
		}


		switch (data_type) {
		case KDE_MLO_GTK:
			pos = build_mlo_gtk_kde(entry_ptr, basic.link_id, pos, offset);
			break;
#ifdef DOT11W_PMF_SUPPORT
		case KDE_MLO_IGTK:
			pos = build_mlo_igtk_kde(entry_ptr, basic.link_id, pos, offset);
			break;
#endif
#ifdef BCN_PROTECTION_SUPPORT
		case KDE_MLO_BIGTK:
			pos = build_mlo_bigtk_kde(entry_ptr, basic.link_id, pos, offset);
			break;
#endif
		case KDE_MLO_LINK:
			pos = build_mlo_link_kde(entry_ptr, basic.link_id, pos, offset);
			break;
		default:
			return -1;
		}
next:
		if (!is_all_link)
			break;

		i++;
		if (i == MLD_LINK_MAX)
			break;
		entry_ptr = tmp_entry_ptr[i];
	};

	return 0;
}

int parse_all_mlo_link_kde(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN u8 **mlo_kde,
	IN u8 mlo_kde_num)
{
	u8 i, link_index = 0;
	KDE_HDR *kde_ptr;
	u8 link_id;
	u8 *link_mac_addr = NULL;
	int ret = 0;
	struct _MAC_TABLE_ENTRY *entry_ptr = NULL;
	struct mld_entry_t *mld_entry = NULL;
	struct _MAC_TABLE_ENTRY *tmp_entry_ptr[MLD_LINK_MAX];

	mt_rcu_read_lock();
	mld_entry = get_mld_entry_by_mac(entry->mlo.mld_addr);
	if (!mld_entry) {
		MTWF_DBG(entry->pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
			"mld_entry=NULL\n");
		mt_rcu_read_unlock();
		return -1;
	}
	for (link_index = 0; link_index < MLD_LINK_MAX; link_index++)
		tmp_entry_ptr[link_index] = mld_entry->link_entry[link_index];

	mt_rcu_read_unlock();

	for (i = 0; i < mlo_kde_num; i++) {
		kde_ptr = (KDE_HDR *) mlo_kde[i];

		/* todo: rsne/rsnxe sanity */
		ret = parse_mlo_link_kde(kde_ptr->octet,
			kde_ptr->Len - 4, &link_id, &link_mac_addr, NULL, NULL);

		if (ret)
			return ret;

		if (link_id >= MLD_LINK_MAX) {
			MTWF_DBG(entry->pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
					"wrong link id %d\n", link_id);
			return -1;
		}

		entry_ptr = tmp_entry_ptr[link_id];

		if (!entry_ptr ||
			!NdisEqualMemory(entry_ptr->Addr, link_mac_addr, MAC_ADDR_LEN)) {
			MTWF_DBG(entry->pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
					"wrong link mac addr %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(link_mac_addr));
			return -1;
		}
	}
	return 0;
}

int parse_all_mlo_gtk_kde(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN u8 **mlo_kde,
	IN u8 mlo_kde_num)
{
	u8 i;
	KDE_HDR *kde_ptr;
	u8 key_id = 0;
	u8 tx = 0;
	u8 link_id = 0;
	u32 gtk_len = 0;
	struct _MAC_TABLE_ENTRY *entry_ptr = NULL;
	struct MLO_GTK_KDE *pmlo_gtk_kde = NULL;
	PSECURITY_CONFIG sae_config = NULL;
	struct mld_entry_t *mld_entry = NULL;


	for (i = 0; i < mlo_kde_num; i++) {
		kde_ptr = (KDE_HDR *) mlo_kde[i];
		pmlo_gtk_kde = (struct MLO_GTK_KDE *) kde_ptr->octet;
		link_id = (u8)pmlo_gtk_kde->link_id;
		key_id = pmlo_gtk_kde->kid;/*key id*/
		tx = pmlo_gtk_kde->tx;/*tx*/
		link_id = pmlo_gtk_kde->link_id;/*link_id*/
		if (link_id >= MLD_LINK_MAX) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
			"(%s)[%d]: ERROR, link_id: %d > MLD_LINK_MAX(%d).\n",
			__func__, __LINE__, link_id, MLD_LINK_MAX);
			return -1;
		}

		mt_rcu_read_lock();
		mld_entry = get_mld_entry_by_mac(entry->mlo.mld_addr);
		if (!mld_entry) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
			"ERROR, mld_entry=NULL\n");
			mt_rcu_read_unlock();
			return -1;
		}
		entry_ptr = mld_entry->link_entry[link_id];
		mt_rcu_read_unlock();

		if (!entry_ptr || !entry_ptr->wdev) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
			"ERROR, can't find mld info of the link_entry or wdev is null.\n");
			return -1;
		}
		/*todo: pn*/

		gtk_len = kde_ptr->Len - 11;/*gtk*/
		if ((gtk_len < LEN_WEP40) || (gtk_len > LEN_MAX_GTK)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
				"ERROR: GTK Key length is invalid (%u)\n", gtk_len);
			return -1;
		}
		/*update sec info to sae_config*/
		sae_config = &entry_ptr->SecConfig;
		os_move_mem(sae_config, &entry->SecConfig, sizeof(SECURITY_CONFIG));

		sae_config->GroupKeyId = key_id;
		NdisZeroMemory(sae_config->GTK, LEN_MAX_GTK);
		NdisMoveMemory(sae_config->GTK, pmlo_gtk_kde->gtk, gtk_len);

		MTWF_DBG(entry_ptr->pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_INFO,
			""MACSTR"linkid: %d, key_id: %d, gtk_len: %u.\n",
			MAC2STR(entry_ptr->Addr), link_id, key_id, gtk_len);
		hex_dump("MLO_GTK_KDE", (UCHAR *)kde_ptr, kde_ptr->Len+2);
	}
	return 0;
}
#endif /* RT_CFG80211_SUPPORT */
int mlo_install_key(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN struct _ASIC_SEC_INFO *pInfo,
	IN BOOLEAN bAE,
	IN BOOLEAN is_install)
{
	struct _SEC_KEY_INFO *pKey = &pInfo->Key;
	struct _MAC_TABLE_ENTRY *entry_ptr;
	struct mld_entry_t *mld_entry = NULL;
	struct _MAC_TABLE_ENTRY *tmp_entry_ptr[MLD_LINK_MAX];
	u8 i = 0, link_index = 0;

	mt_rcu_read_lock();
	mld_entry = get_mld_entry_by_mac(entry->mlo.mld_addr);

	if (!mld_entry) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
		"ERROR, mld_entry=NULL\n");
		mt_rcu_read_unlock();
		return -1;
	}

	for (link_index = 0; link_index < MLD_LINK_MAX; link_index++)
		tmp_entry_ptr[link_index] = mld_entry->link_entry[link_index];

	mt_rcu_read_unlock();

	if (IS_CIPHER_TKIP(pInfo->Cipher))
		pKey->KeyLen = LEN_TKIP_TK;
	else if (IS_CIPHER_CCMP128(pInfo->Cipher))
		pKey->KeyLen = LEN_CCMP128_TK;
	else if (IS_CIPHER_CCMP256(pInfo->Cipher))
		pKey->KeyLen = LEN_CCMP256_TK;
	else if (IS_CIPHER_GCMP128(pInfo->Cipher))
		pKey->KeyLen = LEN_GCMP128_TK;
	else if (IS_CIPHER_GCMP256(pInfo->Cipher))
		pKey->KeyLen = LEN_GCMP256_TK;

	if (pInfo->IGTKKeyLen != 0) {
		if (IS_CIPHER_BIP_GMAC128(pInfo->Cipher)
			|| IS_CIPHER_BIP_GMAC256(pInfo->Cipher)
			|| IS_CIPHER_BIP_CMAC256(pInfo->Cipher))
			; /* do nothing due to hw not support*/
		else if (pKey->KeyLen + pInfo->IGTKKeyLen > sizeof(pKey->Key))
			; /* do nothing due to fw not support*/
		else if (IS_CIPHER_CCMP128(pInfo->Cipher) ||
			IS_CIPHER_CCMP256(pInfo->Cipher) ||
			IS_CIPHER_GCMP128(pInfo->Cipher) ||
			IS_CIPHER_GCMP256(pInfo->Cipher)) {
			os_move_mem(&pKey->Key[pKey->KeyLen], pInfo->IGTK, pInfo->IGTKKeyLen);
			pKey->KeyLen += pInfo->IGTKKeyLen;
		}
	}

	if (IS_CIPHER_TKIP(pInfo->Cipher)) {
		if (bAE) {
			os_move_mem(pKey->TxMic, &pKey->Key[LEN_TK], LEN_TKIP_MIC);
			os_move_mem(pKey->RxMic, &pKey->Key[LEN_TK + 8], LEN_TKIP_MIC);
		} else {
			os_move_mem(pKey->TxMic, &pKey->Key[LEN_TK + 8], LEN_TKIP_MIC);
			os_move_mem(pKey->RxMic, &pKey->Key[LEN_TK], LEN_TKIP_MIC);
		}
	}

	if (!is_install)
		return 0;

	do {
		entry_ptr = tmp_entry_ptr[i++];
		if (!entry_ptr)
			continue;

		pInfo->Wcid = entry_ptr->wcid;
		pInfo->BssIndex = entry_ptr->func_tb_idx;
		os_move_mem(pInfo->PeerAddr, entry_ptr->Addr, MAC_ADDR_LEN);
		hex_dump("PTK:", pInfo->Key.Key, 32);
		MTWF_DBG(entry_ptr->pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_INFO,
					"install key for wcid = %d\n",
					entry_ptr->wcid);

		HW_ADDREMOVE_KEYTABLE(entry_ptr->pAd, pInfo);
		RtmpusecDelay(5000);/*delay 5ms for updating key*/
	} while (i < MLD_LINK_MAX);

	return 0;
}

int mlo_update_port_secure(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN struct _ASIC_SEC_INFO *asic_sec_info)
{
	struct _MAC_TABLE_ENTRY *entry_ptr;
	u8 i = 0, link_index = 0;
	struct _STA_TR_ENTRY *tr_entry;
	struct _RTMP_ADAPTER *ad = entry->pAd;
	struct mld_entry_t *mld_entry = NULL;
	struct _MAC_TABLE_ENTRY *tmp_entry_ptr[MLD_LINK_MAX];

	mt_rcu_read_lock();
	mld_entry = get_mld_entry_by_mac(entry->mlo.mld_addr);
	if (!mld_entry) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
			"ERROR, mld_entry=NULL\n");
		mt_rcu_read_unlock();
		return -1;
	}

	for (link_index = 0; link_index < MLD_LINK_MAX; link_index++)
		tmp_entry_ptr[link_index] = mld_entry->link_entry[link_index];
	mt_rcu_read_unlock();

	do {
		entry_ptr = tmp_entry_ptr[i++];
		if (!entry_ptr)
			continue;
		WifiSysUpdatePortSecur(entry_ptr->pAd, entry_ptr, asic_sec_info);

		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_INFO,
					"portsecure starec update for wcid = %d\n",
					entry_ptr->wcid);
	} while (i < MLD_LINK_MAX);

	RtmpusecDelay(5000);/*delay 5ms for wpa Update Portl*/

	i = 0;
	do {
		entry_ptr = tmp_entry_ptr[i++];
		if (!entry_ptr)
			continue;
		ad = entry_ptr->pAd;
		tr_entry = tr_entry_get(ad, entry_ptr->wcid);
		tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
	} while (i < MLD_LINK_MAX);

	return 0;
}


#ifndef RT_CFG80211_SUPPORT
int mlo_install_group_key(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN BOOLEAN bAE,
	IN BOOLEAN is_install)
{
	struct _MAC_TABLE_ENTRY *entry_ptr = NULL;
	ASIC_SEC_INFO info = {0};
	u8 idx = 0;
	struct mld_entry_t *mld_entry = NULL;
	struct _MAC_TABLE_ENTRY *tmp_entry_ptr[MLD_LINK_MAX];

	if (!entry) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
					"entry is null.\n");
		return -1;
	}

	mt_rcu_read_lock();
	mld_entry = get_mld_entry_by_mac(entry->mlo.mld_addr);
	if (!mld_entry) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
		"ERROR, mld_entry=NULL\n");
		mt_rcu_read_unlock();
		return -1;
	}

	for (idx = 0; idx < MLD_LINK_MAX; idx++)
		tmp_entry_ptr[idx] = mld_entry->link_entry[idx];
	mt_rcu_read_unlock();

	for (idx = 0; idx < MLD_LINK_MAX; idx++) {
		entry_ptr = tmp_entry_ptr[idx];
		if (!entry_ptr)
			continue;
		os_zero_mem(&info, sizeof(info));
		info.Operation = SEC_ASIC_ADD_GROUP_KEY;
		info.Direction = SEC_ASIC_KEY_RX;
		info.Wcid = entry_ptr->wdev->bss_info_argument.bmc_wlan_idx;
		info.BssIndex = entry_ptr->wdev->func_idx;
		info.Cipher = entry_ptr->SecConfig.GroupCipher;
		info.KeyIdx = entry_ptr->SecConfig.GroupKeyId;
		os_move_mem(&info.PeerAddr[0], entry_ptr->Addr, MAC_ADDR_LEN);
		os_move_mem(info.Key.Key, entry_ptr->SecConfig.GTK, LEN_MAX_GTK);
		/*TODO: IGTK/BITGK*/

		hex_dump("GTK", entry_ptr->SecConfig.GTK, LEN_MAX_GTK);
		/* Prevent the GTK reinstall key attack */
		if ((entry_ptr->SecConfig.LastGroupKeyId != entry_ptr->SecConfig.GroupKeyId) ||
			!NdisEqualMemory(entry_ptr->SecConfig.LastGTK, entry_ptr->SecConfig.GTK, LEN_MAX_GTK)) {
			WPAInstallKey(entry_ptr->pAd, &info, bAE, is_install);
			entry_ptr->SecConfig.LastGroupKeyId = entry_ptr->SecConfig.GroupKeyId;
			os_move_mem(entry_ptr->SecConfig.LastGTK, entry_ptr->SecConfig.GTK, LEN_MAX_GTK);
			entry_ptr->AllowUpdateRSC = TRUE;
			MTWF_DBG(entry_ptr->pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_INFO,
				"GTK install done,wcid(%d),"MACSTR"\n",
				entry_ptr->wcid, MAC2STR(entry_ptr->Addr));
		} else {
			MTWF_DBG(entry_ptr->pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
				"GTK skip install,wcid(%d)"MACSTR"\n",
				entry_ptr->wcid, MAC2STR(entry_ptr->Addr));
		}
	}

	return 0;
}
#endif /* RT_CFG80211_SUPPORT */
#endif
