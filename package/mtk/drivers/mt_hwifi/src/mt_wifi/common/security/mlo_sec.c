/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2019, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/
#ifndef RT_CFG80211_SUPPORT
#include "rt_config.h"

#ifdef DOT11_EHT_BE
u8 *build_mlo_link_kde(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN u8 link_id,
	IN u8 *buf,
	OUT ULONG * offset)
{
	uint8_t len = 13;
	uint8_t rsnxe_len = 0;
	struct _SECURITY_CONFIG *sae_group;

	sae_group = &entry->wdev->SecConfig;

	buf[len] = sae_group->RSNE_EID[1][0];
	len += 1;
	buf[len] = sae_group->BCN_RSNE_Len[1];
	len += 1;

	NdisMoveMemory(buf + len, &sae_group->BCN_RSNE_Content[1][0], sae_group->BCN_RSNE_Len[1]);
	len += sae_group->BCN_RSNE_Len[1];

	rsnxe_len = build_rsnxe_ie(entry->wdev, sae_group, buf + len);
	len += rsnxe_len;

	WPA_ConstructKdeHdr(KDE_MLO_LINK, len - 6, buf);

	/* Link information */
	*(buf + 6) = link_id;

	if (sae_group->BCN_RSNE_Len[1])
		*(buf + 6) |= (1 << 4);

	if (rsnxe_len)
		*(buf + 6) |= (1 << 5);

	NdisMoveMemory(buf + 7, entry->wdev->if_addr, MAC_ADDR_LEN);

	*offset += len;

	return buf + len;
}

int parse_mlo_link_kde(
	IN u8 *buf,
	IN u32 buf_len,
	OUT u8 *link_id,
	OUT u8 **link_mac,
	OUT u8 **rsne,
	OUT u8 **rsnxe)
{
	EID_STRUCT *eid;
	u8 is_rsne_existed;
	u8 is_rsnxe_existed;

	if (buf_len < 7) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_DEBUG,
			"wrong kde len %d\n", buf_len);
		return -1;
	}

	is_rsne_existed = (buf[0] & 0x10) ? TRUE : FALSE;
	is_rsnxe_existed = (buf[0] & 0x20) ? TRUE : FALSE;

	if (link_id)
		*link_id = buf[0] & 0xf;

	if (link_mac)
		*link_mac = buf + 1;

	if (rsne) {
		if (!is_rsne_existed)
			*rsne = NULL;
		else
			*rsne = buf + 1 + MAC_ADDR_LEN;
	}

	if (rsnxe) {
		if (!is_rsnxe_existed)
			*rsnxe = NULL;
		else if (is_rsne_existed) {
			eid = (EID_STRUCT *)buf + 1 + MAC_ADDR_LEN;
			*rsnxe = buf + 1 + MAC_ADDR_LEN + eid->Len + 2;
		} else
			*rsnxe = buf + 1 + MAC_ADDR_LEN;
	}

	return 0;
}

u8 *build_mlo_gtk_kde(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN u8 link_id,
	IN u8 *buf,
	OUT ULONG * offset)
{
	UINT32 pn_type_mask = TSC_TYPE_GTK_PN_MASK;
	u8 tx_tsc[LEN_WPA_TSC] = {0};
	uint8_t gtk_len = 0;
	struct _SECURITY_CONFIG *sae_group;

	sae_group = &entry->wdev->SecConfig;

	/* Decide the GTK length */
	if (IS_CIPHER_CCMP128(sae_group->GroupCipher))
		gtk_len = LEN_CCMP128_TK;
	else if (IS_CIPHER_CCMP256(sae_group->GroupCipher))
		gtk_len = LEN_CCMP256_TK;
	else if (IS_CIPHER_GCMP128(sae_group->GroupCipher))
		gtk_len = LEN_GCMP128_TK;
	else if (IS_CIPHER_GCMP256(sae_group->GroupCipher))
		gtk_len = LEN_GCMP256_TK;

	WPA_ConstructKdeHdr(KDE_MLO_GTK, gtk_len + 7, buf);

	*(buf + 6) = (sae_group->GroupKeyId & 0x03) | (link_id << 4);

	/* Get Group TxTsc form Asic*/
	AsicGetTxTsc((struct _RTMP_ADAPTER *)entry->pAd, entry->wdev, pn_type_mask, tx_tsc);

	NdisMoveMemory(buf + 7, tx_tsc, LEN_WPA_TSC);
	NdisMoveMemory(buf + 13, sae_group->GTK, gtk_len);
	*offset += 13 + gtk_len;

	return buf + 13 + gtk_len;
}

