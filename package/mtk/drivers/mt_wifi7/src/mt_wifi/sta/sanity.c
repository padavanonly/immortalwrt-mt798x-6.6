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
	sanity.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	John Chang  2004-09-01      add WMM support
*/
#include "rt_config.h"
#ifdef DOT11R_FT_SUPPORT
#include "ft/ft.h"
#endif /* DOT11R_FT_SUPPORT */

extern UCHAR CISCO_OUI[];

extern UCHAR WPA_OUI[];
extern UCHAR RSN_OUI[];
extern UCHAR WME_INFO_ELEM[];
extern UCHAR WME_PARM_ELEM[];
extern UCHAR RALINK_OUI[];
extern UCHAR BROADCOM_OUI[];

/*
 *    ==========================================================================
 *    Description:
 *	MLME message sanity check
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
BOOLEAN MlmeStartReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT CHAR Ssid[],
	OUT UCHAR *pSsidLen)
{
	MLME_START_REQ_STRUCT *Info;

	Info = (MLME_START_REQ_STRUCT *) (Msg);

	if (Info->SsidLen > MAX_LEN_OF_SSID) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_SYNC, DBG_LVL_INFO,
			"fail - wrong SSID length\n");
		return FALSE;
	}

	*pSsidLen = Info->SsidLen;
	NdisMoveMemory(Ssid, Info->Ssid, *pSsidLen);
	return TRUE;
}

/*
 *    ==========================================================================
 *    Description:
 *	MLME message sanity check
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *
 *    IRQL = DISPATCH_LEVEL
 *
 *    ==========================================================================
 */
BOOLEAN PeerAssocRspSanity(
	IN struct wifi_dev *wdev,
	IN VOID * pMsg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr2,
	OUT USHORT * pCapabilityInfo,
	OUT USHORT * pStatus,
	OUT USHORT * pAid,
	OUT UCHAR *pNewExtChannelOffset,
	OUT PEDCA_PARM pEdcaParm,
	OUT EXT_CAP_INFO_ELEMENT * pExtCapInfo,
	OUT UCHAR *pCkipFlag,
	OUT IE_LISTS * ie_list)
{
	CHAR IeType, *Ptr;
	PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;
	PEID_STRUCT pEid;
	ULONG Length = 0;
	struct common_ies *cmm_ies = &ie_list->cmm_ies;
	struct frag_ie_info frag_info = {0};
	struct legacy_rate *rate = &cmm_ies->rate;
	UINT8 sup_rate_len;

	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)wdev->sys_handle;
#if defined(DOT11R_FT_SUPPORT)
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif /* DOT11R_FT_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
	unsigned char map_cap = 0;
#ifdef MAP_R2
	UCHAR map_profile;
	UINT16 map_vid;
#endif
#endif
#ifdef DOT11R_FT_SUPPORT
	FT_MIC_CONTENT ft_mic_cont;
#endif /* DOT11R_FT_SUPPORT */
	INT remain_ie_len;

	/* If pkt's length is smaller than basic (802.11 header + Fixed IE) then reject */
	/* 2 is for Tag Number and length (Supported Rate and Bss Membership Selector) */
	if (MsgLen < LENGTH_802_11
				+ LENGTH_802_11_CAP_INFO
				+ LENGTH_802_11_STATUS_CODE
				+ LENGTH_802_11_AID
				+ 2) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			"Fail -> wrong format of ASSOC RSP / REASSOC RSP pkt!\n");
		return FALSE;
	}
	/* record end of frame for IE defragmentation */
	frag_info.frm_end = (UINT8 *)pMsg + MsgLen;

	*pNewExtChannelOffset = 0xff;
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
	Ptr = (CHAR *) pFrame->Octet;
	Length += LENGTH_802_11;
	NdisMoveMemory(pCapabilityInfo, &pFrame->Octet[0], 2);
	Length += 2;
	NdisMoveMemory(pStatus, &pFrame->Octet[2], 2);
	Length += 2;
	*pCkipFlag = 0;
	pEdcaParm->bValid = FALSE;

	if (*pStatus != MLME_SUCCESS)
		return TRUE;

	NdisMoveMemory(pAid, &pFrame->Octet[4], 2);
	Length += 2;
	/* Aid already swaped byte order in RTMPFrameEndianChange() for big endian platform */
	*pAid = (*pAid) & 0x3fff;	/* AID is low 14-bit */
	/* -- get supported rates from payload and advance the pointer */
	IeType = pFrame->Octet[6];
	sup_rate_len = pFrame->Octet[7];
	Length += 2;

	if (sup_rate_len > MAX_LEN_OF_SUPPORTED_RATES) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			"Fail -> malformity SupportedRates IE! (sup_rate_len is too large)\n");
		return FALSE;
	}

	if ((Length + sup_rate_len) > MsgLen) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			"Fail -> malformity SupportedRates IE! (parse IE larger than pkt size)\n");
		return FALSE;
	}
	Length += sup_rate_len;

	if ((IeType != IE_SUPP_RATES) || parse_support_rate_ie(rate, (EID_STRUCT *)&pFrame->Octet[6]) == FALSE) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			"fail - wrong SupportedRates IE\n");
		return FALSE;
	}
