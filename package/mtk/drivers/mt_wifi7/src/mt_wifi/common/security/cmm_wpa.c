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
	wpa.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Jan	Lee		03-07-22		Initial
	Paul Lin	03-11-28		Modify for supplicant
*/
#include "rt_config.h"

/* --------------------EddySEC Start---------------- */

UCHAR SES_OUI[] = {0x00, 0x90, 0x4c};

/* WPA OUI*/
UCHAR OUI_WPA[3]                           = {0x00, 0x50, 0xF2};
UCHAR OUI_WPA_NONE_AKM[4]     = {0x00, 0x50, 0xF2, 0x00};
UCHAR OUI_WPA_VERSION[4]          = {0x00, 0x50, 0xF2, 0x01};
UCHAR OUI_WPA_WEP40[4]             = {0x00, 0x50, 0xF2, 0x01};
UCHAR OUI_WPA_TKIP[4]                  = {0x00, 0x50, 0xF2, 0x02};
UCHAR OUI_WPA_CCMP[4]               = {0x00, 0x50, 0xF2, 0x04};
UCHAR OUI_WPA_WEP104[4]           = {0x00, 0x50, 0xF2, 0x05};
UCHAR OUI_WPA_8021X_AKM[4]     = {0x00, 0x50, 0xF2, 0x01};
UCHAR OUI_WPA_PSK_AKM[4]         = {0x00, 0x50, 0xF2, 0x02};

/* WPA2 OUI*/
UCHAR OUI_WPA2[3]					   = {0x00, 0x0F, 0xAC};
UCHAR OUI_WPA2_CIPHER[3]                        = {0x00, 0x0F, 0xAC};
UCHAR OUI_WPA2_CIPHER_WEP40[4]          = {0x00, 0x0F, 0xAC, 0x01};
UCHAR OUI_WPA2_CIPHER_TKIP[4]               = {0x00, 0x0F, 0xAC, 0x02};
UCHAR OUI_WPA2_CIPHER_CCMP128[4]       = {0x00, 0x0F, 0xAC, 0x04};
UCHAR OUI_WPA2_CIPHER_WEP104[4]         = {0x00, 0x0F, 0xAC, 0x05};
UCHAR OUI_WPA2_CIPHER_BIPCMAC128[4]  = {0x00, 0x0F, 0xAC, 0x06};
UCHAR OUI_WPA2_CIPHER_GCMP128[4]       = {0x00, 0x0F, 0xAC, 0x08};
UCHAR OUI_WPA2_CIPHER_GCMP256[4]       = {0x00, 0x0F, 0xAC, 0x09};
UCHAR OUI_WPA2_CIPHER_CCMP256[4]       = {0x00, 0x0F, 0xAC, 0x0A};
UCHAR OUI_WPA2_CIPHER_BIPGMAC128[4] = {0x00, 0x0F, 0xAC, 0x0B};
UCHAR OUI_WPA2_CIPHER_BIPGMAC256[4] = {0x00, 0x0F, 0xAC, 0x0C};
UCHAR OUI_WPA2_CIPHER_BIPCMAC256[4]  = {0x00, 0x0F, 0xAC, 0x0D};

UCHAR OUI_WPA2_AKM_8021X[4]                      = {0x00, 0x0F, 0xAC, 0x01};
UCHAR OUI_WPA2_AKM_PSK[4]                          = {0x00, 0x0F, 0xAC, 0x02};
UCHAR OUI_WPA2_AKM_FT_8021X[4]                = {0x00, 0x0F, 0xAC, 0x03};
UCHAR OUI_WPA2_AKM_FT_PSK[4]                    = {0x00, 0x0F, 0xAC, 0x04};
UCHAR OUI_WPA2_AKM_8021X_SHA256[4]       = {0x00, 0x0F, 0xAC, 0x05};
UCHAR OUI_WPA2_AKM_PSK_SHA256[4]           = {0x00, 0x0F, 0xAC, 0x06};
UCHAR OUI_WPA2_AKM_TDLS[4]                        = {0x00, 0x0F, 0xAC, 0x07};
UCHAR OUI_WPA2_AKM_SAE_SHA256[4]           = {0x00, 0x0F, 0xAC, 0x08};
UCHAR OUI_WPA2_AKM_FT_SAE_SHA256[4]     = {0x00, 0x0F, 0xAC, 0x09};
UCHAR OUI_WPA2_AKM_SUITEB_SHA256[4]      = {0x00, 0x0F, 0xAC, 0x0B};
UCHAR OUI_WPA2_AKM_SUITEB_SHA384[4]      = {0x00, 0x0F, 0xAC, 0x0C};
UCHAR OUI_WPA2_AKM_FT_8021X_SHA384[4]  = {0x00, 0x0F, 0xAC, 0x0D};
#ifdef OCE_FILS_SUPPORT
UCHAR OUI_WPA2_AKM_FILS_SHA256[4]       = {0x00, 0x0F, 0xAC, 0x0E};
UCHAR OUI_WPA2_AKM_FILS_SHA384[4]       = {0x00, 0x0F, 0xAC, 0x0F};
#endif /* OCE_FILS_SUPPORT */
UCHAR OUI_WPA2_AKM_OWE[4]  = {0x00, 0x0F, 0xAC, 0x12/*d'18*/};
UCHAR OUI_WPA2_AKM_SAE_EXT[4]      = {0x00, 0x0F, 0xAC, 0x18/*d'24*/};
UCHAR OUI_WPA2_AKM_FT_SAE_EXT[4]      = {0x00, 0x0F, 0xAC, 0x19/*d'25*/};

/* --------------------EddySEC END---------------- */


#ifdef CONFIG_HOTSPOT_R3
UCHAR    OUI_OSEN_AKM[4] = {0x50, 0x6F, 0x9A, 0x01};
#endif
#ifdef CONFIG_HOTSPOT_R2
UCHAR	OSEN_IE[] = {0x50, 0x6f, 0x9a, 0x12, 0x00, 0x0f, 0xac, 0x07, 0x01, 0x00, 0x00, 0x0f, 0xac, 0x04, 0x01, 0x00, 0x50, 0x6f, 0x9a, 0x01, 0x00, 0x00};
UCHAR	OSEN_IELEN = sizeof(OSEN_IE);
#endif

#ifndef RT_CFG80211_SUPPORT

BUILD_TIMER_FUNCTION(WPAStartFor4WayExec);
BUILD_TIMER_FUNCTION(WPAStartFor2WayExec);
BUILD_TIMER_FUNCTION(WPAHandshakeMsgRetryExec);

static VOID WpaEAPPacketAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID WpaEAPOLASFAlertAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID WpaEAPOLLogoffAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID WpaEAPOLStartAction(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID WpaEAPOLKeyAction(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID WpaEAPOLRetryAction(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID wpa_2way_action(
	IN struct _RTMP_ADAPTER *ad,
	IN MLME_QUEUE_ELEM * Elem);



/*
    ==========================================================================
    Description:
	association state machine init, including state transition and timer init
    Parameters:
	S - pointer to the association state machine
    ==========================================================================
 */
VOID WpaStateMachineInit(
	IN  PRTMP_ADAPTER   pAd,
	IN  STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(S, (STATE_MACHINE_FUNC *)Trans, MAX_WPA_PTK_STATE, MAX_WPA_MSG, (STATE_MACHINE_FUNC)Drop, WPA_PTK, WPA_MACHINE_BASE);
	StateMachineSetAction(S, WPA_PTK, MT2_EAPPacket, (STATE_MACHINE_FUNC)WpaEAPPacketAction);
	StateMachineSetAction(S, WPA_PTK, MT2_EAPOLStart, (STATE_MACHINE_FUNC)WpaEAPOLStartAction);
	StateMachineSetAction(S, WPA_PTK, MT2_EAPOLLogoff, (STATE_MACHINE_FUNC)WpaEAPOLLogoffAction);
	StateMachineSetAction(S, WPA_PTK, MT2_EAPOLKey, (STATE_MACHINE_FUNC)WpaEAPOLKeyAction);
	StateMachineSetAction(S, WPA_PTK, MT2_EAPOLASFAlert, (STATE_MACHINE_FUNC)WpaEAPOLASFAlertAction);
	StateMachineSetAction(S, WPA_PTK, MT2_EAPOLRetry, (STATE_MACHINE_FUNC)WpaEAPOLRetryAction);
	StateMachineSetAction(S, WPA_PTK, MT2_EAPOL2way, (STATE_MACHINE_FUNC)wpa_2way_action);
}

/*
    ==========================================================================
    Description:
	this is state machine function.
	When receiving EAP packets which is  for 802.1x authentication use.
	Not use in PSK case
    Return:
    ==========================================================================
*/
VOID WpaEAPPacketAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
}

VOID WpaEAPOLASFAlertAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
}

VOID WpaEAPOLLogoffAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
}
#endif /* RT_CFG80211_SUPPORT */

UCHAR sec_get_kek_len(struct _SECURITY_CONFIG *pSecConfig)
{
#ifdef OCE_FILS_SUPPORT
	if (IS_AKM_FILS_SHA256(pSecConfig->AKMMap)) {
		return 32;
	} else if (IS_AKM_FILS_SHA384(pSecConfig->AKMMap)) {
		return 64;
	}
#endif /* OCE_FILS_SUPPORT */

	return 0;
}

UCHAR sec_get_kck_len(struct _SECURITY_CONFIG *pSecConfig)
{
#ifdef OCE_FILS_SUPPORT
	if (IS_AKM_FILS(pSecConfig->AKMMap)) {
		return 0;
	}
#endif /* OCE_FILS_SUPPORT */

	return 0;
}

UCHAR sec_get_cipher_key_len(UINT32 cipher)
{
	if (IS_CIPHER_TKIP(cipher))
		return LEN_TKIP_TK;
	else if (IS_CIPHER_CCMP128(cipher))
		return LEN_CCMP128_TK;
	else if (IS_CIPHER_CCMP256(cipher))
		return LEN_CCMP256_TK;
	else if (IS_CIPHER_GCMP128(cipher))
		return LEN_GCMP128_TK;
	else if (IS_CIPHER_GCMP256(cipher))
		return LEN_GCMP256_TK;

	return 0;
}

/*
	========================================================================

	Routine Description:
		Classify WPA EAP message type

	Arguments:
		EAPType		Value of EAP message type
		MsgType		Internal Message definition for MLME state machine

	Return Value:
		TRUE		Found appropriate message type
		FALSE		No appropriate message type

	IRQL = DISPATCH_LEVEL

	Note:
		All these constants are defined in wpa_cmm.h
		For supplicant, there is only EAPOL Key message avaliable

	========================================================================
*/
BOOLEAN	WpaMsgTypeSubst(
	IN	UCHAR	EAPType,
	OUT	INT * MsgType)
{
	switch (EAPType) {
	case EAPPacket:
		*MsgType = MT2_EAPPacket;
		break;

	case EAPOLStart:
		*MsgType = MT2_EAPOLStart;
		break;

	case EAPOLLogoff:
		*MsgType = MT2_EAPOLLogoff;
		break;

	case EAPOLKey:
		*MsgType = MT2_EAPOLKey;
		break;

	case EAPOLASFAlert:
		*MsgType = MT2_EAPOLASFAlert;
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

/**
 * inc_iv_byte - Increment arbitrary length byte array
 * @counter: Pointer to byte array
 * @len: Length of the counter in bytes
 *
 * This function increments the least byte of the counter by one and continues
 * rolling over to more significant bytes if the byte was incremented from
 * 0xff to 0x00.
 */
void inc_iv_byte(UCHAR *iv, UINT len, UINT cnt)
{
	int	pos = 0;
	int	carry = 0;
	UCHAR	pre_iv;

	while (pos < len) {
		pre_iv = iv[pos];

		if (carry == 1)
			iv[pos]++;
		else
			iv[pos] += cnt;

		if (iv[pos] > pre_iv)
			break;

		carry = 1;
		pos++;
	}

	if (pos >= len)
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_WARN,
			"!!! overflow !!!\n");

}


/*
    ==========================================================================
    Description:
		Report the EAP message type

	Arguments:
		msg		-	EAPOL_PAIR_MSG_1
					EAPOL_PAIR_MSG_2
					EAPOL_PAIR_MSG_3
					EAPOL_PAIR_MSG_4
					EAPOL_GROUP_MSG_1
					EAPOL_GROUP_MSG_2

    Return:
	 message type string

    ==========================================================================
*/
RTMP_STRING *GetEapolMsgType(CHAR msg)
{
	if (msg == EAPOL_PAIR_MSG_1)
		return "Pairwise Message 1";
	else if (msg == EAPOL_PAIR_MSG_2)
		return "Pairwise Message 2";
	else if (msg == EAPOL_PAIR_MSG_3)
		return "Pairwise Message 3";
	else if (msg == EAPOL_PAIR_MSG_4)
		return "Pairwise Message 4";
	else if (msg == EAPOL_GROUP_MSG_1)
		return "Group Message 1";
	else if (msg == EAPOL_GROUP_MSG_2)
		return "Group Message 2";
	else
		return "Invalid Message";
}


UCHAR RTMPExtractKeyIdxFromIVHdr(UCHAR *pIV, UINT8 CipherAlg)
{
	UCHAR keyIdx = 0xFF;

	/* extract the key index from IV header */
	switch (CipherAlg) {
	case Ndis802_11WEPEnabled:
	case Ndis802_11TKIPEnable:
	case Ndis802_11AESEnable:
	case Ndis802_11GroupWEP40Enabled:
	case Ndis802_11GroupWEP104Enabled:
		keyIdx = (*(pIV + 3) & 0xc0) >> 6;
		break;
	}

	return keyIdx;
}

#if defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT)

/*
	========================================================================

	Routine Description:
		Some received frames can't decrypt by Asic, so decrypt them by software.

	Arguments:
		pAd				-	pointer to our pAdapter context
	PeerWepStatus	-	indicate the encryption type

	Return Value:
		NDIS_STATUS_SUCCESS		-	decryption successful
		NDIS_STATUS_FAILURE		-	decryption failure

	========================================================================
*/
NDIS_STATUS	RTMPSoftDecryptionAction(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pHdr,
	IN UCHAR UserPriority,
	IN PCIPHER_KEY pKey,
#ifdef CONFIG_STA_SUPPORT
	IN UCHAR wdev_idx,
#endif
	INOUT PUCHAR pData,
	INOUT UINT16 * DataByteCnt)
{
	switch (pKey->CipherAlg) {
	case CIPHER_WEP64:
	case CIPHER_WEP128:

		/* handle WEP decryption */
		if (RTMPSoftDecryptWEP((PSEC_KEY_INFO) pKey, pData, &(*DataByteCnt)) == FALSE) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WEP, DBG_LVL_ERROR,
				"ERROR : SW decrypt WEP data fails.\n");
			/* give up this frame*/
			return NDIS_STATUS_FAILURE;
		}

		break;

	case CIPHER_TKIP:

		/* handle TKIP decryption */
		if (RTMPSoftDecryptTKIP(pAd, pHdr, UserPriority, pKey,
#ifdef CONFIG_STA_SUPPORT
								wdev_idx,
#endif
								 pData, &(*DataByteCnt)) == FALSE) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_TKIP, DBG_LVL_ERROR,
				"ERROR : SW decrypt TKIP data fails.\n");
			/* give up this frame*/
			return NDIS_STATUS_FAILURE;
		}

		break;

	case CIPHER_AES:

		/* handle AES decryption */
		if (RTMPSoftDecryptCCMP(pAd, pHdr, pKey, pData, &(*DataByteCnt)) == FALSE) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_AES, DBG_LVL_ERROR,
				"ERROR : SW decrypt AES data fails.\n");
			/* give up this frame*/
			return NDIS_STATUS_FAILURE;
		}

		break;

	default:
		/* give up this frame*/
		return NDIS_STATUS_FAILURE;
		break;
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* SOFT_ENCRYPT || SW_CONNECT_SUPPORT */


#if defined(SOFT_ENCRYPT)
CIPHER_KEY *RTMPSwCipherKeySelection(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pIV,
	IN struct _RX_BLK *pRxBlk,
	IN MAC_TABLE_ENTRY *pEntry)
{
	PCIPHER_KEY pKey = NULL;
	UCHAR keyIdx = 0;
	UINT8 CipherAlg = Ndis802_11EncryptionDisabled;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);

	ASSERT(pStaCfg);
#endif

	if ((pEntry == NULL) ||
		(RX_BLK_TEST_FLAG(pRxBlk, fRX_STA)) ||
		(RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS)) ||
		(RX_BLK_TEST_FLAG(pRxBlk, fRX_MESH)))
		return NULL;

	if (pRxBlk->pRxInfo->U2M)
		CipherAlg = pEntry->WepStatus;
	else {
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			CipherAlg = pStaCfg->GroupCipher;
		}
#endif /* CONFIG_STA_SUPPORT */
	}

	keyIdx = RTMPExtractKeyIdxFromIVHdr(pIV, CipherAlg);

	if (keyIdx > 3) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
			"Invalid key index(%d) !!!\n",
			keyIdx);
		return NULL;
	}

	if ((CipherAlg == Ndis802_11WEPEnabled)
		|| (CipherAlg == Ndis802_11GroupWEP40Enabled)
		|| (CipherAlg == Ndis802_11GroupWEP104Enabled))
		pKey = &pAd->SharedKey[pEntry->func_tb_idx][keyIdx];
	else if ((CipherAlg == Ndis802_11TKIPEnable) ||
			 (CipherAlg == Ndis802_11AESEnable)) {
		if (pRxBlk->pRxInfo->U2M)
			pKey = &pEntry->PairwiseKey;
		else
			pKey = &pAd->SharedKey[pEntry->func_tb_idx][keyIdx];
	}

	return pKey;
}
#endif /* SOFT_ENCRYPT */

#if defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT)
VOID RTMPSoftConstructIVHdr(
	IN	UCHAR			CipherAlg,
	IN	UCHAR			key_id,
	IN	PUCHAR			pTxIv,
	OUT PUCHAR			pHdrIv,
	OUT	UINT8			*hdr_iv_len)
{
	*hdr_iv_len = 0;
		if ((CipherAlg == CIPHER_WEP64) || (CipherAlg == CIPHER_WEP128)) {
			/* Construct and insert 4-bytes WEP IV header to MPDU header */
			RTMPConstructWEPIVHdr(key_id, pTxIv, pHdrIv);
			*hdr_iv_len = LEN_WEP_IV_HDR;
		} else if (CipherAlg == CIPHER_TKIP)
			;
		else if (CipherAlg == CIPHER_AES) {
			/* Construct and insert 8-bytes CCMP header to MPDU header */
			RTMPConstructCCMPHdr(key_id, pTxIv, pHdrIv);
			*hdr_iv_len = LEN_CCMP_HDR;
		}
}

VOID RTMPSoftEncryptionAction(
	IN	PRTMP_ADAPTER pAd,
	IN	UCHAR			CipherAlg,
	IN	PUCHAR			pHdr,
	IN	PUCHAR			pSrcBufData,
	IN	UINT32			SrcBufLen,
	IN	UCHAR			KeyIdx,
	IN	PCIPHER_KEY		pKey,
	OUT	UINT8			*ext_len)
{
	*ext_len = 0;
		if ((CipherAlg == CIPHER_WEP64) || (CipherAlg == CIPHER_WEP128)) {
			/* Encrypt the MPDU data by software*/
			RTMPSoftEncryptWEP(pKey->TxTsc,
							   (PSEC_KEY_INFO)pKey,
							   pSrcBufData,
							   SrcBufLen);
			*ext_len = LEN_ICV;
		} else if (CipherAlg == CIPHER_TKIP)
			;
		else if (CipherAlg == CIPHER_AES) {
			/* Encrypt the MPDU data by software*/
			RTMPSoftEncryptCCMP(pAd,
								pHdr,
								pKey->TxTsc,
								pKey->Key,
								pSrcBufData,
								SrcBufLen);
			*ext_len = LEN_CCMP_MIC;
		}
}
#endif /* SOFT_ENCRYPT || SW_CONNECT_SUPPORT */

#ifdef APCLI_CFG80211_SUPPORT
int WPA_Extract_extended_regclass_IE(UCHAR *assoc_ie, UINT assoc_ie_len, struct EXCLUDED_IES *elems)
{
	PEID_STRUCT pEid;
	UINT start = 0;
	UCHAR *pBuf = assoc_ie;

	for (; start < assoc_ie_len; ) {
		pEid = (PEID_STRUCT) pBuf;
		switch (pEid->Eid) {
		case IE_EXT_CAPABILITY:
			elems->extended_capa = pBuf;
			elems->extended_capa_len = pEid->Len;
		break;
		case IE_SUPP_REG_CLASS:
			elems->operating_class = pBuf;
			elems->operating_class_len = pEid->Len;
		break;
		default:
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "unknown IEs ( %d) len = %d\n", pEid->Eid, pEid->Len);
		break;
		}
		pBuf += (pEid->Len + 2);
		if (pBuf > (assoc_ie + assoc_ie_len)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
				"pBuf return 0\n");
			return 0;
		}
		start += (pEid->Len + 2);
		if (start > assoc_ie_len) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
				"start fail return 0\n");
			return 0;
		}
	}
	return 1;
}
#endif /* APCLI_CFG80211_SUPPORT */

PUINT8	WPA_ExtractSuiteFromRSNIE(
	IN	PUINT8	rsnie,
	IN	UINT	rsnie_len,
	IN	UINT8	type,
	OUT	UINT8	*count)
{
	PEID_STRUCT pEid;
	INT			len;
	PUINT8		pBuf;
	INT			offset = 0;

	pEid = (PEID_STRUCT)rsnie;
	len = rsnie_len - 2;	/* exclude IE and length*/
	pBuf = (PUINT8)&pEid->Octet[0];
	/* set default value*/
	*count = 0;

	/* Check length*/
	if ((len <= 0) || (pEid->Len != len)) {
		MTWF_DBG_NP(DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_WARN,
			"%s: The length is invalid\n", __func__);
		goto out;
	}

	/* Check WPA or WPA2*/
	if (pEid->Eid == IE_WPA) {
		/* Check the length */
		if (len < sizeof(RSNIE)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
				"The length is too short for WPA\n");
			goto out;
		} else {
			PRSNIE	pRsnie;
			UINT16	u_cnt;

			pRsnie = (PRSNIE)pBuf;
			u_cnt = cpu2le16(pRsnie->ucount);
			offset = sizeof(RSNIE) + (LEN_OUI_SUITE * (u_cnt - 1));

			if (len < offset) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
					"The expected length(%d) exceed the remaining length(%d) for WPA-RSN\n",
					offset, len);
				goto out;
			} else {
				/* Get the group cipher*/
				if (type == GROUP_SUITE) {
					*count = 1;
					return pRsnie->mcast;
				}
				/* Get the pairwise cipher suite*/
				else if (type == PAIRWISE_SUITE) {
					MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
						"The count of pairwise cipher is %d\n",
						u_cnt);
					*count = u_cnt;
					return pRsnie->ucast[0].oui;
				}
			}
		}
	} else if (pEid->Eid == IE_RSN) {
		if (len < sizeof(RSNIE2)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
				"The length is too short for WPA2\n");
			goto out;
		} else {
			PRSNIE2	pRsnie2;
			UINT16	u_cnt;

			pRsnie2 = (PRSNIE2)pBuf;
			u_cnt = cpu2le16(pRsnie2->ucount);
			offset = sizeof(RSNIE2) + (LEN_OUI_SUITE * (u_cnt - 1));

			if (len < offset) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
					"The expected length(%d) exceed the remaining length(%d) for WPA2-RSN\n",
					offset, len);
				goto out;
			} else {
				/* Get the group cipher*/
				if (type == GROUP_SUITE) {
					*count = 1;
					return pRsnie2->mcast;
				}
				/* Get the pairwise cipher suite*/
				else if (type == PAIRWISE_SUITE) {
					MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
						"The count of pairwise cipher is %d\n",
						u_cnt);
					*count = u_cnt;
					return pRsnie2->ucast[0].oui;
				}
			}
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "Unknown IE (%d)\n", pEid->Eid);
		goto out;
	}

	/* skip group cipher and pairwise cipher suite	*/
	pBuf += offset;
	len -= offset;

	/* Ready to extract the AKM information and its count */
	if (len < sizeof(RSNIE_AUTH)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"The length of AKM of RSN is too short\n");
		goto out;
	} else {
		PRSNIE_AUTH	pAkm;
		UINT16		a_cnt;
		/* pointer to AKM count */
		pAkm = (PRSNIE_AUTH)pBuf;
		a_cnt = cpu2le16(pAkm->acount);
		offset = sizeof(RSNIE_AUTH) + (LEN_OUI_SUITE * (a_cnt - 1));

		if (len < offset) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
				"The expected length(%d) exceed the remaining length(%d) for AKM\n",
				offset, len);
			goto out;
		} else {
			/* Get the AKM suite */
			if (type == AKM_SUITE) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
					"The count of AKM is %d\n",
					a_cnt);
				*count = a_cnt;
				return pAkm->auth[0].oui;
			}
		}
	}

	/* For WPA1, the remaining shall be ignored. */
	if (pEid->Eid == IE_WPA) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"The remaining shall be ignored in WPA mode\n");
		goto out;
	}

	/* skip the AKM capability */
	pBuf += offset;
	len -= offset;

	/* Parse the RSN Capabilities */
	if (len < sizeof(RSN_CAPABILITIES)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"The peer RSNIE doesn't include RSN-Cap\n");
		goto out;
	} else {
		/* Report the content of the RSN capabilities */
		if (type == RSN_CAP_INFO) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
				"Extract RSN Capabilities\n");
			*count = 1;
			return pBuf;
		}

		/* skip RSN capability (2-bytes) */
		offset = sizeof(RSN_CAPABILITIES);
		pBuf += offset;
		len -= offset;
	}

	/* Extract PMKID-list field */
	if (len < sizeof(UINT16)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"The peer RSNIE doesn't include PMKID list Count\n");
		goto out;
	} else {
		UINT16	p_count;
		PUINT8	pPmkidList = NULL;

		NdisMoveMemory(&p_count, pBuf, sizeof(UINT16));
		p_count = cpu2le16(p_count);

		/* Get count of the PMKID list */
		if (p_count > 0) {
			PRSNIE_PMKID	pRsnPmkid;
			/* the expected length of PMKID-List field */
			offset = sizeof(RSNIE_PMKID) + (LEN_PMKID * (p_count - 1));

			/* sanity check about the length of PMKID-List field */
			if (len < offset) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
					" The expected length(%d) exceed the remaining length(%d) in PMKID-field\n",
					offset, len);
				goto out;
			}

			/* pointer to PMKID field */
			pRsnPmkid = (PRSNIE_PMKID)pBuf;
			pPmkidList = pRsnPmkid->pmkid[0].list;
		} else {
			/* The PMKID field shall be without PMKID-List */
			offset = sizeof(UINT16);
			pPmkidList = NULL;
		}

		/* Extract PMKID list and its count */
		if (type == PMKID_LIST) {
			*count = p_count;
			return pPmkidList;
		}

		/* skip the PMKID field */
		pBuf += offset;
		len -= offset;
	}

#ifdef DOT11W_PMF_SUPPORT

	/* Get group mamagement cipher */
	if (len < LEN_OUI_SUITE) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO,
			"[PMF]The peer RSNIE doesn't include Group_mgmt_cipher_suite\n");
		goto out;
	} else {
		offset = LEN_OUI_SUITE;

		/* Get Group-Mgmt-Cipher_Suite information */
		if (type == G_MGMT_SUITE) {
			*count = 1;
			return pBuf;
		} else {
			/* skip the Group-Mgmt-Cipher_Suite field */
			pBuf += offset;
			len -= offset;
		}
	}

#endif /* DOT11W_PMF_SUPPORT */
out:
	*count = 0;
	return NULL;
}

VOID WpaShowAllsuite(
	IN	PUINT8	rsnie,
	IN	UINT	rsnie_len)
{
	PUINT8 pSuite = NULL;
	UINT8 count;

	hex_dump("RSNIE", rsnie, rsnie_len);
	/* group cipher*/
	pSuite = WPA_ExtractSuiteFromRSNIE(rsnie, rsnie_len, GROUP_SUITE, &count);

	if (pSuite != NULL)
		hex_dump("group cipher", pSuite, 4 * count);

	/* pairwise cipher*/
	pSuite = WPA_ExtractSuiteFromRSNIE(rsnie, rsnie_len, PAIRWISE_SUITE, &count);

	if (pSuite != NULL)
		hex_dump("pairwise cipher", pSuite, 4 * count);

	/* AKM*/
	pSuite = WPA_ExtractSuiteFromRSNIE(rsnie, rsnie_len, AKM_SUITE, &count);

	if (pSuite != NULL)
		hex_dump("AKM suite", pSuite, 4 * count);

	/* PMKID*/
	pSuite = WPA_ExtractSuiteFromRSNIE(rsnie, rsnie_len, PMKID_LIST, &count);

	if (pSuite != NULL)
		hex_dump("PMKID", pSuite, LEN_PMKID);
}

#ifdef RT_CFG80211_SUPPORT
BOOLEAN RTMPIsValidIEs(UCHAR *Ies, INT32 Len)
{
	/* Validate if the IE is in correct format. */
	INT32 Pos = 0;
	INT32 IeLen = 0;

	while (Pos < Len) {
		IeLen = (INT32)(Ies[Pos + 1]) + 2;

		if (IeLen < 0)
			return FALSE;

		Pos += IeLen;
	}

	if (Pos == Len)
		return TRUE;
	else
		return FALSE;
}

#endif /* RT_CFG80211_SUPPORT */


/* --------------------Eddy---------------- */
/*
	========================================================================

	Routine Description:
		The pseudo-random function(PRF) that hashes various inputs to
		derive a pseudo-random value. To add liveness to the pseudo-random
		value, a nonce should be one of the inputs.

		It is used to generate PTK, GTK or some specific random value.

	Arguments:
		UCHAR	*key,		-	the key material for HMAC_SHA1 use
		INT		key_len		-	the length of key
		UCHAR	*prefix		-	a prefix label
		INT		prefix_len	-	the length of the label
		UCHAR	*data		-	a specific data with variable length
		INT		data_len	-	the length of a specific data
		INT		len			-	the output lenght

	Return Value:
		UCHAR	*output		-	the calculated result

	Note:
		802.11i-2004	Annex H.3

	========================================================================
*/
#ifndef RT_CFG80211_SUPPORT
VOID PRF(
	IN UCHAR *key,
	IN INT key_len,
	IN UCHAR *prefix,
	IN INT prefix_len,
	IN UCHAR *data,
	IN INT data_len,
	OUT UCHAR *output,
	IN INT len)
{
	INT i;
	UCHAR *input;
	INT currentindex = 0;
	INT total_len;
	/* Allocate memory for input*/
	os_alloc_mem(NULL, (PUCHAR *)&input, 1024);

	if (input == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "!!!no memory!!!\n");
		return;
	}

	/* Generate concatenation input*/
	NdisMoveMemory(input, prefix, prefix_len);
	/* Concatenate a single octet containing 0*/
	input[prefix_len] =	0;
	/* Concatenate specific data*/
	NdisMoveMemory(&input[prefix_len + 1], data, data_len);
	total_len =	prefix_len + 1 + data_len;
	/* Concatenate a single octet containing 0*/
	/* This octet shall be update later*/
	input[total_len] = 0;
	total_len++;

	/* Iterate to calculate the result by hmac-sha-1*/
	/* Then concatenate to last result*/
	for	(i = 0;	i <	(len + 19) / 20; i++) {
		RT_HMAC_SHA1(key, key_len, input, total_len, &output[currentindex], SHA1_DIGEST_SIZE);
		currentindex +=	20;
		/* update the last octet */
		input[total_len - 1]++;
	}

	os_free_mem(input);
}

UCHAR get_mic_len_by_alg(IN UINT32 key_deri_alg, UINT *mic_len)
{
	if (key_deri_alg == SEC_KEY_DERI_SHA256)
		*mic_len = SHA256_DIGEST_SIZE;
	else if (key_deri_alg == SEC_KEY_DERI_SHA384)
		*mic_len = SHA384_DIGEST_SIZE;
	else if (key_deri_alg == SEC_KEY_DERI_SHA512)
		*mic_len = SHA512_DIGEST_SIZE;
	else
		return FALSE;

	return TRUE;
}


static VOID kdf_cmm(
	IN PUINT8 key,
	IN INT key_len,
	IN PUINT8 label,
	IN INT label_len,
	IN PUINT8 data,
	IN INT data_len,
	OUT PUINT8 output,
	IN USHORT len,
	IN USHORT len_in_bits,
	IN UINT32 key_deri_alg)
{
	USHORT	i;
	UCHAR   *input;
	INT		currentindex = 0;
	INT		total_len;
	UINT    mic_len;

	if (!get_mic_len_by_alg(key_deri_alg, &mic_len)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "get mic len fail(alg = %d)\n", key_deri_alg);
		return;
	}

	os_alloc_mem(NULL, (PUCHAR *)&input, 1024);

	if (input == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "!!!KDF: no memory!!!\n");
		return;
	}

	NdisZeroMemory(input, 1024);
	/* Initial concatenated value (i || label || Context || Length)*/
	/* concatenate 16-bit unsigned integer, its initial value is 1.	*/
	input[0] = 1;
	input[1] = 0;
	total_len = 2;
	/* concatenate a prefix string*/
	NdisMoveMemory(&input[total_len], label, label_len);
	total_len += label_len;
	/* concatenate the context*/
	NdisMoveMemory(&input[total_len], data, data_len);
	total_len += data_len;
	/* concatenate the length in bits (16-bit unsigned integer)*/
	input[total_len] = (len_in_bits & 0xFF);
	input[total_len + 1] = (len_in_bits & 0xFF00) >> 8;
	total_len += 2;

	for	(i = 1;	i <= ((len_in_bits + (mic_len * 8 - 1)) / (mic_len * 8)); i++) {
		UINT cur_mic_len = mic_len;

		if ((len - currentindex) < mic_len)
			cur_mic_len = len - currentindex;
		/* HMAC-SHA256 derives output */
		if (key_deri_alg == SEC_KEY_DERI_SHA256)
			RT_HMAC_SHA256((UCHAR *)key, key_len, input, total_len, (UCHAR *)&output[currentindex], cur_mic_len);
		else if (key_deri_alg == SEC_KEY_DERI_SHA384)
			RT_HMAC_SHA384((UCHAR *)key, key_len, input, total_len, (UCHAR *)&output[currentindex], cur_mic_len);
		else if (key_deri_alg == SEC_KEY_DERI_SHA512)
			RT_HMAC_SHA512((UCHAR *)key, key_len, input, total_len, (UCHAR *)&output[currentindex], cur_mic_len);
		currentindex +=	cur_mic_len; /* next concatenation location*/
		input[0]++;			/* increment octet count*/
	}

	os_free_mem(input);
}



/*
	========================================================================

	Routine Description:
		The key derivation function(KDF) is defined in IEEE 802.11r/D9.0, 8.5.1.5.2

	Arguments:

	Return Value:

	Note:
		Output - KDF-Length (K, label, Context) where
		Input:    K, a 256-bit key derivation key
				  label, a string identifying the purpose of the keys derived using this KDF
				  Context, a bit string that provides context to identify the derived key
				  Length, the length of the derived key in bits
		Output: a Length-bit derived key

		result - ""
		iterations - (Length+255)/256
		do i = 1 to iterations
			result - result || HMAC-SHA256(K, i || label || Context || Length)
		od
		return first Length bits of result, and securely delete all unused bits

		In this algorithm, i and Length are encoded as 16-bit unsigned integers.

	========================================================================
*/
VOID KDF_256(
	IN PUINT8 key,
	IN INT key_len,
	IN PUINT8 label,
	IN INT label_len,
	IN PUINT8 data,
	IN INT data_len,
	OUT PUINT8 output,
	IN USHORT len)
{
	kdf_cmm(key, key_len, label, label_len, data, data_len, output, len, (len << 3), SEC_KEY_DERI_SHA256);
}

VOID KDF_256_bit_len(
	IN PUINT8 key,
	IN INT key_len,
	IN PUINT8 label,
	IN INT label_len,
	IN PUINT8 data,
	IN INT data_len,
	OUT PUINT8 output,
	IN USHORT len,
	IN USHORT len_bit,
	IN UCHAR is_leftmost)
{
	kdf_cmm(key, key_len, label, label_len, data, data_len, output, len, len_bit, SEC_KEY_DERI_SHA256);

	if (len_bit % 8) {
		UCHAR idx = len;
		UCHAR shift_bit = len_bit % 8;

		if (is_leftmost)
			output[idx - 1] &= ~BITS(0, 7 - shift_bit);
		else {
			do {
				idx--;
				output[idx] >>= (8 - shift_bit);
				if (idx != 0)
					output[idx] |= (output[idx - 1] << shift_bit);
				else
					break;
			} while (TRUE);
		}
	}
}



VOID KDF_384(
	IN PUINT8 key,
	IN INT key_len,
	IN PUINT8 label,
	IN INT label_len,
	IN PUINT8 data,
	IN INT data_len,
	OUT PUINT8 output,
	IN USHORT len)
{
	kdf_cmm(key, key_len, label, label_len, data, data_len, output, len, (len << 3), SEC_KEY_DERI_SHA384);
}

VOID KDF_512(
	IN PUINT8 key,
	IN INT key_len,
	IN PUINT8 label,
	IN INT label_len,
	IN PUINT8 data,
	IN INT data_len,
	OUT PUINT8 output,
	IN USHORT len)
{
	kdf_cmm(key, key_len, label, label_len, data, data_len, output, len, (len << 3), SEC_KEY_DERI_SHA512);
}

static VOID hkdf_expand_cmm(IN UCHAR *secret,
			IN INT secret_len,
			IN UCHAR *info,
			IN INT info_len,
			OUT UCHAR *output,
			INT output_Len,
			IN UINT32 key_deri_alg)
{
	UCHAR T[SHA512_DIGEST_SIZE];
	UCHAR iter = 1;
	const unsigned char *addr[3];
	UINT len[3];
	UINT pos, clen;
	UINT mic_len;

	if (!get_mic_len_by_alg(key_deri_alg, &mic_len)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "get mic len fail(alg = %d)\n", key_deri_alg);
		return;
	}

	addr[0] = T;
	len[0] = mic_len;
	addr[1] = info;
	len[1] = info_len;
	addr[2] = &iter;
	len[2] = 1;

	if (key_deri_alg == SEC_KEY_DERI_SHA256)
		RT_HMAC_SHA256_VECTOR(secret, secret_len, 2, &addr[1], &len[1], T, mic_len);
	else if (key_deri_alg == SEC_KEY_DERI_SHA384)
		RT_HMAC_SHA384_VECTOR(secret, secret_len, 2, &addr[1], &len[1], T, mic_len);
	else if (key_deri_alg == SEC_KEY_DERI_SHA512)
		RT_HMAC_SHA512_VECTOR(secret, secret_len, 2, &addr[1], &len[1], T, mic_len);

	pos = 0;
	for (;;) {
		clen = output_Len - pos;
		if (clen > mic_len)
			clen = mic_len;
		memcpy(output + pos, T, clen);
		pos += clen;

		if (pos == output_Len)
			break;

		if (iter == 255) {
			os_zero_mem(output, output_Len);
			os_zero_mem(T, mic_len);
			return;
		}
		iter++;

		if (key_deri_alg == SEC_KEY_DERI_SHA256)
			RT_HMAC_SHA256_VECTOR(secret, secret_len, 3, addr, len, T, mic_len);
		else if (key_deri_alg == SEC_KEY_DERI_SHA384)
			RT_HMAC_SHA384_VECTOR(secret, secret_len, 3, addr, len, T, mic_len);
		else if (key_deri_alg == SEC_KEY_DERI_SHA512)
			RT_HMAC_SHA512_VECTOR(secret, secret_len, 3, addr, len, T, mic_len);
	}

	os_zero_mem(T, mic_len);
}



VOID HKDF_expand_sha256(IN UCHAR *secret,
			IN INT secret_len,
			IN UCHAR *info,
			IN INT info_len,
			OUT UCHAR *output,
			INT output_Len)
{
	hkdf_expand_cmm(secret, secret_len, info, info_len, output, output_Len, SEC_KEY_DERI_SHA256);
}

VOID HKDF_expand_sha384(IN UCHAR *secret,
			IN INT secret_len,
			IN UCHAR *info,
			IN INT info_len,
			OUT UCHAR *output,
			INT output_Len)
{
	hkdf_expand_cmm(secret, secret_len, info, info_len, output, output_Len, SEC_KEY_DERI_SHA384);
}

VOID HKDF_expand_sha512(IN UCHAR *secret,
			IN INT secret_len,
			IN UCHAR *info,
			IN INT info_len,
			OUT UCHAR *output,
			INT output_Len)
{
	hkdf_expand_cmm(secret, secret_len, info, info_len, output, output_Len, SEC_KEY_DERI_SHA512);
}



/*
	========================================================================

	Routine Description:
		Generate random number by software.

	Arguments:
		pAd		-	pointer to our pAdapter context
		macAddr	-	pointer to local MAC address

	Return Value:

	Note:
		802.1ii-2004  Annex H.5

	========================================================================
*/
VOID	GenRandom(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			*macAddr,
	OUT	UCHAR			*random)
{
	INT		i, curr;
	UCHAR	local[80], KeyCounter[32];
	UCHAR	result[80];
	ULONG	CurrentTime;
	UCHAR	prefix[] = {'I', 'n', 'i', 't', ' ', 'C', 'o', 'u', 'n', 't', 'e', 'r'};
	/* Zero the related information*/
	NdisZeroMemory(result, 80);
	NdisZeroMemory(local, 80);
	NdisZeroMemory(KeyCounter, 32);

	for	(i = 0;	i <	32;	i++) {
		/* copy the local MAC address*/
		COPY_MAC_ADDR(local, macAddr);
		curr =	MAC_ADDR_LEN;
		/* concatenate the current time*/
		NdisGetSystemUpTime(&CurrentTime);
		NdisMoveMemory(&local[curr],  &CurrentTime,	sizeof(CurrentTime));
		curr +=	sizeof(CurrentTime);
		/* concatenate the last result*/
		NdisMoveMemory(&local[curr],  result, 32);
		curr +=	32;
		/* concatenate a variable */
		NdisMoveMemory(&local[curr],  &i,  2);
		curr +=	2;
		/* calculate the result*/
		PRF(KeyCounter, 32, prefix, 12, local, curr, result, 32);
	}

	NdisMoveMemory(random, result,	32);
}


/*
	========================================================================

	Routine	Description:
		Copy frame from waiting queue into relative ring buffer and set
	appropriate ASIC register to kick hardware encryption before really
	sent out to air.

	Arguments:
		pAd		Pointer	to our adapter
		PNDIS_PACKET	Pointer to outgoing Ndis frame
		NumberOfFrag	Number of fragment required

	Return Value:
		None

	Note:

	========================================================================
*/
#endif /* RT_CFG80211_SUPPORT */

VOID RTMPToWirelessSta(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PUCHAR pHeader802_3,
	IN UINT HdrLen,
	IN PUCHAR pData,
	IN UINT DataLen,
	IN BOOLEAN bClearFrame)
{
	PNDIS_PACKET pPacket;
	NDIS_STATUS Status;
	struct wifi_dev *wdev;
	struct sk_buff *skb;
	/* UINT8 debug_level_bkp = DebugLevel; */

	if ((!pEntry) || (!IS_ENTRY_CLIENT(pEntry) && !IS_ENTRY_PEER_AP(pEntry)
#ifdef MAC_REPEATER_SUPPORT
					  && (!IS_ENTRY_REPEATER(pEntry))
#endif
					 ))
		return;

	Status = RTMPAllocateNdisPacket(pAd, &pPacket, pHeader802_3, HdrLen, pData, DataLen);

	if (Status != NDIS_STATUS_SUCCESS)
		return;

	if (NdisEqualMemory((pHeader802_3 + MAC_ADDR_LEN * 2), EAPOL, LENGTH_802_3_TYPE))
		RTMP_SET_PACKET_EAPOL(pPacket, 1);

	RTMPSetPacketProtocol(pPacket, pHeader802_3, HdrLen);

	RTMP_SET_PACKET_CLEAR_EAP_FRAME(pPacket, (bClearFrame ? 1 : 0));
	RTMP_SET_PACKET_WCID(pPacket, pEntry->wcid);

	/* TODO: shiang-usw, fix this! */
	if (!pEntry->wdev) {
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return;
	}
	RTMP_SET_PACKET_WDEV(pPacket, pEntry->wdev->wdev_idx);
	RTMP_SET_PACKET_MOREDATA(pPacket, FALSE);

	skb = RTPKT_TO_OSPKT(pPacket);
	if (!skb->dev)
		skb->dev = pEntry->wdev->if_dev;

	wdev = pEntry->wdev;

	if (!wdev) {
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return;
	}

	send_data_pkt(pAd, wdev, pPacket);
}


/* Eddy: should be move to auth file */
VOID MlmeDeAuthAction(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN USHORT           Reason,
	IN BOOLEAN          bDataFrameFirst)
{
	PUCHAR          pOutBuffer = NULL;
	ULONG           FrameLen = 0;
	HEADER_802_11   DeAuthHdr;
	NDIS_STATUS     NStatus;

	if (pEntry) {
#ifdef CONFIG_STA_SUPPORT
		PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);
#endif
		/* Send out a Deauthentication request frame*/
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

		/* send wireless event - for send disassication */
		RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
				 "Send DEAUTH frame with ReasonCode(%d) to "MACSTR"\n",
				  Reason, MAC2STR(pEntry->Addr));
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->Addr, pEntry->wdev->if_addr, pStaCfg->Bssid);
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->Addr,
							 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.if_addr,
							 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid);
		}
#endif /* CONFIG_AP_SUPPORT */
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(HEADER_802_11), &DeAuthHdr,
						  2,  &Reason,
						  END_OF_ARGS);

		if (bDataFrameFirst)
			MiniportMMRequest(pAd, MGMT_USE_QUEUE_FLAG, pOutBuffer, FrameLen, NULL);
		else
			MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);

		MlmeFreeMemory(pOutBuffer);
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr, DIAG_CONN_DEAUTH_COM, Reason);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				Reason);
#endif
#ifdef MAP_R2
			if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
				wapp_handle_sta_disassoc(pAd, pEntry->wcid, Reason);
#endif

		/* ApLogEvent(pAd, pEntry->Addr, EVENT_DISASSOCIATED);*/
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM)
		if (!pEntry->IsKeep) {
#endif /* CONFIG_HOTSPOT_R2 */
			/*usd dispatch level due to MlmeDeAuthAction will be called in timer task*/
			if (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
					"[PMF] need to delay 10ms before delete entry!\n");
				RtmpOsMsDelay(10);
			}
			mac_entry_delete(pAd, pEntry, TRUE);
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM)
		}
#endif /* CONFIG_HOTSPOT_R2 */

	}
}


/*
    ==========================================================================
    Description:
		Check whether the received frame is EAP frame.

	Arguments:
		pAd				-	pointer to our pAdapter context
		pEntry			-	pointer to active entry
		pData			-	the received frame
		DataByteCount	-	the received frame's length
		wdev_idx	-	indicate the interface index

    Return:
	 TRUE			-	This frame is EAP frame
	 FALSE			-	otherwise
    ==========================================================================
*/
BOOLEAN RTMPCheckWPAframe(
	IN RTMP_ADAPTER * pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN UCHAR *pData,
	IN ULONG DataByteCount,
	IN UCHAR wdev_idx,
	IN BOOLEAN eth_frame)
{
	ULONG Body_len, min_len = (LENGTH_802_1_H + LENGTH_EAPOL_H);
#ifndef RT_CFG80211_SUPPORT
	BOOLEAN Cancelled;
#endif /*#ifndef RT_CFG80211_SUPPORT*/
	struct wifi_dev *wdev;
	ASSERT(wdev_idx <= WDEV_NUM_MAX);

	if (wdev_idx >= WDEV_NUM_MAX)
		return FALSE;

	wdev = pAd->wdev_list[wdev_idx];

	do {
		if (eth_frame) {
			min_len = (LENGTH_802_3 + LENGTH_EAPOL_H);
			break;
		}
	} while (FALSE);

	if (DataByteCount < min_len)
		return FALSE;

	/* Skip LLC or ETH header */
	if (eth_frame == TRUE)
		pData += LENGTH_802_3;
	else {
		/* Cisco 1200 AP may send packet with SNAP_BRIDGE_TUNNEL*/
		if (NdisEqualMemory(SNAP_802_1H, pData, 6) ||
			NdisEqualMemory(SNAP_BRIDGE_TUNNEL, pData, 6))
			pData += 6;
	}

	/* Skip 2-bytes EAPoL type */
	if (NdisEqualMemory(EAPOL, pData, 2))
		pData += 2;
	else if (NdisEqualMemory(EAPOL, pData + LENGTH_802_1Q, 2))
		pData += LENGTH_802_1Q + 2;
	else
		return FALSE;

	switch (*(pData + 1)) {
	case EAPPacket:
		Body_len = (*(pData + 2) << 8) | (*(pData + 3));
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef IDS_SUPPORT

			if ((*(pData + 4)) == EAP_CODE_REQUEST)
				pAd->ApCfg.RcvdEapReqCount++;

#endif /* IDS_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "Receive EAP-Packet frame, TYPE = 0, Length = %ld\n", Body_len);
		break;

	case EAPOLStart:
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"Receive EAPOL-Start frame, TYPE = 1\n");

		if (pEntry->EnqueueEapolStartTimerRunning != EAPOL_START_DISABLE) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
				"Cancel the EnqueueEapolStartTimerRunning\n");
#ifndef RT_CFG80211_SUPPORT
			RTMPCancelTimer(&pEntry->SecConfig.StartFor4WayTimer, &Cancelled);
#endif /*RT_CFG80211_SUPPORT*/
			pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
		}

		break;

	case EAPOLLogoff:
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"Receive EAPOLLogoff frame, TYPE = 2\n");
		break;

	case EAPOLKey:
		Body_len = (*(pData + 2) << 8) | (*(pData + 3));
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"Receive EAPOL-Key frame, TYPE = 3, Length = %ld\n", Body_len);
		break;

	case EAPOLASFAlert:
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"Receive EAPOLASFAlert frame, TYPE = 4\n");
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

#ifndef RT_CFG80211_SUPPORT
/*
* F(P, S, c, i) = U1 xor U2 xor ... Uc
* U1 = PRF(P, S || Int(i))
* U2 = PRF(P, U1)
* Uc = PRF(P, Uc-1)
*/
static void WPA_F(char *password, unsigned char *ssid, unsigned int ssidlength, int iterations, int count, unsigned char *output)
{
	unsigned char digest[36], digest1[SHA1_DIGEST_SIZE] = {0};
	unsigned int i, j, len;

	len = strlen(password);
	/* U1 = PRF(P, S || int(i)) */
	memcpy(digest, ssid, ssidlength);
	digest[ssidlength] = (unsigned char)((count >> 24) & 0xff);
	digest[ssidlength + 1] = (unsigned char)((count >> 16) & 0xff);
	digest[ssidlength + 2] = (unsigned char)((count >> 8) & 0xff);
	digest[ssidlength + 3] = (unsigned char)(count & 0xff);
	RT_HMAC_SHA1((unsigned char *) password, len, digest, ssidlength + 4, digest1, SHA1_DIGEST_SIZE); /* for WPA update*/
	/* output = U1 */
	NdisCopyMemory(output, digest1, SHA1_DIGEST_SIZE);

	for (i = 1; i < iterations; i++) {
		/* Un = PRF(P, Un-1) */
		RT_HMAC_SHA1((unsigned char *) password, len, digest1, SHA1_DIGEST_SIZE, digest, SHA1_DIGEST_SIZE); /* for WPA update*/
		NdisCopyMemory(digest1, digest, SHA1_DIGEST_SIZE);

		/* output = output xor Un */
		for (j = 0; j < SHA1_DIGEST_SIZE; j++)
			output[j] ^= digest[j];
	}
}


/*
* password - ascii string up to 63 characters in length
* ssid - octet string up to 32 octets
* ssidlength - length of ssid in octets
* output must be 40 octets in length and outputs 256 bits of key
*/
INT WPAPasswordHash(
	IN CHAR *password,
	IN UCHAR *ssid,
	IN INT ssidlength,
	OUT UCHAR *output)
{
	if ((strlen(password) > 63) || (ssidlength > 32))
		return 0;

	WPA_F(password, ssid, ssidlength, 4096, 1, output);
	WPA_F(password, ssid, ssidlength, 4096, 2, &output[SHA1_DIGEST_SIZE]);
	return 1;
}
#endif /* RT_CFG80211_SUPPORT */

INT SetWPAPSKKey(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *keyString,
	IN INT keyStringLen,
	IN UCHAR *pHashStr,
	IN INT hashStrLen,
	OUT PUCHAR pPMKBuf)
{
	UCHAR keyMaterial[40] = {0};

	if ((keyStringLen < 8) || (keyStringLen > 64)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"WPAPSK Key length(%d) error, required 8 ~ 64 characters!(keyStr=%s)\n",
			keyStringLen, keyString);
		return FALSE;
	}

	NdisZeroMemory(pPMKBuf, 32);

	if (keyStringLen == 64)
		AtoH(keyString, pPMKBuf, 32);
	else {
#ifndef RT_CFG80211_SUPPORT
		WPAPasswordHash(keyString, pHashStr, hashStrLen, keyMaterial);
#endif /* RT_CFG80211_SUPPORT */
		NdisMoveMemory(pPMKBuf, keyMaterial, 32);
	}

	return TRUE;
}


/* For TKIP frame, calculate the MIC value */
BOOLEAN rtmp_chk_tkip_mic(
	IN RTMP_ADAPTER * pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _RX_BLK * pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	UCHAR *pData = pRxBlk->pData;
	USHORT DataSize = pRxBlk->DataSize;
	UCHAR UserPriority = pRxBlk->UserPriority;
	UCHAR *pDA, *pSA;
	CHAR *pKey = NULL;

	if ((FC->FrDs == 1) && (FC->ToDs == 1)) {
		pDA = pRxBlk->Addr3;
		pSA = pRxBlk->Addr4;
	} else if ((FC->FrDs == 1) && (FC->ToDs == 0)) {
		pDA = pRxBlk->Addr1;
		pSA = pRxBlk->Addr3;
	} else if ((FC->FrDs == 0) && (FC->ToDs == 1)) {
		pDA = pRxBlk->Addr3;
		pSA = pRxBlk->Addr2;
	} else {
		/* FrDS = 0; ToDS = 0 => IBSS, Non-AP to Non-AP in BSS */
		pDA = pRxBlk->Addr1;
		pSA = pRxBlk->Addr2;
	}

#ifdef HDR_TRANS_RX_SUPPORT

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
		pData = (pData + LENGTH_802_3);
		DataSize = (DataSize - LENGTH_802_3);
	}

#endif /* HDR_TRANS_RX_SUPPORT */

	pKey = &pEntry->SecConfig.PTK[OFFSET_OF_TKIP_RX_MIC];

	if (RTMPTkipCompareMICValue(pAd,
								pData,
								pDA,
								pSA,
								pKey,
								UserPriority,
								DataSize) == FALSE) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_TKIP, DBG_LVL_ERROR, "Rx MIC Value error 2\n");
#ifdef CONFIG_AP_SUPPORT

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_STA))
			RTMP_HANDLE_COUNTER_MEASURE(pAd, pEntry);

#endif /* CONFIG_AP_SUPPORT */
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return FALSE;
	}

	return TRUE;
}

static BOOLEAN WPAMakeRsnIeCipher(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN UCHAR ie_idx,
	OUT UCHAR *rsn_len)
{
	SEC_RSNIE_TYPE ElememtId = pSecConfig->RSNE_Type[ie_idx];
	PUCHAR pRsnIe = &pSecConfig->RSNE_Content[ie_idx][0];
	UCHAR	PairwiseCnt = 0;
	*rsn_len = 0;

	if (ElememtId == SEC_RSNIE_WPA1_IE) {
		RSNIE *pRsnie_cipher = (RSNIE *) pRsnIe;
		UCHAR *rsnie_cipher_oui = (UCHAR *)pRsnie_cipher + offsetof(RSNIE, ucast[0].oui);
		/* Assign OUI and version*/
		NdisMoveMemory(pRsnie_cipher->oui, OUI_WPA_VERSION, 4);
		pRsnie_cipher->version = 1;

		/* Group cipher */
		if (IS_CIPHER_CCMP128(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_CCMP, 4);
		else if (IS_CIPHER_TKIP(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_TKIP, 4);
		else if (IS_CIPHER_WEP104(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_WEP104, 4);
		else if (IS_CIPHER_WEP40(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_WEP40, 4);
		else {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "not support group cipher on WPA1 (GroupCipher=0x%x)\n", pSecConfig->GroupCipher);
			return FALSE;
		}

		/* Pairwise cipher */
		if (IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher)) {
			NdisMoveMemory(rsnie_cipher_oui + PairwiseCnt * 4, OUI_WPA_CCMP, 4);
			PairwiseCnt++;
		}

		if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher)) {
			NdisMoveMemory(rsnie_cipher_oui + PairwiseCnt * 4, OUI_WPA_TKIP, 4);
			PairwiseCnt++;
		}

		pRsnie_cipher->ucount = PairwiseCnt;
		*rsn_len = sizeof(RSNIE) + (4 * (PairwiseCnt - 1));
		/* swap for big-endian platform*/
		pRsnie_cipher->version = cpu2le16(pRsnie_cipher->version);
		pRsnie_cipher->ucount = cpu2le16(pRsnie_cipher->ucount);
	} else if (ElememtId == SEC_RSNIE_WPA2_IE) {
		RSNIE2 *pRsnie_cipher = (RSNIE2 *) pRsnIe;
		UCHAR *rsnie_cipher_oui = (UCHAR *)pRsnie_cipher + offsetof(RSNIE2, ucast[0].oui);
		/* Assign the verson as 1*/
		pRsnie_cipher->version = 1;

		/* Group cipher */
		if (IS_CIPHER_CCMP256(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_CCMP256, 4);
		else if (IS_CIPHER_GCMP256(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_GCMP256, 4);
		else if (IS_CIPHER_CCMP128(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_CCMP128, 4);
		else if (IS_CIPHER_GCMP128(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_GCMP128, 4);
		else if (IS_CIPHER_TKIP(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_TKIP, 4);
		else if (IS_CIPHER_WEP104(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_WEP104, 4);
		else if (IS_CIPHER_WEP40(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_WEP40, 4);
		else {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "not support group cipher on WPA2 (GroupCipher=0x%x)\n", pSecConfig->GroupCipher);
			return FALSE;
		}

		/* Pairwise cipher */
		if (IS_CIPHER_CCMP256(pSecConfig->PairwiseCipher)) {
			NdisMoveMemory(rsnie_cipher_oui + PairwiseCnt * 4, OUI_WPA2_CIPHER_CCMP256, 4);
			PairwiseCnt++;
		}
		if (IS_CIPHER_GCMP256(pSecConfig->PairwiseCipher)) {
			NdisMoveMemory(rsnie_cipher_oui + PairwiseCnt * 4, OUI_WPA2_CIPHER_GCMP256, 4);
			PairwiseCnt++;
		}
		if (IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher)) {
			NdisMoveMemory(rsnie_cipher_oui + PairwiseCnt * 4, OUI_WPA2_CIPHER_CCMP128, 4);
			PairwiseCnt++;
		}
		if (IS_CIPHER_GCMP128(pSecConfig->PairwiseCipher)) {
			NdisMoveMemory(rsnie_cipher_oui + PairwiseCnt * 4, OUI_WPA2_CIPHER_GCMP128, 4);
			PairwiseCnt++;
		}
		if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher)) {
			if (IS_AKM_WPA3PSK(pSecConfig->AKMMap)) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
					"not support group cipher on SAE\n");
				return FALSE;
			}
			NdisMoveMemory(rsnie_cipher_oui + PairwiseCnt * 4, OUI_WPA2_CIPHER_TKIP, 4);
			PairwiseCnt++;
		}

		pRsnie_cipher->ucount = PairwiseCnt;
		*rsn_len = sizeof(RSNIE2) + (4 * (PairwiseCnt - 1));
		/* swap for big-endian platform*/
		pRsnie_cipher->version = cpu2le16(pRsnie_cipher->version);
		pRsnie_cipher->ucount = cpu2le16(pRsnie_cipher->ucount);
	}

	return TRUE;
}


static BOOLEAN WPAMakeRsnIeAKM(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN UINT32 wdev_type,
	IN UCHAR ie_idx,
	OUT UCHAR *rsn_len)
{
	SEC_RSNIE_TYPE ElememtId = pSecConfig->RSNE_Type[ie_idx];
	PRSNIE_AUTH pRsnie_auth = (RSNIE_AUTH *) (&pSecConfig->RSNE_Content[ie_idx][0] + (*rsn_len));
	UCHAR	*rsnie_auth_oui = (UCHAR *)pRsnie_auth + offsetof(RSNIE_AUTH, auth[0].oui);
	UCHAR	AkmCnt = 0;
	unsigned int offset = 0;

	if (ElememtId == SEC_RSNIE_WPA1_IE) {
		if (IS_AKM_WPA1(pSecConfig->AKMMap)) {
			NdisMoveMemory(rsnie_auth_oui, OUI_WPA_8021X_AKM, 4);
			AkmCnt++;
		} else if (IS_AKM_WPA1PSK(pSecConfig->AKMMap)) {
			NdisMoveMemory(rsnie_auth_oui, OUI_WPA_PSK_AKM, 4);
			AkmCnt++;
		} else if (IS_AKM_WPANONE(pSecConfig->AKMMap)) {
			NdisMoveMemory(rsnie_auth_oui, OUI_WPA_NONE_AKM, 4);
			AkmCnt++;
		} else {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
				"not support AKM on WPA1\n");
			return FALSE;
		}
	} else if (ElememtId == SEC_RSNIE_WPA2_IE) {
		if (IS_AKM_WPA2(pSecConfig->AKMMap) && pSecConfig->ft_only == FALSE) {
#ifdef DOT11W_PMF_SUPPORT
			if (pSecConfig->PmfCfg.UsePMFConnect
				&& (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA256)
				&& (wdev_type == WDEV_TYPE_STA)) {
				NdisMoveMemory(
					rsnie_auth_oui + offset, OUI_WPA2_AKM_8021X_SHA256, 4);
				AkmCnt++;
				offset += 4;
			} else if (pSecConfig->PmfCfg.PMFSHA256 && (wdev_type == WDEV_TYPE_AP)) {
				if ((pSecConfig->PmfCfg.MFPR == FALSE) ||
					(IS_AKM_WPA2(pSecConfig->AKMMap) && IS_AKM_WPA3(pSecConfig->AKMMap) &&
					 pSecConfig->PmfCfg.MFPR == TRUE)) {
					NdisMoveMemory(
						rsnie_auth_oui + offset, OUI_WPA2_AKM_8021X, 4);
					AkmCnt++;
					offset += 4;
				}

				NdisMoveMemory(
					rsnie_auth_oui + offset, OUI_WPA2_AKM_8021X_SHA256, 4);
				AkmCnt++;
				offset += 4;
#ifdef DOT11R_FT_SUPPORT

				if (IS_AKM_FT_WPA2(pSecConfig->AKMMap)) {
					NdisMoveMemory(
						rsnie_auth_oui + offset, OUI_WPA2_AKM_FT_8021X, 4);
					AkmCnt++;
					offset += 4;
				}
#endif /* DOT11R_FT_SUPPORT */
			} else
#endif /* DOT11W_PMF_SUPPORT */
			{
				NdisMoveMemory(rsnie_auth_oui + offset, OUI_WPA2_AKM_8021X, 4);
				AkmCnt++;
				offset += 4;
#ifdef DOT11R_FT_SUPPORT

				if (IS_AKM_FT_WPA2(pSecConfig->AKMMap)) {
					NdisMoveMemory(rsnie_auth_oui + offset,
								   OUI_WPA2_AKM_FT_8021X, 4);
					AkmCnt++;
					offset += 4;
				}

#endif /* DOT11R_FT_SUPPORT */

#ifdef OCE_FILS_SUPPORT
				if (IS_AKM_FILS_SHA256(pSecConfig->AKMMap)) {
					NdisMoveMemory(rsnie_auth_oui + offset,
								   OUI_WPA2_AKM_FILS_SHA256, 4);
					AkmCnt++;
					offset += 4;
				} else if (IS_AKM_FILS_SHA384(pSecConfig->AKMMap)) {
					NdisMoveMemory(rsnie_auth_oui + offset,
								   OUI_WPA2_AKM_FILS_SHA384, 4);
					AkmCnt++;
					offset += 4;
				}
#endif /* OCE_FILS_SUPPORT */
#ifdef CONFIG_HOTSPOT_R3
				if (IS_AKM_OSEN(pSecConfig->AKMMap)) {
					NdisMoveMemory(rsnie_auth_oui + offset,
									OUI_OSEN_AKM, 4);
					AkmCnt++;
					offset += 4;
				}
#endif
			}
		} else if (IS_AKM_FT_WPA2(pSecConfig->AKMMap)) {
			NdisMoveMemory(rsnie_auth_oui + offset, OUI_WPA2_AKM_FT_8021X, 4);
			AkmCnt++;
			offset += 4;
		}

		if (IS_AKM_WPA2PSK(pSecConfig->AKMMap) && pSecConfig->ft_only == FALSE) {
#ifdef DOT11W_PMF_SUPPORT
			if (pSecConfig->PmfCfg.UsePMFConnect
				&& (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA256)
				&& (wdev_type == WDEV_TYPE_STA)) {
				NdisMoveMemory(rsnie_auth_oui + offset, OUI_WPA2_AKM_PSK_SHA256, 4);
				AkmCnt++;
				offset += 4;
			} else if (pSecConfig->PmfCfg.PMFSHA256 && (wdev_type == WDEV_TYPE_AP)) {
				if (pSecConfig->PmfCfg.MFPR == FALSE) {
					NdisMoveMemory(
						rsnie_auth_oui + offset, OUI_WPA2_AKM_PSK, 4);
					AkmCnt++;
					offset += 4;
				}

				NdisMoveMemory(rsnie_auth_oui + offset, OUI_WPA2_AKM_PSK_SHA256, 4);
				AkmCnt++;
				offset += 4;
#ifdef DOT11R_FT_SUPPORT
				if (IS_AKM_FT_WPA2PSK(pSecConfig->AKMMap)) {
					NdisMoveMemory(rsnie_auth_oui + offset,
								   OUI_WPA2_AKM_FT_PSK, 4);
					AkmCnt++;
					offset += 4;
				}

#endif /* DOT11R_FT_SUPPORT */
			} else
#endif /* DOT11W_PMF_SUPPORT */
			{
				NdisMoveMemory(rsnie_auth_oui + offset, OUI_WPA2_AKM_PSK, 4);
				AkmCnt++;
				offset += 4;
#ifdef DOT11R_FT_SUPPORT

				if (IS_AKM_FT_WPA2PSK(pSecConfig->AKMMap)) {
					NdisMoveMemory(rsnie_auth_oui + offset,
								   OUI_WPA2_AKM_FT_PSK, 4);
					AkmCnt++;
					offset += 4;
				}

#endif /* DOT11R_FT_SUPPORT */
			}
		} else if (IS_AKM_FT_WPA2PSK(pSecConfig->AKMMap)) {
			NdisMoveMemory(rsnie_auth_oui + offset, OUI_WPA2_AKM_FT_PSK, 4);
			AkmCnt++;
			offset += 4;
		}

		if (IS_AKM_WPA2_SHA256(pSecConfig->AKMMap)) {
			NdisMoveMemory(rsnie_auth_oui + offset, OUI_WPA2_AKM_8021X_SHA256, 4);
			AkmCnt++;
			offset += 4;
		}

		/*prevent insert akm6 twice*/
		if (IS_AKM_WPA2PSK_SHA256(pSecConfig->AKMMap) && !(IS_AKM_WPA2PSK(pSecConfig->AKMMap) && pSecConfig->PmfCfg.PMFSHA256)) {
			NdisMoveMemory(rsnie_auth_oui + offset, OUI_WPA2_AKM_PSK_SHA256, 4);
			AkmCnt++;
			offset += 4;
		}

#ifdef DOT11_SAE_SUPPORT

		if (IS_AKM_SAE_SHA256(pSecConfig->AKMMap) && pSecConfig->ft_only == FALSE) {
			NdisMoveMemory(rsnie_auth_oui + offset, OUI_WPA2_AKM_SAE_SHA256, 4);
			AkmCnt++;
			offset += 4;
		}

		if (IS_AKM_SAE_EXT(pSecConfig->AKMMap) && pSecConfig->ft_only == FALSE) {
			NdisMoveMemory(rsnie_auth_oui + 4 * AkmCnt, OUI_WPA2_AKM_SAE_EXT, 4);
			AkmCnt++;
			offset += 4;
		}

#ifdef DOT11R_FT_SUPPORT
		if (IS_AKM_FT_SAE_SHA256(pSecConfig->AKMMap)) {
			NdisMoveMemory(rsnie_auth_oui + offset, OUI_WPA2_AKM_FT_SAE_SHA256, 4);
			AkmCnt++;
			offset += 4;
		}

		if (IS_AKM_FT_SAE_EXT(pSecConfig->AKMMap)) {
			NdisMoveMemory(rsnie_auth_oui + 4 * AkmCnt, OUI_WPA2_AKM_FT_SAE_EXT, 4);
			AkmCnt++;
			offset += 4;
		}
#endif /* DOT11R_FT_SUPPORT */
#endif /* DOT11_SAE_SUPPORT */

		if (IS_AKM_DPP(pSecConfig->AKMMap)) {
			NdisMoveMemory(rsnie_auth_oui + offset, DPP_OUI, 4);
			AkmCnt++;
			offset += 4;
		}

#if defined(DOT11_SUITEB_SUPPORT) || defined(HOSTAPD_SUITEB_SUPPORT)

		if (IS_AKM_SUITEB_SHA256(pSecConfig->AKMMap) && pSecConfig->ft_only == FALSE) {
			NdisMoveMemory(rsnie_auth_oui + offset, OUI_WPA2_AKM_SUITEB_SHA256, 4);
			AkmCnt++;
			offset += 4;
		}

		if (IS_AKM_SUITEB_SHA384(pSecConfig->AKMMap)) {
			NdisMoveMemory(rsnie_auth_oui + offset, OUI_WPA2_AKM_SUITEB_SHA384, 4);
			AkmCnt++;
			offset += 4;
		}

#endif /* DOT11_SUITEB_SUPPORT */
#ifdef DOT11R_FT_SUPPORT

		if (IS_AKM_FT_WPA2_SHA384(pSecConfig->AKMMap)) {
			NdisMoveMemory(rsnie_auth_oui + offset, OUI_WPA2_AKM_FT_8021X_SHA384, 4);
			AkmCnt++;
			offset += 4;
		}

#endif /* DOT11R_FT_SUPPORT */
		if (IS_AKM_OWE(pSecConfig->AKMMap)) {
			NdisMoveMemory(rsnie_auth_oui + offset, OUI_WPA2_AKM_OWE, 4);
			AkmCnt++;
			offset += 4;
		}

	}

	pRsnie_auth->acount = AkmCnt;
	pRsnie_auth->acount = cpu2le16(pRsnie_auth->acount);
	/* update current RSNIE length*/
	(*rsn_len) += (sizeof(RSNIE_AUTH) + (4 * (AkmCnt - 1)));
	return TRUE;
}


static BOOLEAN WPAMakeRsnIeCap(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN UCHAR ie_idx,
	OUT UCHAR *rsn_len)
{
	SEC_RSNIE_TYPE ElememtId = pSecConfig->RSNE_Type[ie_idx];
#if defined(CCN34_SPLIT_MAC_SUPPORT) || !defined(DISABLE_HOSTAPD_BEACON)
	RSN_CAPABILITIES *pRSN_Cap = (RSN_CAPABILITIES *) (&pSecConfig->RSNE_Content[ie_idx][0] + (*rsn_len));
#endif

	if (ElememtId == SEC_RSNIE_WPA2_IE) {
#if defined(DISABLE_HOSTAPD_BEACON) && !defined(CCN34_SPLIT_MAC_SUPPORT)
	memcpy((&pSecConfig->RSNE_Content[ie_idx][0] + (*rsn_len)), pSecConfig->RsnCap, 2);
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "[RSN IE CAP]: RSN CAP %02x %02x\n", pSecConfig->RsnCap[0], pSecConfig->RsnCap[1]);
#else
#if defined(DOT1X_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
		pRSN_Cap->field.PreAuth = (pSecConfig->PreAuth == TRUE) ? 1 : 0;
#endif /* DOT1X_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT
		pRSN_Cap->field.MFPC = (pSecConfig->PmfCfg.MFPC) ? 1 : 0;
		pRSN_Cap->field.MFPR = (pSecConfig->PmfCfg.MFPR) ? 1 : 0;

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF] RSNIE Capability MFPC=%d, MFPR=%d\n", pRSN_Cap->field.MFPC, pRSN_Cap->field.MFPR);
#endif /* DOT11W_PMF_SUPPORT */
		pRSN_Cap->field.ocvc = (pSecConfig->ocv_support) ? 1 : 0;
		pRSN_Cap->word = cpu2le16(pRSN_Cap->word);
#endif /*DISABLE_HOSTAPD_BEACON*/
		(*rsn_len) += sizeof(RSN_CAPABILITIES); /* update current RSNIE length*/
	}

	return TRUE;
}


static BOOLEAN WPAInsertRsnIePMKID(
	IN PMAC_TABLE_ENTRY pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN UCHAR ie_idx,
	OUT UCHAR *rsn_len)
{

	SEC_RSNIE_TYPE ElememtId = pSecConfig->RSNE_Type[ie_idx];
	UINT8 *pBuf = (&pSecConfig->RSNE_Content[ie_idx][0] + (*rsn_len));

#ifdef CONFIG_HOTSPOT_R3
	if (pSecConfig->bIsWPA2EntOSEN) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_ERROR,
			"No PMKID in RSN as OSEN is enabled\n");
		return TRUE;
	}
#endif
	if (ElememtId == SEC_RSNIE_WPA2_IE) {
#ifdef DOT11R_FT_SUPPORT
		if (pEntry && IS_ENTRY_CLIENT(pEntry) &&  pEntry->FT_Status != 0) {
			PUINT8	pmkid_ptr = NULL;
			UINT8	pmkid_len = 0;
			UINT8	extra_len = 0;
			UINT16	pmk_count = 0;

			if (pEntry->FT_Status == TX_AUTH_RSP)
				pmkid_ptr = pEntry->FT_PMK_R0_NAME;
			else
				pmkid_ptr = pEntry->FT_PMK_R1_NAME;

			pmkid_len = LEN_PMK_NAME;

			if (pmkid_len > 0 && ((pmkid_len & 0x0f) == 0)) {
				extra_len = sizeof(UINT16) + pmkid_len;

				pmk_count = (pmkid_len >> 4);
				pmk_count = cpu2le16(pmk_count);
			} else {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
					"no FT PMKID-List included(%d).\n", pmkid_len);
			}

		    /* Insert PMKID-List field */
			if (extra_len > 0) {
				hex_dump("FT_PMKID", pmkid_ptr, pmkid_len);

				NdisMoveMemory(pBuf, &pmk_count, 2);
				NdisMoveMemory((pBuf+2), pmkid_ptr, pmkid_len);
				(*rsn_len) += extra_len;
			}
		} else
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11_SAE_SUPPORT
		if (IS_AKM_SAE(pSecConfig->AKMMap)
			&& pEntry && (IS_ENTRY_CLIENT(pEntry)
						|| IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)
						) && is_pmkid_cache_in_sec_config(pSecConfig)) {

			UINT16	pmk_count = 1;

			pmk_count = cpu2le16(pmk_count);
			NdisMoveMemory(pBuf, &pmk_count, sizeof(pmk_count));
			NdisMoveMemory(pBuf+sizeof(pmk_count), pSecConfig->pmkid, LEN_PMKID);
			(*rsn_len) += sizeof(pmk_count) + LEN_PMKID;
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_ERROR,
					" (SAE) including the PMKID.\n");
		} else
#endif

#ifdef DPP_SUPPORT
		if (IS_AKM_DPP(pSecConfig->AKMMap)
			&& pEntry && (IS_ENTRY_CLIENT(pEntry)
						|| IS_ENTRY_PEER_AP(pEntry)
						) && is_pmkid_cache_in_sec_config(pSecConfig)) {

			UINT16	pmk_count = 1;

			pmk_count = cpu2le16(pmk_count);
			NdisMoveMemory(pBuf, &pmk_count, sizeof(pmk_count));
			NdisMoveMemory(pBuf+sizeof(pmk_count), pSecConfig->pmkid, LEN_PMKID);
			(*rsn_len) += sizeof(pmk_count) + LEN_PMKID;
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_ERROR,
					"(DPP) including the PMKID.\n");
		} else
#endif /* DPP_SUPPORT */

		if (IS_AKM_OWE(pSecConfig->AKMMap)
			&& pEntry && (IS_ENTRY_CLIENT(pEntry)
			|| IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)
			) && is_pmkid_cache_in_sec_config(pSecConfig)) {
			UINT16	pmk_count = 1;

			pmk_count = cpu2le16(pmk_count);
			NdisMoveMemory(pBuf, &pmk_count, sizeof(pmk_count));
			NdisMoveMemory(pBuf+sizeof(pmk_count), pSecConfig->pmkid, LEN_PMKID);
			(*rsn_len) += sizeof(pmk_count) + LEN_PMKID;
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_ERROR,
					"(OWE) including the PMKID.\n");
		} else
#ifdef DOT11W_PMF_SUPPORT
		/* http://w1.fi/cgit/hostap/commit/?id=44fa5e747b7aca39285e2511d5c94684e0723b6b */
		if (pSecConfig->PmfCfg.MFPC
#ifdef RT_CFG80211_SUPPORT
		&& !IS_CIPHER_BIP_CMAC128(pSecConfig->PmfCfg.igtk_cipher)
#endif
		) {
			UCHAR ZeroPmkID[2] = {0x00, 0x00};

			NdisMoveMemory(pBuf, ZeroPmkID, 2);
			(*rsn_len) += 2;
		} else
#endif /* DOT11W_PMF_SUPPORT */
		if (0)
			; /* for build pass */
	}

	return TRUE;
}


VOID WPAMakeRSNIE (
	IN UINT32 wdev_type,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN PMAC_TABLE_ENTRY pEntry)
{
	UCHAR ie_idx = 0;

	/* Initiate some related information */
	for (ie_idx = 0; ie_idx < SEC_RSNIE_NUM; ie_idx++) {
		pSecConfig->RSNE_Type[ie_idx] = SEC_RSNIE_NONE;
		pSecConfig->RSNE_Len[ie_idx] = 0;
		NdisZeroMemory(pSecConfig->RSNE_Content[ie_idx], MAX_LEN_OF_RSNIE);
	}

	/* Check AKM support per wdev type  */
	switch (wdev_type) {
#ifdef CONFIG_AP_SUPPORT

	case WDEV_TYPE_AP:
		if ((pSecConfig->AKMMap & AKM_AP_MASK) == 0)
			return;

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "==>(AP)\n");
		break;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	case WDEV_TYPE_STA:
		if ((pSecConfig->AKMMap & AKM_STA_MASK) == 0)
					return;

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "==>(STA)\n");
		break;
#endif /* CONFIG_STA_SUPPORT */
	}

	if (IS_AKM_WPA1(pSecConfig->AKMMap)
		|| IS_AKM_WPA1PSK(pSecConfig->AKMMap)
		|| IS_AKM_WPANONE(pSecConfig->AKMMap)) {
		pSecConfig->RSNE_Type[0] = SEC_RSNIE_WPA1_IE;
		pSecConfig->RSNE_EID[0][0] = IE_WPA;
	}


	if (IS_AKM_WPA2(pSecConfig->AKMMap) ||
	    IS_AKM_WPA2PSK(pSecConfig->AKMMap) ||
	    IS_AKM_WPA3PSK(pSecConfig->AKMMap) ||
#ifdef DOT11R_FT_SUPPORT
	    IS_AKM_FT_WPA2PSK(pSecConfig->AKMMap) ||
	    IS_AKM_FT_SAE_EXT(pSecConfig->AKMMap) ||
#endif /* DOT11R_FT_SUPPORT */
	    IS_AKM_DPP(pSecConfig->AKMMap) ||
	    IS_AKM_WPA3_192BIT(pSecConfig->AKMMap) ||
	    IS_AKM_OWE(pSecConfig->AKMMap)) {
		pSecConfig->RSNE_Type[1] = SEC_RSNIE_WPA2_IE;
		pSecConfig->RSNE_EID[1][0] = IE_WPA2;
	}

	for (ie_idx = 0; ie_idx < SEC_RSNIE_NUM; ie_idx++) {
		UCHAR p_offset = 0;

		if (pSecConfig->RSNE_Type[ie_idx] == SEC_RSNIE_NONE)
			continue;

		else {
			/* Build the primary RSNIE*/
			/* 1. insert cipher suite*/
			if (WPAMakeRsnIeCipher(pSecConfig, ie_idx, &p_offset) == FALSE)
				continue;

			ASSERT(p_offset < 255);

			if (p_offset >= 255)
				continue;

			/* 2. insert AKM*/
			if (WPAMakeRsnIeAKM(pSecConfig, wdev_type, ie_idx, &p_offset) == FALSE)
				continue;

			ASSERT(p_offset < 255);

			if (p_offset >= 255)
				continue;

			/* 3. insert capability*/
			if (WPAMakeRsnIeCap(pSecConfig, ie_idx, &p_offset) == FALSE)
				continue;

			ASSERT(p_offset < 255);

			if (p_offset >= 255)
				continue;

			/* 4. Insert PMKID */
			if (WPAInsertRsnIePMKID(pEntry, pSecConfig, ie_idx, &p_offset) == FALSE)
				continue;

			ASSERT(p_offset < 255);

			if (p_offset >= 255)
				continue;

#ifdef DOT11W_PMF_SUPPORT

			/* 5. Insert Group Management Cipher*/
			if (PMF_MakeRsnIeGMgmtCipher(pSecConfig, ie_idx, &p_offset) == FALSE)
				continue;

			ASSERT(p_offset < 255);

			if (p_offset >= 255)
				continue;

#endif /* DOT11W_PMF_SUPPORT */
			pSecConfig->RSNE_Len[ie_idx] = p_offset;
			hex_dump("The RSNE", &pSecConfig->RSNE_Content[ie_idx][0], pSecConfig->RSNE_Len[ie_idx]);
		}
	}
}

BOOLEAN WPACheckGroupCipher(
	IN struct _SECURITY_CONFIG *pSecConfigSelf,
	IN struct _SECURITY_CONFIG *pSecConfigEntry,
	IN PEID_STRUCT eid_ptr)
{
	CHAR *pCipher = NULL;

	if (eid_ptr->Len >= 6) {
		/* WPA and WPA2 format not the same in RSN_IE */
		if (eid_ptr->Eid == IE_WPA) {
			if (IS_CIPHER_TKIP(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA_TKIP;
				SET_CIPHER_TKIP(pSecConfigEntry->GroupCipher);
			} else if (IS_CIPHER_CCMP128(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA_CCMP;
				SET_CIPHER_CCMP128(pSecConfigEntry->GroupCipher);
			}

			if (pCipher && NdisEqualMemory(&eid_ptr->Octet[6], pCipher, 4))
				return TRUE;
		} else if (eid_ptr->Eid == IE_WPA2) {
			if (IS_CIPHER_WEP40(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA2_CIPHER_WEP40;
				SET_CIPHER_WEP40(pSecConfigEntry->GroupCipher);
			} else if (IS_CIPHER_WEP104(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA2_CIPHER_WEP104;
				SET_CIPHER_WEP104(pSecConfigEntry->GroupCipher);
			} else if (IS_CIPHER_TKIP(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA2_CIPHER_TKIP;
				SET_CIPHER_TKIP(pSecConfigEntry->GroupCipher);
			} else if (IS_CIPHER_CCMP128(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA2_CIPHER_CCMP128;
				SET_CIPHER_CCMP128(pSecConfigEntry->GroupCipher);
			} else if (IS_CIPHER_CCMP256(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA2_CIPHER_CCMP256;
				SET_CIPHER_CCMP256(pSecConfigEntry->GroupCipher);
			} else if (IS_CIPHER_GCMP128(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA2_CIPHER_GCMP128;
				SET_CIPHER_GCMP128(pSecConfigEntry->GroupCipher);
			} else if (IS_CIPHER_GCMP256(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA2_CIPHER_GCMP256;
				SET_CIPHER_GCMP256(pSecConfigEntry->GroupCipher);
			}

			if (pCipher && NdisEqualMemory(&eid_ptr->Octet[2], pCipher, 4))
				return TRUE;
		}
	}

	CLEAR_GROUP_CIPHER(pSecConfigEntry);
	return FALSE;
}


BOOLEAN WPACheckUcast(
	IN struct wifi_dev *wdev,
	IN struct _SECURITY_CONFIG *pSecConfigEntry,
	IN PEID_STRUCT eid_ptr)
{
	PUCHAR pStaTmp;
	USHORT Count, i;
	struct _SECURITY_CONFIG *pSecConfigSelf;
#ifdef CFG_RSNO_SUPPORT
	struct _SECURITY_CONFIG *pSecConfigSelf_ext;
#endif /* CFG_RSNO_SUPPORT */

	/* Store STA RSN_IE capability */
	pStaTmp = (PUCHAR)&eid_ptr->Octet[0];

	if (eid_ptr->Eid == IE_WPA2) {
		/* skip Version(2),Multicast cipter(4) 2+4==6 */
		/* point to number of unicast */
		pStaTmp += 6;
	} else if (eid_ptr->Eid == IE_WPA) {
		/* skip OUI(4),Vesrion(2),Multicast cipher(4) 4+2+4==10 */
		/* point to number of unicast */
		pStaTmp += 10;
	} else {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "[ERROR] invalid IE=%d\n", eid_ptr->Eid);
		return FALSE;
	}

	/* Store unicast cipher count */
	NdisMoveMemory(&Count, pStaTmp, sizeof(USHORT));
	Count = cpu2le16(Count);
	/* pointer to unicast cipher */
	pStaTmp += sizeof(USHORT);

	for (i = 0; i < Count; i++) {
		if (eid_ptr->Eid == IE_WPA) {
			if (NdisEqualMemory(pStaTmp, &OUI_WPA_TKIP, 4))
				SET_CIPHER_TKIP(pSecConfigEntry->PairwiseCipher);
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA_CCMP, 4))
				SET_CIPHER_CCMP128(pSecConfigEntry->PairwiseCipher);
		} else if (eid_ptr->Eid == IE_WPA2) {
			if (NdisEqualMemory(pStaTmp, &OUI_WPA2_CIPHER_TKIP, 4))
				SET_CIPHER_TKIP(pSecConfigEntry->PairwiseCipher);
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_CIPHER_CCMP128, 4))
				SET_CIPHER_CCMP128(pSecConfigEntry->PairwiseCipher);
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_CIPHER_CCMP256, 4))
				SET_CIPHER_CCMP256(pSecConfigEntry->PairwiseCipher);
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_CIPHER_GCMP128, 4))
				SET_CIPHER_GCMP128(pSecConfigEntry->PairwiseCipher);
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_CIPHER_GCMP256, 4))
				SET_CIPHER_GCMP256(pSecConfigEntry->PairwiseCipher);
		}

		pStaTmp += 4;
		Count--;
	}

	pSecConfigSelf = &wdev->SecConfig;
	if ((pSecConfigSelf->PairwiseCipher & pSecConfigEntry->PairwiseCipher) > 0)
		return TRUE;

#ifdef CFG_RSNO_SUPPORT
	pSecConfigSelf_ext = &wdev->SecConfig_ext;
	if ((pSecConfigSelf_ext->PairwiseCipher & pSecConfigEntry->PairwiseCipher) > 0)
		return TRUE;
#endif /* CFG_RSNO_SUPPORT */

	CLEAR_PAIRWISE_CIPHER(pSecConfigEntry);
	return FALSE;
}


BOOLEAN WPACheckAKM(
	IN struct wifi_dev *wdev,
	IN struct _SECURITY_CONFIG *pSecConfigEntry,
	IN PEID_STRUCT eid_ptr)
{
	PUCHAR pStaTmp;
	USHORT Count, i;
	UINT32 self_akmmap;
#ifdef CFG_RSNO_SUPPORT
	UINT32 self_akmmap_ext;
#endif /* CFG_RSNO_SUPPORT */

	/* Store STA RSN_IE capability */
	pStaTmp = (PUCHAR)&eid_ptr->Octet[0];

	if (eid_ptr->Eid == IE_WPA2) {
		/* skip Version(2),Multicast cipter(4) 2+4==6 */
		/* point to number of unicast */
		pStaTmp += 6;
	} else if (eid_ptr->Eid == IE_WPA) {
		/* skip OUI(4),Vesrion(2),Multicast cipher(4) 4+2+4==10 */
		/* point to number of unicast */
		pStaTmp += 10;
	} else {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "Unknown WPAIE, WPAIE=%d\n", eid_ptr->Eid);
		return FALSE;
	}

	/* Store unicast cipher count */
	NdisMoveMemory(&Count, pStaTmp, sizeof(USHORT));
	Count = cpu2le16(Count);
	/* pointer to unicast cipher */
	pStaTmp += sizeof(USHORT);
	/* Skip all unicast cipher suite */
	pStaTmp += 4 * Count;
	/* Store AKM count */
	NdisMoveMemory(&Count, pStaTmp, sizeof(USHORT));
	Count = cpu2le16(Count);
	/*pointer to AKM cipher */
	pStaTmp += sizeof(USHORT);

	/* Ellis todo: if akm count > 1, key_deri_alg will be overwriten, pmf akm workaround need to be fixed first */
	for (i = 0; i < Count; i++) {
		if (eid_ptr->Eid == IE_WPA) {
			if (NdisEqualMemory(pStaTmp, &OUI_WPA_8021X_AKM, 4))
				SET_AKM_WPA1(pSecConfigEntry->AKMMap);
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA_PSK_AKM, 4))
				SET_AKM_WPA1PSK(pSecConfigEntry->AKMMap);
		} else if (eid_ptr->Eid == IE_WPA2) {
			if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_8021X, 4)) {
				SET_AKM_WPA2(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA1;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_PSK, 4)) {
				SET_AKM_WPA2PSK(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA1;
			}

#ifdef DOT11R_FT_SUPPORT
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_FT_8021X, 4)) {
				SET_AKM_FT_WPA2(pSecConfigEntry->AKMMap);
				SET_AKM_WPA2(pSecConfigEntry->AKMMap);
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_FT_PSK, 4)) {
				SET_AKM_FT_WPA2PSK(pSecConfigEntry->AKMMap);
				SET_AKM_WPA2PSK(pSecConfigEntry->AKMMap);
			}

#endif /* DOT11R_FT_SUPPORT */
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_8021X_SHA256, 4)) {
#ifdef DOT11W_PMF_SUPPORT
				SET_AKM_WPA2(pSecConfigEntry->AKMMap);
#else
				SET_AKM_WPA2_SHA256(pSecConfigEntry->AKMMap);
#endif /* DOT11W_PMF_SUPPORT */
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_PSK_SHA256, 4)) {
#ifdef DOT11W_PMF_SUPPORT
				SET_AKM_WPA2PSK(pSecConfigEntry->AKMMap);
#else
				SET_AKM_WPA2PSK_SHA256(pSecConfigEntry->AKMMap);
#endif /* DOT11W_PMF_SUPPORT */
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_TDLS, 4))
				SET_AKM_TDLS(pSecConfigEntry->AKMMap);
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_SAE_SHA256, 4)) {
				SET_AKM_SAE_SHA256(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_FT_SAE_SHA256, 4)) {
				SET_AKM_FT_SAE_SHA256(pSecConfigEntry->AKMMap);
				SET_AKM_SAE_SHA256(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_SAE_EXT, 4)) {
				SET_AKM_SAE_EXT(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA384;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_FT_SAE_EXT, 4)) {
				SET_AKM_FT_SAE_EXT(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA384;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_SUITEB_SHA256, 4)) {
				SET_AKM_SUITEB_SHA256(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_SUITEB_SHA384, 4)) {
				SET_AKM_SUITEB_SHA384(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA384;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_FT_8021X_SHA384, 4)) {
				SET_AKM_FT_WPA2_SHA384(pSecConfigEntry->AKMMap);
				SET_AKM_SUITEB_SHA384(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA384;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_OWE, 4)) {
				SET_AKM_OWE(pSecConfigEntry->AKMMap);
				/* OWE cannot derive key_deri_alg by OWE akm.
				 * it shall parse ECDH parameter to determine it.
				 * set a temporary alg here.
				 */
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
			}
#ifdef OCE_FILS_SUPPORT
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_FILS_SHA256, 4)) {
				SET_AKM_FILS_SHA256(pSecConfigEntry->AKMMap);
				SET_AKM_WPA2(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_FILS_SHA384, 4)) {
				SET_AKM_FILS_SHA384(pSecConfigEntry->AKMMap);
				SET_AKM_WPA2(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA384;
			}
#endif /* OCE_FILS_SUPPORT */
			else if (NdisEqualMemory(pStaTmp, DPP_OUI, 4)) {
				SET_AKM_DPP(pSecConfigEntry->AKMMap);
				/* TODO, need to check for prime number and select kdf accordingly*/
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
					"Setting dpp akm\n");
			}

#ifdef CONFIG_HOTSPOT_R3
			else if (NdisEqualMemory(pStaTmp, &OUI_OSEN_AKM, 4)) {
				pSecConfigEntry->bIsWPA2EntOSEN = TRUE;
				SET_AKM_OSEN(pSecConfigEntry->AKMMap);
				SET_AKM_WPA2(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA1;
			}
#endif
		}

		pStaTmp += 4;
	}

	self_akmmap = wdev->SecConfig.AKMMap;
#ifdef CFG_RSNO_SUPPORT
	self_akmmap_ext = wdev->SecConfig_ext.AKMMap;
#endif /* CFG_RSNO_SUPPORT */

	if (wdev->SecConfig.ft_only)
		CLEAR_NONFT_AKM(self_akmmap);

	if ((self_akmmap & pSecConfigEntry->AKMMap) > 0)
		return TRUE;
#ifdef CFG_RSNO_SUPPORT
	else if ((self_akmmap_ext & pSecConfigEntry->AKMMap) > 0)
		return TRUE;
#endif /* CFG_RSNO_SUPPORT */
	CLEAR_SEC_AKM(pSecConfigEntry->AKMMap);
	return FALSE;
}


static BOOLEAN wpa_check_pmkid(
	IN PUINT8 rsnie_ptr,
	IN UINT rsnie_len,
	IN UINT32 akm)
{
	UINT8 count = 0;
	PUINT8 pBuf = NULL;

	if (IS_AKM_WPA2PSK(akm)) {
#ifdef DOT11R_FT_SUPPORT
		if (IS_AKM_FT_WPA2PSK(akm))
			return TRUE;
#endif /* DOT11R_FT_SUPPORT */
		pBuf = WPA_ExtractSuiteFromRSNIE(rsnie_ptr, rsnie_len, PMKID_LIST, &count);

		if (count > 0)
			return FALSE;
	}

	return TRUE;
}

BOOLEAN wpa_check_rsn_cap(
	IN struct wifi_dev *wdev,
	IN struct _SECURITY_CONFIG *sec_cfg_entry,
	IN PUINT8 rsnie_ptr,
	IN UINT rsnie_len)
{
	UINT8 count = 0;
	PUINT8 pBuf = NULL;
	RSN_CAPABILITIES rsn_cap;
	struct _SECURITY_CONFIG *sec_cfg_self = &wdev->SecConfig;
#ifdef CFG_RSNO_SUPPORT
	struct _SECURITY_CONFIG *sec_cfg_self_ext = &wdev->SecConfig_ext;
#endif /* CFG_RSNO_SUPPORT */

	pBuf = WPA_ExtractSuiteFromRSNIE(rsnie_ptr, rsnie_len, RSN_CAP_INFO, &count);

	if (pBuf == NULL)
		return FALSE;

	NdisMoveMemory(&rsn_cap, pBuf, sizeof(RSN_CAPABILITIES));
	rsn_cap.word = cpu2le16(rsn_cap.word);

	if (sec_cfg_self->ocv_support
#ifdef CFG_RSNO_SUPPORT
		|| sec_cfg_self_ext->ocv_support
#endif /* CFG_RSNO_SUPPORT */
		)
		sec_cfg_entry->ocv_support = rsn_cap.field.ocvc ? TRUE : FALSE;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_INFO,
		"ocv support %d\n", sec_cfg_entry->ocv_support);

	return TRUE;
}


/* Ellis: not ready for WAPI */
BOOLEAN wpa_rsne_sanity(
	IN PUCHAR rsnie_ptr,
	IN UCHAR rsnie_len,
	OUT UCHAR *end_field)
{
	EID_STRUCT  *eid_ptr;
	PUCHAR pStaTmp;
	USHORT ver;
	USHORT Count;
	UINT32 len = 0;

	eid_ptr = (EID_STRUCT *)rsnie_ptr;

	if ((rsnie_len < 2) || (eid_ptr->Len + 2) != rsnie_len) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : the len is invalid !!!\n");
		return FALSE;
	}

	if (eid_ptr->Len < MIN_LEN_OF_RSNIE) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : len is too short(len = %d) !!!\n", eid_ptr->Len);
		return FALSE;
	}

	/* Store STA RSN_IE capability */
	pStaTmp = (PUCHAR)&eid_ptr->Octet[0];

	/* check Element ID */
	if (eid_ptr->Eid == IE_WPA2)
		;
	else if (eid_ptr->Eid == IE_WPA) {
		/* skip OUI(4) */
		pStaTmp += 4;
		len += 4;
	} else if (eid_ptr->Eid == IE_WAPI)
		return TRUE;
	else {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : invalid IE=%d\n", eid_ptr->Eid);
		return FALSE;
	}

	/* check version */
	len += 2;
	if (eid_ptr->Len < len) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : no version(len = %d)\n", eid_ptr->Len);
		return FALSE;
	}
	NdisMoveMemory(&ver, pStaTmp, sizeof(ver));
	ver = cpu2le16(ver);

	if (ver != 1) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : unknown version(%d)\n", ver);
		return FALSE;
	}

	if (eid_ptr->Len == len) {
		/* None of the optional fields are included in the RSNE */
		*end_field = RSN_FIELD_NONE;
		return TRUE;
	}

	pStaTmp += sizeof(USHORT);

	/* check group cipher suite */
	len += LEN_OUI_SUITE;
	if (eid_ptr->Len < len) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : group cipher is truncated(len = %d)\n", eid_ptr->Len);
		return FALSE;
	} else if (eid_ptr->Len == len) {
		/* Group Data Cipher Suite are included in the RSNE */
		*end_field = RSN_FIELD_GROUP_CIPHER;
		return TRUE;
	}

	pStaTmp += LEN_OUI_SUITE;

	/* check pairwise cipher suite */
	len += 2;
	if (eid_ptr->Len < len) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] :  pairwise cipher suite cnt is truncated(%d)\n", eid_ptr->Len);
		return FALSE;
	}

	/* Store pairwise cipher count */
	NdisMoveMemory(&Count, pStaTmp, sizeof(USHORT));
	Count = cpu2le16(Count);

	len += Count * LEN_OUI_SUITE;

	if (eid_ptr->Len < len) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : Pairwise Cipher Suite is truncated(len %d)\n", eid_ptr->Len);
		return FALSE;
	} else if (eid_ptr->Len == len) {
		/* Pairwise Cipher Suite are included in the RSNE */
		*end_field = RSN_FIELD_PAIRWISE_CIPHER;
		return TRUE;
	}

	pStaTmp += sizeof(USHORT) + Count * LEN_OUI_SUITE;

	/* check akm suite */
	len += 2;
	if (eid_ptr->Len < len) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] :  akm suite cnt is truncated(%d)\n", eid_ptr->Len);
		return FALSE;
	}

	/* Store akm count */
	NdisMoveMemory(&Count, pStaTmp, sizeof(USHORT));
	Count = cpu2le16(Count);

	len += Count * LEN_OUI_SUITE;
	if (eid_ptr->Len < len) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : Akm Suite is truncated(len %d)\n", eid_ptr->Len);
		return FALSE;
	} else if (eid_ptr->Len == len) {
		/* akm Suite are included in the RSNE */
		*end_field = RSN_FIELD_AKM;
		return TRUE;
	}

	pStaTmp += sizeof(USHORT) + Count * LEN_OUI_SUITE;

	/* check rsn capabilities */
	len += 2;
	if (eid_ptr->Len < len) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : RSN capabilities is truncated(len %d)\n",  eid_ptr->Len);
		return FALSE;
	} else if (eid_ptr->Len == len) {
		if (eid_ptr->Eid == IE_WPA) {
			*end_field = RSN_FIELD_AKM;
		} else {
			/* rsn capabilities are included in the RSNE */
			*end_field = RSN_FIELD_RSN_CAP;
		}
		return TRUE;
	}

	pStaTmp += 2;

	/* check PMKID */
	len += 2;
	if (eid_ptr->Len < len) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] :  pmkid cnt is truncated(%d)\n",  eid_ptr->Len);
		return FALSE;
	}

	/* Store pmkid count */
	NdisMoveMemory(&Count, pStaTmp, sizeof(USHORT));
	Count = cpu2le16(Count);

	len += Count * 16;

	if (eid_ptr->Len < len) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : PMKID is truncated(len %d)\n",  eid_ptr->Len);
		return FALSE;
	} else if (eid_ptr->Len == len) {
		/* PMKID are included in the RSNE */
		*end_field = RSN_FIELD_PMKID;
		return TRUE;
	}

	pStaTmp += 2 + Count * 16;


#ifdef DOT11W_PMF_SUPPORT
	/* check Group Management Cipher Suite*/
	len += LEN_OUI_SUITE;
	if (eid_ptr->Len < len) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : Group Management Cipher Suite is truncated(len %d)\n",  eid_ptr->Len);
		return FALSE;
	} else if (eid_ptr->Len == len) {
		/* PMKID are included in the RSNE */
		*end_field = RSN_FIELD_GROUP_MGMT_CIPHER;
		return TRUE;
	}

	pStaTmp += LEN_OUI_SUITE;
#endif

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"extensible element len %d\n", eid_ptr->Len - len);
	*end_field = RSN_FIELD_EXTENSIBLE_ELE;
	return TRUE;
}



UINT WPAValidateRSNIE (
	IN struct wifi_dev *wdev,
	IN struct _SECURITY_CONFIG *pSecConfigEntry,
	IN PUCHAR pRsnIe,
	IN UCHAR rsnie_len)
{
	PEID_STRUCT  eid_ptr;
	UCHAR end_field = 0;
	UINT res = MLME_SUCCESS;
	struct _SECURITY_CONFIG *pSecConfigSelf = &wdev->SecConfig;
#ifdef DOT1X_SUPPORT
	pSecConfigEntry->IEEE8021X = pSecConfigSelf->IEEE8021X;
#endif /* DOT1X_SUPPORT */

	if (rsnie_len == 0) {
		if ((IS_AKM_OPEN(pSecConfigSelf->AKMMap) && IS_CIPHER_NONE(pSecConfigSelf->PairwiseCipher)) ||
			IS_AKM_WPA1(pSecConfigEntry->AKMMap) || IS_AKM_WPA1PSK(pSecConfigEntry->AKMMap) ||
			IS_CIPHER_WEP(pSecConfigEntry->PairwiseCipher))
			return MLME_SUCCESS;
		else {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"Encryption of the AP can't allow sta with use None Encryptype access\n");
			return MLME_CANNOT_SUPPORT_CAP;
		}
	}

	eid_ptr = (PEID_STRUCT)pRsnIe;

	if (wpa_rsne_sanity(pRsnIe, rsnie_len, &end_field) == FALSE) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"wpa_rsne_sanity fail\n");
		return MLME_UNSPECIFY_FAIL;
	}

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
		"peer RSNE end field is %d\n", end_field);


	/* Check group cipher */
	if ((end_field >= RSN_FIELD_GROUP_CIPHER)
		&& !WPACheckGroupCipher(pSecConfigSelf, pSecConfigEntry, eid_ptr)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : invalid group cipher !!!\n");
		return MLME_INVALID_GROUP_CIPHER;
	} else if (end_field < RSN_FIELD_GROUP_CIPHER
			&& IS_CIPHER_CCMP128(pSecConfigSelf->GroupCipher))
		pSecConfigEntry->GroupCipher = pSecConfigSelf->GroupCipher;
	else if (end_field < RSN_FIELD_GROUP_CIPHER) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : invalid group cipher(peer use default cipher) !!!\n");
		return MLME_INVALID_GROUP_CIPHER;
	}

	/* Check pairwise cipher */
	if ((end_field >= RSN_FIELD_PAIRWISE_CIPHER)
		&& !WPACheckUcast(wdev, pSecConfigEntry, eid_ptr)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : invalid pairwise cipher !!!\n");
		return MLME_INVALID_PAIRWISE_CIPHER;
	} else if (end_field < RSN_FIELD_PAIRWISE_CIPHER
			&& IS_CIPHER_CCMP128(pSecConfigSelf->PairwiseCipher))
		pSecConfigEntry->PairwiseCipher = pSecConfigSelf->PairwiseCipher; /* Ellis todo mixed mode */
	else if (end_field < RSN_FIELD_PAIRWISE_CIPHER) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : invalid pairwise cipher(peer use default cipher) !!!\n");
		return MLME_INVALID_PAIRWISE_CIPHER;
	}

	/* Check AKM */
	if ((end_field >= RSN_FIELD_AKM)
		&& !WPACheckAKM(wdev, pSecConfigEntry, eid_ptr)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : invalid AKM !!!\n");
		return MLME_INVALID_AKMP;
	} else if (end_field < RSN_FIELD_AKM
			&& IS_AKM_WPA2(pSecConfigSelf->AKMMap))
		pSecConfigEntry->AKMMap = pSecConfigSelf->AKMMap; /* Ellis todo mixed mode */
	else if (end_field < RSN_FIELD_AKM) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : invalid AKM(peer use default akm) !!!\n");
		return MLME_INVALID_AKMP;
	}

	/* Check PMKID */
	if ((end_field >= RSN_FIELD_PMKID)
		&& !wpa_check_pmkid(pRsnIe, rsnie_len, pSecConfigEntry->AKMMap)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : invalid PMKID !!!\n");
		return MLME_UNSPECIFY_FAIL;
	}

#ifdef DOT11W_PMF_SUPPORT
	else {
		res = PMF_RsnCapableValidation(pRsnIe, rsnie_len, pSecConfigSelf->PmfCfg.MFPC, pSecConfigSelf->PmfCfg.MFPR,
					pSecConfigSelf->PmfCfg.igtk_cipher, end_field, pSecConfigEntry);
		if (res != MLME_SUCCESS)
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"[PMF] : Invalid PMF Capability !!!\n");
	}

#endif /* DOT11W_PMF_SUPPORT */

	if (end_field >= RSN_FIELD_RSN_CAP
		&& !wpa_check_rsn_cap(wdev, pSecConfigEntry, pRsnIe, rsnie_len)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : invalid rsn cap !!!\n");
		return MLME_UNSPECIFY_FAIL;
	}
	return res;
}

#ifndef RT_CFG80211_SUPPORT
VOID WpaDerivePTK(
	IN	UCHAR *PMK,
	IN	UCHAR *ANonce,
	IN	UCHAR *AA,
	IN	UCHAR *SNonce,
	IN	UCHAR *SA,
	OUT UCHAR *output,
	IN	UINT len)
{
	UCHAR concatenation[76];
	UINT CurrPos = 0;
	UCHAR temp[32];
	UCHAR Prefix[] = {'P', 'a', 'i', 'r', 'w', 'i', 's', 'e', ' ', 'k', 'e', 'y', ' ', 'e', 'x', 'p', 'a', 'n', 's', 'i', 'o', 'n'};
	/* initiate the concatenation input*/
	NdisZeroMemory(temp, sizeof(temp));
	NdisZeroMemory(concatenation, 76);

	/* Get smaller address*/
	if (RTMPCompareMemory(SA, AA, 6) == 1)
		NdisMoveMemory(concatenation, AA, 6);
	else
		NdisMoveMemory(concatenation, SA, 6);

	CurrPos += 6;

	/* Get larger address*/
	if (RTMPCompareMemory(SA, AA, 6) == 1)
		NdisMoveMemory(&concatenation[CurrPos], SA, 6);
	else
		NdisMoveMemory(&concatenation[CurrPos], AA, 6);

	/* store the larger mac address for backward compatible of */
	/* ralink proprietary STA-key issue		*/
	NdisMoveMemory(temp, &concatenation[CurrPos], MAC_ADDR_LEN);
	CurrPos += 6;

	/* Get smaller Nonce*/
	if (RTMPCompareMemory(ANonce, SNonce, 32) == 0)
		NdisMoveMemory(&concatenation[CurrPos], temp, 32);	/* patch for ralink proprietary STA-key issue*/
	else if (RTMPCompareMemory(ANonce, SNonce, 32) == 1)
		NdisMoveMemory(&concatenation[CurrPos], SNonce, 32);
	else
		NdisMoveMemory(&concatenation[CurrPos], ANonce, 32);

	CurrPos += 32;

	/* Get larger Nonce*/
	if (RTMPCompareMemory(ANonce, SNonce, 32) == 0)
		NdisMoveMemory(&concatenation[CurrPos], temp, 32);	/* patch for ralink proprietary STA-key issue*/
	else if (RTMPCompareMemory(ANonce, SNonce, 32) == 1)
		NdisMoveMemory(&concatenation[CurrPos], ANonce, 32);
	else
		NdisMoveMemory(&concatenation[CurrPos], SNonce, 32);

	CurrPos += 32;
	/* Use PRF to generate PTK*/
	PRF(PMK, LEN_PMK, Prefix, 22, concatenation, 76, output, len);
}

VOID WpaDerivePTK_KDF_256_w_pmk_len(
	IN UCHAR *PMK,
	IN UCHAR *ANonce,
	IN UCHAR *AA,
	IN UCHAR *SNonce,
	IN UCHAR *SA,
	OUT UCHAR *output,
	IN UINT	len,
	IN INT key_len)
{
	UCHAR concatenation[76];
	UINT CurrPos = 0;
	UCHAR temp[32];
	UCHAR Prefix[] = {'P', 'a', 'i', 'r', 'w', 'i', 's', 'e', ' ', 'k', 'e', 'y', ' ',
					  'e', 'x', 'p', 'a', 'n', 's', 'i', 'o', 'n'
					 };
	/* initiate the concatenation input */
	NdisZeroMemory(temp, sizeof(temp));
	NdisZeroMemory(concatenation, 76);

	/* Get smaller address */
	if (RTMPCompareMemory(SA, AA, 6) == 1)
		NdisMoveMemory(concatenation, AA, 6);
	else
		NdisMoveMemory(concatenation, SA, 6);

	CurrPos += 6;

	/* Get larger address */
	if (RTMPCompareMemory(SA, AA, 6) == 1)
		NdisMoveMemory(&concatenation[CurrPos], SA, 6);
	else
		NdisMoveMemory(&concatenation[CurrPos], AA, 6);

	/* store the larger mac address for backward compatible of
	   ralink proprietary STA-key issue */
	NdisMoveMemory(temp, &concatenation[CurrPos], MAC_ADDR_LEN);
	CurrPos += 6;

	/* Get smaller Nonce */
	if (RTMPCompareMemory(ANonce, SNonce, 32) == 0)
		NdisMoveMemory(&concatenation[CurrPos], temp, 32);
	else if (RTMPCompareMemory(ANonce, SNonce, 32) == 1)
		NdisMoveMemory(&concatenation[CurrPos], SNonce, 32);
	else
		NdisMoveMemory(&concatenation[CurrPos], ANonce, 32);

	CurrPos += 32;

	/* Get larger Nonce */
	if (RTMPCompareMemory(ANonce, SNonce, 32) == 0)
		NdisMoveMemory(&concatenation[CurrPos], temp, 32);
	else if (RTMPCompareMemory(ANonce, SNonce, 32) == 1)
		NdisMoveMemory(&concatenation[CurrPos], ANonce, 32);
	else
		NdisMoveMemory(&concatenation[CurrPos], SNonce, 32);

	CurrPos += 32;
	hex_dump("[PMF]PMK", PMK, LEN_PMK);
	hex_dump("[PMF]concatenation=", concatenation, 76);
	/* Calculate a key material through FT-KDF */

	KDF_256(PMK, key_len, Prefix, 22, concatenation, 76, output, len);
}


/*
	========================================================================

	Routine Description:
		It utilizes PRF-384 or PRF-512 to derive session-specific keys from a PMK.
		It shall be called by 4-way handshake processing.
	Note:
		Refer to IEEE 802.11i-2004 8.5.1.2

	========================================================================
*/

VOID WpaDerivePTK_KDF_256(
	IN UCHAR * PMK,
	IN UCHAR * ANonce,
	IN UCHAR * AA,
	IN UCHAR * SNonce,
	IN UCHAR * SA,
	OUT UCHAR * output,
	IN UINT	len)
{
	WpaDerivePTK_KDF_256_w_pmk_len(PMK, ANonce, AA, SNonce, SA, output, len, LEN_PMK);
}


/*
	========================================================================

	Routine Description:
		It utilizes PRF-384 or PRF-512 to derive session-specific keys from a PMK.
		It shall be called by 4-way handshake processing.
	Note:
		Refer to IEEE 802.11i-2004 8.5.1.2

	========================================================================
*/

VOID WpaDerivePTK_KDF_384(
	IN UCHAR *PMK,
	IN UCHAR *ANonce,
	IN UCHAR *AA,
	IN UCHAR *SNonce,
	IN UCHAR *SA,
	OUT UCHAR *output,
	IN UINT	len)
{
	UCHAR concatenation[76];
	UINT CurrPos = 0;
	UCHAR temp[32];
	UCHAR Prefix[] = {'P', 'a', 'i', 'r', 'w', 'i', 's', 'e', ' ', 'k', 'e', 'y', ' ',
					  'e', 'x', 'p', 'a', 'n', 's', 'i', 'o', 'n'
					 };
	/* initiate the concatenation input */
	NdisZeroMemory(temp, sizeof(temp));
	NdisZeroMemory(concatenation, 76);

	/* Get smaller address */
	if (RTMPCompareMemory(SA, AA, 6) == 1)
		NdisMoveMemory(concatenation, AA, 6);
	else
		NdisMoveMemory(concatenation, SA, 6);

	CurrPos += 6;

	/* Get larger address */
	if (RTMPCompareMemory(SA, AA, 6) == 1)
		NdisMoveMemory(&concatenation[CurrPos], SA, 6);
	else
		NdisMoveMemory(&concatenation[CurrPos], AA, 6);

	/* store the larger mac address for backward compatible of
	   ralink proprietary STA-key issue */
	NdisMoveMemory(temp, &concatenation[CurrPos], MAC_ADDR_LEN);
	CurrPos += 6;

	/* Get smaller Nonce */
	if (RTMPCompareMemory(ANonce, SNonce, 32) == 0)
		NdisMoveMemory(&concatenation[CurrPos], temp, 32);
	else if (RTMPCompareMemory(ANonce, SNonce, 32) == 1)
		NdisMoveMemory(&concatenation[CurrPos], SNonce, 32);
	else
		NdisMoveMemory(&concatenation[CurrPos], ANonce, 32);

	CurrPos += 32;

	/* Get larger Nonce */
	if (RTMPCompareMemory(ANonce, SNonce, 32) == 0)
		NdisMoveMemory(&concatenation[CurrPos], temp, 32);
	else if (RTMPCompareMemory(ANonce, SNonce, 32) == 1)
		NdisMoveMemory(&concatenation[CurrPos], ANonce, 32);
	else
		NdisMoveMemory(&concatenation[CurrPos], SNonce, 32);

	CurrPos += 32;
	hex_dump("[PMF]PMK", PMK, LEN_PMK_SHA384);
	hex_dump("[PMF]concatenation=", concatenation, 76);
	/* Calculate a key material through FT-KDF */
	KDF_384(PMK, LEN_PMK_SHA384, Prefix, 22, concatenation, 76, output, len);
}



VOID WpaDeriveGTK(
	IN  UCHAR *GMK,
	IN  UCHAR *GNonce,
	IN  UCHAR *AA,
	OUT UCHAR *output,
	IN  UINT len)
{
	UCHAR   concatenation[76];
	UINT    CurrPos = 0;
	UCHAR   temp[80] = {0};
	UCHAR Prefix[] = {'G', 'r', 'o', 'u', 'p', ' ', 'k', 'e', 'y', ' ', 'e', 'x', 'p', 'a', 'n', 's', 'i', 'o', 'n'};

	NdisMoveMemory(&concatenation[CurrPos], AA, 6);
	CurrPos += 6;
	NdisMoveMemory(&concatenation[CurrPos], GNonce, 32);
	CurrPos += 32;
	PRF(GMK, LEN_PMK, Prefix,  19, concatenation, 38, temp, len);
	NdisMoveMemory(output, temp, len);
}
#endif /* RT_CFG80211_SUPPORT */
VOID WPA_ConstructKdeHdr(
	IN UINT8 data_type,
	IN UINT8 data_len,
	OUT PUCHAR pBuf)
{
	PKDE_HDR pHdr;

	pHdr = (PKDE_HDR)pBuf;
	NdisZeroMemory(pHdr, sizeof(KDE_HDR));
	pHdr->Type = WPA_KDE_TYPE;
	/* The Length field specifies the number of octets in the OUI, Data
	   Type, and Data fields. */
	pHdr->Len = 4 + data_len;
	NdisMoveMemory(pHdr->OUI, OUI_WPA2, 3);
	pHdr->DataType = data_type;
}

VOID WPAInstallKey(
	IN PRTMP_ADAPTER pAd,
	IN struct _ASIC_SEC_INFO *pInfo,
	IN BOOLEAN bAE,
	IN BOOLEAN is_install)
{
	struct _SEC_KEY_INFO *pKey = &pInfo->Key;

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

	if (IS_CIPHER_TKIP(pInfo->Cipher)) {
		if (bAE) {
			os_move_mem(pKey->TxMic, &pKey->Key[LEN_TK], LEN_TKIP_MIC);
			os_move_mem(pKey->RxMic, &pKey->Key[LEN_TK + 8], LEN_TKIP_MIC);
		} else {
			os_move_mem(pKey->TxMic, &pKey->Key[LEN_TK + 8], LEN_TKIP_MIC);
			os_move_mem(pKey->RxMic, &pKey->Key[LEN_TK], LEN_TKIP_MIC);
		}
	}

	if (is_install)
		HW_ADDREMOVE_KEYTABLE(pAd, pInfo);
}

#ifndef RT_CFG80211_SUPPORT
VOID WPACalculateMIC(
	IN UCHAR KeyDescVer,
	IN UINT32 AKMMap,
	IN UCHAR *PTK,
	IN UINT8 key_deri_alg,
	OUT PEAPOL_PACKET pMsg)
{
	UCHAR *OutBuffer;
	ULONG FrameLen = 0;
	UCHAR	mic[LEN_KEY_DESC_MIC_MAX];
	UCHAR	digest[80] = {0};
	UINT8 mic_len = LEN_KEY_DESC_MIC;
	/* allocate memory for MIC calculation*/
	os_alloc_mem(NULL, (PUCHAR *)&OutBuffer, 512);

	if (OutBuffer == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"!!!CalculateMIC: no memory!!!\n");
		return;
	}

	/* make a frame for calculating MIC.*/
	MakeOutgoingFrame(OutBuffer, &FrameLen,
					  CONV_ARRARY_TO_UINT16(pMsg->Body_Len) + 4, pMsg,
					  END_OF_ARGS);
	NdisZeroMemory(mic, sizeof(mic));

	/* Calculate MIC*/
	if (KeyDescVer == KEY_DESC_AES) {
		RT_HMAC_SHA1(PTK, LEN_PTK_KCK, OutBuffer,  FrameLen, digest, SHA1_DIGEST_SIZE);
		NdisMoveMemory(mic, digest, LEN_KEY_DESC_MIC);
	} else if (KeyDescVer == KEY_DESC_TKIP)
		RT_HMAC_MD5(PTK, LEN_PTK_KCK, OutBuffer, FrameLen, mic, MD5_DIGEST_SIZE);
	else if ((KeyDescVer == KEY_DESC_NOT_DEFINED) && IS_AKM_SHA384(AKMMap)) {
		RT_HMAC_SHA384(PTK, LEN_PTK_KCK_SHA384, OutBuffer, FrameLen, mic, LEN_KEY_DESC_MIC_SHA384);
		mic_len = LEN_KEY_DESC_MIC_SHA384;
	} else if ((KeyDescVer == KEY_DESC_NOT_DEFINED) &&
		   IS_AKM_OWE(AKMMap) &&
		   (key_deri_alg == SEC_KEY_DERI_SHA256)) {
		RT_HMAC_SHA256(PTK, LEN_PTK_KCK, OutBuffer, FrameLen, mic, LEN_KEY_DESC_MIC);
		mic_len = LEN_KEY_DESC_MIC;
#ifdef DPP_SUPPORT
	} else if ((KeyDescVer == KEY_DESC_NOT_DEFINED) &&
		  IS_AKM_DPP(AKMMap) &&
		  (key_deri_alg == SEC_KEY_DERI_SHA256)) {
		RT_HMAC_SHA256(PTK, LEN_PTK_KCK, OutBuffer, FrameLen, mic, LEN_KEY_DESC_MIC);
		mic_len = LEN_KEY_DESC_MIC;
#endif /* DPP_SUPPORT */
	} else if ((KeyDescVer == KEY_DESC_NOT_DEFINED) &&
		   IS_AKM_OWE(AKMMap) &&
		   (key_deri_alg == SEC_KEY_DERI_SHA384)) {
		/*TODO: OWE SHA521*/
		RT_HMAC_SHA384(PTK, LEN_PTK_KCK_SHA384, OutBuffer, FrameLen, mic, LEN_KEY_DESC_MIC_SHA384);
		mic_len = LEN_KEY_DESC_MIC_SHA384;
	} else if ((KeyDescVer == KEY_DESC_EXT)
		|| (KeyDescVer == KEY_DESC_NOT_DEFINED)) {
		UINT mlen = AES_KEY128_LENGTH;

		AES_CMAC(OutBuffer, FrameLen, PTK, LEN_PTK_KCK, mic, &mlen);
	}

	/* store the calculated MIC*/
	NdisMoveMemory(pMsg->KeyDesc.KeyMicAndData, mic, mic_len);
	os_free_mem(OutBuffer);
}
#endif /* RT_CFG80211_SUPPORT */

VOID WPAInsertRSNIE (
	IN PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUINT8 rsnie_ptr,
	IN UINT8  rsnie_len,
	IN PUINT8 pmkid_ptr,
	IN UINT8  pmkid_len)
{
	PUCHAR	pTmpBuf;
	ULONG	TempLen = 0;
	UINT8	extra_len = 0;
	UINT16	pmk_count = 0;
	UCHAR	ie_num;
	UINT8	total_len = 0;

	pTmpBuf = pFrameBuf;

	/* PMKID-List Must larger than 0 and the multiple of 16. */
	if (pmkid_len > 0 && ((pmkid_len & 0x0f) == 0)) {
		extra_len = sizeof(UINT16) + pmkid_len;
		pmk_count = (pmkid_len >> 4);
		pmk_count = cpu2le16(pmk_count);
	} else
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "no PMKID-List included(%d).\n", pmkid_len);

	if (rsnie_len != 0) {
		ie_num = IE_WPA;
		total_len = rsnie_len;

		if (NdisEqualMemory(rsnie_ptr + 2, OUI_WPA2_CIPHER, sizeof(OUI_WPA2_CIPHER))) {
			ie_num = IE_RSN;
			total_len += extra_len;
		}

		/* construct RSNIE body */
		MakeOutgoingFrame(pTmpBuf, &TempLen,
						  1, &ie_num,
						  1, &total_len,
						  rsnie_len, rsnie_ptr,
						  END_OF_ARGS);
		pTmpBuf += TempLen;
		*pFrameLen = *pFrameLen + TempLen;

		if (ie_num == IE_RSN) {
			/* Insert PMKID-List field */
			if (extra_len > 0) {
				MakeOutgoingFrame(pTmpBuf, &TempLen,
								  2, &pmk_count,
								  pmkid_len, pmkid_ptr,
								  END_OF_ARGS);
				pTmpBuf += TempLen;
				*pFrameLen = *pFrameLen + TempLen;
			}
		}
	}

	return;
}


/*
	========================================================================
	Routine Description:
		Construct KDE common format
		Its format is below,
		+--------------------+
		| Type (0xdd)		 |  1 octet
		+--------------------+
		| Length			 |	1 octet
		+--------------------+
		| OUI				 |  3 octets
		+--------------------+
		| Data Type			 |	1 octet
		+--------------------+
	Note:
		It's defined in IEEE 802.11-2007 Figure 8-25.

	========================================================================
*/
VOID WPAConstructKdeHdr(
	IN UINT8 data_type,
	IN UINT8 data_len,
	OUT PUCHAR pBuf)
{
	PKDE_HDR pHdr;

	pHdr = (PKDE_HDR)pBuf;
	NdisZeroMemory(pHdr, sizeof(KDE_HDR));
	pHdr->Type = WPA_KDE_TYPE;
	/* The Length field specifies the number of octets in the OUI, Data
	   Type, and Data fields. */
	pHdr->Len = 4 + data_len;
	NdisMoveMemory(pHdr->OUI, OUI_WPA2, 3);
	pHdr->DataType = data_type;
}

UCHAR wpa3_test_ctrl; /* for wpa3 certifcation ap testbed specfic cmd */
UCHAR wpa3_test_ctrl2; /* for wpa3 certifcation ap testbed specfic cmd */
const UCHAR wfa_oui[3] = {0x50, 0x6F, 0x9A};
#ifndef RT_CFG80211_SUPPORT
static void build_transition_disable_kde(
	IN struct _SECURITY_CONFIG *pSecGroup,
	OUT UCHAR *buf,
	OUT ULONG * frame_len)
{
	struct transition_disable_bitmap bitmap = {0};
	UCHAR insert_kde = FALSE;
	KDE_HDR *hdr_ptr;
	UCHAR force_mode = FALSE;
	UCHAR val = 0;

	/* auto mode */
	if (IS_AKM_WPA3PSK(pSecGroup->AKMMap)) {
		bitmap.wpa3_psk = 1;
		insert_kde = TRUE;
	}

#ifdef DOT11_SAE_SUPPORT
	if (IS_AKM_SAE(pSecGroup->AKMMap) && pSecGroup->sae_cap.sae_pk_en) {
		bitmap.sae_pk = 1;
		insert_kde = TRUE;
	}
#endif

	/*force mode by profile*/
	if (pSecGroup->td_value_fixed_en) {
		val = pSecGroup->td_value;
		force_mode = TRUE;
	}

	/*force mode by iwpriv*/
	if (wpa3_test_ctrl == 11) {
		val = wpa3_test_ctrl2;
		force_mode = TRUE;
	}

	if (force_mode) {
		if (val) {
			os_move_mem(&bitmap, &val, sizeof(UCHAR));
			insert_kde = TRUE;
		} else
			insert_kde = FALSE;
	}

	/* priority: iwpriv > profile > auto */
	if (insert_kde) {
		hdr_ptr = (KDE_HDR *)buf;
		hdr_ptr->Type = IE_VENDOR_SPECIFIC;
		hdr_ptr->Len = 4 + sizeof(struct transition_disable_bitmap);
		os_move_mem(hdr_ptr->OUI, (UCHAR *)wfa_oui, sizeof(wfa_oui));
		hdr_ptr->DataType = WFA_KDE_TTI;
		os_move_mem(hdr_ptr->octet, &bitmap, sizeof(struct transition_disable_bitmap));
		hex_dump_with_cat_and_lvl("transition disable kde", buf, hdr_ptr->Len + 2,
						DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO);
		*frame_len = *frame_len + hdr_ptr->Len + 2;
	}
}

static VOID insert_oci_kde(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN UCHAR *buf,
	OUT ULONG *offset)
{
	ULONG len;

	if (wdev == NULL)
		return;

	build_oci_common_field(ad, wdev, FALSE, buf + LEN_KDE_HDR, &len);
	WPA_ConstructKdeHdr(KDE_OCI, len, buf);

	*offset += LEN_KDE_HDR + len;

	hex_dump_with_cat_and_lvl("oci kde", buf,
		LEN_KDE_HDR + len, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_INFO); /* todo: change to trace*/
}

static UCHAR parse_oci_kde(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN UCHAR *buf,
	IN UINT32 buf_len)
{
	if (wdev == NULL) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			" wdev is null.\n");
		return FALSE;
	}

	return parse_oci_common_field(ad, wdev, FALSE, buf, buf_len);
}

#ifdef DOT11_EHT_BE
static VOID build_mac_addr_kde(
	IN u8 *mac_addr,
	IN u8 *buf,
	OUT ULONG * offset)
{
	WPA_ConstructKdeHdr(KDE_MAC_ADDR, MAC_ADDR_LEN, buf);
	NdisMoveMemory(buf + LEN_KDE_HDR, mac_addr, MAC_ADDR_LEN);
	*offset += LEN_KDE_HDR + MAC_ADDR_LEN;
}

static UCHAR parse_mac_addr_kde(
	IN u8 *mac_addr,
	IN u8 *buf,
	IN UINT32 buf_len)
{
	if (buf_len != MAC_ADDR_LEN) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
			"wrong mac addr len %d.\n", buf_len);
		return FALSE;
	}

	if (!NdisEqualMemory(buf, mac_addr, MAC_ADDR_LEN)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
			"wrong mlo mac addr %02x:%02x:%02x:%02x:%02x:%02x.\n",
			PRINT_MAC(buf));
		return FALSE;
	}

	return TRUE;
}
#endif

u32 is_eapol_key_data_element_present(
	IN u8 msg_type,
	IN u8 is_wpa2,
	IN u8 is_mlo_en,
	IN u8 is_single_link)
{
	u32 is_present = 0;

	switch (msg_type) {
	case EAPOL_PAIR_MSG_1:
		if (is_mlo_en)
			is_present |= MAC_ADDR_PRESNET;
		break;
	case EAPOL_PAIR_MSG_2:
		is_present |=
			(RSNE_PRESNET |
			 OCI_PRESNET |
			 MDIE_PRESNET |
			 FTIE_PRESNET |
			 RSNXE_PRESNET);
		if (is_mlo_en) {
			is_present |= MAC_ADDR_PRESNET;
			if (!is_single_link)
				is_present |= MLO_LINK_PRESNET;
		}
		break;
	case EAPOL_PAIR_MSG_3:
		is_present |=
			(OCI_PRESNET |
			 MDIE_PRESNET |
			 FTIE_PRESNET |
			 TIE_PRESNET |
			 TTI_PRESNET);
		if (is_mlo_en) {
			is_present |=
			(MAC_ADDR_PRESNET |
			 MLO_LINK_PRESNET |
			 MLO_GTK_PRESNET);
		} else {
			is_present |=
			(RSNE_PRESNET |
			 GTK_PRESNET |
			 RSNXE_PRESNET);
		}
		break;
	case EAPOL_PAIR_MSG_4:
		if (is_mlo_en)
			is_present |= MAC_ADDR_PRESNET;
		break;
	case EAPOL_GROUP_MSG_1:
		is_present |=  OCI_PRESNET;
		if (is_mlo_en)
			is_present |= MLO_GTK_PRESNET;
		else
			is_present |= GTK_PRESNET;
		break;
	case EAPOL_GROUP_MSG_2:
		is_present |=  OCI_PRESNET;
		break;
	default:
		break;
	}

	if (!is_wpa2) {
		is_present &=
			~(OCI_PRESNET |
			  MDIE_PRESNET |
			  FTIE_PRESNET |
			  TIE_PRESNET |
			  TTI_PRESNET |
			  RSNXE_PRESNET);
		if (msg_type == EAPOL_PAIR_MSG_3)
			is_present &= ~GTK_PRESNET;
	}

	return is_present;
}

static void build_eapol_key_data_element(
	IN PMAC_TABLE_ENTRY pEntry,
	IN u8 msg_type,
	IN struct _SECURITY_CONFIG *sec_pairwise,
	IN struct _SECURITY_CONFIG *sec_group,
	IN u8 *buf,
	OUT ULONG * offset)

{
	u8 is_mlo_en = FALSE;
	u8 is_single_link = FALSE;
	u32 is_ie_presnet;
#ifdef DOT11_EHT_BE
	u8 is_all_link = (msg_type == EAPOL_PAIR_MSG_3) ? TRUE : FALSE;
	struct mld_entry_t *mld_entry = NULL;
#endif
#ifdef MAP_R3
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)pEntry->pAd;
#endif
	u8 is_wpa2 = TRUE;

	/* Choose WPA2 or not*/
	if (IS_AKM_WPA1(sec_pairwise->AKMMap) || IS_AKM_WPA1PSK(sec_pairwise->AKMMap))
		is_wpa2 = FALSE;

#ifdef DOT11_EHT_BE
	if (pEntry) {
		is_mlo_en = pEntry->mlo.mlo_en;
		if (is_mlo_en) {
			mt_rcu_read_lock();
			mld_entry = get_mld_entry_by_mac(pEntry->mlo.mld_addr);
			if (mld_entry)
				is_single_link = (mld_entry->link_num == 1) ? TRUE : FALSE;
			else
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
					"mld_entry=NULL\n");
			mt_rcu_read_unlock();
		}
	}
#endif

	is_ie_presnet = is_eapol_key_data_element_present(msg_type, is_wpa2, is_mlo_en, is_single_link);

	/* Encapsulate RSNIE & RSNXE in pairwise_msg2 & pairwise_msg3 */
	if (is_ie_presnet & RSNE_PRESNET) {
		UCHAR *RSNIE = NULL;
		UCHAR RSNIE_LEN = 0;
		CHAR rsne_idx = 0;
		SEC_RSNIE_TYPE RSNType = SEC_RSNIE_WPA1_IE;

#ifdef DOT11R_FT_SUPPORT
		if (IS_FT_RSN_STA(pEntry)) {
			/* YF_FT */
			pEntry->FT_Status = TX_EAPOL_3;
			WPAMakeRSNIE(pEntry->wdev->wdev_type, sec_group, pEntry);
		}
#endif /* DOT11R_FT_SUPPORT */

		if (is_wpa2)
			RSNType = SEC_RSNIE_WPA2_IE;

		for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
			if (sec_group->RSNE_Type[rsne_idx] == RSNType) {
				RSNIE = &sec_group->RSNE_Content[rsne_idx][0];
				RSNIE_LEN = sec_group->RSNE_Len[rsne_idx];
				break;
			}
		}


		WPAInsertRSNIE(&buf[*offset],
					   offset,
					   RSNIE,
					   RSNIE_LEN,
					   NULL,
					   0);
	}

	if (is_ie_presnet & OCI_PRESNET && sec_pairwise->ocv_support)
		insert_oci_kde(pEntry->pAd, pEntry->wdev, &buf[*offset], offset);

#ifdef DOT11R_FT_SUPPORT
	/* Encapsulate MDIE if FT is enabled*/
	if (is_ie_presnet & MDIE_PRESNET && IS_FT_RSN_STA(pEntry)) {
		/*	The MDIE shall be the same as those provided in
			the AP's (Re)association Response frame. */
		if (msg_type == EAPOL_PAIR_MSG_3) {
			NdisMoveMemory(&buf[*offset], pEntry->InitialMDIE, 5);
			*offset += 5;
		} else if (msg_type == EAPOL_PAIR_MSG_2)
			FT_InsertMdIE(&buf[*offset],
						  offset,
						  pEntry->MdIeInfo.MdId,
						  pEntry->MdIeInfo.FtCapPlc);
	}
#endif /* DOT11R_FT_SUPPORT */

	/* Only for pairwise_msg3_WPA2 and group_msg1*/
	if (is_ie_presnet & GTK_PRESNET) {
		uint8_t gtk_len = 0;

		/* Decide the GTK length */
		if (IS_CIPHER_TKIP(sec_group->GroupCipher))
			gtk_len = LEN_TKIP_TK;
		else if (IS_CIPHER_CCMP128(sec_group->GroupCipher))
			gtk_len = LEN_CCMP128_TK;
		else if (IS_CIPHER_CCMP256(sec_group->GroupCipher))
			gtk_len = LEN_CCMP256_TK;
		else if (IS_CIPHER_GCMP128(sec_group->GroupCipher))
			gtk_len = LEN_GCMP128_TK;
		else if (IS_CIPHER_GCMP256(sec_group->GroupCipher))
			gtk_len = LEN_GCMP256_TK;

		/* Insert GTK KDE format in WAP2 mode */
		if (is_wpa2) {
			/* Construct the common KDE format */
			WPAConstructKdeHdr(KDE_GTK, 2 + gtk_len, &buf[*offset]);
			*offset += sizeof(KDE_HDR);
			/* GTK KDE format - 802.11i-2004  Figure-43x*/
			buf[*offset] = (sec_group->GroupKeyId & 0x03);
			buf[*offset + 1] = 0x00;	/* Reserved Byte*/
			*offset += 2;
		}

		/* Fill in GTK */
#if defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT)

		if (MBSS_GET(pEntry->pMbss)->HotSpotCtrl.HotSpotEnable
			&& MBSS_GET(pEntry->pMbss)->HotSpotCtrl.DGAFDisable) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
				"[HOTSPOT]:Unique GTK for each STA\n");
			NdisMoveMemory(&buf[*offset], sec_pairwise->HsUniGTK, gtk_len);
		} else
#endif /* defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT) */
		{
			NdisMoveMemory(&buf[*offset], sec_group->GTK, gtk_len);
		}

		*offset += gtk_len;
#ifdef DOT11W_PMF_SUPPORT

		/* Insert IGTK KDE to Key_Data field */
		if ((is_wpa2)
			&& (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO,
				"[PMF] Insert IGTK\n");
			PMF_InsertIGTKKDE(pEntry->pAd, pEntry->func_tb_idx, &buf[*offset], offset);
		}

#endif /* DOT11W_PMF_SUPPORT */
#ifdef BCN_PROTECTION_SUPPORT
		if (sec_group->bcn_prot_cfg.bcn_prot_en) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_INFO, "Insert BIGTK\n");
			insert_bigtk_kde(pEntry->pAd, pEntry->func_tb_idx, &buf[*offset], offset);
		}
#endif
	}

#ifdef MAP_R3
	if (pEntry &&
		((IS_MAP_ENABLE(ad) &&
		!IS_MAP_CERT_ENABLE(ad))
		|| !IS_MAP_ENABLE(ad)))
#endif
	{
		if (is_ie_presnet & RSNXE_PRESNET) {
			ULONG rsnxe_len;

			rsnxe_len = build_rsnxe_ie(pEntry->wdev, sec_group, buf + *offset);

			if (wpa3_test_ctrl == 1)
				buf[*offset + 2] = 0;

			*offset += rsnxe_len;
		}
	}

#ifdef DOT11_EHT_BE
	if (is_ie_presnet & MAC_ADDR_PRESNET) {
		UCHAR *mld_addr = pEntry->wdev->bss_info_argument.mld_info.mld_addr;

		build_mac_addr_kde(mld_addr, &buf[*offset], offset);
		MTWF_DBG(pEntry->pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_INFO,
			"Insert MLO_MAC_ADDR_KDE\n");
	}

	if (is_ie_presnet & MLO_LINK_PRESNET) {
		build_mlo_kde(KDE_MLO_LINK, pEntry, TRUE, &buf[*offset], offset);
		MTWF_DBG(pEntry->pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_INFO,
			"Insert KDE_MLO_LINK\n");
	}

	if (is_ie_presnet & MLO_GTK_PRESNET) {
		build_mlo_kde(KDE_MLO_GTK, pEntry, is_all_link, &buf[*offset], offset);
		MTWF_DBG(pEntry->pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_INFO,
			"Insert KDE_MLO_GTK\n");
#ifdef DOT11W_PMF_SUPPORT
		if (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE) {
			build_mlo_kde(KDE_MLO_IGTK, pEntry, is_all_link, &buf[*offset], offset);
			MTWF_DBG(pEntry->pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_WARN,
				"Insert KDE_MLO_IGTK\n");
		}
#endif
#ifdef BCN_PROTECTION_SUPPORT
		if (sec_group->bcn_prot_cfg.bcn_prot_en) {
			build_mlo_kde(KDE_MLO_BIGTK, pEntry, is_all_link, &buf[*offset], offset);
			MTWF_DBG(pEntry->pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_INFO,
				"Insert KDE_MLO_BIGTK\n");
		}
#endif
	}
#endif

#ifdef DOT11R_FT_SUPPORT
	/* Encapsulate the related IE of FT when FT is enabled */
	if (is_ie_presnet & FTIE_PRESNET && IS_FT_RSN_STA(pEntry)) {
		/*	Encapsulate FTIE. The MDIE shall be the same
			as those provided in the AP's (Re)association Response frame. */

		NdisMoveMemory(&buf[*offset], pEntry->InitialFTIE, pEntry->InitialFTIE_Len);
		*offset += pEntry->InitialFTIE_Len;
	}

	if (is_ie_presnet & TIE_PRESNET && IS_FT_RSN_STA(pEntry)) {
		FT_InsertTimeoutIntervalIE(&buf[*offset], offset,
			REASSOC_DEADLINE_INTERVAL, MAC_TABLE_ASSOC_TIMEOUT);

		FT_InsertTimeoutIntervalIE(&buf[*offset], offset,
			KEY_LIFETIME_INTERVAL, 600); /* Sync PMKID Cache Period */
	}
#endif /* DOT11R_FT_SUPPORT */
	/* Adding OCI KDE as KeyData to EAPOL group/Pair MSG2 if support is enabled */
	if (is_wpa2 && ((msg_type == EAPOL_PAIR_MSG_2) || (msg_type == EAPOL_GROUP_MSG_2))) {
		if ((pEntry && pEntry->wdev &&
			(pEntry->wdev->wdev_type == WDEV_TYPE_STA))) {
			struct _SECURITY_CONFIG *pSecConfig = &pEntry->wdev->SecConfig;

			if (pSecConfig && pSecConfig->ocv_support) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_INFO,
				"Insert OCV in PAIR/Group msg2\n");
				insert_oci_kde(pEntry->pAd, pEntry->wdev, &buf[*offset], offset);
			}
		}
	}
	if (is_ie_presnet & TTI_PRESNET)
		build_transition_disable_kde(sec_group, &buf[*offset], offset);
}

VOID WPAConstructEapolKeyData(
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR MsgType,
	IN	UCHAR keyDescVer,
	IN struct _SECURITY_CONFIG *pSecPairwise,
	IN struct _SECURITY_CONFIG *pSecGroup,
	OUT PEAPOL_PACKET pMsg)
{
	UCHAR *mpool, *Key_Data, *eGTK;
	ULONG data_offset;
	BOOLEAN bWPA2 = TRUE;
	UINT8 mic_len = LEN_KEY_DESC_MIC;
	UINT8 *key_data_len_ptr = NULL;
	UINT8 *key_data_ptr = NULL;

	/* Choose WPA2 or not*/
	if (IS_AKM_WPA1(pSecPairwise->AKMMap) || IS_AKM_WPA1PSK(pSecPairwise->AKMMap))
		bWPA2 = FALSE;

	/* allocate memory pool*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, 1500);

	if (mpool == NULL)
		return;

	if (IS_AKM_SHA384(pSecPairwise->AKMMap) || (pSecPairwise->key_deri_alg == SEC_KEY_DERI_SHA384))
		mic_len = LEN_KEY_DESC_MIC_SHA384;

#ifdef OCE_FILS_SUPPORT
	if (IS_AKM_FILS_Entry(pEntry))
		mic_len = 0;
#endif /* OCE_FILS_SUPPORT */

	key_data_len_ptr = pMsg->KeyDesc.KeyMicAndData + mic_len;
	key_data_ptr = key_data_len_ptr + 2;

	/* eGTK Len = 512 */
	eGTK = (UCHAR *) ROUND_UP(mpool, 4);
	/* Key_Data Len = 512 */
	Key_Data = (UCHAR *) ROUND_UP(eGTK + 512, 4);
	NdisZeroMemory(Key_Data, 512);
	SET_UINT16_TO_ARRARY(key_data_len_ptr, 0);
	data_offset = 0;

	build_eapol_key_data_element(pEntry, MsgType,
		pSecPairwise, pSecGroup, &Key_Data[data_offset], &data_offset);

	/* If the Encrypted Key Data subfield (of the Key Information field)
	   is set, the entire Key Data field shall be encrypted. */
	/* This whole key-data field shall be encrypted if a GTK is included.*/
	/* Encrypt the data material in key data field with KEK*/
	if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1)) {
#ifdef OCE_FILS_SUPPORT
		if (IS_AKM_FILS_Entry(pEntry)) {
			NdisMoveMemory(key_data_ptr, Key_Data, data_offset);
		} else
#endif /* OCE_FILS_SUPPORT */
		{
			if ((keyDescVer == KEY_DESC_EXT)
				|| (keyDescVer == KEY_DESC_NOT_DEFINED)
				|| (keyDescVer == KEY_DESC_AES)) {
				UCHAR remainder = 0;
				UCHAR pad_len = 0;
				UINT wrap_len = 0;
				uint8_t kck_len = LEN_PTK_KCK;
				uint8_t kek_len = LEN_PTK_KEK;

				if (IS_AKM_SHA384(pSecPairwise->AKMMap) ||
				   (pSecPairwise->key_deri_alg == SEC_KEY_DERI_SHA384)) {
					kck_len = LEN_PTK_KCK_SHA384;
					kek_len = LEN_PTK_KEK_SHA384;
				}

				/* Key Descriptor Version 2 or 3: AES key wrap, defined in IETF RFC 3394, */
				/* shall be used to encrypt the Key Data field using the KEK field from */
				/* the derived PTK.*/
				/* If the Key Data field uses the NIST AES key wrap, then the Key Data field */
				/* shall be padded before encrypting if the key data length is less than 16 */
				/* octets or if it is not a multiple of 8. The padding consists of appending*/
				/* a single octet 0xdd followed by zero or more 0x00 octets. */
				remainder = data_offset & 0x07;

				if (remainder != 0) {
					INT i;

					pad_len = (8 - remainder);
					Key_Data[data_offset] = 0xDD;

					for (i = 1; i < pad_len; i++)
						Key_Data[data_offset + i] = 0;

					data_offset += pad_len;
				}

				AES_Key_Wrap(Key_Data, (UINT) data_offset,
							 &pSecPairwise->PTK[kck_len], kek_len,
							 eGTK, &wrap_len);
				data_offset = wrap_len;
			} else {
				TKIP_GTK_KEY_WRAP(&pSecPairwise->PTK[LEN_PTK_KCK],
								  pMsg->KeyDesc.KeyIv,
								  Key_Data,
								  data_offset,
								  eGTK);
			}

			NdisMoveMemory(key_data_ptr, eGTK, data_offset);
		}
	} else
		NdisMoveMemory(key_data_ptr, Key_Data, data_offset);

	/* Update key data length field and total body length*/
	SET_UINT16_TO_ARRARY(key_data_len_ptr, data_offset);
	INC_UINT16_TO_ARRARY(pMsg->Body_Len, data_offset);
	os_free_mem(mpool);
}


VOID WPAConstructEapolMsg(
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR MsgType,
	IN struct _SECURITY_CONFIG *pSecPairwise,
	IN struct _SECURITY_CONFIG *pSecGroup,
	OUT PEAPOL_PACKET pMsg)
{
	BOOLEAN bWPA2 = TRUE;
	UCHAR KeyDescVer = 0;
	UINT msg_len = MIN_LEN_OF_EAPOL_KEY_MSG;

	/* Choose WPA2 or not*/
	if (IS_AKM_WPA1(pSecPairwise->AKMMap) || IS_AKM_WPA1PSK(pSecPairwise->AKMMap))
		bWPA2 = FALSE;

	/* Init Packet and Fill header */
	pMsg->ProVer = EAPOL_VER;
	pMsg->ProType = EAPOLKey;

#ifdef OCE_FILS_SUPPORT
	if (IS_AKM_FILS_Entry(pEntry))
		msg_len -= LEN_KEY_DESC_MIC;
#endif /* OCE_FILS_SUPPORT */

	/* Default 95 bytes, the EAPoL-Key descriptor exclude Key-data field*/
	SET_UINT16_TO_ARRARY(pMsg->Body_Len, msg_len);

	/* Fill in EAPoL descriptor*/
	if (bWPA2)
		pMsg->KeyDesc.Type = WPA2_KEY_DESC;
	else
		pMsg->KeyDesc.Type = WPA1_KEY_DESC;

	if (IS_AKM_WPA3_192BIT(pSecPairwise->AKMMap) ||
	    IS_AKM_WPA3PSK(pSecPairwise->AKMMap) ||
#ifdef DPP_SUPPORT
	    IS_AKM_DPP(pSecPairwise->AKMMap) ||
#endif /* DPP_SUPPORT */
	    IS_AKM_OWE(pSecPairwise->AKMMap) ||
#ifdef CONFIG_HOTSPOT_R2
	    CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE) ||
#endif /* CONFIG_HOTSPOT_R2 */
#ifdef OCE_FILS_SUPPORT
		IS_AKM_FILS_Entry(pEntry) ||
#endif /* OCE_FILS_SUPPORT */
	    (pSecPairwise->key_deri_alg == SEC_KEY_DERI_SHA384))
		KeyDescVer = KEY_DESC_NOT_DEFINED;
	/* Key Descriptor Version (bits 0-2) specifies the key descriptor version type*/
	/* AKM is 00-0F-AC:3, 00-0F-AC:4, , 00-0F-AC:5, , 00-0F-AC:6 */
	else if (IS_AKM_SHA256(pSecPairwise->AKMMap)
#ifdef DOT11W_PMF_SUPPORT
		|| (pSecPairwise->key_deri_alg == SEC_KEY_DERI_SHA256)
#endif /* DOT11W_PMF_SUPPORT */
	   )
		KeyDescVer = KEY_DESC_EXT;
	else if (((MsgType <= EAPOL_PAIR_MSG_4) && IS_CIPHER_TKIP(pSecPairwise->PairwiseCipher))
			 || ((MsgType >= EAPOL_GROUP_MSG_1) && IS_CIPHER_TKIP(pSecPairwise->PairwiseCipher)))   /* Must see pairwise cipher*/
		KeyDescVer = KEY_DESC_TKIP;
#ifdef CONFIG_HOTSPOT_R3
	else if (pSecPairwise->bIsWPA2EntOSEN == TRUE)
		KeyDescVer = KEY_DESC_NOT_DEFINED;
#endif
	else
		KeyDescVer = KEY_DESC_AES;

	pMsg->KeyDesc.KeyInfo.KeyDescVer = KeyDescVer;

	/* Specify Key Type as Group(0) or Pairwise(1)*/
	if (MsgType >= EAPOL_GROUP_MSG_1)
		pMsg->KeyDesc.KeyInfo.KeyType = GROUPKEY;
	else
		pMsg->KeyDesc.KeyInfo.KeyType = PAIRWISEKEY;

	/* Specify Key Index, only group_msg1_WPA1*/
	if (!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1))
		pMsg->KeyDesc.KeyInfo.KeyIndex = pSecGroup->GroupKeyId;

	if (MsgType == EAPOL_PAIR_MSG_3)
		pMsg->KeyDesc.KeyInfo.Install = 1;

	if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3) || (MsgType == EAPOL_GROUP_MSG_1))
		pMsg->KeyDesc.KeyInfo.KeyAck = 1;

#ifdef OCE_FILS_SUPPORT
	if (!IS_AKM_FILS_Entry(pEntry))
#endif /* OCE_FILS_SUPPORT */
	if (MsgType != EAPOL_PAIR_MSG_1)
		pMsg->KeyDesc.KeyInfo.KeyMic = 1;

#ifdef OCE_FILS_SUPPORT
	if (!IS_AKM_FILS_Entry(pEntry))
#endif /* OCE_FILS_SUPPORT */
	if ((IS_AKM_SHA384(pSecPairwise->AKMMap)) || (pSecPairwise->key_deri_alg == SEC_KEY_DERI_SHA384))
		INC_UINT16_TO_ARRARY(pMsg->Body_Len, LEN_KEY_DESC_MIC_SHA384 - LEN_KEY_DESC_MIC);

	if ((bWPA2 && (MsgType >= EAPOL_PAIR_MSG_3)) ||
		(!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1)))
		pMsg->KeyDesc.KeyInfo.Secure = 1;

	/* This subfield shall be set, and the Key Data field shall be encrypted, if
	   any key material (e.g., GTK or SMK) is included in the frame. */
	if (bWPA2 && ((MsgType == EAPOL_PAIR_MSG_3)
				  || (MsgType == EAPOL_GROUP_MSG_1)))
		pMsg->KeyDesc.KeyInfo.EKD_DL = 1;

	/* key Information element has done. */
	*(USHORT *)(&pMsg->KeyDesc.KeyInfo) = cpu2le16(*(USHORT *)(&pMsg->KeyDesc.KeyInfo));

	/* Fill in Key Length*/
	if (bWPA2) {
		/* In WPA2 mode, the field indicates the length of pairwise key cipher, */
		/* so only pairwise_msg_1 and pairwise_msg_3 need to fill. */
		if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3)) {
			if (IS_CIPHER_TKIP(pSecPairwise->PairwiseCipher))
				pMsg->KeyDesc.KeyLength[1] = LEN_TKIP_TK;
			else if (IS_CIPHER_CCMP128(pSecPairwise->PairwiseCipher))
				pMsg->KeyDesc.KeyLength[1] = LEN_CCMP128_TK;
			else if (IS_CIPHER_CCMP256(pSecPairwise->PairwiseCipher))
				pMsg->KeyDesc.KeyLength[1] = LEN_CCMP256_TK;
			else if (IS_CIPHER_GCMP128(pSecPairwise->PairwiseCipher))
				pMsg->KeyDesc.KeyLength[1] = LEN_GCMP128_TK;
			else if (IS_CIPHER_GCMP256(pSecPairwise->PairwiseCipher))
				pMsg->KeyDesc.KeyLength[1] = LEN_GCMP256_TK;
		}
	} else {
		UINT32 Cipher = 0;

		if (MsgType >= EAPOL_GROUP_MSG_1)
			Cipher = pSecGroup->GroupCipher;
		else
			Cipher = pSecPairwise->PairwiseCipher;

		if (IS_CIPHER_TKIP(Cipher))
			pMsg->KeyDesc.KeyLength[1] = LEN_TKIP_TK;
		else if (IS_CIPHER_CCMP128(Cipher))
			pMsg->KeyDesc.KeyLength[1] = LEN_CCMP128_TK;
		else if (IS_CIPHER_CCMP256(Cipher))
			pMsg->KeyDesc.KeyLength[1] = LEN_CCMP256_TK;
		else if (IS_CIPHER_GCMP128(Cipher))
			pMsg->KeyDesc.KeyLength[1] = LEN_GCMP128_TK;
		else if (IS_CIPHER_GCMP256(Cipher))
			pMsg->KeyDesc.KeyLength[1] = LEN_GCMP256_TK;
	}

	/* Fill in replay counter */
	NdisMoveMemory(pMsg->KeyDesc.ReplayCounter, pSecPairwise->Handshake.ReplayCounter, LEN_KEY_DESC_REPLAY);

	/* Fill Key Nonce field	  */
	/* ANonce : pairwise_msg1 & pairwise_msg3*/
	/* SNonce : pairwise_msg2*/
	/* GNonce : group_msg1_wpa1 */
	if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3))
		NdisMoveMemory(pMsg->KeyDesc.KeyNonce, pSecPairwise->Handshake.ANonce, LEN_KEY_DESC_NONCE);
	else if (MsgType == EAPOL_PAIR_MSG_2)
		NdisMoveMemory(pMsg->KeyDesc.KeyNonce, pSecPairwise->Handshake.SNonce, LEN_KEY_DESC_NONCE);
	else if (!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1)) {
		NdisMoveMemory(pMsg->KeyDesc.KeyNonce, pSecGroup->Handshake.GNonce, LEN_KEY_DESC_NONCE);
		/* Fill key IV - WPA2 as 0, WPA1 as random*/
		/* Suggest IV be random number plus some number,*/
		NdisMoveMemory(pMsg->KeyDesc.KeyIv, &pSecGroup->Handshake.GNonce[16], LEN_KEY_DESC_IV);
		pMsg->KeyDesc.KeyIv[15] += 2;
	}

	/* Fill Key RSC field		 */
	/* It contains the RSC for the GTK being installed.*/
	if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1)) {
#ifdef DOT11_EHT_BE
		if (pEntry->mlo.mlo_en)
			NdisZeroMemory(pMsg->KeyDesc.KeyRsc, 6);
		else
#endif
			NdisMoveMemory(pMsg->KeyDesc.KeyRsc, pSecGroup->Handshake.RSC, 6);
	}

	/* Clear Key MIC field for MIC calculation later   */
#ifdef OCE_FILS_SUPPORT
	if (!IS_AKM_FILS_Entry(pEntry))
#endif /* OCE_FILS_SUPPORT */
	{
		if (IS_AKM_SHA384(pSecPairwise->AKMMap) || (pSecPairwise->key_deri_alg == SEC_KEY_DERI_SHA384))
			NdisZeroMemory(pMsg->KeyDesc.KeyMicAndData, LEN_KEY_DESC_MIC_SHA384);
		else
			NdisZeroMemory(pMsg->KeyDesc.KeyMicAndData, LEN_KEY_DESC_MIC);
	}
	WPAConstructEapolKeyData(pEntry,
							 MsgType,
							 KeyDescVer,
							 pSecPairwise,
							 pSecGroup,
							 pMsg);

	/* Calculate MIC and fill in KeyMic Field except Pairwise Msg 1.*/
#ifdef OCE_FILS_SUPPORT
	if (!IS_AKM_FILS_Entry(pEntry))
#endif /* OCE_FILS_SUPPORT */
	if (MsgType != EAPOL_PAIR_MSG_1)
		WPACalculateMIC(KeyDescVer, pSecPairwise->AKMMap, pSecPairwise->PTK, pSecPairwise->key_deri_alg, pMsg);

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_DEBUG, "===>for %s %s\n", ((bWPA2) ? "WPA2" : "WPA"), GetEapolMsgType(MsgType));
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_DEBUG, "	     Body length = %d\n", CONV_ARRARY_TO_UINT16(pMsg->Body_Len));
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_DEBUG, "	     Key length  = %d\n", CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyLength));
}

static VOID parse_generic_sec(
	IN  UCHAR * ie,
	IN  UCHAR ie_len,
	OUT UCHAR * *wpa_ie,
	OUT UCHAR * wpa_ie_len,
	OUT UCHAR * *rsn_ie,
	OUT UCHAR * rsn_ie_len)
{
	PEID_STRUCT         pEid;
	while (ie_len > sizeof(RSNIE3)) {
		pEid = (PEID_STRUCT) ie;
		/* Check for RSN IE presence */
		if (pEid->Eid == IE_RSN) {
			*rsn_ie = ie;
			*rsn_ie_len = ie[1] + 2;
		}
		/* Check for WPA IE presence */
		else if ((pEid->Eid == IE_WPA) && (NdisEqualMemory(pEid->Octet, WPA_OUI, 4))) {
			*wpa_ie = ie;
			*wpa_ie_len = ie[1] + 2;
		}
		ie += (pEid->Len + 2);
		ie_len  -= (pEid->Len + 2);
	}

	if (*wpa_ie)
		hex_dump("WPA IE:", *wpa_ie, *wpa_ie_len);
	if (*rsn_ie)
		hex_dump("RSN IE:", *rsn_ie, *rsn_ie_len);
}

static void get_ie_ptr_from_key_data(
	IN  PUCHAR          data,
	IN  UCHAR           data_len,
	OUT struct key_data_element_ptr *ele_ptr)
{
	PUCHAR vie_ptr;
	int len;
	PEID_STRUCT eid_ptr;

	NdisZeroMemory(ele_ptr, sizeof(struct key_data_element_ptr));

	vie_ptr = data;
	len	 = (int)data_len;

	while (len > 0) {
		eid_ptr = (PEID_STRUCT) vie_ptr;

		/* WPA2 RSN IE */
		if (eid_ptr->Eid == IE_RSN)
			ele_ptr->rsne_ptr = (UCHAR *)eid_ptr;
		else if (eid_ptr->Eid == IE_RSNXE)
			ele_ptr->rsnxe_ptr = (UCHAR *)eid_ptr;
#ifdef DPP_SUPPORT
		else if ((eid_ptr->Eid == IE_RSN) && (NdisEqualMemory(eid_ptr->Octet, DPP_OUI, 4)))
			ele_ptr->dpp_ptr = (UCHAR *)eid_ptr;
#endif
#ifdef CONFIG_HOTSPOT_R2
		else if ((eid_ptr->Eid == IE_WPA) && (NdisEqualMemory(eid_ptr->Octet, OSEN_IE, 4)))
			ele_ptr->osen_ptr = (UCHAR *)eid_ptr;
#endif /* CONFIG_HOTSPOT_R2 */
		/* WPA IE*/
		else if ((eid_ptr->Eid == IE_WPA) && (NdisEqualMemory(eid_ptr->Octet, WPA_OUI, 4)))
			ele_ptr->wpa_ie_ptr = (UCHAR *)eid_ptr;
#ifdef DOT11R_FT_SUPPORT
		else if (eid_ptr->Eid == IE_FT_MDIE)
			ele_ptr->mdie_ptr = (UCHAR *)eid_ptr;
		else if (eid_ptr->Eid == IE_FT_FTIE)
			ele_ptr->ftie_ptr = (UCHAR *)eid_ptr;
#endif
		else if (eid_ptr->Eid == WPA_KDE_TYPE) {
			KDE_HDR *kde = (KDE_HDR *)eid_ptr;

			if (NdisEqualMemory(kde->OUI, OUI_WPA2_CIPHER, 3)) {
				if (kde->DataType == KDE_GTK)
					ele_ptr->gtk_kde_ptr = (UCHAR *)eid_ptr;
				else if (kde->DataType == KDE_IGTK)
					ele_ptr->igtk_kde_ptr = (UCHAR *)eid_ptr;
				else if (kde->DataType == KDE_BIGTK)
					ele_ptr->bigtk_kde_ptr = (UCHAR *)eid_ptr;
				else if (kde->DataType == KDE_OCI)
					ele_ptr->oci_kde_ptr = (UCHAR *)eid_ptr;
#ifdef DOT11_EHT_BE
				else if (kde->DataType == KDE_MAC_ADDR) {
					ele_ptr->mac_addr_kde_ptr = (UCHAR *)eid_ptr;
				} else if (kde->DataType == KDE_MLO_LINK) {
					ele_ptr->mlo_link_kde_ptr[ele_ptr->mlo_link_num++] = (UCHAR *)eid_ptr;
				} else if (kde->DataType == KDE_MLO_GTK) {
					ele_ptr->mlo_gtk_kde_ptr[ele_ptr->mlo_gtk_num++] = (UCHAR *)eid_ptr;
					hex_dump("KDE_MLO_GTK", (UCHAR *)kde, (kde->Len)+2);
				}
				else if (kde->DataType == KDE_MLO_IGTK)
					ele_ptr->mlo_igtk_kde_ptr[ele_ptr->mlo_igtk_num++] = (UCHAR *)eid_ptr;
				else if (kde->DataType == KDE_MLO_BIGTK)
					ele_ptr->mlo_bigtk_kde_ptr[ele_ptr->mlo_bigtk_num++] = (UCHAR *)eid_ptr;
#endif
				else
					MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_WARN,
						"unknown wpa2 kde(%d) in eapol key data\n", kde->DataType);
			} else if (NdisEqualMemory(kde->OUI, wfa_oui, 3)) {
				if (kde->DataType == WFA_KDE_TTI)
					ele_ptr->tti_kde_ptr = (UCHAR *)eid_ptr;
				else
					MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_WARN,
						"unknown wfa kde(%d) in eapol key data\n", kde->DataType);
			}
		} else
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_WARN,
				"unknown ie(%d) in eapol key data\n", eid_ptr->Eid);

		if (eid_ptr->Len + 2 > len) { /* it would be a normal behavior if need to padding */
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_DEBUG,
				"IE len is larger than total len\n");
			hex_dump_with_cat_and_lvl("key data", data, data_len, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_DEBUG);
			break;
		}
		vie_ptr += (eid_ptr->Len + 2);
		len  -= (eid_ptr->Len + 2);
	}
}


static BOOLEAN RTMPCheckRSNIE(
	IN  struct key_data_element_ptr *ele_ptr,
	IN  BOOLEAN         bWPA2,
	IN  MAC_TABLE_ENTRY *pEntry)
{
	PEID_STRUCT         pEid;
	UCHAR *ptr;
	BOOLEAN		    result = FALSE;
	PUCHAR		    wpa_ie = NULL;
	PUCHAR		    rsn_ie = NULL;
	UCHAR		    wpa_ie_len = 0;
	UCHAR		    rsn_ie_len = 0;


	parse_generic_sec(&pEntry->RSN_IE[0], pEntry->RSNIE_Len, &wpa_ie, &wpa_ie_len, &rsn_ie, &rsn_ie_len);

		/* WPA2 RSN IE */
	if (ele_ptr->rsne_ptr) {
		ptr = ele_ptr->rsne_ptr;
		pEid = (EID_STRUCT *) ptr;

		if ((bWPA2) &&
			(rsn_ie && (pEid->Eid == rsn_ie[0])) &&
			((pEid->Len + 2) >= (rsn_ie[1] + 2))) {

			if (NdisEqualMemory(ptr, rsn_ie, rsn_ie[1] + 2)) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
					"==> RSN IE matched, Length(%d)\n",
						(pEid->Len + 2));
				result = TRUE;
			}
#ifdef DOT11R_FT_SUPPORT
			else {
				struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;

				if (IS_AKM_FT_WPA2PSK(pSecConfig->AKMMap) ||
				    IS_AKM_FT_WPA2(pSecConfig->AKMMap) ||
				    IS_AKM_FT_SAE_SHA256(pSecConfig->AKMMap) ||
					IS_AKM_FT_WPA2_SHA384(pSecConfig->AKMMap)) {
					PUINT8 pRecSuite = NULL, pConSuite = NULL;
					UINT8 RecCount = 0, ConCount = 0;
					BOOLEAN CheckRes = TRUE;

					/* group cipher*/
					pRecSuite = WPA_ExtractSuiteFromRSNIE(ptr, (pEid->Len + 2), GROUP_SUITE, &RecCount);
					pConSuite = WPA_ExtractSuiteFromRSNIE(rsn_ie, rsn_ie[1] + 2, GROUP_SUITE, &ConCount);
					if ((RecCount == ConCount) && (pRecSuite) && (pConSuite) &&
					    NdisEqualMemory(pRecSuite, pConSuite, (4 * RecCount))) {
					} else {
						if (pRecSuite)
							hex_dump("RecIE", pRecSuite, (4 * RecCount));

						if (pConSuite)
							hex_dump("ConIE", pConSuite, (4 * ConCount));
						MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
							"GROUP_SUITE mis-matched under FT\n");
						CheckRes = FALSE;
					}

					/* PAIRWISE_SUITE */
					pRecSuite = WPA_ExtractSuiteFromRSNIE(ptr, (pEid->Len + 2), PAIRWISE_SUITE, &RecCount);
					pConSuite = WPA_ExtractSuiteFromRSNIE(rsn_ie, rsn_ie[1] + 2, PAIRWISE_SUITE, &ConCount);
					if ((RecCount == ConCount) && (pRecSuite) && (pConSuite) &&
					     NdisEqualMemory(pRecSuite, pConSuite, (4 * RecCount))) {
					} else {
						if (pRecSuite)
							hex_dump("RecIE", pRecSuite, (4 * RecCount));

						if (pConSuite)
							hex_dump("ConIE", pConSuite, (4 * ConCount));
						MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
							"PAIRWISE_SUITE mis-matched under FT\n");
						CheckRes = FALSE;
					}

					/* AKM_SUITE */
					pRecSuite = WPA_ExtractSuiteFromRSNIE(ptr, (pEid->Len + 2), AKM_SUITE, &RecCount);
					pConSuite = WPA_ExtractSuiteFromRSNIE(rsn_ie, rsn_ie[1] + 2, AKM_SUITE, &ConCount);
					if ((RecCount == ConCount) && (pRecSuite) && (pConSuite) &&
						 NdisEqualMemory(pRecSuite, pConSuite, (4 * RecCount))) {
					} else {
						if (pRecSuite)
							hex_dump("RecIE", pRecSuite, (4 * RecCount));

						if (pConSuite)
							hex_dump("ConIE", pConSuite, (4 * ConCount));
						MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
							"AKM_SUITE mis-matched under FT\n");
						CheckRes = FALSE;
					}


					/* RSN_CAP_INFO */
					pRecSuite = WPA_ExtractSuiteFromRSNIE(ptr, (pEid->Len + 2), RSN_CAP_INFO, &RecCount);
					pConSuite = WPA_ExtractSuiteFromRSNIE(rsn_ie, rsn_ie[1] + 2, RSN_CAP_INFO, &ConCount);
					if ((RecCount == ConCount) && (pRecSuite) && (pConSuite) &&
					     NdisEqualMemory(pRecSuite, pConSuite, sizeof(RSN_CAPABILITIES))) {
					} else {
						MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
							"RSN_CAP_INFO mis-matched under FT\n");
						CheckRes = FALSE;
					}

					if (CheckRes) {
						MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
							"==> RSN IE matched under FT and Length(%d)\n",
							(pEid->Len + 2));
						result = TRUE;
					}
				}
			}
#endif /* DOT11R_FT_SUPPORT */
		} else
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
				"RSN IE Lens mis-matched\n");

	}
#ifdef DPP_SUPPORT
	if (ele_ptr->dpp_ptr) {
		ptr = ele_ptr->dpp_ptr;
		pEid = (EID_STRUCT *) ptr;
		if ((NdisEqualMemory(ptr, DPP_OUI, 4))) {
			result = TRUE;
		}
	}
#endif /* DPP_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2
	if (ele_ptr->osen_ptr) {
		ptr = ele_ptr->osen_ptr;
		pEid = (EID_STRUCT *) ptr;
		if ((CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE) &&
			(pEid->Eid == pEntry->OSEN_IE[0]) &&
			(pEid->Len + 2) >= pEntry->OSEN_IE_Len) &&
			(NdisEqualMemory(ptr, pEntry->OSEN_IE, pEntry->OSEN_IE_Len))) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
				"==> HS2.0 OSEN IE matched, Length(%d)\n",
					(pEid->Len + 2));
			result = TRUE;

		} else
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
				"HS2.0 OSEN IE mis-matched\n");
	}
#endif /* CONFIG_HOTSPOT_R2 */
	/* WPA IE*/
	if (ele_ptr->wpa_ie_ptr) {
		ptr = ele_ptr->wpa_ie_ptr;
		pEid = (EID_STRUCT *) ptr;
		if ((!bWPA2) &&
			(wpa_ie_len == (pEid->Len + 2)) &&
			(wpa_ie && NdisEqualMemory(ptr, wpa_ie, wpa_ie[1] + 2))) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
				"WPA/WPAPSK IE matched, Length(%d)\n",
				(pEid->Len + 2));
			result = TRUE;

		} else {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
				"WPA/WPAPSK IE mis-matched\n");
		}

	}

	return result;

}

BOOLEAN WPAParseEapolKeyData(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pKeyData,
	IN UINT16 KeyDataLen,
	IN	UCHAR GroupKeyIndex,
	IN	UCHAR MsgType,
	IN	BOOLEAN bWPA2,
	IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR GTK[MAX_LEN_GTK];
	UCHAR GTKLEN = 0;
	UCHAR DefaultIdx = 0;
	struct key_data_element_ptr ele_ptr;
	EID_STRUCT *eid_ptr;
	KDE_HDR *kde_ptr;
#ifdef DOT11W_PMF_SUPPORT
	UCHAR IGTK[LEN_MAX_IGTK];
	UCHAR IGTKLEN = 0;
	UCHAR IPN[LEN_WPA_TSC];
	UINT8 IGTK_KeyIdx;/* It shall be 4 or 5 */
#endif /*DOT11W_PMF_SUPPORT */
	u8 is_mlo_en = FALSE;
	u8 is_single_link = FALSE;
	u32 is_ie_presnet;
#ifdef DOT11_EHT_BE
	struct mld_entry_t *mld_entry = NULL;
#endif /* DOT11_EHT_BE */

#ifdef DOT11_EHT_BE
	if (pEntry) {
		is_mlo_en = pEntry->mlo.mlo_en;
		if (is_mlo_en) {
			mt_rcu_read_lock();
			mld_entry = get_mld_entry_by_mac(pEntry->mlo.mld_addr);
			if (mld_entry)
				is_single_link = (mld_entry->link_num == 1) ? TRUE : FALSE;
			else
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
					"mld_entry=NULL\n");
			mt_rcu_read_unlock();
		}
	}
#endif

	is_ie_presnet = is_eapol_key_data_element_present(MsgType, bWPA2, is_mlo_en, is_single_link);

	NdisZeroMemory(GTK, MAX_LEN_GTK);
	if (KeyDataLen >= 255) { /* Check boundary before assign u16 to u8 */
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
					"(line%d)invalid key len(=%d)\n", __LINE__, KeyDataLen);
		return FALSE;
	}
	get_ie_ptr_from_key_data(pKeyData, (UCHAR)KeyDataLen, &ele_ptr);

	/* Verify The RSN IE contained in pairewise_msg_2 && pairewise_msg_3 and skip it*/
	if (is_ie_presnet & RSNE_PRESNET) {
		/* Check RSN IE whether it is WPA2/WPA2PSK */
		if (pEntry && (!RTMPCheckRSNIE(&ele_ptr, bWPA2, pEntry))) {
			eid_ptr = (EID_STRUCT *) ele_ptr.rsne_ptr;
			/* send wireless event - for RSN IE different*/
			RTMPSendWirelessEvent(pAd, IW_RSNIE_DIFF_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "RSN_IE Different in msg %d of 4-way handshake!\n", MsgType);
			if (ele_ptr.rsne_ptr)
				hex_dump("Receive RSN_IE ",
					ele_ptr.rsne_ptr, eid_ptr->Len + 2);
			else
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
					"Peer doesn't have rsn ie!\n");
			hex_dump("Desired RSN_IE ", pEntry->RSN_IE, pEntry->RSNIE_Len);
			return FALSE;
		} else {
			if (bWPA2)
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
				"RTMPParseEapolKeyData ==> WPA2/WPA2PSK RSN IE matched in Msg 3\n");
			else
				return TRUE;
		}
#ifdef MAP_R3
		if (!(IS_MAP_ENABLE(pAd) && IS_MAP_CERT_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd))) {
#endif
			if ((is_ie_presnet & RSNXE_PRESNET) && ele_ptr.rsnxe_ptr) {
				UINT status_code;
				eid_ptr = (EID_STRUCT *) ele_ptr.rsnxe_ptr;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
					"peer rsnxe is presented\n");
				status_code = parse_rsnxe_ie(&pEntry->SecConfig, (UCHAR *)eid_ptr, eid_ptr->Len + 2, FALSE);
				if (status_code != MLME_SUCCESS)
					return FALSE;
			}
#ifdef DOT11_SAE_SUPPORT
			if (pEntry && IS_AKM_SAE(pEntry->SecConfig.AKMMap) &&
				(pEntry->SecConfig.sae_conn_type || pEntry->SecConfig.rsnxe_len != 0) &&
				ele_ptr.rsnxe_ptr == NULL) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
					"Should include rsnxe in 4-way msg2/3, rsnxe_len = %d, sae_conn_type = %d\n",
					pEntry->SecConfig.rsnxe_len, pEntry->SecConfig.sae_conn_type);
				return FALSE;
			}
#endif
#ifdef MAP_R3
		}
#endif
	}

	/* Parse KDE format in pairwise_msg_3_WPA2 && group_msg_1_WPA2*/
	if (bWPA2 && is_ie_presnet & GTK_PRESNET) {

		if (ele_ptr.gtk_kde_ptr) {
			GTK_KDE *pKdeGtk;

			kde_ptr = (KDE_HDR *) ele_ptr.gtk_kde_ptr;

			pKdeGtk = (PGTK_KDE) &kde_ptr->octet[0];
			DefaultIdx = pKdeGtk->Kid;
			/* Get GTK length - refer to IEEE 802.11i-2004 p.82 */
			GTKLEN = kde_ptr->Len - 6;

			if ((GTKLEN < LEN_WEP40) || (GTKLEN > LEN_MAX_GTK)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "ERROR: GTK Key length is invalid (%d)\n", GTKLEN);
				return FALSE;
			}

			NdisMoveMemory(GTK, pKdeGtk->GTK, GTKLEN);
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "GTK in KDE format ,DefaultKeyID=%d, KeyLen=%d\n", DefaultIdx, GTKLEN);
		}
#ifdef APCLI_SUPPORT
		/* copy transition disable bitmap from msg3 to STA */
		if (ele_ptr.tti_kde_ptr &&
		(pEntry && pEntry->wdev &&
		(pEntry->wdev->wdev_type == WDEV_TYPE_STA))) {

			PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);

			kde_ptr = (KDE_HDR *) ele_ptr.tti_kde_ptr;

			if (pStaCfg && pStaCfg->ApCliTransDisableSupported) {
				os_move_mem(&(pStaCfg->ApCli_tti_bitmap),
						kde_ptr->octet,
						sizeof(struct transition_disable_bitmap));
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
					"Transition Disable bits from AP: wpa3_psk:%d sae_pk:%d wpa3_ent:%d OWE:%d\n",
						pStaCfg->ApCli_tti_bitmap.wpa3_psk,
						pStaCfg->ApCli_tti_bitmap.sae_pk,
						pStaCfg->ApCli_tti_bitmap.wpa3_ent,
						pStaCfg->ApCli_tti_bitmap.enhanced_open);
			} else {
				if (pStaCfg)
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
					"Apcli Transition Disable:ApCliTransDisableSupported:%d\n", pStaCfg->ApCliTransDisableSupported);
				else
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
						"Apcli Transition Disable::pStaCfg is NULL\n");
			}
		}
#endif /* #ifdef APCLI_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT
		if (ele_ptr.igtk_kde_ptr) {
			kde_ptr = (KDE_HDR *) ele_ptr.igtk_kde_ptr;

			if (PMF_ExtractIGTKKDE(&kde_ptr->octet[0], kde_ptr->Len - 4, IGTK, &IGTKLEN, IPN, &IGTK_KeyIdx) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "PMF_ExtractIGTKKDE: FAIL\n");
				return FALSE;
			}
		}
#endif /* DOT11W_PMF_SUPPORT */
	} else if (is_ie_presnet & GTK_PRESNET) { /* WPA1 case */
		DefaultIdx = GroupKeyIndex;
		GTKLEN = (UCHAR)KeyDataLen;
		if (GTKLEN > LEN_MAX_GTK) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
				"ERROR: GTK Key length is invalid (%d)\n", GTKLEN);
			return FALSE;
		}
		NdisMoveMemory(GTK, pKeyData, KeyDataLen);
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
			"GTK without KDE, DefaultKeyID=%d, KeyLen=%d\n", DefaultIdx, GTKLEN);
	}

	if (pEntry && (is_ie_presnet & OCI_PRESNET)) {
		if (ele_ptr.oci_kde_ptr) {
			kde_ptr = (KDE_HDR *) ele_ptr.oci_kde_ptr;

			if (!parse_oci_kde(pEntry->pAd, pEntry->wdev, &kde_ptr->octet[0],  kde_ptr->Len - 4))
				return FALSE;
		} else if (pEntry->SecConfig.ocv_support) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
				"ERROR: oci kde isn't existed\n");
			return FALSE;
		}
	}

	/* Sanity check - shared key index must be 0 ~ 3*/
	if (DefaultIdx > 3) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "ERROR: GTK Key index(%d) is invalid in %s %s\n", DefaultIdx, ((bWPA2) ? "WPA2" : "WPA"), GetEapolMsgType(MsgType));
		return FALSE;
	}

#ifdef DOT11_EHT_BE
	if (pEntry) {
		if (is_ie_presnet & MAC_ADDR_PRESNET) {
			if (!ele_ptr.mac_addr_kde_ptr) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
					"ERROR: mac addr kde isn't existed\n");
				return FALSE;
			}

			kde_ptr = (KDE_HDR *) ele_ptr.mac_addr_kde_ptr;
			if (!parse_mac_addr_kde(
						pEntry->mlo.mld_addr,
						&kde_ptr->octet[0],  kde_ptr->Len - 4)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
					"ERROR: mac addr kde parse fail.\n");
				return FALSE;
			}
		}

		if (is_ie_presnet & MLO_LINK_PRESNET) {
			if (ele_ptr.mlo_link_num == 0) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
					"ERROR: mac link kde isn't existed\n");
				return FALSE;
			}

			if (parse_all_mlo_link_kde(pEntry,
				ele_ptr.mlo_link_kde_ptr, ele_ptr.mlo_link_num)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
					"ERROR: mlo link kde parse fail.\n");
				return FALSE;
			}
		}

		/* for sta mode(M2-4way, M1-2way) */
		if (is_ie_presnet & MLO_GTK_PRESNET) {
			if (ele_ptr.mlo_gtk_num == 0) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
					"ERROR: mac gtk kde isn't existed\n");
				return FALSE;
			}

			if (parse_all_mlo_gtk_kde(pEntry,
				ele_ptr.mlo_gtk_kde_ptr, ele_ptr.mlo_gtk_num)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
					"ERROR: mlo gtk kde parse fail.\n");
				return FALSE;
			}
		}
	}
#endif

#ifdef CONFIG_STA_SUPPORT
	if (pEntry && IS_ENTRY_PEER_AP(pEntry)) {
		ASIC_SEC_INFO Info = {0};

		/* set key material, TxMic and RxMic		*/
		NdisMoveMemory(pEntry->SecConfig.GTK, GTK, GTKLEN);
		pEntry->SecConfig.GroupKeyId = DefaultIdx;
		/* Set key material to Asic */
		os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
		Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
		Info.Direction = SEC_ASIC_KEY_RX;
		Info.Wcid = pEntry->wdev->bss_info_argument.bmc_wlan_idx;
		Info.BssIndex = BSS0;
		Info.Cipher = pEntry->SecConfig.GroupCipher;
		Info.KeyIdx = pEntry->SecConfig.GroupKeyId;
		os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
		os_move_mem(Info.Key.Key, pEntry->SecConfig.GTK, LEN_MAX_GTK);
#ifdef DOT11W_PMF_SUPPORT

		if (IGTKLEN != 0) {
#ifdef SOFT_ENCRYPT
			UINT8 idx = 0;
			PPMF_CFG pPmfCfg = &pAd->StaCfg[pEntry->func_tb_idx].wdev.SecConfig.PmfCfg;

			pPmfCfg->IGTK_KeyIdx = IGTK_KeyIdx;

			if (pPmfCfg->IGTK_KeyIdx == 5)
				idx = 1;

			NdisMoveMemory(&pPmfCfg->IPN[idx][0], IPN, LEN_WPA_TSC);
			NdisMoveMemory(&pPmfCfg->IGTK[idx][0], IGTK, IGTKLEN);
			/* hex_dump("IGTK===>",IGTK,IGTKLEN); */
#endif /* SOFT_ENCRYPT */
			Info.Cipher |= pEntry->SecConfig.PmfCfg.igtk_cipher;
			os_move_mem(Info.IGTK, IGTK, IGTKLEN);
			Info.IGTKKeyLen = IGTKLEN;
		}

#endif /* DOT11W_PMF_SUPPORT */

#ifdef DOT11_EHT_BE
		if (pEntry->mlo.mlo_en)
			mlo_install_group_key(pEntry, FALSE, TRUE);
		else
#endif
		{
			/* Prevent the GTK reinstall key attack */
			if ((pEntry->SecConfig.LastGroupKeyId != pEntry->SecConfig.GroupKeyId) ||
				!NdisEqualMemory(pEntry->SecConfig.LastGTK, pEntry->SecConfig.GTK, LEN_MAX_GTK)) {
				WPAInstallKey(pAd, &Info, FALSE, TRUE);
				pEntry->SecConfig.LastGroupKeyId = pEntry->SecConfig.GroupKeyId;
				os_move_mem(pEntry->SecConfig.LastGTK, pEntry->SecConfig.GTK, LEN_MAX_GTK);
				pEntry->AllowUpdateRSC = TRUE;
			} else {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
					"the Group reinstall attack, skip install key (%d)\n", pEntry->wcid);
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}


BOOLEAN WpaMessageSanity(
	IN PRTMP_ADAPTER pAd,
	IN PEAPOL_PACKET pMsg,
	IN ULONG MsgLen,
	IN UCHAR MsgType,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry,
	IN UCHAR *PTK)
{
	UCHAR mic[LEN_KEY_DESC_MIC_MAX], digest[80]; /*, KEYDATA[MAX_LEN_OF_RSNIE];*/
	UCHAR *KEYDATA = NULL;
	BOOLEAN bWPA2 = TRUE;
	KEY_INFO EapolKeyInfo;
	BOOLEAN bReplayDiff = FALSE;
	PHANDSHAKE_PROFILE pHandshake4Way  = &pSecConfig->Handshake;
	UINT8 mic_len = LEN_KEY_DESC_MIC;
	UINT8 *key_data_len_ptr = NULL;
	UINT8 *key_data_ptr = NULL;
	UINT16 key_data_len = 0;

	/* 0. Check MsgType*/
	if ((MsgType > EAPOL_GROUP_MSG_2) || (MsgType < EAPOL_PAIR_MSG_1)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "The message type is invalid(%d)!\n", MsgType);
		return FALSE;
	}

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&KEYDATA, MAX_LEN_OF_RSNIE);

	if (KEYDATA == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "Allocate memory fail!!!\n");
		return FALSE;
	}

	NdisZeroMemory(mic, sizeof(mic));
	NdisZeroMemory(digest, sizeof(digest));
	NdisZeroMemory(KEYDATA, MAX_LEN_OF_RSNIE);
	NdisZeroMemory((PUCHAR)&EapolKeyInfo, sizeof(EapolKeyInfo));
	NdisMoveMemory((PUCHAR)&EapolKeyInfo, (PUCHAR)&pMsg->KeyDesc.KeyInfo, sizeof(KEY_INFO));
	*((USHORT *)&EapolKeyInfo) = cpu2le16(*((USHORT *)&EapolKeyInfo));

	/* Choose WPA2 or not*/
	if (IS_AKM_WPA1(pSecConfig->AKMMap) || IS_AKM_WPA1PSK(pSecConfig->AKMMap))
		bWPA2 = FALSE;

	/* 1. Replay counter check */
	if (MsgType == EAPOL_PAIR_MSG_1 || MsgType == EAPOL_PAIR_MSG_3 || MsgType == EAPOL_GROUP_MSG_1) { /* For supplicant*/
		/* First validate replay counter, only accept message with larger replay counter.*/
		/* Let equal pass, some AP start with all zero replay counter*/
		UCHAR ZeroReplay[LEN_KEY_DESC_REPLAY];

		NdisZeroMemory(ZeroReplay, LEN_KEY_DESC_REPLAY);

		if ((RTMPCompareMemory(pMsg->KeyDesc.ReplayCounter, pHandshake4Way->ReplayCounter, LEN_KEY_DESC_REPLAY) != 1) &&
			(RTMPCompareMemory(pMsg->KeyDesc.ReplayCounter, ZeroReplay, LEN_KEY_DESC_REPLAY) != 0))
			bReplayDiff = TRUE;
	} else if (MsgType == EAPOL_PAIR_MSG_2 || MsgType == EAPOL_PAIR_MSG_4 || MsgType == EAPOL_GROUP_MSG_2) {	/* For authenticator*/
		/* check Replay Counter coresponds to MSG from authenticator, otherwise discard*/
		if (!NdisEqualMemory(pMsg->KeyDesc.ReplayCounter, pHandshake4Way->ReplayCounter, LEN_KEY_DESC_REPLAY))
			bReplayDiff = TRUE;
	}

	/* Replay Counter different condition*/
	if (bReplayDiff) {
		/* send wireless event - for replay counter different*/
		RTMPSendWirelessEvent(pAd, IW_REPLAY_COUNTER_DIFF_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);

		if (MsgType < EAPOL_GROUP_MSG_1)
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "Replay Counter Different in pairwise msg %d of 4-way handshake!\n", MsgType);
		else
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "Replay Counter Different in group msg %d of 2-way handshake!\n", (MsgType - EAPOL_PAIR_MSG_4));

		hex_dump("Receive replay counter ", pMsg->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);
		hex_dump("Current replay counter ", pHandshake4Way->ReplayCounter, LEN_KEY_DESC_REPLAY);
		goto LabelErr;
	}

	if (IS_AKM_SHA384(pSecConfig->AKMMap) || (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384))
		mic_len  = LEN_KEY_DESC_MIC_SHA384;

#ifdef OCE_FILS_SUPPORT
    /* from 802.11AI, when using AEAD cipher, the EAPOL Key MIC is not present */
	if (IS_AKM_FILS_Entry(pEntry))
		mic_len = 0;
#endif /* OCE_FILS_SUPPORT */

	key_data_len_ptr = pMsg->KeyDesc.KeyMicAndData + mic_len;
	key_data_ptr = key_data_len_ptr + 2;

	/* 2. Verify MIC except Pairwise Msg1*/
	if ((MsgType != EAPOL_PAIR_MSG_1)
#ifdef OCE_FILS_SUPPORT
	/* from 802.11AI, when using AEAD cipher, the EAPOL Key MIC is not present */
		&& (!IS_AKM_FILS_Entry(pEntry))
#endif /* OCE_FILS_SUPPORT */
	   ){
		UCHAR rcvd_mic[LEN_KEY_DESC_MIC_MAX];
		UINT eapol_len = CONV_ARRARY_TO_UINT16(pMsg->Body_Len) + 4;
		UINT min_eapol_len = MIN_LEN_OF_EAPOL_KEY_FRAME_EXCEPT_MIC + mic_len;
		UINT max_eapol_len = min_eapol_len + MAX_LEN_OF_RSNIE; /* MAX_LEN_OF_RSNIE: len for KEYDATA alloc_mem */
		/* Record the received MIC for check later*/
		NdisMoveMemory(rcvd_mic, pMsg->KeyDesc.KeyMicAndData, mic_len);
		NdisZeroMemory(pMsg->KeyDesc.KeyMicAndData, mic_len);

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"peer KeyDescVer = %d\n", EapolKeyInfo.KeyDescVer);

		if (EapolKeyInfo.KeyDescVer == KEY_DESC_TKIP) { /* TKIP*/
			if ((eapol_len < min_eapol_len) || (eapol_len > max_eapol_len))
				goto LabelErr;
			RT_HMAC_MD5(PTK, LEN_PTK_KCK, (PUCHAR)pMsg, eapol_len, mic, MD5_DIGEST_SIZE);
		} else if (EapolKeyInfo.KeyDescVer == KEY_DESC_AES) {	/* AES        */
			if ((eapol_len < min_eapol_len) || (eapol_len > max_eapol_len))
				goto LabelErr;
			RT_HMAC_SHA1(PTK, LEN_PTK_KCK, (PUCHAR)pMsg, eapol_len, digest, SHA1_DIGEST_SIZE);
			NdisMoveMemory(mic, digest, LEN_KEY_DESC_MIC);
		} else if (EapolKeyInfo.KeyDescVer == KEY_DESC_NOT_DEFINED && IS_AKM_SHA384(pSecConfig->AKMMap)) {
			if ((eapol_len < min_eapol_len) || (eapol_len > max_eapol_len))
				goto LabelErr;
			RT_HMAC_SHA384(PTK, LEN_PTK_KCK_SHA384, (PUCHAR)pMsg, eapol_len, mic, LEN_KEY_DESC_MIC_SHA384);
		} else if (IS_AKM_WPA3PSK(pSecConfig->AKMMap)) {
			UINT mlen = AES_KEY128_LENGTH;
			AES_CMAC((PUCHAR)pMsg, eapol_len, PTK, LEN_PTK_KCK, mic, &mlen);
		} else if (IS_AKM_OWE(pSecConfig->AKMMap) && (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA256)) {
			/*FIXME: OWE SHA521 support*/
			if ((eapol_len < min_eapol_len) || (eapol_len > max_eapol_len))
				goto LabelErr;
			RT_HMAC_SHA256(PTK, LEN_PTK_KEK, (PUCHAR)pMsg, eapol_len, mic, LEN_KEY_DESC_MIC);
#ifdef DPP_SUPPORT
		} else if (IS_AKM_DPP(pSecConfig->AKMMap) && (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA256)) {
			if ((eapol_len < min_eapol_len) || (eapol_len > max_eapol_len))
				goto LabelErr;
			RT_HMAC_SHA256(PTK, LEN_PTK_KEK, (PUCHAR)pMsg, eapol_len, mic, LEN_KEY_DESC_MIC);
#endif /* DPP_SUPPORT */
		} else if (IS_AKM_OWE(pSecConfig->AKMMap) && (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384)) {
			if ((eapol_len < min_eapol_len) || (eapol_len > max_eapol_len))
				goto LabelErr;
			RT_HMAC_SHA384(PTK,
				       LEN_PTK_KCK_SHA384,
				       (PUCHAR)pMsg,
				       eapol_len,
				       mic,
				       LEN_KEY_DESC_MIC_SHA384);
		} else if ((EapolKeyInfo.KeyDescVer == KEY_DESC_EXT)
			|| (EapolKeyInfo.KeyDescVer == KEY_DESC_NOT_DEFINED)) { /* AES-128 */
			UINT mlen = AES_KEY128_LENGTH;
			AES_CMAC((PUCHAR)pMsg, eapol_len, PTK, LEN_PTK_KCK, mic, &mlen);
		}

		if (!NdisEqualMemory(rcvd_mic, mic, LEN_KEY_DESC_MIC)) {
			/* send wireless event - for MIC different*/
			RTMPSendWirelessEvent(pAd, IW_MIC_DIFF_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);

			if (MsgType < EAPOL_GROUP_MSG_1)
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "MIC Different in pairwise msg %d of 4-way handshake!\n", MsgType);
			else
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "MIC Different in group msg %d of 2-way handshake!\n", (MsgType - EAPOL_PAIR_MSG_4));

			pHandshake4Way->ReasonCode = REASON_NO_LONGER_VALID;

			hex_dump("Received MIC", rcvd_mic, LEN_KEY_DESC_MIC);
			hex_dump("Desired  MIC", mic, LEN_KEY_DESC_MIC);
			goto LabelErr;
		}
	}

	/* 1. Decrypt the Key Data field if GTK is included.*/
	/* 2. Extract the context of the Key Data field if it exist.	 */
	/* The field in pairwise_msg_2_WPA1(WPA2) & pairwise_msg_3_WPA1 is clear.*/
	/* The field in group_msg_1_WPA1(WPA2) & pairwise_msg_3_WPA2 is encrypted.*/
	key_data_len = CONV_ARRARY_TO_UINT16(key_data_len_ptr);
	if ((key_data_len > 0) && (key_data_len <= MAX_LEN_OF_RSNIE)) {
		UCHAR GroupKeyIndex = 0;

		/* Decrypt this field */
		if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1)
#ifdef OCE_FILS_SUPPORT
			|| (IS_AKM_FILS_Entry(pEntry) && (MsgType == EAPOL_PAIR_MSG_2))
#endif /* OCE_FILS_SUPPORT */
			) {
#ifdef OCE_FILS_SUPPORT
			if (IS_AKM_FILS_Entry(pEntry) && (key_data_len <= MAX_LEN_OF_RSNIE)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
					"AEAD Decrypt the message 2\n");
				NdisMoveMemory(KEYDATA, key_data_ptr, key_data_len);
			} else
#endif /* OCE_FILS_SUPPORT */
			if ((EapolKeyInfo.KeyDescVer == KEY_DESC_EXT)
				|| (EapolKeyInfo.KeyDescVer == KEY_DESC_AES)
				|| (EapolKeyInfo.KeyDescVer == KEY_DESC_NOT_DEFINED)) {
				UINT aes_unwrap_len = 0;
				UINT8 kck_len = LEN_PTK_KCK;
				UINT8 kek_len = LEN_PTK_KEK;

				if (IS_AKM_SHA384(pSecConfig->AKMMap)
					|| (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384)) {
					kck_len = LEN_PTK_KCK_SHA384;
					kek_len = LEN_PTK_KEK_SHA384;
				}

				/* AES */

				AES_Key_Unwrap(key_data_ptr,
						key_data_len,
						&PTK[kck_len], kek_len,
						KEYDATA, &aes_unwrap_len);
				SET_UINT16_TO_ARRARY(key_data_len_ptr, aes_unwrap_len);
			} else {
				TKIP_GTK_KEY_UNWRAP(&PTK[LEN_PTK_KCK],
						pMsg->KeyDesc.KeyIv,
						key_data_ptr,
						key_data_len,
						KEYDATA);
			}

			if (!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))
				GroupKeyIndex = EapolKeyInfo.KeyIndex;
		} else if ((MsgType == EAPOL_PAIR_MSG_2)
					|| (MsgType == EAPOL_GROUP_MSG_2)
					|| (MsgType == EAPOL_PAIR_MSG_3 && !bWPA2)) {
			if (key_data_len <= MAX_LEN_OF_RSNIE)
				NdisMoveMemory(KEYDATA, key_data_ptr, key_data_len);
		} else { /* it may have PMKID to check in msg1 */
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_DEBUG,
				"The Key Data Length should be zero !!!\n");
			goto LabelOK;
		}

		/* Parse Key Data field to */
		/* 1. verify RSN IE for pairwise_msg_2_WPA1(WPA2) ,pairwise_msg_3_WPA1(WPA2)*/
		/* 2. verify KDE format for pairwise_msg_3_WPA2, group_msg_1_WPA2*/
		/* 3. update shared key for pairwise_msg_3_WPA2, group_msg_1_WPA1(WPA2)*/
		if (!WPAParseEapolKeyData(pAd, KEYDATA, key_data_len,
								  GroupKeyIndex, MsgType, bWPA2, pEntry))
			goto LabelErr;
	}

LabelOK:

	if (KEYDATA != NULL)
		os_free_mem(KEYDATA);

	return TRUE;
LabelErr:
#ifdef CONFIG_MAP_SUPPORT
#ifdef MAP_R3
		if (IS_MAP_ENABLE(pAd) &&
			(!IS_AKM_SAE(pSecConfig->AKMMap) &&  !IS_AKM_DPP(pSecConfig->AKMMap))
			&& IS_MAP_R3_ENABLE(pAd) &&
			pEntry &&
			(pAd->ReconfigTrigger == TRUE) &&
			(pEntry->wdev->wdev_type == WDEV_TYPE_STA)) {
			wapp_send_trigger_reconfig(pAd, pEntry->wdev);
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "Reconfigtrig\n");
		}
#endif /* MAP_R3 */
#endif /* CONFIG_MAP_SUPPORT */
	if (KEYDATA != NULL)
		os_free_mem(KEYDATA);

	return FALSE;
}


VOID WPABuildPairMsg1(
	IN PRTMP_ADAPTER pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR Header802_3[14];
	UCHAR *mpool;
	PEAPOL_PACKET	pEapolFrame;
	STA_TR_ENTRY *tr_entry;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	UINT GroupMsg1Len = 0;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
		"===> wcid = %d\n", pEntry->wcid);

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : The interface is closed...\n");
		return;
	}

	if ((!pEntry) || IS_ENTRY_NONE(pEntry)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : The entry doesn't exist.\n");
		return;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		BSS_STRUCT *pMbss = (BSS_STRUCT *)pEntry->wdev->func_dev;

		if (pEntry->wdev != &pMbss->wdev) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "[ERROR] : cannot get binding wdev(%p).\n", pEntry->wdev);
			return;
		}
	}

#endif /* CONFIG_AP_SUPPORT */
	pHandshake4Way = &pSecConfig->Handshake;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"===> STA(:"MACSTR"), WpaState(%d)\n",
			MAC2STR(pEntry->Addr), pHandshake4Way->WpaState);

	/* Check the status*/
	if ((pHandshake4Way->WpaState > AS_PTKSTART) || (pHandshake4Way->WpaState < AS_INITPMK)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR] : Not expect calling\n");
		return;
	}

#ifdef WSC_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		BSS_STRUCT *pMbss = (BSS_STRUCT *)pEntry->wdev->func_dev;

		if (MAC_ADDR_EQUAL(pEntry->Addr, pMbss->wdev.WscControl.EntryAddr) &&
			pMbss->wdev.WscControl.EapMsgRunning) {
			pHandshake4Way->WpaState = AS_NOTUSE;
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
				"This is a WSC-Enrollee. Not expect calling WPAStart4WayHS here\n");
			return;
		}
	}
#endif /* WSC_AP_SUPPORT */
	/* Increment replay counter by 1*/
	ADD_ONE_To_64BIT_VAR(pHandshake4Way->ReplayCounter);
	/* Randomly generate ANonce */
	GenRandom(pAd, (UCHAR *)pHandshake4Way->AAddr, pHandshake4Way->ANonce);
	/* Allocate memory for output*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);

	if (mpool == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "no memory!!!\n");
		return;
	}

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	/* Construct EAPoL message - Pairwise Msg 1*/
	/* EAPOL-Key(0,0,1,0,P,0,0,ANonce,0,DataKD_M1) */
	pHandshake4Way->MsgType = EAPOL_PAIR_MSG_1;
	WPAConstructEapolMsg(pEntry,
						 EAPOL_PAIR_MSG_1,
						 pSecConfig, /* Pairwise */
						 &pEntry->wdev->SecConfig, /* Group */
						 pEapolFrame);
#ifdef CONFIG_AP_SUPPORT

	/* If PMKID match in WPA2-enterprise mode, fill PMKID into Key data field and update PMK here	*/
	if ((IS_AKM_WPA2(pSecConfig->AKMMap) || IS_AKM_WPA3_192BIT(pSecConfig->AKMMap) || IS_AKM_OWE(pSecConfig->AKMMap))
		&& (is_pmkid_cache_in_sec_config(pSecConfig))) {
		UINT8 mic_len = LEN_KEY_DESC_MIC;
		UINT8 *key_data_len_ptr = NULL;
		UINT8 *key_data_ptr = NULL;
		UINT8 pmk_len = LEN_PMK;

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "===> Add PMKID\n");

		if (IS_AKM_SHA384(pSecConfig->AKMMap) || (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384)) {
			mic_len = LEN_KEY_DESC_MIC_SHA384;
			pmk_len = LEN_PMK_SHA384;
		}

		hex_dump("PMKID", pSecConfig->pmkid, LEN_PMKID);
		hex_dump("PMK", pSecConfig->pmk_cache, pmk_len);

		key_data_len_ptr = pEapolFrame->KeyDesc.KeyMicAndData + mic_len;
		key_data_ptr = key_data_len_ptr + CONV_ARRARY_TO_UINT16(key_data_len_ptr) + 2;
		/* Fill in value for KDE */
		key_data_ptr[0] = 0xDD;
		key_data_ptr[2] = 0x00;
		key_data_ptr[3] = 0x0F;
		key_data_ptr[4] = 0xAC;
		key_data_ptr[5] = 0x04;
		NdisMoveMemory(key_data_ptr + 6, pSecConfig->pmkid, LEN_PMKID);
		NdisMoveMemory(&pSecConfig->PMK, pSecConfig->pmk_cache, pmk_len);
		key_data_ptr[1] = 0x14;/* 4+LEN_PMKID*/
		INC_UINT16_TO_ARRARY(key_data_len_ptr, 6 + LEN_PMKID);
		INC_UINT16_TO_ARRARY(pEapolFrame->Body_Len, 6 + LEN_PMKID);
	}

#ifdef DOT11W_PMF_SUPPORT
	else if ((IS_AKM_WPA3PSK(pSecConfig->AKMMap) ||
#ifdef DPP_SUPPORT
		  IS_AKM_DPP(pSecConfig->AKMMap) ||
#endif /* DPP_SUPPORT */
		  IS_AKM_OWE(pSecConfig->AKMMap)) &&
		  (pSecConfig->PmfCfg.UsePMFConnect == TRUE)) {
		UCHAR digest[80], PMK_key[20];
		UINT8 mic_len = LEN_KEY_DESC_MIC;
		UINT8 *key_data_len_ptr = NULL;
		UINT8 *key_data_ptr = NULL;
		UINT8 pmk_len = LEN_PMK;

		if (IS_AKM_SHA384(pSecConfig->AKMMap) || pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384) {
			mic_len = LEN_KEY_DESC_MIC_SHA384;
			pmk_len = LEN_PMK_SHA384;
		}

		key_data_len_ptr = pEapolFrame->KeyDesc.KeyMicAndData + mic_len;
		key_data_ptr = key_data_len_ptr + CONV_ARRARY_TO_UINT16(key_data_len_ptr) + 2;

		key_data_ptr[0] = 0xDD;
		key_data_ptr[2] = 0x00;
		key_data_ptr[3] = 0x0F;
		key_data_ptr[4] = 0xAC;
		key_data_ptr[5] = 0x04;

		if (is_pmkid_cache_in_sec_config(pSecConfig) == FALSE) {
			NdisMoveMemory(&PMK_key[0], "PMK Name", 8);
			NdisMoveMemory(&PMK_key[8], pHandshake4Way->AAddr, MAC_ADDR_LEN);
			NdisMoveMemory(&PMK_key[14], pHandshake4Way->SAddr, MAC_ADDR_LEN);

#ifdef DOT11_SAE_SUPPORT
			if (IS_AKM_SAE(pSecConfig->AKMMap)) {
				UCHAR is_mlo_connect = FALSE;
				struct __SAE_CFG *sae_cfg = PD_GET_SAE_CFG_PTR(pAd->physical_dev);

#ifdef DOT11_EHT_BE
				if (pEntry->mlo.mlo_en)
					is_mlo_connect = TRUE;
#endif

				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
					"[SAE]pmkid not found\n");
				if (!sae_get_pmk_cache(sae_cfg, pHandshake4Way->AAddr, pHandshake4Way->SAddr, is_mlo_connect, digest, NULL)) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, "derive pmkid fail\n");
					os_free_mem(mpool);
					return;
				}
			}
#endif
#ifdef CONFIG_OWE_SUPPORT
			if (IS_AKM_OWE(pSecConfig->AKMMap)) {
				OWE_INFO *owe = &pSecConfig->owe;

				NdisMoveMemory(key_data_ptr + 6, owe->pmkid, LEN_PMKID);
			} else
#endif /*CONFIG_OWE_SUPPORT*/
				NdisMoveMemory(key_data_ptr + 6, digest, LEN_PMKID);
		} else {
			NdisMoveMemory(key_data_ptr + 6, pSecConfig->pmkid, LEN_PMKID);
			NdisMoveMemory(&pSecConfig->PMK, pSecConfig->pmk_cache, LEN_PMK);
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
					"PMKID found for WPA2PSK/WPA3PSK\n");
		}

		key_data_ptr[1] = 0x14;/* 4+LEN_PMKID */
		INC_UINT16_TO_ARRARY(key_data_len_ptr, 6 + LEN_PMKID);
		INC_UINT16_TO_ARRARY(pEapolFrame->Body_Len, 6 + LEN_PMKID);
	}

#endif /* DOT11W_PMF_SUPPORT */
#endif
	/* Make outgoing frame: Authenticator send to Supplicant */
	MAKE_802_3_HEADER(Header802_3, pHandshake4Way->SAddr, pHandshake4Way->AAddr, EAPOL);
	tr_entry = tr_entry_get(pAd, pEntry->wcid);
	GroupMsg1Len = CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4;
	/*add for colgin coverity ALPS05331062*/
	if ((GroupMsg1Len < (MIN_LEN_OF_EAPOL_KEY_MSG - LEN_KEY_DESC_MIC))
		|| (GroupMsg1Len > TX_EAPOL_BUFFER)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"<===  error!pair Message 1 length = %d\n",
			GroupMsg1Len);
		os_free_mem(mpool);
		return;
	}
	RTMPToWirelessSta(pAd, pEntry, Header802_3,
					  LENGTH_802_3, (PUCHAR)pEapolFrame,
					  GroupMsg1Len,
					  (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);
	/* Trigger Retry Timer*/
	RTMPModTimer(&pHandshake4Way->MsgRetryTimer, PEER_MSG1_RETRY_EXEC_INTV);
	/* Update State*/
	pHandshake4Way->WpaState = AS_PTKSTART;
	pHandshake4Way->GTKState = REKEY_NEGOTIATING;
	os_free_mem(mpool);
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "<=== send Msg1 of 4-way\n");
}

VOID WPABuildPairMsg2(
	IN PRTMP_ADAPTER pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR Header802_3[14];
	UCHAR *mpool;
	PEAPOL_PACKET	pEapolFrame;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	BOOLEAN is_unencrypted = TRUE;
	STA_TR_ENTRY *tr_entry;
	UINT GroupMsg1Len = 0;

	/* Allocate memory for output*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);

	if (mpool == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "no memory!!!\n");
		return;
	}


	pHandshake4Way = &pSecConfig->Handshake;
	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	/* Construct EAPoL message - Pairwise Msg 2*/
	/*  EAPOL-Key(0,1,0,0,P,0,0,SNonce,MIC,DataKD_M2)*/
	WPAConstructEapolMsg(pEntry,
						 EAPOL_PAIR_MSG_2,
						 pSecConfig, /* Pairwise */
						 pSecConfig, /* Group */
						 pEapolFrame);
	/* Make outgoing frame: Supplicant send to Authenticator */
	MAKE_802_3_HEADER(Header802_3, pHandshake4Way->AAddr, pHandshake4Way->SAddr, EAPOL);

	/* Encrypt EAPOL frames if session is present */
	tr_entry = tr_entry_get(pAd, pEntry->wcid);
	if (pSecConfig->is_eapol_encrypted && tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
		is_unencrypted = FALSE;
	GroupMsg1Len = CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4;
	/*add for colgin coverity ALPS05330464*/
	if ((GroupMsg1Len < (MIN_LEN_OF_EAPOL_KEY_MSG - LEN_KEY_DESC_MIC))
			|| (GroupMsg1Len > TX_EAPOL_BUFFER)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"<=== error!pair Message 2 length = %d\n",
			GroupMsg1Len);
		os_free_mem(mpool);
		return;
	}
	RTMPToWirelessSta(pAd, pEntry,
					  Header802_3, sizeof(Header802_3), (PUCHAR)pEapolFrame,
					  GroupMsg1Len, is_unencrypted);
	RtmpusecDelay(6000);    //Added Delay for workaround of SAE-5.7.3 Test case
	os_free_mem(mpool);
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "<=== send Msg2 of 4-way\n");
}


VOID WPABuildPairMsg3(
	IN PRTMP_ADAPTER    pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR Header802_3[14];
	UCHAR *mpool;
	PEAPOL_PACKET pEapolFrame;
	STA_TR_ENTRY *tr_entry;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	ASIC_SEC_INFO Info = {0};
	UINT32 pn_type_mask = TSC_TYPE_GTK_PN_MASK;
	UCHAR tx_tsc[MAX_TSC_TYPE * LEN_WPA_TSC] = {0};
#if defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT)
	UCHAR HSClientGTK[32];
	/* UCHAR *gtk_ptr = NULL; */
#endif
	UINT GroupMsg1Len = 0;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"===> STA(:"MACSTR")\n", MAC2STR(pEntry->Addr));

#ifdef OCE_FILS_SUPPORT
	if (IS_AKM_FILS_Entry(pEntry)) {
		struct fils_info *filsInfo = &pEntry->filsInfo;
		if (filsInfo->is_pending_decrypt) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
				"1x daemon process the rquest and ongoing on this line\n");
			return;
		}
	}
#endif /* OCE_FILS_SUPPORT */

	/* Allocate memory for input*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);

	if (mpool == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "no memory!!!\n");
		return;
	}

	pHandshake4Way = &pSecConfig->Handshake;
	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	/* Increment replay counter by 1*/
	ADD_ONE_To_64BIT_VAR(pHandshake4Way->ReplayCounter);

	if (pEntry->wdev
#ifdef DOT11_EHT_BE
		&& !pEntry->mlo.mlo_en
#endif
		) {
		struct _SECURITY_CONFIG *sec_cfg_group = &pEntry->wdev->SecConfig;

#ifdef BCN_PROTECTION_SUPPORT
		if (sec_cfg_group->bcn_prot_cfg.bcn_prot_en)
			pn_type_mask |= TSC_TYPE_BIGTK_PN_MASK;
#endif

		/* Get Group TxTsc form Asic*/
		AsicGetTxTsc(pAd, pEntry->wdev, pn_type_mask, tx_tsc);

		os_move_mem(sec_cfg_group->Handshake.RSC, tx_tsc, LEN_WPA_TSC);

#ifdef BCN_PROTECTION_SUPPORT
		if (sec_cfg_group->bcn_prot_cfg.bcn_prot_en) {
			UCHAR key_idx = get_bigtk_table_idx(&sec_cfg_group->bcn_prot_cfg);

			os_move_mem(&sec_cfg_group->bcn_prot_cfg.bipn[key_idx][0], &tx_tsc[LEN_WPA_TSC], LEN_WPA_TSC);
		}
#endif
	}

#if defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT)

	if (pEntry->wdev
		&& pEntry->wdev->func_idx >= 0
		&& pAd->ApCfg.MBSSID[pEntry->wdev->func_idx].HotSpotCtrl.HotSpotEnable
		&& pAd->ApCfg.MBSSID[pEntry->wdev->func_idx].HotSpotCtrl.DGAFDisable) {
		/* Radom GTK for hotspot sation client */
		GenRandom(pAd, pEntry->Addr, HSClientGTK);
		/* gtk_ptr = HSClientGTK; */
		os_move_mem(pSecConfig->HsUniGTK, HSClientGTK, LEN_MAX_GTK);
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"Random unique GTK for each mobile device when dgaf disable\n");
		hex_dump("GTK", pSecConfig->HsUniGTK, 32);
	}

#endif
	/* Set key material to Asic */
	os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
	Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
	Info.Direction = SEC_ASIC_KEY_BOTH;
	Info.Wcid = pEntry->wcid;
	Info.BssIndex = pEntry->func_tb_idx;
	Info.Cipher = pSecConfig->PairwiseCipher;
	Info.KeyIdx = pSecConfig->PairwiseKeyId;
	os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);

#ifdef OCE_FILS_SUPPORT
	if (IS_AKM_FILS(pSecConfig->AKMMap)) {
		UCHAR kck_len = 0, kek_len = 0, tk_len = 0;

		tk_len = sec_get_cipher_key_len(pSecConfig->PairwiseCipher);
		kck_len = sec_get_kck_len(pSecConfig);
		kek_len = sec_get_kek_len(pSecConfig);

		os_move_mem(Info.Key.Key, &pSecConfig->PTK[kck_len + kek_len], tk_len);
	} else
#endif /* OCE_FILS_SUPPORT */
	{
		if (IS_AKM_SHA384(pSecConfig->AKMMap) || (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384))
			os_move_mem(Info.Key.Key, &pSecConfig->PTK[LEN_PTK_KCK_SHA384 + LEN_PTK_KEK_SHA384], LEN_TK_SHA384);
		else
			os_move_mem(Info.Key.Key, &pSecConfig->PTK[LEN_PTK_KCK + LEN_PTK_KEK], (LEN_TK + LEN_TK2));
	}

	if (pAd->FragFrame.wcid == pEntry->wcid) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"\nClear Wcid = %d FragBuffer !!!!!\n",
			pEntry->wcid);
		RESET_FRAGFRAME(pAd->FragFrame);
	}

#ifdef DOT11_EHT_BE
	if (pEntry->mlo.mlo_en)
		mlo_install_key(pEntry, &Info, TRUE, TRUE);
	else
#endif
		WPAInstallKey(pAd, &Info, TRUE, TRUE);
	/* Construct EAPoL message - Pairwise Msg 3*/
	pHandshake4Way->MsgType = EAPOL_PAIR_MSG_3;
	if (pEntry->wdev) {
		WPAConstructEapolMsg(pEntry,
							 EAPOL_PAIR_MSG_3,
							 pSecConfig, /* Pairwise */
							 &pEntry->wdev->SecConfig, /* Group */
							 pEapolFrame);
	}

#ifdef OCE_FILS_SUPPORT
	if (IS_AKM_FILS_Entry(pEntry)) {
		struct fils_info *filsInfo = &pEntry->filsInfo;

		if (!filsInfo->is_pending_encrypt) {
			if (filsInfo->pending_ie) {
				os_free_mem(filsInfo->pending_ie);
				filsInfo->pending_ie = NULL;
				filsInfo->pending_ie_len = 0;
			}

			filsInfo->pending_ie_len = CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4;
			/*add for colgin coverity ALPS05330949*/
			if ((filsInfo->pending_ie_len < (MIN_LEN_OF_EAPOL_KEY_MSG - LEN_KEY_DESC_MIC))
				|| (filsInfo->pending_ie_len > TX_EAPOL_BUFFER)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA,
					DBG_LVL_ERROR, "<=== error!pair Message 3 length = %d pending_ie_len\n",
					filsInfo->pending_ie_len);
				os_free_mem(mpool);
				return;
			}
			os_alloc_mem(NULL, (UCHAR **)&filsInfo->pending_ie, filsInfo->pending_ie_len);
			if (!filsInfo->pending_ie) {
				os_free_mem(mpool);
				return;
			}

			NdisZeroMemory(filsInfo->pending_ie, filsInfo->pending_ie_len);
			NdisCopyMemory(filsInfo->pending_ie, pEapolFrame, filsInfo->pending_ie_len);
			filsInfo->is_pending_encrypt = TRUE;

			DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_AEAD_ENCR_EVENT);
			os_free_mem(mpool);
			return;
		}
	}
#endif /* OCE_FILS_SUPPORT */

	/* Make outgoing frame: Authenticator send to Supplicant */
	MAKE_802_3_HEADER(Header802_3, pHandshake4Way->SAddr, pHandshake4Way->AAddr, EAPOL);
	tr_entry = tr_entry_get(pAd, pEntry->wcid);
	GroupMsg1Len = CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4;
	if ((GroupMsg1Len < (MIN_LEN_OF_EAPOL_KEY_MSG))
		|| (GroupMsg1Len > TX_EAPOL_BUFFER)) {/*add for colgin coverity ALPS05330949*/
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"<=== error!pair Message 3 length = %d\n",
			GroupMsg1Len);
		os_free_mem(mpool);
		return;
	}
	RTMPToWirelessSta(pAd, pEntry, Header802_3, LENGTH_802_3,
					  (PUCHAR)pEapolFrame,
					  GroupMsg1Len,
					  (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);
	RTMPSetTimer(&pHandshake4Way->MsgRetryTimer, PEER_MSG3_RETRY_EXEC_INTV);
	os_free_mem(mpool);
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "<=== send Msg3 of 4-way\n");
}


VOID WPABuildPairMsg4(
	IN PRTMP_ADAPTER pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR Header802_3[14];
	UCHAR *mpool;
	STA_TR_ENTRY *tr_entry;
	PEAPOL_PACKET	pEapolFrame;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	ASIC_SEC_INFO Info = {0};
	BOOLEAN is_unencrypted = TRUE;
	UINT GroupMsg1Len = 0;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pApCliEntry;
#endif /* CONFIG_STA_SUPPORT */
	if (pEntry == NULL) {/*ALPS05330180*/
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "pEntry is NULL!\n");
		return;
	}

	/* Allocate memory for output*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);

	if (mpool == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "no memory!!!\n");
		return;
	}

	pHandshake4Way = &pSecConfig->Handshake;
	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	/* Construct EAPoL message - Pairwise Msg 4*/
	WPAConstructEapolMsg(pEntry,
						 EAPOL_PAIR_MSG_4,
						 pSecConfig, /* Pairwise */
						 pSecConfig, /* Group */
						 pEapolFrame);
	/* Update WpaState*/
	pSecConfig->Handshake.WpaState = AS_PTKINITDONE;
	/* Set key material to Asic */
	os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
	Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
	Info.Direction = SEC_ASIC_KEY_BOTH;
	Info.Wcid = pEntry->wcid;
	Info.BssIndex = pEntry->func_tb_idx;
	Info.Cipher = pSecConfig->PairwiseCipher;
	Info.KeyIdx = pSecConfig->PairwiseKeyId;
	os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);

	if (IS_AKM_SHA384(pSecConfig->AKMMap) || (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384))
		os_move_mem(Info.Key.Key, &pSecConfig->PTK[LEN_PTK_KCK_SHA384 + LEN_PTK_KEK_SHA384], LEN_TK_SHA384);
	else
		os_move_mem(Info.Key.Key, &pSecConfig->PTK[LEN_PTK_KCK + LEN_PTK_KEK], (LEN_TK + LEN_TK2));
	tr_entry = tr_entry_get(pAd, pEntry->wcid);
	if (tr_entry == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "tr_entry is NULL!\n");
		return;
	}

	pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;


	/* Prevent the M3 reinstall attack, install key only Nonce changed */
	if (pHandshake4Way->AllowInsPTK == TRUE) {

		if (pAd->FragFrame.wcid == pEntry->wcid) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
				"\n Clear Wcid = %d FragBuffer !!!!!\n",
				pEntry->wcid);
			RESET_FRAGFRAME(pAd->FragFrame);
		}
#ifdef DOT11_EHT_BE
		if (pEntry->mlo.mlo_en) {
			mlo_install_key(pEntry, &Info, FALSE, TRUE);
			mlo_update_port_secure(pEntry, NULL);
		} else
#endif
		{
			/* move port secured before EAPOL msg sent
			out to prevent drop on LMAC AC reset,
			related to FAST_EAPOL_WAR */
			WPAInstallKey(pAd, &Info, FALSE, TRUE);
			WifiSysUpdatePortSecur(pAd, pEntry, NULL);
			RtmpusecDelay(5000);/*delay 5ms for wpa key install and update port*/
		}
		pHandshake4Way->AllowInsPTK = FALSE;
		pEntry->AllowUpdateRSC = TRUE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"The M3 reinstall attack, skip install key\n");
	}

	/* Make outgoing frame: Supplicant send to Authenticator */
	MAKE_802_3_HEADER(Header802_3, pHandshake4Way->AAddr, pHandshake4Way->SAddr, EAPOL);
	if (pSecConfig->is_eapol_encrypted)
		is_unencrypted = FALSE;
	GroupMsg1Len = CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4;
	/*add for colgin coverity ALPS05330838*/
	if ((GroupMsg1Len < (MIN_LEN_OF_EAPOL_KEY_MSG - LEN_KEY_DESC_MIC))
		|| (GroupMsg1Len > TX_EAPOL_BUFFER)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"<=== error!pair Message 4 length = %d\n",
			GroupMsg1Len);
		os_free_mem(mpool);
		return;
	}
	RTMPToWirelessSta(pAd, pEntry,
					  Header802_3, sizeof(Header802_3), (PUCHAR)pEapolFrame,
					  GroupMsg1Len, is_unencrypted);

	os_free_mem(mpool);
	tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (IS_ENTRY_PEER_AP(pEntry)) {
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			UCHAR	ifIdx = 0;
			PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pEntry->func_tb_idx];

			if (pStaCfg->ApCliAutoConnectRunning == TRUE) {
				pStaCfg->ApCliAutoConnectRunning = FALSE;
				MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "Apcli auto connected:WPABuildPairMsg4(),pAd->ApCfg.ApCliAutoConnectRunning[%d]=%d\n",
						 ifIdx, pStaCfg->ApCliAutoConnectRunning);
			}
#endif /* APCLI_AUTO_CONNECT_SUPPORT*/

#ifdef CONFIG_STA_SUPPORT
	pApCliEntry = &pAd->StaCfg[tr_entry->func_tb_idx];
#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT) && !defined(MTK_HOSTAPD_SUPPORT)
	if (IS_MAP_ENABLE(pAd) && IS_CIPHER_AES_Entry(pEntry) &&
		(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))/*For security AES case*/
		wapp_send_apcli_association_change(WAPP_APCLI_ASSOCIATED, pAd, pApCliEntry);
#endif /*WAPP_SUPPORT*/
#if defined(MWDS) || defined(CONFIG_MAP_SUPPORT)
			if (pEntry && (pEntry->func_tb_idx < MAX_APCLI_NUM) && tr_entry) {
				if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) {
#ifdef MWDS
					if ((!IS_AKM_WPA1(pSecConfig->AKMMap)) && (!IS_AKM_WPA1PSK(pSecConfig->AKMMap)))
						MWDSAPCliPeerEnable(pAd, &pAd->StaCfg[pEntry->func_tb_idx], pEntry);
#endif
#if defined(CONFIG_MAP_SUPPORT)  && !defined(MTK_HOSTAPD_SUPPORT)
					MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
							"APCLIENT MAP_ENABLE\n");
#ifdef A4_CONN
					if (IS_MAP_ENABLE(pAd)) {
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
						if (pEntry->mlo.mlo_en)
							map_a4_mlo_peer_enable(pAd, &pAd->StaCfg[pEntry->func_tb_idx], pEntry, FALSE);
						else
#endif
							map_a4_peer_enable(pAd, pEntry, FALSE);
					}
#endif
					map_send_bh_sta_wps_done_event(pAd, pEntry, FALSE);
#endif
#ifdef WAPP_SUPPORT
					//Saidul todo: Remove, no need to send from here
					wapp_send_ap_join_event(pAd, pApCliEntry);
#endif
			}
			}
#endif /* defined(MWDS) || defined(CONFIG_MAP_SUPPORT) */
#ifdef WH_EVENT_NOTIFIER
			if (pEntry && tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
				EventHdlr pEventHdlrHook = NULL;

				pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_EXT_UPLINK_STAT);
				if (pEventHdlrHook && pEntry->wdev)
					pEventHdlrHook(pAd, pEntry, (UINT32)WHC_UPLINK_STAT_CONNECTED);
			}
#endif /* WH_EVENT_NOTIFIER */
#endif /* CONFIG_STA_SUPPORT */
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	if (pEntry->wdev) {
		STA_PORT_SECURED_BY_WDEV(pAd, pEntry->wdev);
	} else {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"[ERROR]pEntry->wdev is null\n");
		return;
	}
#endif /* CONFIG_STA_SUPPORT */

	/* move port secured before EAPOL msg sent out to prevent drop on LMAC AC reset, related to FAST_EAPOL_WAR */
	/* WifiSysUpdatePortSecur(pAd, pEntry, &Info); */
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			 "===> WifiSysUpdatePortSecur called by, wcid=%d, PortSecured=%d\n",
			  pEntry->wcid, STATE_PORT_SECURE);

/* Cache the PMK against PMKID, generated in process_ecdh_element() */
#ifdef CONFIG_STA_SUPPORT
#if defined(CONFIG_OWE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
	if (IS_AKM_OWE(pSecConfig->AKMMap)) {
		OWE_INFO *owe = &pEntry->SecConfig.owe;
		PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);
		UINT pmk_len = LEN_PMK;
		struct wifi_dev *wdev = pEntry->wdev;
		UINT32 sec_akm = 0;

		SET_AKM_OWE(sec_akm);

		if (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384)
			pmk_len = LEN_PMK_SHA384;

		if (pStaCfg && owe->pmkid) {
			sta_add_pmkid_cache(pAd, pHandshake4Way->AAddr, owe->pmkid, pSecConfig->PMK, pmk_len, pStaCfg->wdev.func_idx, wdev, sec_akm
				, pStaCfg->Ssid, pStaCfg->SsidLen);
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_INFO,
							"Store PMKID for MAC=>"MACSTR"\n", MAC2STR(pEntry->Addr));
			hex_dump("PMK cache ID", owe->pmkid, LEN_PMKID);
			hex_dump("PMK key", pSecConfig->PMK, pmk_len);
		}
	}
#endif /*CONFIG_OWE_SUPPORT*/
#endif /*CONFIG_STA_SUPPORT*/

#if defined(DOT11_SAE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
	if (IS_AKM_WPA3PSK(pSecConfig->AKMMap)) {
		struct __SAE_CFG *sae_cfg = PD_GET_SAE_CFG_PTR(pAd->physical_dev);

		set_sae_instance_removable(
			sae_cfg, pSecConfig->Handshake.SAddr, pSecConfig->Handshake.AAddr);
	}
#endif

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "<=== send Msg4 of 4-way\n");
}


VOID WPABuildGroupMsg1(
	IN PRTMP_ADAPTER pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR Header802_3[14];
	UCHAR *mpool;
	PEAPOL_PACKET pEapolFrame;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	UINT32 pn_type_mask = TSC_TYPE_GTK_PN_MASK;
	UCHAR tx_tsc[MAX_TSC_TYPE * LEN_WPA_TSC] = {0};
	UINT GroupMsg1Len = 0;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "===>\n");

	if (pEntry->wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "pEntry->wdev is NULL!!!\n");
		return;
	}

	/* Allocate memory for output*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);

	if (mpool == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "no memory!!!\n");
		return;
	}

	pHandshake4Way = &pSecConfig->Handshake;
	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	/* Increment replay counter by 1*/
	ADD_ONE_To_64BIT_VAR(pSecConfig->Handshake.ReplayCounter);

	if (pEntry->wdev
#ifdef DOT11_EHT_BE
		&& !pEntry->mlo.mlo_en
#endif
		) {
		struct _SECURITY_CONFIG *sec_cfg_group = &pEntry->wdev->SecConfig;

#ifdef BCN_PROTECTION_SUPPORT
		if (sec_cfg_group->bcn_prot_cfg.bcn_prot_en)
			pn_type_mask |= TSC_TYPE_BIGTK_PN_MASK;
#endif

		/* Get Group TxTsc form Asic*/
		AsicGetTxTsc(pAd, pEntry->wdev, pn_type_mask, tx_tsc);

		os_move_mem(sec_cfg_group->Handshake.RSC, tx_tsc, LEN_WPA_TSC);

#ifdef BCN_PROTECTION_SUPPORT
		if (sec_cfg_group->bcn_prot_cfg.bcn_prot_en) {
			UCHAR key_idx = get_bigtk_table_idx(&sec_cfg_group->bcn_prot_cfg);

			os_move_mem(&sec_cfg_group->bcn_prot_cfg.bipn[key_idx][0], &tx_tsc[LEN_WPA_TSC], LEN_WPA_TSC);
		}
#endif
	}

	/* Construct EAPoL message - Group Msg 1*/
	pHandshake4Way->MsgType = EAPOL_GROUP_MSG_1;
	WPAConstructEapolMsg(pEntry,
						 EAPOL_GROUP_MSG_1,
						 pSecConfig, /* Pairwise */
						 &pEntry->wdev->SecConfig, /* Group */
						 pEapolFrame);
	/* Make outgoing frame: Authenticator send to Supplicant */
	MAKE_802_3_HEADER(Header802_3, pHandshake4Way->SAddr, pHandshake4Way->AAddr, EAPOL);
	GroupMsg1Len = CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4;
	/*add for colgin coverity ALPS05330430*/
	if ((GroupMsg1Len < (MIN_LEN_OF_EAPOL_KEY_MSG - LEN_KEY_DESC_MIC))
		|| (GroupMsg1Len > TX_EAPOL_BUFFER)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"<=== error! Group Message 1 length = %d\n",
			GroupMsg1Len);
		os_free_mem(mpool);
		return;
	}
	RTMPToWirelessSta(pAd, pEntry,
					  Header802_3, LENGTH_802_3,
					  (PUCHAR)pEapolFrame,
					  GroupMsg1Len, FALSE);
	os_free_mem(mpool);

	/* Trigger Retry Timer*/
	/* When group retry counter > limit, extend rekey time interval */
	if (pHandshake4Way->MsgRetryCounter > GROUP_MSG1_RETRY_LIMIT)
		RTMPModTimer(&pHandshake4Way->MsgRetryTimer, GROUP_MSG1_RETRY_EXEC_EXTEND);
	else
		RTMPModTimer(&pHandshake4Way->MsgRetryTimer, GROUP_MSG1_RETRY_EXEC_INTV);

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "<=== send out Group Message 1\n");
}


VOID WPABuildGroupMsg2(
	IN PRTMP_ADAPTER pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR Header802_3[14];
	UCHAR *mpool;
	PEAPOL_PACKET pEapolFrame;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	UINT GroupMsg1Len = 0;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "===>\n");
	if (pEntry == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "pEntry NULL!!!\n");
		return;
	}
	/* Allocate memory for output*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);

	if (mpool == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "no memory!!!\n");
		return;
	}

	pHandshake4Way = &pSecConfig->Handshake;
	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	/* Construct EAPoL message - Group Msg 2*/
	WPAConstructEapolMsg(pEntry,
						 EAPOL_GROUP_MSG_2,
						 pSecConfig, /* Pairwise */
						 pSecConfig, /* Group */
						 pEapolFrame);

    /* Make outgoing frame: Supplicant send to Authenticator */
	MAKE_802_3_HEADER(Header802_3, pHandshake4Way->AAddr, pHandshake4Way->SAddr, EAPOL);
	GroupMsg1Len = CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4;
	/*add for colgin coverity ALPS05330771*/
	if ((GroupMsg1Len < (MIN_LEN_OF_EAPOL_KEY_MSG - LEN_KEY_DESC_MIC))
		|| (GroupMsg1Len > TX_EAPOL_BUFFER)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"<=== error!group Message 2 length = %d\n",
			GroupMsg1Len);
		os_free_mem(mpool);
		return;
	}
	RTMPToWirelessSta(pAd, pEntry,
					  Header802_3, sizeof(Header802_3), (PUCHAR)pEapolFrame,
					  GroupMsg1Len, FALSE);
	os_free_mem(mpool);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
#ifdef A4_CONN
		if (pEntry && IS_ENTRY_PEER_AP(pEntry) && (pEntry->func_tb_idx < MAX_APCLI_NUM)) {
			STA_TR_ENTRY *tr_entry = NULL;

			if (IS_WCID_VALID(pAd, pEntry->wcid))
				tr_entry = tr_entry_get(pAd, pEntry->wcid);

			if (tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
#if defined(CONFIG_MAP_SUPPORT) && !defined(MTK_HOSTAPD_SUPPORT)
#if defined(WAPP_SUPPORT)
				PSTA_ADMIN_CONFIG pApCliEntry = &pAd->StaCfg[pEntry->func_tb_idx];
				/*For security TKIP case*/
				if (IS_MAP_ENABLE(pAd))
					wapp_send_apcli_association_change(WAPP_APCLI_ASSOCIATED,
							pAd, pApCliEntry);
#endif /*WAPP_SUPPORT*/
#endif /* CONFIG_MAP_SUPPORT */
#ifdef MWDS
				MWDSAPCliPeerEnable(pAd, &pAd->StaCfg[pEntry->func_tb_idx], pEntry);
#endif
#if defined(CONFIG_MAP_SUPPORT) && !defined(MTK_HOSTAPD_SUPPORT)
				if (IS_MAP_ENABLE(pAd)) {
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
					if (pEntry->mlo.mlo_en)
						map_a4_mlo_peer_enable(pAd, &pAd->StaCfg[pEntry->func_tb_idx], pEntry, FALSE);
					else
#endif
						map_a4_peer_enable(pAd, pEntry, FALSE);
				}
#endif
		}
		}
#endif /* A4_CONN */
#ifdef WH_EVENT_NOTIFIER
		if (pEntry && IS_ENTRY_PEER_AP(pEntry)) {
			STA_TR_ENTRY *tr_entry = NULL;

			if (IS_WCID_VALID(pAd, pEntry->wcid))
				tr_entry = tr_entry_get(pAd, pEntry->wcid);

			if (tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
				EventHdlr pEventHdlrHook = NULL;

				pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_EXT_UPLINK_STAT);
				if (pEventHdlrHook && pEntry->wdev)
					pEventHdlrHook(pAd, pEntry, (UINT32)WHC_UPLINK_STAT_CONNECTED);
			}
		}
#endif /* WH_EVENT_NOTIFIER */
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "<=== send out Group Message 2\n");
}

VOID PeerPairMsg1Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem)
{
	UCHAR PTK[120] = {0};
	PEAPOL_PACKET pReceiveEapol;
	UINT MsgLen;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	unsigned char hdr_len = LENGTH_802_11;
	UINT8 len_ptk = LEN_AES_PTK;
	PHEADER_802_11 pHdr;
	struct time_log tl;
	PHEADER_802_11 pHeader;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
		"===> wcid = %d\n", pEntry->wcid);
#ifdef CONFIG_MAP_SUPPORT
#ifdef MAP_R3
	if ((pSecConfig->Handshake.WpaState == AS_PTKINIT_NEGOTIATING) &&
		(IS_MAP_ENABLE(pAd) &&
		IS_MAP_R3_ENABLE(pAd))) {
		pAd->ReconfigTrigger = TRUE;
	} else
		pAd->ReconfigTrigger = FALSE;
#endif /* MAP_R3 */
#endif /* CONFIG_MAP_SUPPORT */

	pHeader = (PHEADER_802_11)Elem->Msg;
	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
	if (pHeader->FC.Order == 1)
		hdr_len += LENGTH_802_11_HTC;
	if (pHeader->FC.SubType == SUBTYPE_QDATA)
		hdr_len += LENGTH_802_11_QOS_FIELD;

	pHdr = (PHEADER_802_11)Elem->Msg;
	if (Elem->MsgLen < (hdr_len + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
		return;

	pHandshake4Way = &pSecConfig->Handshake;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
		"AA = %02X:%02X:%02X:%02X:%02X:%02X,SA = %02X:%02X:%02X:%02X:%02X:%02X\n",
		PRINT_MAC(pHandshake4Way->AAddr), PRINT_MAC(pHandshake4Way->SAddr));

	/* skip 802.11_header(24-byte) and LLC_header(8) */
	pReceiveEapol = (PEAPOL_PACKET)&Elem->Msg[hdr_len + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;

	/* Sanity Check peer Pairwise message 2 - Replay Counter, MIC, RSNIE*/
	if (WpaMessageSanity(pAd, pReceiveEapol, MsgLen, EAPOL_PAIR_MSG_1, pSecConfig, pEntry, pSecConfig->PTK) == FALSE)
		return;

	/* Store Replay counter, it will use to verify message 3 and construct message 2*/
	NdisMoveMemory(pHandshake4Way->ReplayCounter, pReceiveEapol->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);
	/* Store ANonce*/
	NdisMoveMemory(pHandshake4Way->ANonce, pReceiveEapol->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);
	/* Generate random SNonce*/
	GenRandom(pAd, (UCHAR *)pHandshake4Way->SAddr, pHandshake4Way->SNonce);
	pHandshake4Way->AllowInsPTK = TRUE;
	pEntry->AllowUpdateRSC = FALSE;
	pEntry->SecConfig.LastGroupKeyId = 0;
	NdisZeroMemory(pEntry->SecConfig.LastGTK, LEN_MAX_GTK);

#ifdef CONFIG_MAP_SUPPORT
#ifdef MAP_R3
	if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd)) {
		if (is_pmkid_cache_in_sec_config(pSecConfig)) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
				"PMKID cached copy PMK from it\n");
			NdisMoveMemory(pSecConfig->PMK, pSecConfig->pmk_cache, LEN_PMK);
		}
	} else
#endif /* MAP_R3 */
#endif /* CONFIG_MAP_SUPPORT */
	if (is_pmkid_cache_in_sec_config(pSecConfig)) {
		NdisMoveMemory(pSecConfig->PMK, pSecConfig->pmk_cache, LEN_PMK);
	}

	if (IS_AKM_OWE(pSecConfig->AKMMap) && (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384))
		len_ptk = LEN_OWE_PTK_SHA384;
	else if (IS_AKM_SHA384(pSecConfig->AKMMap))
		len_ptk = LEN_PTK_SHA384;

	if (IS_AKM_SHA384(pSecConfig->AKMMap)
		|| pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384) {
		WpaDerivePTK_KDF_384(pSecConfig->PMK,
					  pHandshake4Way->ANonce,		/* ANONCE*/
					  pHandshake4Way->AAddr,
					  pHandshake4Way->SNonce,		/* SNONCE*/
					  pHandshake4Way->SAddr,
					  PTK,
					  len_ptk);
		NdisMoveMemory(pSecConfig->PTK, PTK, len_ptk);
		hex_dump("SHA384 PTK", PTK, len_ptk);
	} else if (IS_AKM_OWE(pSecConfig->AKMMap)) {
		INT key_len = LEN_PMK;

		if (len_ptk > LEN_AES_PTK)
			key_len = LEN_PMK_SHA384;
		WpaDerivePTK_KDF_256_w_pmk_len(pSecConfig->PMK,
					  pHandshake4Way->ANonce,		/* ANONCE*/
					  pHandshake4Way->AAddr,
					  pHandshake4Way->SNonce,		/* SNONCE*/
					  pHandshake4Way->SAddr,
					  PTK,
					  len_ptk,
					  key_len);
		NdisMoveMemory(pSecConfig->PTK, PTK, len_ptk);
		hex_dump("PTK", PTK, len_ptk);
	} else if (IS_AKM_SAE(pSecConfig->AKMMap)) {
		len_ptk = LEN_PTK_KCK + LEN_PTK_KEK;
		len_ptk += sec_get_cipher_key_len(pSecConfig->PairwiseCipher);
		WpaDerivePTK_KDF_256(pSecConfig->PMK,
					 pHandshake4Way->ANonce,		/* ANONCE*/
					 pHandshake4Way->AAddr,
					 pHandshake4Way->SNonce,		/* SNONCE*/
					 pHandshake4Way->SAddr,
					 PTK,
					 len_ptk);
		NdisMoveMemory(pSecConfig->PTK, PTK, len_ptk);
	} else
#ifdef DOT11W_PMF_SUPPORT
	if (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA256) {
		len_ptk = LEN_PTK_KCK + LEN_PTK_KEK;
		len_ptk += sec_get_cipher_key_len(pSecConfig->PairwiseCipher);
		WpaDerivePTK_KDF_256(pSecConfig->PMK,
					  pHandshake4Way->ANonce,		/* ANONCE*/
					  pHandshake4Way->AAddr,
					  pHandshake4Way->SNonce,		/* SNONCE*/
					  pHandshake4Way->SAddr,
					  PTK,
					  len_ptk);   /* Must is 48 bytes */
		NdisMoveMemory(pSecConfig->PTK, PTK, len_ptk);
		hex_dump("PTK", PTK, len_ptk);
	} else
#endif /* DOT11W_PMF_SUPPORT */
	{
		/* Derive PTK*/
		WpaDerivePTK(pSecConfig->PMK,
					 pHandshake4Way->ANonce,		/* ANONCE*/
					 pHandshake4Way->AAddr,
					 pHandshake4Way->SNonce,		/* SNONCE*/
					 pHandshake4Way->SAddr,
					 PTK,
					 LEN_PTK);
		NdisMoveMemory(pSecConfig->PTK, PTK, LEN_PTK);
		len_ptk = LEN_PTK;
	}
#ifdef MAP_R3
	pSecConfig->ptk_len = len_ptk;
	if (IS_AKM_SAE(pSecConfig->AKMMap)) {
		if (len_ptk > LEN_AES_PTK)
			pSecConfig->pmk_len = LEN_PMK_SHA384;
		else
			pSecConfig->pmk_len = LEN_PMK;
	} else
		pSecConfig->pmk_len = LEN_PMK;
#endif /* MAP_R3 */

	/* Update WpaState*/
	pSecConfig->Handshake.WpaState = AS_PTKINIT_NEGOTIATING;
	pSecConfig->is_eapol_encrypted = pHdr->FC.Wep;
	WPABuildPairMsg2(pAd, pSecConfig, pEntry);
	log_time_end(LOG_TIME_CONNECTION, "peer_msg1", DBG_LVL_INFO, &tl);
}


VOID PeerPairMsg2Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem)
{
	UCHAR PTK[120] = {0};
	BOOLEAN Cancelled;
	PHEADER_802_11 pHeader;
	PEAPOL_PACKET pReceiveEapol;
	UINT MsgLen;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	unsigned char hdr_len = LENGTH_802_11;
	UINT8 ptkLen = LEN_AES_PTK;
#ifdef DOT11R_FT_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	UCHAR FT_PMK_R0[32] = {0};
	UCHAR FT_PMK_R0_NAME[16] = {0};
	UCHAR FT_PMK_R1[32] = {0};
	UCHAR FT_PMK_R1_NAME[16] = {0};
	UCHAR PTK_NAME[16] = {0};
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11R_FT_SUPPORT */
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
		"===> wcid = %d\n", pEntry->wcid);

	if (Elem->MsgLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
		return;

	pHandshake4Way = &pSecConfig->Handshake;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"===> STA(:"MACSTR"), WpaState(%d)\n",
			MAC2STR(pEntry->Addr), pHandshake4Way->WpaState);

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
		"AA = %02X:%02X:%02X:%02X:%02X:%02X,SA = %02X:%02X:%02X:%02X:%02X:%02X\n",
		PRINT_MAC(pHandshake4Way->AAddr), PRINT_MAC(pHandshake4Way->SAddr));

	/* check Entry in valid State*/
	if (pHandshake4Way->WpaState < AS_PTKSTART)
		return;
	/* Prevent the Replayed Msg2 Attack */
	if (pHandshake4Way->WpaState == AS_PTKINITDONE) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"reject the Replayed Msg2\n");
		return;
	}

	/* pointer to 802.11 header*/
	pHeader = (PHEADER_802_11)Elem->Msg;
	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
	if (pHeader->FC.Order == 1)
		hdr_len += LENGTH_802_11_HTC;
	if (pHeader->FC.SubType == SUBTYPE_QDATA)
		hdr_len += LENGTH_802_11_QOS_FIELD;

	if (Elem->MsgLen < (hdr_len + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
		return;

	/* skip 802.11_header(24-byte) and LLC_header(8) */
	pReceiveEapol = (PEAPOL_PACKET)&Elem->Msg[hdr_len + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;

#ifdef OCE_FILS_SUPPORT
	if (IS_AKM_FILS_Entry(pEntry)) {
		struct fils_info *filsInfo = &pEntry->filsInfo;
		if (filsInfo->is_pending_decrypt) {
			if (filsInfo->last_pending_id == pHeader->Sequence) {
			    goto skip_ptk;
			} else {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
					"1x daemon ongoing\n");
				return;
			}
		}
	}
#endif /* OCE_FILS_SUPPORT */

	/* Store SNonce*/
	NdisMoveMemory(pHandshake4Way->SNonce, pReceiveEapol->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);

	if (IS_AKM_OWE(pSecConfig->AKMMap) && (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384))
		ptkLen = LEN_OWE_PTK_SHA384;
	else if (IS_AKM_SHA384(pSecConfig->AKMMap))
		ptkLen = LEN_PTK_SHA384;

#ifdef OCE_FILS_SUPPORT
	if (IS_AKM_FILS_SHA256(pSecConfig->AKMMap)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"receive the msg2 using FILS-SHA256\n");
		ptkLen = 0 + 32 + LEN_TK; /* FILS-SH256: KCK + KEK + TK */
		WpaDerivePTK_KDF_256(pSecConfig->PMK,
					  pHandshake4Way->ANonce,
					  pHandshake4Way->AAddr,
					  pHandshake4Way->SNonce,
					  pHandshake4Way->SAddr,
					  PTK,
					  ptkLen);
	} else if (IS_AKM_FILS_SHA384(pSecConfig->AKMMap)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"receive the msg2 using FILS-SHA384\n");
		ptkLen = 0 + 64 + LEN_TK; /* FILS-SH384: KCK + KEK + TK */
		WpaDerivePTK_KDF_384(pSecConfig->PMK,
			pHandshake4Way->ANonce, pHandshake4Way->AAddr,
			pHandshake4Way->SNonce, pHandshake4Way->SAddr,
			PTK, ptkLen);
	} else
#endif /* OCE_FILS_SUPPORT */
#ifdef DOT11R_FT_SUPPORT

	if (IS_FT_RSN_STA(pEntry)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			UINT8 ptk_len;
			UCHAR apidx = 0;
			struct wifi_dev *wdev;
			PFT_CFG pFtCfg;

			if (pEntry->func_tb_idx >= pAd->ApCfg.BssidNum) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "func_tb_idx=%d, return here.\n", pEntry->func_tb_idx);
				return;
			} else
				apidx = pEntry->func_tb_idx;

			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			pFtCfg = &wdev->FtCfg;
			FT_DerivePMKR0(pSecConfig->PMK, LEN_PMK,
						   (PUINT8)pAd->ApCfg.MBSSID[apidx].Ssid,
						   pAd->ApCfg.MBSSID[apidx].SsidLen,
						   pFtCfg->FtMdId,
						   pFtCfg->FtR0khId,
						   pFtCfg->FtR0khIdLen,
						   pHandshake4Way->AAddr,
						   FT_PMK_R0,
						   FT_PMK_R0_NAME);

			FT_DerivePMKR1(FT_PMK_R0,
						   FT_PMK_R0_NAME,
						   wdev->bssid, /* R1KHID*/
						   pHandshake4Way->SAddr,		/* S1KHID*/
						   FT_PMK_R1,
						   FT_PMK_R1_NAME);

			if (IS_CIPHER_TKIP_Entry(pEntry))
				ptk_len = 32 + 32;
			else
				ptk_len = 32 + 16;

			FT_DerivePTK(FT_PMK_R1,
						 FT_PMK_R1_NAME,
						 pHandshake4Way->ANonce,
						 pHandshake4Way->SNonce,
						 wdev->bssid, /* Bssid*/
						 pHandshake4Way->SAddr,								/* sta mac*/
						 ptk_len,
						 PTK,
						 PTK_NAME);
			ptkLen = LEN_MAX_PTK;
		}
#endif /* CONFIG_AP_SUPPORT */
	} else
#endif /* DOT11R_FT_SUPPORT */
	if (IS_AKM_SHA384(pSecConfig->AKMMap)
		|| pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384) {
		WpaDerivePTK_KDF_384(pSecConfig->PMK,
			pHandshake4Way->ANonce,		/* ANONCE*/
			pHandshake4Way->AAddr,
			pHandshake4Way->SNonce,		/* SNONCE*/
			pHandshake4Way->SAddr,
			PTK,
			ptkLen);
		hex_dump("PTK SHA384", PTK, ptkLen);
	} else
	if (IS_AKM_OWE(pSecConfig->AKMMap)) {
		INT key_len = LEN_PMK;

		if (ptkLen > LEN_AES_PTK)
			key_len = LEN_PMK_SHA384;
		WpaDerivePTK_KDF_256_w_pmk_len(pSecConfig->PMK,
			pHandshake4Way->ANonce,		/* ANONCE*/
			pHandshake4Way->AAddr,
			pHandshake4Way->SNonce,		/* SNONCE*/
			pHandshake4Way->SAddr,
			PTK,
			ptkLen,
			key_len);
		hex_dump("OWE PTK", PTK, ptkLen);
	} else
	if (IS_AKM_SAE(pSecConfig->AKMMap)) {
		ptkLen = LEN_PTK_KCK + LEN_PTK_KEK;
		ptkLen += sec_get_cipher_key_len(pSecConfig->PairwiseCipher);
		WpaDerivePTK_KDF_256(pSecConfig->PMK,
					  pHandshake4Way->ANonce,		/* ANONCE*/
					  pHandshake4Way->AAddr,
					  pHandshake4Way->SNonce,		/* SNONCE*/
					  pHandshake4Way->SAddr,
					  PTK,
					  ptkLen);   /* Must is 48 bytes */
		hex_dump("PTK", PTK, ptkLen);
	} else
#ifdef DOT11W_PMF_SUPPORT
	if (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA256) {
		WpaDerivePTK_KDF_256(pSecConfig->PMK,
					  pHandshake4Way->ANonce,		/* ANONCE*/
					  pHandshake4Way->AAddr,
					  pHandshake4Way->SNonce,		/* SNONCE*/
					  pHandshake4Way->SAddr,
					  PTK,
					  ptkLen);   /* Must is 48 bytes */
	} else
#endif /* DOT11W_PMF_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2
	if ((CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE))
#ifdef CONFIG_HOTSPOT_R3
	|| (pSecConfig->bIsWPA2EntOSEN == TRUE)
#endif
	){
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "got msg2 derivePTK\n");
		WpaDerivePTK_KDF_256(pSecConfig->PMK,
					  pHandshake4Way->ANonce,		/* ANONCE*/
					  pHandshake4Way->AAddr,
					  pHandshake4Way->SNonce,		/* SNONCE*/
					  pHandshake4Way->SAddr,
					  PTK,
					  ptkLen);   /* Must is 48 bytes */
	} else
#endif /* CONFIG_HOTSPOT_R2 */
	{
		ptkLen = LEN_PTK;
		/* Derive PTK*/
		WpaDerivePTK(pSecConfig->PMK,
					 pHandshake4Way->ANonce,		/* ANONCE*/
					 pHandshake4Way->AAddr,
					 pHandshake4Way->SNonce,		/* SNONCE*/
					 pHandshake4Way->SAddr,
					 PTK,
					 ptkLen);
	}

#ifdef OCE_FILS_SUPPORT
skip_ptk:

	if (IS_AKM_FILS_Entry(pEntry)) {
		struct fils_info *filsInfo = &pEntry->filsInfo;

			if (filsInfo->pending_ie) {
				os_free_mem(filsInfo->pending_ie);
				filsInfo->pending_ie_len = 0;
			filsInfo->pending_ie = NULL;
			}

		if (!filsInfo->is_pending_decrypt) {
			filsInfo->pending_ie_len = Elem->MsgLen;
			os_alloc_mem(NULL, (UCHAR **)&filsInfo->pending_ie, filsInfo->pending_ie_len);
			if (!filsInfo->pending_ie) {
				return;
			}

			NdisMoveMemory(filsInfo->pending_ie, Elem->Msg, filsInfo->pending_ie_len);
			NdisMoveMemory(&filsInfo->rssi_info, &Elem->rssi_info, sizeof(struct raw_rssi_info));
			filsInfo->pending_decrypt = PeerPairMsg2Action;

			filsInfo->is_pending_decrypt = TRUE;
			filsInfo->last_pending_id = pHeader->Sequence;

			if (ptkLen >= MAX_FILS_PTK_LEN)/*colgin coverity 10701888*/
				ptkLen = MAX_FILS_PTK_LEN;

			NdisMoveMemory(filsInfo->PTK, PTK, ptkLen);
			filsInfo->PTK_len = ptkLen;

			DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_AEAD_DECR_EVENT);
			return;
		} else {
			filsInfo->is_pending_decrypt = FALSE;
			filsInfo->pending_decrypt = NULL;

			if (filsInfo->last_pending_id != pHeader->Sequence) {
				return;
			}

			if (filsInfo->status != MLME_SUCCESS) {
				return;
			}

			/* restore the PTK from perivous cal. */
			ptkLen = filsInfo->PTK_len;
			NdisMoveMemory(PTK, filsInfo->PTK, ptkLen);

			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
					 "STA - "MACSTR" do FILS AEAD with status %d\n",
					  MAC2STR(pHandshake4Way->SAddr), filsInfo->status);

		}
	}
#endif /* OCE_FILS_SUPPORT */

	/* Sanity Check peer Pairwise message 2 - Replay Counter, MIC, RSNIE*/
	if (WpaMessageSanity(pAd, pReceiveEapol, MsgLen, EAPOL_PAIR_MSG_2, pSecConfig, pEntry, PTK) == FALSE) {
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_AUTH_FAIL, REASON_4WAY_HS_MSG2_FAIL);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				REASON_MIC_FAILURE);
#endif
		return;
	}

	/* refill the information if MIC Pass */
	NdisMoveMemory(pSecConfig->PTK, PTK, ptkLen);
#ifdef MAP_R3
	pSecConfig->ptk_len = ptkLen;
	if (IS_AKM_SAE(pSecConfig->AKMMap)) {
		if (ptkLen > LEN_AES_PTK)
			pSecConfig->pmk_len = LEN_PMK_SHA384;
		else
			pSecConfig->pmk_len = LEN_PMK;
	} else
		pSecConfig->pmk_len = LEN_PMK;
#endif /* MAP_R3 */


#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	if (IS_FT_RSN_STA(pEntry)) {
		NdisMoveMemory(pEntry->FT_PMK_R0, FT_PMK_R0, 32);
		NdisMoveMemory(pEntry->FT_PMK_R0_NAME, FT_PMK_R0_NAME, 16);
		NdisMoveMemory(pEntry->FT_PMK_R1, FT_PMK_R1, 32);
		NdisMoveMemory(pEntry->FT_PMK_R1_NAME, FT_PMK_R1_NAME, 16);
		NdisMoveMemory(pEntry->PTK_NAME, PTK_NAME, 16);
		NdisMoveMemory(pEntry->FT_PTK, PTK, ptkLen);
	}
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


	/* delete retry timer*/
	RTMPCancelTimer(&pHandshake4Way->MsgRetryTimer, &Cancelled);
	pHandshake4Way->MsgRetryCounter = 0;
	/* Change state*/
	pHandshake4Way->WpaState = AS_PTKINIT_NEGOTIATING;
	WPABuildPairMsg3(pAd, pSecConfig, pEntry);
	log_time_end(LOG_TIME_CONNECTION, "peer_msg2", DBG_LVL_INFO, &tl);
}


VOID PeerPairMsg3Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem)
{
	PHEADER_802_11 pHeader;
	PEAPOL_PACKET pReceiveEapol;
	UINT MsgLen;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	unsigned char hdr_len = LENGTH_802_11;
	UCHAR idx = 0;
	BOOLEAN bWPA2 = TRUE;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	/* Choose WPA2 or not*/
	if (IS_AKM_WPA1(pSecConfig->AKMMap) || IS_AKM_WPA1PSK(pSecConfig->AKMMap))
		bWPA2 = FALSE;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
		"===> wcid = %d\n", pEntry->wcid);

	pHandshake4Way = &pSecConfig->Handshake;
	/* Record 802.11 header & the received EAPOL packet Msg3*/
	pHeader = (PHEADER_802_11) Elem->Msg;
	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
	if (pHeader->FC.Order == 1)
		hdr_len += LENGTH_802_11_HTC;
	if (pHeader->FC.SubType == SUBTYPE_QDATA)
		hdr_len += LENGTH_802_11_QOS_FIELD;


	if (Elem->MsgLen < (hdr_len + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
		return;

	pReceiveEapol = (PEAPOL_PACKET) &Elem->Msg[hdr_len + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;

	/* Sanity Check peer Pairwise message 3 - Replay Counter, MIC, RSNIE*/
	if (WpaMessageSanity(pAd, pReceiveEapol, MsgLen, EAPOL_PAIR_MSG_3, pSecConfig, pEntry, pSecConfig->PTK) == FALSE)
		return;

	if ((pHandshake4Way->AllowInsPTK == TRUE) && bWPA2) {
		UCHAR kid = pEntry->SecConfig.LastGroupKeyId;

		pEntry->CCMP_BC_PN[kid] = 0;
		for (idx = 0; idx < (LEN_KEY_DESC_RSC-2); idx++)
			pEntry->CCMP_BC_PN[kid] += ((UINT64)pReceiveEapol->KeyDesc.KeyRsc[idx] << (idx*8));
		pEntry->Init_CCMP_BC_PN_Passed[kid] = FALSE;
		pEntry->AllowUpdateRSC = FALSE;
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"(%d): update CCMP_BC_PN to %llu\n",
			pEntry->wcid, pEntry->CCMP_BC_PN[kid]);
#ifdef MAC_REPEATER_SUPPORT
		/* sync PN of ApCli entry as the time rept rekey */
		if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
			MAC_TABLE_ENTRY *pEntry2 = MacTableLookup(pAd, pHeader->Addr2);

			if (pEntry2 != NULL) {
				pEntry2->CCMP_BC_PN[kid] = pEntry->CCMP_BC_PN[kid];
				pEntry2->Init_CCMP_BC_PN_Passed[kid] = pEntry->Init_CCMP_BC_PN_Passed[kid];
				pEntry2->AllowUpdateRSC = pEntry->AllowUpdateRSC;
			}
		}
#endif
	}

	/* Save Replay counter, it will use construct message 4*/
	NdisMoveMemory(pHandshake4Way->ReplayCounter, pReceiveEapol->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);

	/* Double check ANonce*/
	if (!NdisEqualMemory(pHandshake4Way->ANonce, pReceiveEapol->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE))
		return;

	pSecConfig->is_eapol_encrypted = pHeader->FC.Wep;
	WPABuildPairMsg4(pAd, pSecConfig, pEntry);
#ifdef MAP_R3
	if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd) && IS_MAP_CERT_ENABLE(pAd)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
					 "Connection (%s) to ssid:%s - AKMMap=%s, Passphrase=%s, wcid=%d from=%02X:%02X:%02X:%02X:%02X:%02X\n\n",
					  RTMP_OS_NETDEV_GET_DEVNAME(pEntry->wdev->if_dev),
					  pAd->StaCfg[pEntry->func_tb_idx].Ssid,
					  GetAuthModeStr(pSecConfig->AKMMap),
					  pEntry->wdev->SecConfig.PSK,
					  pEntry->wcid,
					  PRINT_MAC(pEntry->wdev->if_addr));

		hex_dump("PMK", pEntry->SecConfig.PMK, pEntry->SecConfig.pmk_len);
		hex_dump("PTK", pEntry->SecConfig.PTK, pEntry->SecConfig.ptk_len);
		wext_send_sta_info(pAd, pEntry->wdev, pEntry);
	}
#endif /* MAP_R3 */

	log_time_end(LOG_TIME_CONNECTION, "peer_msg3", DBG_LVL_INFO, &tl);
}


#define PTK_LOG_LEN 200
VOID PeerPairMsg4Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem)
{
	PHEADER_802_11 pHeader;
	PEAPOL_PACKET pReceiveEapolM4;
	UINT EapolLen;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	STA_TR_ENTRY *tr_entry;
	BOOLEAN Cancelled;
	unsigned char hdr_len = LENGTH_802_11;
	struct time_log tl;
	INT ret;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
		"===> wcid = %d\n", pEntry->wcid);

	pHandshake4Way = &pSecConfig->Handshake;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"===> STA(:"MACSTR"), WpaState(%d)\n",
			MAC2STR(pEntry->Addr), pHandshake4Way->WpaState);

	if (pHandshake4Way->WpaState < AS_PTKINIT_NEGOTIATING)
		return;

	/* Prevent the Replayed Msg4 Attack */
	if (pHandshake4Way->WpaState == AS_PTKINITDONE) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
		" reject the Replayed Msg4\n");
		return;
	}

	/* pointer to 802.11 header*/
	pHeader = (PHEADER_802_11)Elem->Msg;
	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
	if (pHeader->FC.Order == 1)
		hdr_len += LENGTH_802_11_HTC;
	if (pHeader->FC.SubType == SUBTYPE_QDATA)
		hdr_len += LENGTH_802_11_QOS_FIELD;

	if (Elem->MsgLen < (hdr_len + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
		return;

	/* skip 802.11_header(24-byte) and LLC_header(8) */
	pReceiveEapolM4 = (PEAPOL_PACKET) &Elem->Msg[hdr_len + LENGTH_802_1_H];
	EapolLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;

	/* Sanity Check peer Pairwise message 4 - Replay Counter, MIC*/
	if (WpaMessageSanity(pAd, pReceiveEapolM4, EapolLen, EAPOL_PAIR_MSG_4,
		pSecConfig, pEntry, pSecConfig->PTK) == FALSE) {
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_AUTH_FAIL, REASON_4WAY_HS_MSG4_FAIL);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				REASON_MIC_FAILURE);
#endif

		return;
	}

	/* Sanity Check pEntry->func_tb_idx to avoid out of bound with pAd->ApCfg.MBSSID*/
	if (!VALID_MBSS(pAd, (int)pEntry->func_tb_idx))
		return;

	/* 4. upgrade state */
	RTMPCancelTimer(&pHandshake4Way->MsgRetryTimer, &Cancelled);
	tr_entry = tr_entry_get(pAd, pEntry->wcid);
	pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
	pHandshake4Way->WpaState = AS_PTKINITDONE;
	pHandshake4Way->GTKState = REKEY_ESTABLISHED;

#if defined(DOT1X_SUPPORT) && defined(RADIUS_ACCOUNTING_SUPPORT)

	/* Notify 802.1x daemon to add this sta for accounting*/
	if (IS_AKM_WPA1PSK(pSecConfig->AKMMap)
		|| IS_AKM_WPA2PSK(pSecConfig->AKMMap)
		|| IS_AKM_WPA3PSK(pSecConfig->AKMMap))
		DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_LOCAL_AUTH_ENTRY);

#endif /* DOT1X_SUPPORT && RADIUS_ACCOUNTING_SUPPORT */
#ifdef DOT11_EHT_BE
	if (pEntry->mlo.mlo_en)
		mlo_update_port_secure(pEntry, NULL);
	else
#endif
	{
		WifiSysUpdatePortSecur(pAd, pEntry, NULL);
		RtmpusecDelay(5000);/*delay 5ms for wpa Update Portl*/
		if (tr_entry)
			tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			 "===> WifiSysUpdatePortSecur called by wcid=%d, PortSecured=%d\n",
			pEntry->wcid, STATE_PORT_SECURE);
	}

#ifdef WSC_AP_SUPPORT

	if (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.WscConfMode != WSC_DISABLE)
		WscInformFromWPA(pEntry);

#endif /* WSC_AP_SUPPORT */

	if (IS_AKM_WPA2(pSecConfig->AKMMap)
		|| IS_AKM_WPA2PSK(pSecConfig->AKMMap)
		|| IS_AKM_WPA3_192BIT(pSecConfig->AKMMap)
		|| IS_AKM_WPA3PSK(pSecConfig->AKMMap)
		|| IS_AKM_OWE(pSecConfig->AKMMap)
		|| IS_AKM_DPP(pSecConfig->AKMMap)) {
#ifdef DOT11R_FT_SUPPORT
		if (IS_FT_RSN_STA(pEntry)) {
			PFT_R1HK_ENTRY pR1khEntry;
			PUINT8 pUCipher = NULL;
			PUINT8 pAkm = NULL;
			UINT8 count;

			pUCipher = WPA_ExtractSuiteFromRSNIE(pEntry->RSN_IE, pEntry->RSNIE_Len, PAIRWISE_SUITE, &count);
			pAkm = WPA_ExtractSuiteFromRSNIE(pEntry->RSN_IE, pEntry->RSNIE_Len, AKM_SUITE, &count);
			/* Record the PMK-R0 related information */
			RTMPAddPMKIDCache(PD_GET_PMKID_PTR(pAd->physical_dev),
							  pEntry->wdev->bssid,
							  pHandshake4Way->SAddr,
							  pEntry->FT_PMK_R0_NAME,
							  pEntry->FT_PMK_R0,
							  TRUE,
							  FALSE,
							  LEN_PMK,
							  pAd->ApCfg.MBSSID[pEntry->func_tb_idx].PMKCachePeriod);

			/* Delete previous entry */
			pR1khEntry = FT_R1khEntryTabLookup(pAd, pEntry->FT_PMK_R1_NAME);

			if (pR1khEntry != NULL)
				FT_R1khEntryDelete(pAd, pR1khEntry);

			/* Update R1KH table */
			if (pUCipher != NULL)
				NdisMoveMemory(pEntry->FT_UCipher, pUCipher, 4);

			if (pAkm != NULL)
				NdisMoveMemory(pEntry->FT_Akm, pAkm, 4);

			FT_R1khEntryInsert(pAd,
							   pEntry->FT_PMK_R0_NAME,
							   pEntry->FT_PMK_R1_NAME,
							   pEntry->FT_PMK_R1,
							   pUCipher,
							   pAkm,
							   (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].PMKCachePeriod / OS_HZ),
							   FT_REASSOC_DEADLINE, /* YF_TIE */
							   pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.FtCfg.FtR0khId,
							   pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.FtCfg.FtR0khIdLen,
							   pHandshake4Way->SAddr);
#ifdef IAPP_SUPPORT
			{
				FT_KDP_EVT_ASSOC EvtAssoc;

				EvtAssoc.SeqNum = 0;
				NdisMoveMemory(EvtAssoc.MacAddr, pHandshake4Way->SAddr, MAC_ADDR_LEN);

				{
					FT_KDP_EVENT_INFORM(pAd,
										pEntry->func_tb_idx,
										FT_KDP_SIG_FT_ASSOCIATION,
										&EvtAssoc,
										sizeof(EvtAssoc),
										NULL);
				}
			}
#endif /* IAPP_SUPPORT				*/
			pR1khEntry = FT_R1khEntryTabLookup(pAd, pEntry->FT_PMK_R1_NAME);

			if (pR1khEntry != NULL) {
				pR1khEntry->AKMMap = pSecConfig->AKMMap;
				hex_dump("R1KHTab-R0KHID", pR1khEntry->R0khId, pR1khEntry->R0khIdLen);
				hex_dump("R1KHTab-PairwiseCipher", pR1khEntry->PairwisChipher, 4);
				hex_dump("R1KHTab-AKM", pR1khEntry->AkmSuite, 4);
				hex_dump("R1KHTab-PMKR0Name", pR1khEntry->PmkR0Name, 16);
				hex_dump("R1KHTab-PMKR1Name", pR1khEntry->PmkR1Name, 16);
			} else
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_WARN,
					"The entry in R1KH table doesn't exist\n");
		} else
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT1X_SUPPORT
			if (IS_AKM_WPA2(pSecConfig->AKMMap) || IS_AKM_WPA3_192BIT(pSecConfig->AKMMap)) {
				UCHAR  PMK_key[20];
				UCHAR  digest[80];
				UINT8 pmk_len = LEN_PMK;
				UCHAR is_mlo_connect = FALSE;

				if (IS_AKM_SHA384(pSecConfig->AKMMap) ||
					   (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384))
					pmk_len = LEN_PMK_SHA384;

				if (is_pmkid_cache_in_sec_config(pSecConfig)
					&& NdisEqualMemory(pSecConfig->PMK, pSecConfig->pmk_cache, pmk_len))
					NdisMoveMemory(digest, pSecConfig->pmkid, LEN_PMKID);
				else {
					/* Calculate PMKID, refer to IEEE 802.11i-2004 8.5.1.2*/
					NdisMoveMemory(&PMK_key[0], "PMK Name", 8);
					NdisMoveMemory(&PMK_key[8], pSecConfig->Handshake.AAddr, MAC_ADDR_LEN);
					NdisMoveMemory(&PMK_key[14], pSecConfig->Handshake.SAddr, MAC_ADDR_LEN);
					if (IS_AKM_SHA384(pSecConfig->AKMMap) ||
					   (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384)) {
						RT_HMAC_SHA384(pSecConfig->PTK, LEN_PTK_KCK_SHA384, PMK_key, 20, digest, LEN_PMKID);
					}
#ifdef OCE_FILS_SUPPORT
					/* Todo: why PMF sha256 didn't use it before ? */
					else if (IS_AKM_FILS_SHA256(pSecConfig->AKMMap)) {
						RT_HMAC_SHA256(pSecConfig->PMK, pmk_len, PMK_key, 20, digest, LEN_PMKID);
					}
#endif /* OCE_FILS_SUPPORT */
					else
						RT_HMAC_SHA1(pSecConfig->PMK, pmk_len, PMK_key, 20, digest, SHA1_DIGEST_SIZE);
				}
#ifdef DOT11_EHT_BE
				if (pEntry->mlo.mlo_en)
					is_mlo_connect = TRUE;
#endif

				RTMPAddPMKIDCache(PD_GET_PMKID_PTR(pAd->physical_dev),
						  pSecConfig->Handshake.AAddr,
						  pSecConfig->Handshake.SAddr,
						  digest,
						  pSecConfig->PMK,
						  FALSE,
						  is_mlo_connect,
						  pmk_len,
						  pAd->ApCfg.MBSSID[pEntry->func_tb_idx].PMKCachePeriod);
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "Calc PMKID="MACSTR"\n", MAC2STR(digest));
			}
#endif /* DOT1X_SUPPORT */
#if defined(DOT11_SAE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
			else if (IS_AKM_WPA3PSK(pSecConfig->AKMMap)) {
				struct __SAE_CFG *sae_cfg = PD_GET_SAE_CFG_PTR(pAd->physical_dev);

				set_sae_instance_removable(sae_cfg, pSecConfig->Handshake.AAddr,
					pSecConfig->Handshake.SAddr);
			}
#endif
#if defined(CONFIG_OWE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
			else if (IS_AKM_OWE(pSecConfig->AKMMap)) {
				INT CacheIdx;/* Key cache */

				if (pEntry->SecConfig.owe.pmkid) {
					UINT8 pmk_len = LEN_PMK;
					UCHAR is_mlo_connect = FALSE;
					struct _NDIS_AP_802_11_PMKID *pmkid_cache;

					pmkid_cache = PD_GET_PMKID_PTR(pAd->physical_dev);
#ifdef DOT11_EHT_BE
					if (pEntry->mlo.mlo_en)
						is_mlo_connect = TRUE;
#endif

					if (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384)
						pmk_len = LEN_PMK_SHA384;
					CacheIdx = RTMPValidatePMKIDCache(pmkid_cache,
									pHandshake4Way->AAddr,
									pHandshake4Way->SAddr,
									pEntry->SecConfig.owe.pmkid,
									is_mlo_connect);

					hex_dump("store pmkid:", pEntry->SecConfig.owe.pmkid, LEN_PMKID);
					OS_SEM_LOCK(&pmkid_cache->pmkid_lock);
					RTMPAddPMKIDCache(pmkid_cache,
							pHandshake4Way->AAddr,
							pHandshake4Way->SAddr,
							pEntry->SecConfig.owe.pmkid,
							pEntry->SecConfig.PMK,
							FALSE,
							is_mlo_connect,
							pmk_len,
							pAd->ApCfg.MBSSID[pEntry->func_tb_idx].PMKCachePeriod);
					OS_SEM_UNLOCK(&pmkid_cache->pmkid_lock);
					store_pmkid_cache_in_sec_config(pAd, pEntry, CacheIdx);
				}
			}
#endif /*CONFIG_OWE_SUPPORT*/
#if defined(MWDS) || defined(CONFIG_BS_SUPPORT) || defined(CONFIG_MAP_SUPPORT) || defined(WAPP_SUPPORT) || defined(QOS_R1)
		if (tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
#ifdef MWDS
			MWDSAPPeerEnable(pAd, pEntry);
#endif
#if defined(CONFIG_MAP_SUPPORT) && !defined(MAP_HOSTAPD_SUPPORT)
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "MAP_ENABLE\n");
#if defined(A4_CONN)
			if (IS_MAP_ENABLE(pAd)) {
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
				if (pEntry->mlo.mlo_en)
					map_a4_mlo_peer_enable(pAd, NULL, pEntry, TRUE);
				else
#endif
					map_a4_peer_enable(pAd, pEntry, TRUE);
			}
#endif
#endif /* CONFIG_MAP_SUPPORT */

#ifdef WAPP_SUPPORT
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
			if (!pEntry->mlo.mlo_en)
#endif
			wapp_send_cli_join_event(pAd, pEntry);
#endif
#ifdef QOS_R1
#ifdef MSCS_PROPRIETARY
			if (IS_QOS_ENABLE(pAd) && pAd->SupportFastPath && pEntry->MSCSSupport) {
				pEntry->dabs_trans_id = 1;
				Send_DABS_Announce(pAd, pEntry->wcid);
				RTMPSetTimer(&pEntry->DABSRetryTimer, DABS_WAIT_TIME);
				OS_SET_BIT(DABS_TIMER_RUNNING, &pEntry->DABSTimerFlag);
			}
#endif
#endif
		}
#endif
		if (pEntry && tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
#ifdef WH_EVENT_NOTIFIER
			EventHdlr pEventHdlrHook = NULL;
#endif /* WH_EVENT_NOTIFIER */
#ifdef WH_EVENT_NOTIFIER
			pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_JOIN);
			if (pEventHdlrHook && pEntry->wdev)
				pEventHdlrHook(pAd, pEntry, Elem);
#endif /* WH_EVENT_NOTIFIER */
#ifdef WIFI_TWT_SUPPORT
			twt_peer_join_btwt_id_0(pEntry->wdev, pEntry);
#endif /* WIFI_TWT_SUPPORT */
		}

		/* send wireless event - for set key done WPA2*/
		RTMPSendWirelessEvent(pAd, IW_SET_KEY_DONE_WPA2_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);
#ifdef CONFIG_HOTSPOT_R2

		if (pEntry->IsWNMReqValid == TRUE) {
			struct wnm_req_data *req_data = pEntry->ReqData;

			Send_WNM_Notify_Req(pAd,
								req_data->peer_mac_addr,
								req_data->wnm_req,
								req_data->wnm_req_len,
								req_data->type);
			pEntry->IsWNMReqValid = FALSE;
			os_free_mem(req_data);
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
				"!!!!msg 4 send wnm req\n");
		}

#endif
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM)
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (pEntry->IsBTMReqValid == TRUE) {
				struct btm_req_data *req_data = pEntry->ReqbtmData;

				Send_BTM_Req(pAd,
						req_data->peer_mac_addr,
						req_data->btm_req,
						req_data->btm_req_len);
				pEntry->IsBTMReqValid = FALSE;
				os_free_mem(req_data);
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
					"!!!!msg 4 send btm req\n");
			}
		}
#endif
#endif

#ifdef MBO_SUPPORT
		if (IS_MBO_ENABLE(&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev))
			MboIndicateStaBssidInfo(pAd, &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev, pEntry->Addr);
#endif /* MBO_SUPPORT */

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
				 "AP SETKEYS DONE(%s) - AKMMap=%s, PairwiseCipher=%s, GroupCipher=%s, wcid=%d from "MACSTR"\n\n",
				  RTMP_OS_NETDEV_GET_DEVNAME(pEntry->wdev->if_dev),
				  GetAuthModeStr(pSecConfig->AKMMap),
				  GetEncryModeStr(pSecConfig->PairwiseCipher),
				  GetEncryModeStr(pSecConfig->GroupCipher),
				  pEntry->wcid,
				  MAC2STR(pHandshake4Way->SAddr));

		if ((!IS_AKM_OPEN(GET_SEC_AKM(pSecConfig))) && tr_entry->PortSecured == 1) {
			INT32 i;
#ifdef MASK_PARTIAL_MACADDR  /* Sensitive log */
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "PTK:");
			for (i = 0; i < 64; i++)
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "%02x", pSecConfig->PTK[i]);
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "\n");
#else
			unsigned char *ptk_log;
			/* Allocate memory for ptk log*/
			os_alloc_mem(NULL, (PUCHAR *)&ptk_log, PTK_LOG_LEN);
			if (ptk_log == NULL) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
					"!!!PTK Memory Error\n");
				return;
			}

			NdisZeroMemory(ptk_log, PTK_LOG_LEN);
			ret = snprintf(ptk_log + strlen(ptk_log),
						PTK_LOG_LEN-strlen(ptk_log), "PTK 1st part:");
			if (os_snprintf_error(PTK_LOG_LEN-strlen(ptk_log), ret)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA,
					DBG_LVL_ERROR, "snprintf error!\n");
			}
			for (i = 0; i < 32; i++) {
				ret = snprintf(ptk_log + strlen(ptk_log), PTK_LOG_LEN-strlen(ptk_log), "%02x", pSecConfig->PTK[i]);
				if (os_snprintf_error(PTK_LOG_LEN-strlen(ptk_log), ret)) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA,
						DBG_LVL_ERROR, "snprintf error!\n");
				}
			}
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "%s\n", ptk_log);

			NdisZeroMemory(ptk_log, PTK_LOG_LEN);
			ret = snprintf(ptk_log + strlen(ptk_log),
						PTK_LOG_LEN-strlen(ptk_log), "PTK 2nd part:");
			if (os_snprintf_error(PTK_LOG_LEN-strlen(ptk_log), ret)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA,
					DBG_LVL_ERROR, "snprintf error!\n");
			}
			for (i = 32; i < 64; i++) {
				ret = snprintf(ptk_log + strlen(ptk_log), PTK_LOG_LEN-strlen(ptk_log), "%02x", pSecConfig->PTK[i]);
				if (os_snprintf_error(PTK_LOG_LEN-strlen(ptk_log), ret)) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA,
						DBG_LVL_ERROR, "snprintf error!\n");
				}
			}
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "%s\n", ptk_log);

			if (ptk_log)
				os_free_mem(ptk_log);
#endif
		}
	} else {
		/* 5. init Group 2-way handshake if necessary.*/
		RTMPSetTimer(&pEntry->SecConfig.StartFor2WayTimer, ENQUEUE_EAPOL_2WAY_START_TIMER);
	}

#ifdef MAP_R3
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
				 "Connection (%s) to ssid:%s - AKMMap=%s, Passphrase=%s, wcid=%d from=%02X:%02X:%02X:%02X:%02X:%02X\n\n",
				  RTMP_OS_NETDEV_GET_DEVNAME(pEntry->wdev->if_dev),
				  pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				  GetAuthModeStr(pSecConfig->AKMMap),
				  pEntry->wdev->SecConfig.PSK,
				  pEntry->wcid,
				  PRINT_MAC(pEntry->Addr));

	hex_dump("PMK", pEntry->SecConfig.PMK, pEntry->SecConfig.pmk_len);
	hex_dump("PTK", pEntry->SecConfig.PTK, pEntry->SecConfig.ptk_len);
	wext_send_sta_info(pAd, pEntry->wdev, pEntry);
#endif /* MAP_R3 */
	log_time_end(LOG_TIME_CONNECTION, "peer_msg4", DBG_LVL_INFO, &tl);
}

VOID PeerGroupMsg1Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem)
{
	PHEADER_802_11 pHeader;
	PEAPOL_PACKET pReceiveEapol;
	UINT EapolLen;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	unsigned char hdr_len = LENGTH_802_11;
	UCHAR idx = 0;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "===>\n");
	pHandshake4Way = &pSecConfig->Handshake;
	/* pointer to 802.11 header*/
	pHeader = (PHEADER_802_11)Elem->Msg;
	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
	if (pHeader->FC.Order == 1)
		hdr_len += LENGTH_802_11_HTC;
	if (pHeader->FC.SubType == SUBTYPE_QDATA)
		hdr_len += LENGTH_802_11_QOS_FIELD;

	/* skip 802.11_header(24-byte) and LLC_header(8) */
	pReceiveEapol = (PEAPOL_PACKET) &Elem->Msg[hdr_len + LENGTH_802_1_H];
	EapolLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;

	/* Sanity Check peer group message 1 - Replay Counter, MIC, RSNIE*/
	if (WpaMessageSanity(pAd, pReceiveEapol, EapolLen, EAPOL_GROUP_MSG_1,
		pSecConfig, pEntry, pSecConfig->PTK) == FALSE) {
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_AUTH_FAIL, REASON_2WAY_HS_MSG1_FAIL);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				REASON_MIC_FAILURE);
#endif

		return;
	}

	if (pEntry->AllowUpdateRSC == TRUE) {
		UCHAR kid = pEntry->SecConfig.LastGroupKeyId;

		pEntry->CCMP_BC_PN[kid] = 0;
		for (idx = 0; idx < (LEN_KEY_DESC_RSC-2); idx++)
			pEntry->CCMP_BC_PN[kid] += ((UINT64)pReceiveEapol->KeyDesc.KeyRsc[idx] << (idx*8));
		pEntry->Init_CCMP_BC_PN_Passed[kid] = FALSE;
		pEntry->AllowUpdateRSC = FALSE;
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"(%d): update CCMP_BC_PN to %llu\n",
			pEntry->wcid, pEntry->CCMP_BC_PN[kid]);
#ifdef MAC_REPEATER_SUPPORT
		/* Sync PN of ApCli entry as the time rept rekey */
		if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
			MAC_TABLE_ENTRY *pEntry2 = MacTableLookup(pAd, pHeader->Addr2);

			if (pEntry2 != NULL) {
				pEntry2->CCMP_BC_PN[kid] = pEntry->CCMP_BC_PN[kid];
				pEntry2->Init_CCMP_BC_PN_Passed[kid] = pEntry->Init_CCMP_BC_PN_Passed[kid];
				pEntry2->AllowUpdateRSC = pEntry->AllowUpdateRSC;
			}
		}
#endif
	}

	/* Save Replay counter, it will use to construct message 2*/
	NdisMoveMemory(pHandshake4Way->ReplayCounter, pReceiveEapol->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);
	WPABuildGroupMsg2(pAd, pSecConfig, pEntry);
}

VOID PeerGroupMsg2Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem)
{
	PHEADER_802_11 pHeader;
	PEAPOL_PACKET pReceiveEapolM2;
	UINT EapolLen;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	BOOLEAN Cancelled;
	unsigned char hdr_len = LENGTH_802_11;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "===>\n");
	/* pointer to 802.11 header*/
	pHeader = (PHEADER_802_11)Elem->Msg;

	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
	if (pHeader->FC.Order == 1)
		hdr_len += LENGTH_802_11_HTC;
	if (pHeader->FC.SubType == SUBTYPE_QDATA)
		hdr_len += LENGTH_802_11_QOS_FIELD;

	/* skip 802.11_header(24-byte) and LLC_header(8) */
	pReceiveEapolM2 = (PEAPOL_PACKET) &Elem->Msg[hdr_len + LENGTH_802_1_H];
	EapolLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;
	pHandshake4Way = &pSecConfig->Handshake;

	if (pHandshake4Way->WpaState != AS_PTKINITDONE)
		return;

	/* Sanity Check peer Group message 2 - Replay Counter, MIC*/
	if (WpaMessageSanity(pAd, pReceiveEapolM2, EapolLen, EAPOL_GROUP_MSG_2,
		pSecConfig, pEntry, pSecConfig->PTK) == FALSE) {
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_AUTH_FAIL, REASON_2WAY_HS_MSG2_FAIL);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				REASON_MIC_FAILURE);
#endif

		return;
	}
	RTMPCancelTimer(&pHandshake4Way->MsgRetryTimer, &Cancelled);
	pSecConfig->Handshake.GTKState = REKEY_ESTABLISHED;

#if defined(MWDS) || defined(CONFIG_BS_SUPPORT) || defined(CONFIG_MAP_SUPPORT) || defined(WAPP_SUPPORT)
	{
		STA_TR_ENTRY *tr_entry = NULL;

		if (IS_WCID_VALID(pAd, pEntry->wcid))
			tr_entry = tr_entry_get(pAd, pEntry->wcid);

		if (tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
#ifdef MWDS
			MWDSAPPeerEnable(pAd, pEntry);
#endif /* MWDS */
#if defined(CONFIG_MAP_SUPPORT) && !defined(MTK_HOSTAPD_SUPPORT)
#if defined(WAPP_SUPPORT)
			if (IS_MAP_ENABLE(pAd)) {
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
				if (pEntry->mlo.mlo_en)
					map_a4_mlo_peer_enable(pAd, NULL, pEntry, TRUE);
				else
#endif
					map_a4_peer_enable(pAd, pEntry, TRUE);
			}
#endif /*WAPP_SUPPORT*/
#endif /* CONFIG_MAP_SUPPORT */

#ifdef WAPP_SUPPORT
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
			if (!pEntry->mlo.mlo_en)
#endif
			wapp_send_cli_join_event(pAd, pEntry);
#endif
		}
	}
#endif

#ifdef WH_EVENT_NOTIFIER
	if (IS_WCID_VALID(pAd, pEntry->wcid)) {
		EventHdlr pEventHdlrHook = NULL;
		STA_TR_ENTRY *tr_entry = tr_entry_get(pAd, pEntry->wcid);

		if (tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
			pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_JOIN);
			if (pEventHdlrHook && pEntry->wdev)
				pEventHdlrHook(pAd, pEntry, Elem);
		}
	}
#endif /* WH_EVENT_NOTIFIER */

	if (IS_AKM_WPA2(pSecConfig->AKMMap)
		|| IS_AKM_WPA2PSK(pSecConfig->AKMMap)) {
		/* send wireless event - for set key done WPA2*/
		RTMPSendWirelessEvent(pAd, IW_SET_KEY_DONE_WPA2_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);
	} else {
		/* send wireless event - for set key done WPA*/
		RTMPSendWirelessEvent(pAd, IW_SET_KEY_DONE_WPA1_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_WARN, "AP SETKEYS DONE - AKMMap=%s, PairwiseCipher=%s, GroupCipher=%s from "MACSTR"\n\n",
			 GetAuthModeStr(pSecConfig->AKMMap),
			 GetEncryModeStr(pSecConfig->PairwiseCipher),
			 GetEncryModeStr(pSecConfig->GroupCipher),
			 MAC2STR(pHandshake4Way->SAddr));
}


static VOID WpaEAPOLStartAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	MAC_TABLE_ENTRY *pEntry  = NULL;
	STA_TR_ENTRY *tr_entry  = NULL;
	PSECURITY_CONFIG pSecConfig  = NULL;
	PHEADER_802_11 pHeader;
	struct wifi_dev *wdev;
	struct wifi_dev_ops *ops;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	wdev = Elem->wdev;
	ops = wdev->wdev_ops;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
		"===>\n");
	pHeader = (PHEADER_802_11)Elem->Msg;

	/*For normaol PSK, we enqueue an EAPOL-Start command to trigger the process.*/
	if (Elem->MsgLen == MAC_ADDR_LEN)
		ops->mac_entry_lookup(pAd, Elem->Msg, Elem->wdev, &pEntry);
	else {
		ops->mac_entry_lookup(pAd, pHeader->Addr2, Elem->wdev, &pEntry);

#ifdef WSC_AP_SUPPORT

		/*
		    a WSC enabled AP must ignore EAPOL-Start frames received from clients that associated to
		    the AP with an RSN IE or SSN IE indicating a WPA2-PSK/WPA-PSK authentication method in
		    the assication request.  <<from page52 in Wi-Fi Simple Config Specification version 1.0g>>
		*/
		if (pEntry &&
			(pEntry->func_tb_idx == MAIN_MBSSID) &&
			(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.WscConfMode != WSC_DISABLE) &&
			(IS_AKM_PSK_Entry(pEntry)) && pEntry->bWscCapable) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
				"WPS enabled AP: Ignore EAPOL-Start frames received from clients.\n");
			return;
		}

#endif /* WSC_AP_SUPPORT */
	}

	/*TODO: find the root cause.*/
	if (pEntry == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
				"cannot find entry:"MACSTR"\n", MAC2STR(Elem->Msg));
		return;
	} else
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
			"find entry:"MACSTR"\n", MAC2STR(pEntry->Addr));

	pSecConfig = &pEntry->SecConfig;

	tr_entry = tr_entry_get(pAd, pEntry->wcid);
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
		"PortSecured(%d), WpaState(%d),AKM(0x%x),is_cache(%d)\n",
		tr_entry->PortSecured, pSecConfig->Handshake.WpaState,
		pSecConfig->AKMMap, is_pmkid_cache_in_sec_config(pSecConfig));

	if ((tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
		&& (pSecConfig->Handshake.WpaState < AS_PTKSTART)
		&& (IS_AKM_PSK(pSecConfig->AKMMap)
			|| (IS_AKM_WPA2(pSecConfig->AKMMap) && (is_pmkid_cache_in_sec_config(pSecConfig)))
			|| (IS_AKM_WPA3_192BIT(pSecConfig->AKMMap) && (is_pmkid_cache_in_sec_config(pSecConfig)))
			|| (IS_AKM_OWE(pSecConfig->AKMMap) && (is_pmkid_cache_in_sec_config(pSecConfig))))) {
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
		NdisZeroMemory(pSecConfig->Handshake.ReplayCounter, LEN_KEY_DESC_REPLAY);
		pSecConfig->Handshake.MsgRetryCounter = 0;
#ifndef RT_CFG80211_SUPPORT
		WPABuildPairMsg1(pAd, &pEntry->SecConfig, pEntry);
#endif /* RT_CFG80211_SUPPORT */
	}

	log_time_end(LOG_TIME_CONNECTION, "eapol_start", DBG_LVL_INFO, &tl);
}


/*
    ==========================================================================
    Description:
	This is state machine function.
	When receiving EAPOL packets which is  for 802.1x key management.
	Use both in WPA, and WPAPSK case.
	In this function, further dispatch to different functions according to the received packet.  3 categories are :
	  1.  normal 4-way pairwisekey and 2-way groupkey handshake
	  2.  MIC error (Countermeasures attack)  report packet from STA.
	  3.  Request for pairwise/group key update from STA
    Return:
    ==========================================================================
*/
static VOID WpaEAPOLKeyAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	PHEADER_802_11 pHeader;
	PEAPOL_PACKET pEapol_packet;
	KEY_INFO peerKeyInfo;
	UINT eapol_len;
	PSECURITY_CONFIG pSecConfig  = NULL;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	unsigned char hdr_len = LENGTH_802_11;
	pHeader = (PHEADER_802_11)Elem->Msg;

	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
	if (pHeader->FC.Order == 1)
		hdr_len += LENGTH_802_11_HTC;
	if (pHeader->FC.SubType == SUBTYPE_QDATA)
		hdr_len += LENGTH_802_11_QOS_FIELD;


	pEapol_packet = (PEAPOL_PACKET)&Elem->Msg[hdr_len + LENGTH_802_1_H];
	eapol_len = CONV_ARRARY_TO_UINT16(pEapol_packet->Body_Len) + LENGTH_EAPOL_H;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "Receive EAPoL-Key frame from "MACSTR"\n", MAC2STR(pHeader->Addr2));

	if (eapol_len > Elem->MsgLen - hdr_len - LENGTH_802_1_H) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "The length of EAPoL packet is invalid from "MACSTR"\n", MAC2STR(pHeader->Addr2));
		return;
	}

	NdisMoveMemory((PUCHAR)&peerKeyInfo, (PUCHAR)&pEapol_packet->KeyDesc.KeyInfo, sizeof(KEY_INFO));
	*((USHORT *)&peerKeyInfo) = cpu2le16(*((USHORT *)&peerKeyInfo));

	if (((pEapol_packet->ProVer != EAPOL_VER) && (pEapol_packet->ProVer != EAPOL_VER2)) ||
		((pEapol_packet->KeyDesc.Type != WPA1_KEY_DESC) && (pEapol_packet->KeyDesc.Type != WPA2_KEY_DESC) && (pEapol_packet->KeyDesc.Type != KEY_DESC_EXT))) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "Key descripter does not match with WPA rule from "MACSTR"\n", MAC2STR(pHeader->Addr2));
		return;
	}

#ifdef MAC_REPEATER_SUPPORT
	if (pAd->ApCfg.bMACRepeaterEn) {
		REPEATER_CLIENT_ENTRY *rept = lookup_rept_entry(pAd, pHeader->Addr1);
		if (rept != NULL) {
			pEntry = rept->pMacEntry;

			if (!pEntry)
				return;

			pEntry->SecConfig.STARec_Bssid = rept->main_wdev->bss_info_argument.ucBssIndex;
		}
	}
#endif /* MAC_REPEATER_SUPPORT */

	if (pEntry == NULL)
	{
		UINT32 wdev_type = Elem->wdev->wdev_type;
#ifdef CONFIG_AP_SUPPORT
		if (wdev_type == WDEV_TYPE_AP)
			/* use Elem->Wcid to get pEntry, or it will fail at non-primary link */
			/* original: pEntry = MacTableLookup(pAd, pHeader->Addr2); */
			pEntry = entry_get(pAd, Elem->Wcid);
#endif
#ifdef CONFIG_STA_SUPPORT
		if ((wdev_type == WDEV_TYPE_STA) || (wdev_type == WDEV_TYPE_ADHOC))
			pEntry = MacTableLookup2(pAd, pHeader->Addr2, Elem->wdev);
#endif

		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "MacTableLookup FAILE with A2 "MACSTR"\n", MAC2STR(pHeader->Addr2));
			return;
		}

		pEntry->SecConfig.STARec_Bssid = pEntry->wdev->bss_info_argument.ucBssIndex;
	}

	if (IS_ENTRY_NONE(pEntry)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
				 "pEntry is none ,wcid = %d, A2="MACSTR"\n",
				  pEntry->wcid, MAC2STR(pHeader->Addr2));
		return;
	}

	tr_entry = tr_entry_get(pAd, pEntry->wcid);
	pSecConfig = &pEntry->SecConfig;
	pHandshake4Way = &pSecConfig->Handshake;

	/* The value 1 shall be used for all EAPOL-Key frames to and from a STA when */
	/* neither the group nor pairwise ciphers are CCMP for Key Descriptor 1.*/
	if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher) && (peerKeyInfo.KeyDescVer != KEY_DESC_TKIP)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "Key descripter version not match(TKIP) from "MACSTR"\n", MAC2STR(pHeader->Addr2));
		return;
	}

#ifdef DOT11W_PMF_SUPPORT
	else if ((pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA256) &&
		!(peerKeyInfo.KeyDescVer == KEY_DESC_EXT || peerKeyInfo.KeyDescVer == KEY_DESC_NOT_DEFINED))
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
			"[PMF] Key descripter version not match(AES-128/NOT_DEFINED)\n");

#endif  /* DOT11W_PMF_SUPPORT */
	/* The value 2 shall be used for all EAPOL-Key frames to and from a STA when */
	/* either the pairwise or the group cipher is AES-CCMP for Key Descriptor 2 or 3.*/
	else if ((peerKeyInfo.KeyDescVer == KEY_DESC_AES)
			 && (!(IS_CIPHER_TKIP(pSecConfig->PairwiseCipher)
				   || IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher)
				   || IS_CIPHER_CCMP256(pSecConfig->PairwiseCipher)
				   || IS_CIPHER_GCMP128(pSecConfig->PairwiseCipher)
				   || IS_CIPHER_GCMP256(pSecConfig->PairwiseCipher)))) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "Key descripter version not match peerKeyInfo.KeyDescVer=%d, PairwiseCipher=0x%x  from "MACSTR"\n"
				 , peerKeyInfo.KeyDescVer, pSecConfig->PairwiseCipher, MAC2STR(pHeader->Addr2));
		return;
	}

	/* Check if this STA is in class 3 state and the WPA state is started						*/
	if ((pEntry->Sst == SST_ASSOC) && (pSecConfig->Handshake.WpaState >= AS_INITPSK)) {
		/* Check the Key Ack (bit 7) of the Key Information to determine the Authenticator or not.*/
		/* An EAPOL-Key frame that is sent by the Supplicant in response to an EAPOL-*/
		/* Key frame from the Authenticator must not have the Ack bit set.*/
		if (peerKeyInfo.KeyAck == 1) {
			/* The frame is snet by Authenticator. So the Supplicant side shall handle this.*/
			/*
				IOT with 3COM 3CRWE454G72, Mode : WL-525 : According to 802.11i spec, actually we don't need to check the peerKeyInfo.Secure == 0 form peer RootAP
				We found the AP will set peerKeyInfo.Secure to 1 in EAPOL-MSG1 after we reconnect the AP again, then driver drop the MSG1, DUT can't connect success anymore.

			*/
			if ((peerKeyInfo.Request == 0)
#ifndef FAST_EAPOL_WAR
				&& (peerKeyInfo.Secure == 0)
#endif /* !FAST_EAPOL_WAR */
				&& (peerKeyInfo.Error == 0)
				&& (peerKeyInfo.KeyType == PAIRWISEKEY)) {

				/* Process
				    1. the message 1 of 4-way HS in WPA or WPA2
					EAPOL-Key(0,0,1,0,P,0,0,ANonce,0,DataKD_M1)
				    2. the message 3 of 4-way HS in WPA
					EAPOL-Key(0,1,1,1,P,0,KeyRSC,ANonce,MIC,DataKD_M3)
				 */
				if (peerKeyInfo.KeyMic == 0) {
					if ((!pEntry->sta_rec_valid) && (Elem->Priv < 5)) {
						Elem->Priv++;
						MlmeEnqueueWithWdev(pAd, WPA_STATE_MACHINE, MT2_EAPOLKey,
							Elem->MsgLen, Elem->Msg, Elem->Priv, Elem->wdev, FALSE, NULL);
						return;
					} else
						PeerPairMsg1Action(pAd, pEntry, &pEntry->SecConfig, Elem);
				} else
					PeerPairMsg3Action(pAd, pEntry, &pEntry->SecConfig, Elem);
			} else if ((peerKeyInfo.Secure == 1) && (peerKeyInfo.KeyMic == 1)
					   && (peerKeyInfo.Request == 0) && (peerKeyInfo.Error == 0)) {
				/* Process
				    1. the message 3 of 4-way HS in WPA2
					EAPOL-Key(1,1,1,1,P,0,KeyRSC,ANonce,MIC,DataKD_M3)
				    2. the message 1 of group KS in WPA or WPA2
					EAPOL-Key(1,1,1,0,G,0,Key RSC,0, MIC,GTK[N])
				*/
				if (peerKeyInfo.KeyType == PAIRWISEKEY)
					PeerPairMsg3Action(pAd, pEntry, &pEntry->SecConfig, Elem);
				else
					PeerGroupMsg1Action(pAd, pEntry, &pEntry->SecConfig, Elem);
			}
		} else {
			/* The frame is snet by Supplicant.So the Authenticator side shall handle this. */
#ifdef CONFIG_AP_SUPPORT
			if ((peerKeyInfo.KeyMic == 1) && (peerKeyInfo.Request == 1)
				&& (peerKeyInfo.Error == 1)) {
				/* The Supplicant uses a single Michael MIC Failure Report frame */
				/* to report a MIC failure event to the Authenticator. */
				/* A Michael MIC Failure Report is an EAPOL-Key frame with */
				/* the following Key Information field bits set to 1: */
				/* MIC bit, Error bit, Request bit, Secure bit.*/
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
					"Received an Michael MIC Failure Report, active countermeasure\n");
				RTMP_HANDLE_COUNTER_MEASURE(pAd, pEntry);
			} else
#endif /* CONFIG_AP_SUPPORT */
{
			UCHAR KeyMic = peerKeyInfo.KeyMic;

#ifdef OCE_FILS_SUPPORT
			if (IS_AKM_FILS(pEntry->SecConfig.AKMMap))
				KeyMic = 1;
#endif /* OCE_FILS_SUPPORT */

			if ((peerKeyInfo.Request == 0) && (peerKeyInfo.Error == 0)
				&& (KeyMic == 1)) {
				if (peerKeyInfo.Secure == 0 && peerKeyInfo.KeyType == PAIRWISEKEY) {
					/*
					EAPOL-Key(0,1,0,0,P,0,0,SNonce,MIC,Data) Process:
					1. message 2 of 4-way HS in WPA or WPA2
					2. message 4 of 4-way HS in WPA
					*/
					UINT8 mic_len = LEN_KEY_DESC_MIC;
					UINT8 *key_data_len_ptr = NULL;

					if (IS_AKM_SHA384(pEntry->SecConfig.AKMMap))
						mic_len = LEN_KEY_DESC_MIC_SHA384;

					key_data_len_ptr = pEapol_packet->KeyDesc.KeyMicAndData + mic_len;

					if (CONV_ARRARY_TO_UINT16(key_data_len_ptr) == 0)
						PeerPairMsg4Action(pAd, pEntry, &pEntry->SecConfig, Elem);
					else
						PeerPairMsg2Action(pAd, pEntry, &pEntry->SecConfig, Elem);
				} else if (peerKeyInfo.Secure == 1 && peerKeyInfo.KeyType == PAIRWISEKEY) {
					/* EAPOL-Key(1,1,0,0,P,0,0,0,MIC,0) */
					/* Process message 4 of 4-way HS in WPA2*/
					PeerPairMsg4Action(pAd, pEntry, &pEntry->SecConfig, Elem);
				} else if (peerKeyInfo.Secure == 1 && peerKeyInfo.KeyType == GROUPKEY) {
					/* EAPOL-Key(1,1,0,0,G,0,0,0,MIC,0)*/
					/* Process message 2 of Group key HS in WPA or WPA2 */
					PeerGroupMsg2Action(pAd, pEntry, &pEntry->SecConfig, Elem);
				}
			}
}
		}
	}
}


VOID WPAStartFor4WayExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	MAC_TABLE_ENTRY *pEntry = (PMAC_TABLE_ENTRY) FunctionContext;
	struct _SECURITY_CONFIG *pSecConfig  = NULL;
	PRTMP_ADAPTER pAd = NULL;

	if (!pEntry)
		return;

	pAd = (PRTMP_ADAPTER) pEntry->pAd;
	pSecConfig = &pEntry->SecConfig;

#ifdef DOT11_EHT_BE
	if (pEntry->mlo.mlo_en && !pEntry->mlo.is_setup_link_entry)
		return;
#endif

	if (pSecConfig->Handshake.WpaState >= AS_PTKSTART)
		return;

	if (IS_AKM_PSK(pSecConfig->AKMMap)
		|| (pEntry->EnqueueEapolStartTimerRunning == EAPOL_START_PSK /*For PMKIDCache */)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"Enqueue EAPoL-Start-PSK for sta"MACSTR")\n", MAC2STR(pSecConfig->Handshake.SAddr));

		MlmeEnqueueWithWdev(pAd, WPA_STATE_MACHINE, MT2_EAPOLStart, 6, &pEntry->Addr,
			0, pEntry->wdev, FALSE, NULL);
		RTMP_MLME_HANDLER(pAd);
	}

#ifdef DOT1X_SUPPORT
	else if  (IS_AKM_1X(pSecConfig->AKMMap)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
			"Enqueue EAPoL-Start-1X for sta"MACSTR")\n", MAC2STR(pSecConfig->Handshake.SAddr));
		DOT1X_EapTriggerAction(pAd, pEntry);
	}

#endif /* DOT1X_SUPPORT */
	pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
}


static VOID wpa_2way_action(
	IN struct _RTMP_ADAPTER *ad,
	IN MLME_QUEUE_ELEM * Elem)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct wifi_dev *wdev;
	struct wifi_dev_ops *ops;
	struct _SECURITY_CONFIG *sec_config  = NULL;
	STA_TR_ENTRY *tr_entry;

	wdev = Elem->wdev;
	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"2-way fail due to wdev is null!\n");
		return;
	}
	ops = wdev->wdev_ops;/*ALPS05330043*/
	ops->mac_entry_lookup(ad, Elem->Msg, Elem->wdev, &pEntry);

	if (!pEntry) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR,
			"2-way fail due to pEntry is null!\n");
		return;
	}

	sec_config = &pEntry->SecConfig;
	tr_entry = tr_entry_get(ad, pEntry->wcid);

	if (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)
		return;

	if (sec_config->Handshake.GTKState == REKEY_NEGOTIATING)
		return;

	sec_config->Handshake.MsgRetryCounter = 0;
	sec_config->Handshake.GTKState = REKEY_NEGOTIATING;
	WPABuildGroupMsg1(ad, sec_config, pEntry);
}


VOID WPAStartFor2WayExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)FunctionContext;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) pEntry->pAd;

	MlmeEnqueueWithWdev(ad, WPA_STATE_MACHINE, MT2_EAPOL2way, MAC_ADDR_LEN,
		&pEntry->Addr, 0, pEntry->wdev, FALSE, NULL);
	RTMP_MLME_HANDLER(ad);
}



static VOID WpaEAPOLRetryAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR retryLimit;
	struct wifi_dev *wdev;
	struct wifi_dev_ops *ops;

	wdev = Elem->wdev;
	ops = wdev->wdev_ops;

	ops->mac_entry_lookup(pAd, Elem->Msg, Elem->wdev, &pEntry);

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
		"===>\n");

	if ((pEntry) && IS_ENTRY_CLIENT(pEntry)) {
		struct _SECURITY_CONFIG *pSecConfig  = &pEntry->SecConfig;
		PHANDSHAKE_PROFILE pHandshake = &pSecConfig->Handshake;

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO,
				"===> STA"MACSTR"), WpaState=%d,  MsgType=%d\n",
				MAC2STR(Elem->Msg), pHandshake->WpaState, pHandshake->MsgType);

		if ((pHandshake->MsgType == EAPOL_PAIR_MSG_1 && pHandshake->WpaState >= AS_PTKINIT_NEGOTIATING)
			|| (pHandshake->MsgType == EAPOL_PAIR_MSG_3 && pHandshake->WpaState >= AS_PTKINITDONE)) {
			return;
		}

		pHandshake->MsgRetryCounter++;

		if (IS_AKM_PSK(pSecConfig->AKMMap) || IS_AKM_1X(pSecConfig->AKMMap)) {
			if (pHandshake->MsgType == EAPOL_PAIR_MSG_1) {
#ifdef CONFIG_MAP_SUPPORT
				/* Retry counter limitation */
				/* MAP Certification[MT7621 + 7615D] Data Passing Test TC 4.10.4_ETH_FH24G*/
				/* Broadcom Agent it taking more than 3 seconds in sending Reply for EAP Message 1.*/
				/* Increase the counter to retry EAP message 1 from 2 to  4 times.*/
				if (IS_MAP_ENABLE(pAd))
					retryLimit = PEER_MSG1_RETRY_LIMIT + 2;
				else
#endif /* CONFIG_MAP_SUPPORT */
					retryLimit = PEER_MSG1_RETRY_LIMIT;
				if (pHandshake->MsgRetryCounter > retryLimit) {
					pHandshake->WpaState = AS_INITIALIZE;
					pHandshake->GTKState = REKEY_FAILURE;
					/* send wireless event - for pairwise key handshaking timeout */
					RTMPSendWirelessEvent(pAd, IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);
					MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "4Way-MSG1 timeout with "MACSTR"\n", MAC2STR(pHandshake->SAddr));
					if (pHandshake->ReasonCode == 0)
						pHandshake->ReasonCode = REASON_4_WAY_TIMEOUT;
					MlmeDeAuthAction(pAd, pEntry, pHandshake->ReasonCode, FALSE);
#ifdef MAP_R2
					if (IS_MAP_R2_ENABLE(pAd))
						wapp_send_sta_connect_rejected(pAd, pEntry->wdev, pEntry->Addr,
									pEntry->bssid,
									WAPP_EAPOL,
									REASON_4_WAY_TIMEOUT,
									0, REASON_4_WAY_TIMEOUT);
#endif
				} else {
#ifndef RT_CFG80211_SUPPORT
					WPABuildPairMsg1(pAd, &pEntry->SecConfig, pEntry);
#endif /* RT_CFG80211_SUPPORT */
					MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "ReTry MSG1 of 4-way Handshake, Counter=%d\n", pHandshake->MsgRetryCounter);
				}
			} else if (pHandshake->MsgType == EAPOL_PAIR_MSG_3) {
				if (pHandshake->MsgRetryCounter > PEER_MSG3_RETRY_LIMIT) {
					pHandshake->WpaState = AS_INITIALIZE;
					pHandshake->GTKState = REKEY_FAILURE;
					/* send wireless event - for pairwise key handshaking timeout */
					RTMPSendWirelessEvent(pAd, IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);
					MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "4Way-MSG3 timeout with "MACSTR"\n", MAC2STR(pHandshake->SAddr));
					MlmeDeAuthAction(pAd, pEntry, REASON_4_WAY_TIMEOUT, FALSE);
#ifdef MAP_R2
					if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
						wapp_send_sta_connect_rejected(pAd, pEntry->wdev, pEntry->Addr,
									pEntry->bssid,
									WAPP_EAPOL,
									REASON_4_WAY_TIMEOUT, 0, REASON_4_WAY_TIMEOUT);
#endif
				} else {
					WPABuildPairMsg3(pAd, &pEntry->SecConfig, pEntry);
					MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "ReTry MSG3 of 4-way Handshake, Counter = %d\n", pHandshake->MsgRetryCounter);
				}
			} else if (pHandshake->MsgType == EAPOL_GROUP_MSG_1) {
				if (pHandshake->MsgRetryCounter > GROUP_MSG1_RETRY_LIMIT) {
					pHandshake->GTKState = REKEY_FAILURE;
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_ERROR, "Group rekey timeout from "MACSTR"\n", MAC2STR(pHandshake->SAddr));
				} else {
					WPABuildGroupMsg1(pAd, &pEntry->SecConfig, pEntry);
					MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPA, DBG_LVL_INFO, "ReTry MSG1 of 2-way Handshake, Counter = %d\n", pHandshake->MsgRetryCounter);
				}
			}
		}
	}
}


VOID WPAHandshakeMsgRetryExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)FunctionContext;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) pEntry->pAd;

	MlmeEnqueueWithWdev(ad, WPA_STATE_MACHINE, MT2_EAPOLRetry, MAC_ADDR_LEN,
		&pEntry->Addr, 0, pEntry->wdev, FALSE, NULL);
	RTMP_MLME_HANDLER(ad);
}
#endif /* RT_CFG80211_SUPPORT */

VOID group_key_install(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev)
{
	ASIC_SEC_INFO Info = {0};
	USHORT Wcid;
	struct _SECURITY_CONFIG *sec_cfg = &wdev->SecConfig;

	/* Get a specific WCID to record this MBSS key attribute */
	GET_GroupKey_WCID(wdev, Wcid);
	/* Set key material to Asic */
	os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
	Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
	Info.Direction = SEC_ASIC_KEY_TX;
	Info.Wcid = Wcid;
	Info.BssIndex = wdev->func_idx;
	Info.Cipher = sec_cfg->GroupCipher;
	Info.KeyIdx = sec_cfg->GroupKeyId;
	os_move_mem(&Info.PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
	/* Install Shared key */
	os_move_mem(Info.Key.Key, sec_cfg->GTK, LEN_MAX_GTK);
#ifdef DOT11W_PMF_SUPPORT
	if (sec_cfg->PmfCfg.MFPC == TRUE) {
		UCHAR idx = (sec_cfg->PmfCfg.IGTK_KeyIdx == 4) ? 0 : 1;
		os_move_mem(Info.IGTK, &sec_cfg->PmfCfg.IGTK[idx][0], LEN_MAX_IGTK);
		if (IS_CIPHER_BIP_CMAC128(sec_cfg->PmfCfg.igtk_cipher) ||
			IS_CIPHER_BIP_GMAC128(sec_cfg->PmfCfg.igtk_cipher))
			Info.IGTKKeyLen = LEN_BIP128_IGTK;
		else if (IS_CIPHER_BIP_CMAC256(sec_cfg->PmfCfg.igtk_cipher) ||
			IS_CIPHER_BIP_GMAC256(sec_cfg->PmfCfg.igtk_cipher))
			Info.IGTKKeyLen = LEN_BIP256_IGTK;

		if (Info.IGTKKeyLen != 0)
			Info.Cipher |= sec_cfg->PmfCfg.igtk_cipher;

		Info.igtk_key_idx = sec_cfg->PmfCfg.IGTK_KeyIdx;
	}

#endif /* DOT11W_PMF_SUPPORT */
#ifdef BCN_PROTECTION_SUPPORT
	if (sec_cfg->bcn_prot_cfg.bcn_prot_en == TRUE) {
		UCHAR idx = get_bigtk_table_idx(&sec_cfg->bcn_prot_cfg);
		os_move_mem(Info.bigtk, &sec_cfg->bcn_prot_cfg.bigtk[idx][0], LEN_MAX_BIGTK);
		if (wpa3_test_ctrl == 6)
			os_zero_mem(Info.bigtk, LEN_MAX_BIGTK);

		/* assume that pmf is on and igtk cipher is equal to bigtk cipher */
		/* do nothing for asic_sec_info->Cipher assignment due to igtk cipher == bigtk cipher*/
		Info.bigtk_key_len = Info.IGTKKeyLen;

		Info.bigtk_key_idx = sec_cfg->bcn_prot_cfg.bigtk_key_idx;
	}
#endif

	WPAInstallKey(ad, &Info, TRUE, TRUE);
}


/* wpa3_test_ctrl:
 * 1: wrong rsnxe in msg3
 * 2: sae confirm msg right after commit msg
 * 3: use peer-commit-scalar as own-commit-scalar in sae commit msg
 * 4: send sae commit msg raw date received from ucc
 * 5: only enable sae group received from ucc
 * 6: for bcn protection, carry OMN Element and Channel Switch Element in beacon with wrong mic
 * 7: for bcn protection, carry OMN Element and Channel Switch Element in beacon with wrong bipn
 * 8: for ft rsnxe interoperability issue, set rsnxe_used in staut case
 * 9: for ft mismatched RSNXE contents staut, set mismatch rsnxe in reassoc rsp
 *10: for 6G PMF test, unprotected.
 *11: fix the disable transition mode kde content (second parameter is kde content)
 *12: modify channel in OCI (second parameter is channel)
 */
INT set_wpa3_test(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg)
{
	RTMP_STRING *str  = NULL;
	RTMP_STRING *str2  = NULL;

	if (arg == NULL || strlen(arg) == 0)
		return FALSE;

	str = strsep(&arg, ":");

	if (arg != NULL) {
		str2 = strsep(&arg, ":");
		wpa3_test_ctrl2 = os_str_tol(str2, 0, 10);
	}

	wpa3_test_ctrl = os_str_tol(str, 0, 10);


#ifdef CONFIG_AP_SUPPORT
#ifdef BCN_PROTECTION_SUPPORT
	{
		struct wifi_dev *wdev = NULL;
		POS_COOKIE obj = (POS_COOKIE) ad->OS_Cookie;

		if (wpa3_test_ctrl == 6) {
			wdev = &ad->ApCfg.MBSSID[obj->ioctl_if].wdev;
			group_key_install(ad, wdev);
			UpdateBeaconHandler(ad, wdev, BCN_REASON(BCN_UPDATE_IF_STATE_CHG));
		} else if (wpa3_test_ctrl == 7) {
			wdev = &ad->ApCfg.MBSSID[obj->ioctl_if].wdev;
			bcn_prot_update_bipn(ad, wdev);
			UpdateBeaconHandler(ad, wdev, BCN_REASON(BCN_UPDATE_IF_STATE_CHG));
		}
	}
#endif
#endif

	MTWF_PRINT("%s::wpa3_test_ctrl = %d, wpa3_test_ctrl2 = %d\n", __func__, wpa3_test_ctrl, wpa3_test_ctrl2);

	return TRUE;
}

INT set_transition_disable(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg)
{
#ifdef CONFIG_AP_SUPPORT
	RTMP_STRING *str  = NULL;
	RTMP_STRING *str2  = NULL;
	UCHAR para;
	UCHAR para2 = 0;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE obj = (POS_COOKIE) ad->OS_Cookie;

	if (arg == NULL || strlen(arg) == 0)
		return FALSE;

	str = strsep(&arg, ":");

	if (arg != NULL) {
		str2 = strsep(&arg, ":");
		para2 = os_str_tol(str2, 0, 10);
	}

	para = os_str_tol(str, 0, 10);

	wdev = &ad->ApCfg.MBSSID[obj->ioctl_if].wdev;

	wdev->SecConfig.td_value_fixed_en = (para > 0) ? TRUE : FALSE;
	if (wdev->SecConfig.td_value_fixed_en)
		wdev->SecConfig.td_value = para2;

	MTWF_PRINT("%s::td_value_fixed_en = %d, td_value = %d\n",
			__func__, wdev->SecConfig.td_value_fixed_en, wdev->SecConfig.td_value);
#endif

	return TRUE;
}