#ifdef DOT11W_PMF_SUPPORT
u8 *build_mlo_igtk_kde(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN u8 link_id,
	IN u8 *buf,
	OUT ULONG * offset)
{
	uint8_t igtk_len = LEN_BIP128_IGTK;
	struct _SECURITY_CONFIG *sae_group;
	struct _PMF_CFG *pmf_cfg;
	uint8_t idx = 0;

	sae_group = &entry->wdev->SecConfig;
	pmf_cfg = &sae_group->PmfCfg;
	idx = (pmf_cfg->IGTK_KeyIdx == 4) ? 0 : 1;

	/* Decide the IGTK length */
	if (IS_CIPHER_BIP_CMAC128(pmf_cfg->igtk_cipher)
		|| IS_CIPHER_BIP_GMAC128(pmf_cfg->igtk_cipher))
		igtk_len = LEN_BIP128_IGTK;
	else if (IS_CIPHER_BIP_CMAC256(pmf_cfg->igtk_cipher)
		|| IS_CIPHER_BIP_GMAC256(pmf_cfg->igtk_cipher))
		igtk_len = LEN_BIP256_IGTK;

	WPA_ConstructKdeHdr(KDE_MLO_IGTK, igtk_len + 9, buf);

	*(buf + 6) = pmf_cfg->IGTK_KeyIdx;
	NdisMoveMemory(buf + 8, &pmf_cfg->IPN[idx][0], LEN_WPA_TSC);
	*(buf + 14) = (link_id << 4);

	NdisMoveMemory(buf + 15, pmf_cfg->IGTK, igtk_len);
	*offset += 15 + igtk_len;

	return buf + 15 + igtk_len;
}
#endif

#ifdef BCN_PROTECTION_SUPPORT
u8 *build_mlo_bigtk_kde(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN u8 link_id,
	IN u8 *buf,
	OUT ULONG * offset)
{
	uint8_t bigtk_len = LEN_BIP128_IGTK;
	struct _SECURITY_CONFIG *sae_group;
	struct bcn_protection_cfg *bcn_prot_cfg = NULL;
	uint8_t idx = 0;
	UINT32 pn_type_mask = TSC_TYPE_BIGTK_PN_MASK;
	u8 tx_tsc[LEN_WPA_TSC] = {0};

	sae_group = &entry->wdev->SecConfig;
	bcn_prot_cfg = &sae_group->bcn_prot_cfg;
	idx = (bcn_prot_cfg->bigtk_key_idx == 6) ? 0 : 1;

	/* Decide the IGTK length */
	if (IS_CIPHER_BIP_CMAC128(bcn_prot_cfg->bigtk_cipher)
		|| IS_CIPHER_BIP_GMAC128(bcn_prot_cfg->bigtk_cipher))
		bigtk_len = LEN_BIP128_IGTK;
	else if (IS_CIPHER_BIP_CMAC256(bcn_prot_cfg->bigtk_cipher)
		|| IS_CIPHER_BIP_GMAC256(bcn_prot_cfg->bigtk_cipher))
		bigtk_len = LEN_BIP256_IGTK;

	WPA_ConstructKdeHdr(KDE_MLO_BIGTK, bigtk_len + 9, buf);

	*(buf + 6) = bcn_prot_cfg->bigtk_key_idx;
	/* Get Group TxTsc form Asic*/
	AsicGetTxTsc((struct _RTMP_ADAPTER *)entry->pAd, entry->wdev, pn_type_mask, tx_tsc);
	NdisMoveMemory(buf + 8, tx_tsc, LEN_WPA_TSC);
	*(buf + 14) = (link_id << 4);

	NdisMoveMemory(buf + 15, bcn_prot_cfg->bigtk, bigtk_len);
	*offset += 15 + bigtk_len;

	return (buf + 15 + bigtk_len);
}
#endif
#endif /* RT_CFG80211_SUPPORT */
#endif