#ifdef DOT11R_FT_SUPPORT
	NdisZeroMemory(&ft_mic_cont, sizeof(FT_MIC_CONTENT));
#endif /* DOT11R_FT_SUPPORT */
	/*
	 *   many AP implement proprietary IEs in non-standard order, we'd better
	 *   tolerate mis-ordered IEs to get best compatibility
	 */
	pEid = (PEID_STRUCT) &pFrame->Octet[8 + sup_rate_len];

	/* The length of total Tagged IEs */
	remain_ie_len = MsgLen - (LENGTH_802_11
							+ LENGTH_802_11_CAP_INFO
							+ LENGTH_802_11_STATUS_CODE
							+ LENGTH_802_11_AID
							+ 2 + sup_rate_len); /* for EID and Length */

	/* get variable fields from payload and advance the pointer */
	while ((remain_ie_len >= 2) && ((pEid->Len+2) <= remain_ie_len)) {
		switch (pEid->Eid) {
		case IE_EXT_SUPP_RATES:
			if (parse_support_ext_rate_ie(rate, pEid) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
						"wrong IE_EXT_SUPP_RATES\n");
				return FALSE;
			}

			break;
#ifdef DOT11_N_SUPPORT

		case IE_HT_CAP:
			if (parse_ht_cap_ie(pEid->Len)) {	/* Note: allow extension.!! */
				NdisMoveMemory(&cmm_ies->ht_cap, pEid->Octet, sizeof(HT_CAPABILITY_IE));
				SET_HT_CAPS_EXIST(cmm_ies->ie_exists);
				*(USHORT *)(&cmm_ies->ht_cap.HtCapInfo) = cpu2le16(*(USHORT *)(&cmm_ies->ht_cap.HtCapInfo));
				*(USHORT *)(&cmm_ies->ht_cap.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&cmm_ies->ht_cap.ExtHtCapInfo));
				*(USHORT *)(&cmm_ies->ht_cap.TxBFCap) = le2cpu32(*(USHORT *)(&cmm_ies->ht_cap.TxBFCap));
			} else {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
					"wrong IE_HT_CAP\n");
				return FALSE;
			}
			break;

		case IE_ADD_HT:
			if (parse_ht_info_ie(pEid)) {
				/*
				 *   This IE allows extension, but we can ignore extra bytes beyond our knowledge , so only
				 *   copy first sizeof(ADD_HT_INFO_IE)
				 */
				NdisMoveMemory(&cmm_ies->ht_op, pEid->Octet, sizeof(ADD_HT_INFO_IE));
				SET_HT_OP_EXIST(cmm_ies->ie_exists);
				*(USHORT *)(&cmm_ies->ht_op.AddHtInfo2) = cpu2le16(*(USHORT *)(&cmm_ies->ht_op.AddHtInfo2));
				*(USHORT *)(&cmm_ies->ht_op.AddHtInfo3) = cpu2le16(*(USHORT *)(&cmm_ies->ht_op.AddHtInfo3));
			} else {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
					"wrong IE_ADD_HT\n");
				return FALSE;
			}
			break;

		case IE_SECONDARY_CH_OFFSET:
			if (parse_sec_ch_offset_ie(pEid))
				*pNewExtChannelOffset = pEid->Octet[0];
			else {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
					"wrong IE_SECONDARY_CH_OFFSET\n");
				return FALSE;
			}
			break;
#ifdef DOT11_VHT_AC

		case IE_VHT_CAP:
			if (pEid->Len == sizeof(VHT_CAP_IE)) {
#ifdef CFG_BIG_ENDIAN
				UINT32 tmp_1;
				UINT64 tmp_2;
#endif

				NdisMoveMemory(&cmm_ies->vht_cap, pEid->Octet, sizeof(VHT_CAP_IE));
				SET_VHT_CAPS_EXIST(cmm_ies->ie_exists);

#ifdef CFG_BIG_ENDIAN
				NdisCopyMemory(&tmp_1, &cmm_ies->vht_cap.vht_cap, 4);
				tmp_1 = le2cpu32(tmp_1);
				NdisCopyMemory(&cmm_ies->vht_cap.vht_cap, &tmp_1, 4);

				NdisCopyMemory(&tmp_2, &(cmm_ies->vht_cap.mcs_set), 8);
				tmp_2 = le2cpu64(tmp_2);
				NdisCopyMemory(&(cmm_ies->vht_cap.mcs_set), &tmp_2, 8);
#endif
			} else
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
					"wrong IE_VHT_CAP\n");
			break;

		case IE_VHT_OP:
			if (pEid->Len == sizeof(VHT_OP_IE)) {
#ifdef CFG_BIG_ENDIAN
				UINT16 tmp;
#endif
				NdisMoveMemory(&cmm_ies->vht_op, pEid->Octet, sizeof(VHT_OP_IE));
				SET_VHT_OP_EXIST(cmm_ies->ie_exists);
#ifdef CFG_BIG_ENDIAN
				NdisCopyMemory(&tmp, &cmm_ies->vht_op.basic_mcs_set, sizeof(VHT_MCS_MAP));
				tmp = le2cpu16(tmp);
				NdisCopyMemory(&cmm_ies->vht_op.basic_mcs_set, &tmp, sizeof(VHT_MCS_MAP));
#endif
			} else
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
					"wrong IE_VHT_OP\n");
			break;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

		case IE_VENDOR_SPECIFIC:
#ifdef CONFIG_MAP_SUPPORT
			if (map_check_cap_ie(pEid, &map_cap
#ifdef MAP_R2
				, &map_profile, &map_vid
#endif
				) == TRUE) {
				ie_list->MAP_AttriValue = map_cap;
#ifdef MAP_R2
				ie_list->MAP_ProfileValue = map_profile;
				ie_list->MAP_default_vid = map_vid;
#endif
			}
#endif /* CONFIG_MAP_SUPPORT */

			/* handle WME PARAMTER ELEMENT */
			if (NdisEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24)) {
				PUCHAR ptr;
				int i;
				/* parsing EDCA parameters */
				pEdcaParm->bValid = TRUE;
				pEdcaParm->bQAck = FALSE;	/* pEid->Octet[0] & 0x10; */
				pEdcaParm->bQueueRequest = FALSE;	/* pEid->Octet[0] & 0x20; */
				pEdcaParm->bTxopRequest = FALSE;	/* pEid->Octet[0] & 0x40; */
				pEdcaParm->EdcaUpdateCount =
					pEid->Octet[6] & 0x0f;
				pEdcaParm->bAPSDCapable =
					(pEid->Octet[6] & 0x80) ? 1 : 0;
				ptr = (PUCHAR) &pEid->Octet[8];

				for (i = 0; i < 4; i++) {
					UCHAR aci = (*ptr & 0x60) >> 5;	/* b5~6 is AC INDEX */

					pEdcaParm->bACM[aci] = (((*ptr) & 0x10) == 0x10);	/* b5 is ACM */
					pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;	/* b0~3 is AIFSN */
					pEdcaParm->Cwmin[aci] = *(ptr + 1) & 0x0f;	/* b0~4 is Cwmin */
					pEdcaParm->Cwmax[aci] = *(ptr + 1) >> 4;	/* b5~8 is Cwmax */
					pEdcaParm->Txop[aci] = *(ptr + 2) + 256 * (*(ptr + 3));	/* in unit of 32-us */
					ptr += 4;	/* point to next AC */
				}
			}
#ifdef MWDS
			if (NdisEqualMemory(pEid->Octet, WPS_OUI, 4))
				wdev->wps_ie_flag = TRUE;
			else
				wdev->wps_ie_flag = FALSE;
#endif
			break;
#ifdef DOT11R_FT_SUPPORT

		case IE_FT_MDIE:
			if (pStaCfg && pStaCfg->Dot11RCommInfo.bFtSupport) {
				if (parse_md_ie(pEid) == FALSE) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
							"wrong length of IE_FT_MDIE\n");
					return FALSE;
				}

				/*
				 *   Record the MDIE of (re)association response of
				 *   Initial Mobility Domain Association. It's used in
				 *   FT 4-Way handshaking
				 */
				if (pStaCfg->Dot11RCommInfo.bInMobilityDomain == FALSE) {
					if ((pEid->Len + 2) <= sizeof(pStaCfg->MlmeAux.InitialMDIE))
						NdisMoveMemory(pStaCfg->MlmeAux.InitialMDIE,
									   pEid, pEid->Len + 2);
					else
						MTWF_DBG(pAd,
							DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
							"No enough buffer size for MDIE!\n");
				}

				ft_mic_cont.mdie_ptr = (PUINT8) pEid;
				ft_mic_cont.mdie_len = pEid->Len + 2;
			}

			break;

		case IE_FT_FTIE:
			if (pStaCfg && pStaCfg->Dot11RCommInfo.bFtSupport) {
				pStaCfg->MlmeAux.FtIeInfo.Len = 0;
				if (parse_ft_ie(pEid)) {
					/*
					 *   Record the FTIE of (re)association response of
					 *   Initial Mobility Domain Association. It's used in
					 *   FT 4-Way handshaking
					 */
					if (pStaCfg->Dot11RCommInfo.bInMobilityDomain == FALSE) {
						pStaCfg->MlmeAux.InitialFTIE_Len = pEid->Len + 2;
						if ((pEid->Len + 2) <= sizeof(pStaCfg->MlmeAux.InitialFTIE))
							NdisMoveMemory(pStaCfg->MlmeAux.InitialFTIE, pEid, pEid->Len + 2);
						else
							MTWF_DBG(NULL,
							DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
							"No enough buffer size for FTIE!\n");
					}

					FT_FillFtIeInfo(pEid, &pStaCfg->MlmeAux.FtIeInfo);
					ft_mic_cont.ftie_ptr = (PUINT8) pEid;
					ft_mic_cont.ftie_len = pEid->Len + 2;
				} else {
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
						"pEid->Len(%d) < sizeof(FT_FTIE)\n", pEid->Len);
					return FALSE;
				}
			}

			break;
#endif /* DOT11R_FT_SUPPORT */

		case IE_RSN:
			if (parse_rsn_ie(pEid)) {
#ifdef DOT11R_FT_SUPPORT
				ft_mic_cont.rsnie_ptr = (PUINT8) pEid;
				ft_mic_cont.rsnie_len = pEid->Len + 2;
#endif /* DOT11R_FT_SUPPORT */

			/* Copy whole RSNIE context */
			if ((pEid->Len + 2) <= sizeof(ie_list->RSN_IE))
				NdisMoveMemory(&ie_list->RSN_IE[0], pEid, pEid->Len + 2);
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"No enough buffer size for ie_list->RSN_IE!\n");
				return FALSE;
			}
			ie_list->RSNIE_Len = pEid->Len + 2;
			break;

		case IE_RSNXE:
#ifdef DOT11R_FT_SUPPORT
			ft_mic_cont.rsnxe_ptr = (PUINT8) pEid;
			ft_mic_cont.rsnxe_len = pEid->Len + 2;
#endif /* DOT11R_FT_SUPPORT */
			/* Copy whole RSNXE context */
			if ((pEid->Len + 2) <= sizeof(ie_list->rsnxe_ie))
				NdisMoveMemory(&ie_list->rsnxe_ie, pEid, pEid->Len + 2);
			else
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"No enough buffer size for ie_list->rsnxe_ie!\n");
			ie_list->rsnxe_ie_len = pEid->Len + 2;
			break;


		case IE_EXT_CAPABILITY:
			if (pEid->Len >= 1) {
				UCHAR MaxSize;
				UCHAR MySize = sizeof(EXT_CAP_INFO_ELEMENT);

				MaxSize = min(pEid->Len, MySize);
				NdisMoveMemory(pExtCapInfo, &pEid->Octet[0], MaxSize);
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					"IE_EXT_CAPABILITY!\n");
			}

			break;
		case IE_WLAN_EXTENSION:
		{
			/*parse EXTENSION EID*/
			UCHAR *extension_id = (UCHAR *)pEid + 2;
#ifdef DOT11_HE_AX
			parse_he_assoc_rsp_ies((UINT8 *)pEid, ie_list);
#endif /* DOT11_HE_AX */
#ifdef DOT11_EHT_BE
			if (eht_get_ies((UINT8 *)pEid, &ie_list->cmm_ies, &frag_info) < 0)
				return FALSE;
#endif /* DOT11_EHT_BE */

			switch (*extension_id) {
			case IE_EXTENSION_ID_ECDH:
#ifdef CONFIG_OWE_SUPPORT
			{
				UCHAR *ext_ie_length = (UCHAR *)pEid + 1;
				if (*ext_ie_length <= 3) {
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
					"Ivalid value(*ext_ie_length=%d), drop!\n", *ext_ie_length);
					break;
				}
				os_zero_mem(ie_list->ecdh_ie.public_key, *ext_ie_length-3);
				ie_list->ecdh_ie.ext_ie_id = IE_WLAN_EXTENSION;
				ie_list->ecdh_ie.length = pEid->Len;
				if (pEid->Len <= (1 + 2 + sizeof(ie_list->ecdh_ie.public_key)))
					NdisMoveMemory(&ie_list->ecdh_ie.ext_id_ecdh, pEid->Octet, pEid->Len);
				else
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"No enough buffer size for ecdh_ie.ext_id_ecdh!\n");
			}
#endif /*CONFIG_OWE_SUPPORT*/
				break;
#ifdef DOT11_EHT_BE
			case EID_EXT_EHT_MULTI_LINK:
			case EID_EXT_EHT_CAPS:
			case EID_EXT_EHT_OP:
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"ASSOC RESP extension id: %d\n", *extension_id);
				break;
#endif
			default:
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
					"RESP IE_WLAN_EXTENSION: no handler for extension_id:%d\n", *extension_id);
				break;
			}
		}
			break;
#ifdef DOT11_EHT_BE
		case EID_REDUCED_NEIGHBOR_REPORT:
			if (pEid->Len <= sizeof(ie_list->cmm_ies.rnr_ie)) {
				NdisMoveMemory(&ie_list->cmm_ies.rnr_ie, &pEid->Octet[0], pEid->Len);
				SET_RNR_EXIST(cmm_ies->ie_exists);
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"No enough buffer size for ie_list->cmm_ies.rnr_ie!\n");
			}
			break;
#endif
		default:
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				"ignore unrecognized EID = %d\n", pEid->Eid);
			break;
		}

		if (frag_info.is_frag) {
			remain_ie_len -= frag_info.ie_len_all;
			pEid = (PEID_STRUCT)((UCHAR *)pEid + frag_info.ie_len_all);
			frag_info.is_frag = FALSE;
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"[frag] Jump to next IE: %d, (defrag=%d)\n",
				frag_info.ie_len_all, frag_info.ie_len_defrag);
		} else {
			remain_ie_len -= (2 + pEid->Len);
			pEid = (PEID_STRUCT)((UCHAR *)pEid + 2 + pEid->Len);
		}
	}

#ifdef DOT11R_FT_SUPPORT

	/* Check the MIC during FT */
	if (pStaCfg && pStaCfg->Dot11RCommInfo.bFtSupport &&
		pStaCfg->Dot11RCommInfo.bInMobilityDomain) {
		UINT8 rcvd_mic[16];
		UINT8 ft_mic[16];
		struct _MAC_TABLE_ENTRY *entry = entry_get(pAd, MCAST_WCID);

		NdisZeroMemory(rcvd_mic, 16);
		NdisZeroMemory(ft_mic, 16);
		NdisMoveMemory(rcvd_mic, ft_mic_cont.ftie_ptr + 4, 16);
		FT_CalculateMIC(pAd->CurrentAddress,
						pStaCfg->MlmeAux.Bssid,
						entry->SecConfig.PTK,
						6,
						ft_mic_cont.rsnie_ptr,
						ft_mic_cont.rsnie_len,
						ft_mic_cont.mdie_ptr,
						ft_mic_cont.mdie_len,
						ft_mic_cont.ftie_ptr,
						ft_mic_cont.ftie_len,
						ft_mic_cont.ric_ptr,
						ft_mic_cont.ric_len,
						ft_mic_cont.rsnxe_ptr,
						ft_mic_cont.rsnxe_len,
						ft_mic);

		if (RTMPEqualMemory(ft_mic, rcvd_mic, 16) == FALSE) {
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"MIC is different\n");
			hex_dump("received MIC", rcvd_mic, 16);
			hex_dump("desired  MIC", ft_mic, 16);
			return FALSE;
		}
	}

#endif /* DOT11R_FT_SUPPORT */

	return TRUE;
}


/*
 *    ==========================================================================
 *    Description:
 *
 *	IRQL = DISPATCH_LEVEL
 *
 *    ==========================================================================
 */
BOOLEAN GetTimBit(
	IN CHAR *Ptr,
	IN USHORT Aid,
	OUT UCHAR *TimLen,
	OUT UCHAR *BcastFlag,
	OUT UCHAR *DtimCount,
	OUT UCHAR *DtimPeriod,
	OUT UCHAR *MessageToMe)
{
	UCHAR BitCntl, N1, N2, MyByte, MyBit;
	CHAR *IdxPtr;

	if (*TimLen < 4) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
			"invalid length %d of IE_TIM\n", *TimLen);
		return FALSE;
	}
	IdxPtr = Ptr;
	IdxPtr++;
	*TimLen = *IdxPtr;
	/* get DTIM Count from TIM element */
	IdxPtr++;
	*DtimCount = *IdxPtr;
	/* get DTIM Period from TIM element */
	IdxPtr++;
	*DtimPeriod = *IdxPtr;
	/* get Bitmap Control from TIM element */
	IdxPtr++;
	BitCntl = *IdxPtr;

	if ((*DtimCount == 0) && (BitCntl & 0x01))
		*BcastFlag = TRUE;
	else
		*BcastFlag = FALSE;

	/* Parse Partial Virtual Bitmap from TIM element */
	N1 = BitCntl & 0xfe;	/* N1 is the first bitmap byte# */
	N2 = *TimLen - 4 + N1;	/* N2 is the last bitmap byte# */

	if ((Aid < (N1 << 3)) || (Aid >= ((N2 + 1) << 3)))
		*MessageToMe = FALSE;
	else {
		MyByte = (Aid >> 3) - N1;	/* my byte position in the bitmap byte-stream */
		MyBit = Aid % 16 - ((MyByte & 0x01) ? 8 : 0);
		IdxPtr += (MyByte + 1);

		if (*IdxPtr & (0x01 << MyBit))
			*MessageToMe = TRUE;
		else
			*MessageToMe = FALSE;
	}

	return TRUE;
}
