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
/****************************************************************************
****************************************************************************

   Module Name:
   bcn.c

   Abstract:
   separate Bcn related function

   Revision History:
   Who         When          What
   --------    ----------    ----------------------------------------------
   Carter      2014-1121     created for all interface could send bcn.

*/

#ifdef CONFIG_AP_SUPPORT
#include "rt_config.h"

#define MAX_TRANSMIT_POWER 30

typedef NTSTATUS (*BCN_BPCC_CMDHdlr)(RTMP_ADAPTER*, PCmdQElmt);

struct MT_BCN_BPCC_CMD_TABL_T {
	UINT32 CmdID;
	BCN_BPCC_CMDHdlr CmdHdlr;
};

static struct wifi_dev *_bcn_bpcc_available_tx_wdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
static void _bcn_bpcc_CmdQExit(RTMP_ADAPTER *pAd);
static void _bcn_bpcc_CmdQInit(RTMP_ADAPTER *pAd);
static INT _bcn_bpcc_cmd_thread(ULONG context);
static NTSTATUS _bcn_bpcc_ct_switch_handler(RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
static BOOLEAN _bcn_bpcc_get_value_by_key(RTMP_STRING *key, RTMP_STRING *dest, UINT32 dest_size, RTMP_STRING *buffer);
static VOID _bcn_bpcc_info_dump(struct wifi_dev *wdev, BOOLEAN detail, BOOLEAN reset, BOOLEAN ignore_hist);
static INT _bcn_bpcc_mld_log(struct wifi_dev *wdev, BOOLEAN log);
static BOOLEAN _bcn_bpcc_op_lock(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN legacy);
static VOID _bcn_bpcc_op_lock_statistic_update(RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
	BOOLEAN query_bssmngr, enum BCN_BPCC_IE_TYPE ie_type,
	BOOLEAN lock_result, ULONG get_lock_time_period);
static VOID _bcn_bpcc_op_lock_to(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3);
static BOOLEAN _bcn_bpcc_op_unlock(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
static VOID _bcn_bpcc_trigger(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3);
static NDIS_STATUS _bcn_bpcc_RTEnqueueInternalCmd(
	IN PRTMP_ADAPTER	pAd,
	IN NDIS_OID			Oid,
	IN PVOID			pInformationBuffer,
	IN UINT32			InformationBufferLength);
static VOID _bcn_bpcc_CMDHandler(RTMP_ADAPTER *pAd);
static void _bcn_bpcc_deinit_all(RTMP_ADAPTER *pAd);

static char BCN_BPCC_IE_TYPE_STR[BCN_BPCC_MAX][64] = {
	"BCN_BPCC_CSA",
	"BCN_BPCC_ECSA",
	"BCN_BPCC_EDCA",
	"BCN_BPCC_QUIET",
	"BCN_BPCC_DSSS",
	"BCN_BPCC_HTOP",
	"BCN_BPCC_WBCS",
	"BCN_BPCC_CSW",
	"BCN_BPCC_OMN",
	"BCN_BPCC_QUIETC",
	"BCN_BPCC_VHTOP",
	"BCN_BPCC_HEOP",
	"BCN_BPCC_BTWT",
	"BCN_BPCC_BTWTSET",
	"BCN_BPCC_BSSCCA",
	"BCN_BPCC_MUEDCA",
	"BCN_BPCC_SR",
	"BCN_BPCC_UORA",
	"BCN_BPCC_EHTOP",
	"BCN_BPCC_CH_CHANGED",
	"BCN_BPCC_RECONFIG",
	"BCN_BPCC_ADD_LINK",
	"BCN_BPCC_AT2LM",
	"BCN_BPCC_LEGACY_BCN"
};
#define BCN_BPCC_OP_LOCK_MAX_TIME		(3 * OS_HZ) /* 3 secs */
#define BCN_BPCC_OP_TO_MAX_TIME		(3000) /* 3 secs */
static UINT32 bpcc_op_lock_max_time_adjust = BCN_BPCC_OP_LOCK_MAX_TIME;
static UINT32 bpcc_op_lock_max_to_adjust = BCN_BPCC_OP_TO_MAX_TIME;
enum {
	BPCC_CMDTHREAD_FIRST_CMD_ID = 0,
	BPCC_CMDTHREAD_BPCC_CT_SWITCH = BPCC_CMDTHREAD_FIRST_CMD_ID,
	BPCC_CMDTHREAD_END_CMD_ID
};
static struct MT_BCN_BPCC_CMD_TABL_T _bcn_bpcc_CMDHdlrTable[] = {
	{BPCC_CMDTHREAD_BPCC_CT_SWITCH, _bcn_bpcc_ct_switch_handler},
	{BPCC_CMDTHREAD_END_CMD_ID, NULL}
};

UCHAR PowerConstraintIE[3] = {IE_POWER_CONSTRAINT, 1, 0};

/*
    ==========================================================================
    Description:
	Used to check the necessary to send Beancon.
    return value
	0: mean no necessary.
	1: mean need to send Beacon for the service.
    ==========================================================================
*/
BOOLEAN BeaconTransmitRequired(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	BOOLEAN result = FALSE;
	BCN_BUF_STRUCT *bcn_info = &wdev->bcn_buf;

	if (bcn_info == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_WARN,
				 "Bcn info is null!\n");
		return result;
	}

	if (bcn_info->stop_tx & STOP_BCN_TX_NO_BCN) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_WARN,
				 "Bcn Tx is blocked!\n");
		return result;
	}

#ifdef CONFIG_ATE
	if (!WDEV_WITH_BCN_ABILITY(wdev) || ATE_ON(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_WARN,
			"Bcn Tx is blocked, wdev_type=%d, ATE_ON=%d\n",
			wdev->wdev_type, ATE_ON(pAd));
		return result;
	}
#endif /*CONFIG_ATE*/
	if (bcn_info->BeaconPkt == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
			"no BeaconPkt\n");
		return result;
	}

	do {
#ifdef WDS_SUPPORT
		/*
		 * WDS is bound on main wifi dev which should not issue Beacons
		 * when system operates as bridge mode
		 */
		if (pAd->WdsTab.Mode == WDS_BRIDGE_MODE)
			break;

#endif /* WDS_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
#ifdef QUIET_SUPPORT

		if (IS_RRM_QUIET(wdev))
			break;

#endif /* QUIET_SUPPORT */
#endif /* DOT11K_RRM_SUPPORT */
		if (bcn_info->bBcnSntReq == TRUE) {
			result = TRUE;
			break;
		}
	} while (FALSE);

	return result;
}

INT bcn_buf_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	BCN_BUF_STRUCT *bcn_info = &wdev->bcn_buf;
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 max_v2_bcn_num = cap->max_v2_bcn_num;
#endif

	bcn_info->cap_ie_pos = 0;
	bcn_info->pWdev = wdev;
#ifdef DOT11_EHT_BE
	bcn_info->at2lm_info.vld_id = 0;
#endif /* DOT11_EHT_BE */
	NdisAllocateSpinLock(pAd, &bcn_info->BcnContentLock);

	if (!bcn_info->BeaconPkt) {
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
		if (wdev->func_idx < max_v2_bcn_num)
		Status = RTMPAllocateNdisPacket(pAd, &bcn_info->BeaconPkt, NULL, 0, NULL, MAX_BEACONV2_LENGTH);
		else
		Status = RTMPAllocateNdisPacket(pAd, &bcn_info->BeaconPkt, NULL, 0, NULL, MAX_BEACON_LENGTH);
#else
		Status = RTMPAllocateNdisPacket(pAd, &bcn_info->BeaconPkt, NULL, 0, NULL, MAX_BEACON_LENGTH);
#endif

		if (Status == NDIS_STATUS_FAILURE)
			return Status;
	}

#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT))
		bcn_info->bcn_state = BCN_TX_IDLE;
#endif /* MT_MAC */

	/* init bpcc timer */
	bcn_bpcc_init(pAd, wdev);

	return Status;
}

INT bcn_buf_deinit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	BCN_BUF_STRUCT *bcn_info = &wdev->bcn_buf;

#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT)) {

		if (bcn_info->bcn_state != BCN_TX_IDLE) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR, "Bcn not in idle(%d) when try to free it!\n",
					 bcn_info->bcn_state);
		}

		bcn_info->bcn_state = BCN_TX_UNINIT;
	}

#endif /* MT_MAC */

	if (bcn_info->BeaconPkt) {
		OS_SEM_LOCK(&bcn_info->BcnContentLock);
		RTMPFreeNdisPacket(pAd, bcn_info->BeaconPkt);
		bcn_info->BeaconPkt = NULL;

		OS_SEM_UNLOCK(&bcn_info->BcnContentLock);
	}

	NdisFreeSpinLock(&bcn_info->BcnContentLock);

	/* deinit bpcc timer */
	bcn_bpcc_deinit(pAd, wdev);

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Pre-build a BEACON frame in the shared memory
    return value
	0:  mean no beacon necessary.
	>0: beacon length.
    ==========================================================================
*/
UINT16 MakeBeacon(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN UpdateRoutine)
{
	ULONG FrameLen = 0, UpdatePos = 0;
	UCHAR *pBeaconFrame, *tmac_info;
	union _HTTRANSMIT_SETTING BeaconTransmit = {.word = 0};   /* MGMT frame PHY rate setting when operatin at HT rate. */
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 TXWISize = cap->TXWISize;
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	USHORT PhyMode;
	UCHAR *ptr = NULL;
	BSS_STRUCT *pMbss = NULL;
	BCN_BUF_STRUCT *pbcn_buf = &wdev->bcn_buf;

#ifdef RT_CFG80211_SUPPORT
	if (pAd->cfg80211_ctrl.beaconIsSetFromHostapd == TRUE)
		return -1;
#endif

	OS_SEM_LOCK(&pbcn_buf->BcnContentLock);
	tmac_info = (UCHAR *)GET_OS_PKT_DATAPTR(pbcn_buf->BeaconPkt);

	if (IS_HIF_TYPE(pAd, HIF_MT))
		pBeaconFrame = (UCHAR *)(tmac_info + tx_hw_hdr_len);
	else
		pBeaconFrame = (UCHAR *)(tmac_info + TXWISize);

	/* if (UpdateRoutine == FALSE) */
	/* { */
	/* not periodically update case, need take care Header and IE which is before TIM ie. */
	FrameLen = ComposeBcnPktHead(pAd, wdev, pBeaconFrame);
	pbcn_buf->tim_ie_offset = FrameLen;
	/* } */
	UpdatePos = pbcn_buf->tim_ie_offset;
	PhyMode = wdev->PhyMode;

	if (UpdateRoutine == TRUE)
		FrameLen = UpdatePos;/* update routine, no FrameLen information, update it for later use. */

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		pMbss = wdev->func_dev;
		/* Tim IE, AP mode only. */
		pbcn_buf->cap_ie_pos = sizeof(HEADER_802_11) + TIMESTAMP_LEN + 2;
		/*
		    step 1 - update AP's Capability info, since it might be changed.
		*/
		ptr = pBeaconFrame + pbcn_buf->cap_ie_pos;
#ifdef CFG_BIG_ENDIAN
		*(ptr + 1) = (UCHAR)(pMbss->CapabilityInfo & 0x00ff);
		*ptr = (UCHAR)((pMbss->CapabilityInfo & 0xff00) >> 8);
#else
		*ptr = (UCHAR)(pMbss->CapabilityInfo & 0x00ff);
		*(ptr + 1) = (UCHAR)((pMbss->CapabilityInfo & 0xff00) >> 8);
#endif
		/*
		    step 2 - Add TIM IE and fw would help to update PVB bitmaps.
		*/
		ptr = pBeaconFrame + pbcn_buf->tim_ie_offset;
		FrameLen += BcnTimAdd(pAd, wdev, ptr);
		UpdatePos = FrameLen;
	}

	ComposeBcnPktTail(pAd, wdev, &UpdatePos, pBeaconFrame, TRUE);
	FrameLen = UpdatePos;/* update newest FrameLen. */

#ifdef IGMP_TVM_SUPPORT
		/* ADD TV IE to this packet */
		MakeTVMIE(pAd, wdev, pBeaconFrame, &FrameLen);
#endif /* IGMP_TVM_SUPPORT */
	/* step 6. Since FrameLen may change, update TXWI. */
#ifdef A_BAND_SUPPORT

	if (WMODE_CAP_5G(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode)) {
		BeaconTransmit.field.MODE = MODE_OFDM;
		BeaconTransmit.field.MCS = MCS_RATE_6;
	}

#endif /* A_BAND_SUPPORT */
#ifdef GN_MIXMODE_SUPPORT
	if (pAd->CommonCfg.GNMixMode
		&& (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
			|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)
			|| WMODE_EQUAL(wdev->PhyMode, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G)))) {
		BeaconTransmit.field.MODE = MODE_OFDM;
		BeaconTransmit.field.MCS = MCS_RATE_6;
	}
#endif /* GN_MIXMODE_SUPPORT */
#ifdef OCE_SUPPORT
	if (IS_OCE_ENABLE(wdev) && WMODE_CAP_2G(wdev->PhyMode)) {
		BeaconTransmit.field.MODE =
			(BeaconTransmit.field.MODE >= MODE_OFDM) ? BeaconTransmit.field.MODE : MODE_OFDM;
	}
#endif /* OCE_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
	if (wdev->cfg80211_bcn_rate) {
		BeaconTransmit.field.MODE = wdev->bcn_rate.field.MODE;
		BeaconTransmit.field.MCS = wdev->bcn_rate.field.MCS;
	}
#endif
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	if (wdev->eap.eap_mgmrate_en & (1 << BCN_TYPE)) {
		BeaconTransmit.field.MODE = wdev->eap.mgmphymode[BCN_TYPE].field.MODE;
		BeaconTransmit.field.MCS = wdev->eap.mgmphymode[BCN_TYPE].field.MCS;
	}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

	/*Update current FrameLen in Bcn_Buffer*/
	pbcn_buf->FrameLen = FrameLen;
#ifdef CONFIG_6G_SUPPORT
	/* align tx rate to 6g BC.Probe.Rsp */
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		UCHAR iob_mode = wlan_operate_get_unsolicit_tx_mode(wdev);

		if (iob_mode == UNSOLICIT_TXMODE_HE_SU) {
			BeaconTransmit.field.MODE = MODE_HE;
			BeaconTransmit.field.MCS = MCS_0;
		} else if (iob_mode == UNSOLICIT_TXMODE_NON_HT_DUP) {
			if (pAd->CommonCfg.wifi_cert)
				BeaconTransmit.field.BW = BW_80;
			else {
#ifdef CONFIG_6G_AFC_SUPPORT
				if (is_afc_in_run_state(pAd)
					&& pAd->CommonCfg.AfcSpBwDup == FALSE)
					BeaconTransmit.field.BW = BW_20; /* STD Power*/
				else
#endif /*CONFIG_6G_AFC_SUPPORT*/
					BeaconTransmit.field.BW = BW_80; /* LPI Power*/
			}
			BeaconTransmit.field.MODE = MODE_OFDM;
			BeaconTransmit.field.MCS = MCS_RATE_6;
		}
	}
#endif

	write_tmac_info_offload_pkt(pAd, wdev, FC_TYPE_MGMT, SUBTYPE_BEACON,
								tmac_info, &BeaconTransmit, FrameLen);
#ifdef CFG_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, pBeaconFrame, DIR_WRITE, FALSE);
#endif /* CFG_BIG_ENDIAN */
	OS_SEM_UNLOCK(&pbcn_buf->BcnContentLock);

#ifdef WIFI_DIAG
	diag_bcn_tx(pAd, pMbss, pBeaconFrame, FrameLen);
#endif

	return FrameLen;
}

VOID ComposeWPSIE(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	ULONG FrameLen = *pFrameLen;
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = NULL;
	BOOLEAN bHasWpsIE = FALSE;
#endif
#ifdef CONFIG_AP_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_AP)
		pMbss = wdev->func_dev;

	if (pMbss == NULL)
		return;

#endif

	/* add Simple Config Information Element */
#ifdef DISABLE_HOSTAPD_BEACON
    if (wdev->WscIEBeacon.ValueLen)
#else
    if (((wdev->WscControl.WscConfMode >= 1) && (wdev->WscIEBeacon.ValueLen)))
#endif
		bHasWpsIE = TRUE;

	if (bHasWpsIE) {
		ULONG WscTmpLen = 0;
		MakeOutgoingFrame(pBeaconFrame + FrameLen, &WscTmpLen,
						  wdev->WscIEBeacon.ValueLen, wdev->WscIEBeacon.Value,
						  END_OF_ARGS);
		FrameLen += WscTmpLen;
	}

#ifdef WSC_AP_SUPPORT
	if (pMbss && (wdev->WscControl.WscConfMode != WSC_DISABLE) &&
#ifdef DOT1X_SUPPORT
		IS_IEEE8021X_Entry(&pMbss->wdev) &&
#endif /* DOT1X_SUPPORT */
		IS_CIPHER_WEP_Entry(&pMbss->wdev)) {
		ULONG TempLen = 0;
		UCHAR PROVISION_SERVICE_IE[7] = {0xDD, 0x05, 0x00, 0x50, 0xF2, 0x05, 0x00};

		MakeOutgoingFrame(pBeaconFrame + FrameLen,        &TempLen,
						  7,                            PROVISION_SERVICE_IE,
						  END_OF_ARGS);
		FrameLen += TempLen;
	}

#endif /* WSC_AP_SUPPORT */
	*pFrameLen = FrameLen;
}

VOID MakeErpIE(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	ULONG *pFrameLen,
	UCHAR *pBeaconFrame
)
{
	ULONG FrameLen = *pFrameLen;
	UCHAR *ptr = NULL;
	/* fill ERP IE */
	ptr = (UCHAR *)pBeaconFrame + FrameLen;
	*ptr = IE_ERP;
	*(ptr + 1) = 1;
#ifdef CONFIG_AP_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_AP)
		*(ptr + 2) = pAd->ApCfg.ErpIeContent;

#endif
#ifdef CONFIG_STA_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_STA)
		*(ptr + 2) = 0x04;

#endif
	FrameLen += 3;
	*pFrameLen = FrameLen;
}

#if defined(A_BAND_SUPPORT) && defined(CONFIG_AP_SUPPORT)
VOID MakeChSwitchAnnounceIEandExtend(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame, BOOLEAN bcn)
{
	UCHAR *ptr = NULL;
	ULONG FrameLen = *pFrameLen;
	COMMON_CONFIG *pComCfg = &pAd->CommonCfg;
	struct DOT11_H *pDot11h = NULL;
	UCHAR channel;
	USHORT PhyMode;
	UCHAR bw = BW_20;
	UCHAR reg_cap_bw;
	UCHAR cfg_ht_bw;
#ifdef DOT11_VHT_AC
	UCHAR cfg_vht_bw;
#endif
#ifdef DOT11_HE_AX
	UCHAR cfg_he_bw;
#endif

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"wdev is NULL, return\n");
		return;
	}

	channel = wdev->channel;
	PhyMode = wdev->PhyMode;
	reg_cap_bw = get_channel_bw_cap(wdev, channel);
	cfg_ht_bw = wlan_config_get_ht_bw(wdev);
#ifdef DOT11_VHT_AC
	cfg_vht_bw = wlan_config_get_vht_bw(wdev);
#endif /*DOT11_VHT_AC*/
#ifdef DOT11_HE_AX
	cfg_he_bw = wlan_config_get_he_bw(wdev);
#endif /*DOT11_HE_AX*/

	if (WMODE_CAP_N(PhyMode)) {
		if (cfg_ht_bw)
			bw = BW_40;
		else
			bw = BW_20;
	}

#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(PhyMode)) {
		switch (cfg_vht_bw) {
		case VHT_BW_80:
			bw = BW_80;
			break;

		case VHT_BW_160:
			bw = BW_160;
			break;

		case VHT_BW_8080:
			bw = BW_8080;
			break;

		case VHT_BW_2040:
			if (cfg_ht_bw == BW_40)
				bw = BW_40;
			else
				bw = BW_20;
		default:
			break;
		}
	}

#endif /* DOT11_VHT_AC */

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(PhyMode)) {
		switch (cfg_he_bw) {
		case HE_BW_80:
			bw = BW_80;
			break;
		case HE_BW_160:
			bw = BW_160;
			break;
		case HE_BW_8080:
			bw = BW_8080;
			break;
		case HE_BW_2040:
			if (cfg_ht_bw == BW_40)
				bw = BW_40;
			else
				bw = BW_20;
		default:
			break;
		}
	}
#endif /* DOT11_HE_AX */

/* if bw capability of NewCh is lower than .dat bw config, bw should follow reg_cap_bw*/
	if (bw > reg_cap_bw) {
		if (!(bw == BW_8080 && (reg_cap_bw == BW_80 || reg_cap_bw == BW_160)))
			bw = reg_cap_bw;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"Channel=%d, bw=%d\n", channel, bw);

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"pDot11h is NULL, return\n");
		return;
	}

	ptr = pBeaconFrame + FrameLen;
	*ptr = IE_CHANNEL_SWITCH_ANNOUNCEMENT;
	*(ptr + 1) = 3;
#ifdef ZERO_PKT_LOSS_SUPPORT
	/*if radar not detected on old channel, allow frames*/
	if ((pAd->Zero_Loss_Enable == 1) && (pDot11h->ChannelMode != CHAN_SILENCE_MODE))
		*(ptr + 2) = 0;/*frames allowed*/
	else
#endif /*ZERO_PKT_LOSS_SUPPORT*/
		*(ptr + 2) = 1;/*No further frames*/
	*(ptr + 3) = channel;
	*(ptr + 4) = (pDot11h->CSPeriod - pDot11h->CSCount - 1);

	ptr += 5;
	FrameLen += 5;
#ifdef DOT11_N_SUPPORT
	/* Extended Channel Switch Announcement Element */
	if (pComCfg->bExtChannelSwitchAnnouncement) {
		HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE HtExtChannelSwitchIe;

		build_ext_channel_switch_ie(pAd, &HtExtChannelSwitchIe,
									channel,
									wdev->PhyMode,
									wdev
								   );
		NdisMoveMemory(ptr, &HtExtChannelSwitchIe, sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE));
		ptr += sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE);
		FrameLen += sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE);
	}
#ifdef ZERO_PKT_LOSS_SUPPORT
	/*add secondary channel offset IE
	* for 5Ghz, find ext chn offset using new channel
	*/
	if (pAd->Zero_Loss_Enable) {
		struct GNU_PACKED SecondaryChannelOffsetIe {
			UCHAR		ID;
			UCHAR		Length;
			UCHAR		SecondaryChannelOffset;
		};
		struct SecondaryChannelOffsetIe SecChanOffsetIe;
		UCHAR ext_cha = 0, op_ht_bw = 0;
		int idx;
		UCHAR wfa_ht_ch_ext[] = {
			36, EXTCHA_ABOVE, 40, EXTCHA_BELOW,
			44, EXTCHA_ABOVE, 48, EXTCHA_BELOW,
			52, EXTCHA_ABOVE, 56, EXTCHA_BELOW,
			60, EXTCHA_ABOVE, 64, EXTCHA_BELOW,
			100, EXTCHA_ABOVE, 104, EXTCHA_BELOW,
			108, EXTCHA_ABOVE, 112, EXTCHA_BELOW,
			116, EXTCHA_ABOVE, 120, EXTCHA_BELOW,
			124, EXTCHA_ABOVE, 128, EXTCHA_BELOW,
			132, EXTCHA_ABOVE, 136, EXTCHA_BELOW,
			140, EXTCHA_ABOVE, 144, EXTCHA_BELOW,
			149, EXTCHA_ABOVE, 153, EXTCHA_BELOW,
			157, EXTCHA_ABOVE, 161, EXTCHA_BELOW,
				0, 0};

		op_ht_bw = wlan_operate_get_ht_bw(wdev);
		if (op_ht_bw == BW_40) {
			if (wdev->channel > 14) {
				idx = 0;
				while (wfa_ht_ch_ext[idx] != 0) {
					if (wfa_ht_ch_ext[idx] == wdev->channel) {
						ext_cha = wfa_ht_ch_ext[idx + 1];
						break;
					}
					idx += 2;
				};
				if (wfa_ht_ch_ext[idx] == 0)
					ext_cha = EXTCHA_NONE;
			} else {
				/*2G band case*/
				ext_cha = wlan_operate_get_ext_cha(wdev);
			}
		}
		SecChanOffsetIe.ID = 0x3e;
		SecChanOffsetIe.Length = 0x01;
		SecChanOffsetIe.SecondaryChannelOffset = ext_cha;
		NdisMoveMemory(ptr, &SecChanOffsetIe, sizeof(struct SecondaryChannelOffsetIe));
		ptr += sizeof(struct SecondaryChannelOffsetIe);
		FrameLen += sizeof(struct SecondaryChannelOffsetIe);
	}
#endif /*ZERO_PKT_LOSS_SUPPORT*/

#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(PhyMode)) {
		INT tp_len = 0, wb_len = 0;
		UCHAR *ch_sw_wrapper;
		UCHAR ch_band = wlan_config_get_ch_band(wdev);

		*ptr = IE_CH_SWITCH_WRAPPER;
		ch_sw_wrapper = (UCHAR *)(ptr + 1); /* reserve for length */
		ptr += 2; /* skip len */

		if (bw >= BW_40) {
			WIDE_BW_CH_SWITCH_ELEMENT wb_info;
			*ptr = IE_WIDE_BW_CH_SWITCH;
			*(ptr + 1) = sizeof(WIDE_BW_CH_SWITCH_ELEMENT);
			ptr += 2;
			NdisZeroMemory(&wb_info, sizeof(WIDE_BW_CH_SWITCH_ELEMENT));

			switch (bw) {
			case BW_40:
				wb_info.new_ch_width = 0;
				wb_info.center_freq_1 = vht_cent_ch_freq_40mhz(channel, VHT_BW_2040, ch_band);
				wb_info.center_freq_2 = 0;
				break;

			case BW_80:
				wb_info.new_ch_width = 1;
				wb_info.center_freq_1 = vht_cent_ch_freq(channel, VHT_BW_80, ch_band);
				wb_info.center_freq_2 = 0;
				break;

			case BW_160:
				wb_info.new_ch_width = 1;
				wb_info.center_freq_1 = vht_cent_ch_freq(channel, VHT_BW_80, ch_band);
				wb_info.center_freq_2 = vht_cent_ch_freq(channel, VHT_BW_160, ch_band);
				break;

			case BW_8080:
				wb_info.new_ch_width = 1;
				wb_info.center_freq_1 = vht_cent_ch_freq(channel, VHT_BW_8080, ch_band);
				wb_info.center_freq_2 = wlan_operate_get_cen_ch_2(wdev);
				break;
			}

			NdisMoveMemory(ptr, &wb_info, sizeof(WIDE_BW_CH_SWITCH_ELEMENT));
			wb_len = sizeof(WIDE_BW_CH_SWITCH_ELEMENT);
			ptr += wb_len;
			wb_len += 2;
		}

		if (WMODE_CAP_AX(PhyMode)
			|| WMODE_CAP_AC(PhyMode)
			|| WMODE_CAP_BE(PhyMode)) {
			struct _tx_pwr_env_ie tpe_ie;

			tp_len = build_txpwr_envelope(pAd, wdev, &tpe_ie);
			*ptr = IE_TPE;
			*(ptr + 1) = tp_len;
			ptr += 2;
			ptr += tp_len;
			tp_len += 2;
		}
		*ch_sw_wrapper = wb_len + tp_len;
		FrameLen += (2 + wb_len + tp_len);
	}

#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	*pFrameLen = FrameLen;
}
#endif /* A_BAND_SUPPORT */

VOID MakeHTIe(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	ULONG TmpLen;
	ULONG FrameLen = *pFrameLen;
	UCHAR HtLen, HtLen1;
	COMMON_CONFIG *pComCfg = &pAd->CommonCfg;
	/*UCHAR i; */
	HT_CAPABILITY_IE HtCapabilityTmp;
	HT_CAPABILITY_IE *ht_cap;
#ifdef CFG_BIG_ENDIAN
	ADD_HT_INFO_IE  addHTInfoTmp;
#endif
	ADD_HT_INFO_IE *addht = wlan_operate_get_addht(wdev);
	/* add HT Capability IE */
	HtLen = sizeof(HT_CAPABILITY_IE);
	HtLen1 = sizeof(ADD_HT_INFO_IE);

	if (WMODE_CAP_AX_6G(wdev->PhyMode))
		return;

	ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
	if (ht_cap == NULL)
		return;

	wlan_operate_update_ht_cap(wdev);

#ifndef CFG_BIG_ENDIAN
	NdisMoveMemory(&HtCapabilityTmp, ht_cap, HtLen);
	MakeOutgoingFrame(pBeaconFrame + FrameLen,         &TmpLen,
					  1,                                &HtCapIe,
					  1,                                &HtLen,
					  HtLen,          &HtCapabilityTmp,
					  1,                                &AddHtInfoIe,
					  1,                                &HtLen1,
					  HtLen1,          addht,
					  END_OF_ARGS);
#else
	NdisMoveMemory(&HtCapabilityTmp, ht_cap, HtLen);
	HtCapabilityTmp.HtCapInfo.ChannelWidth = addht->AddHtInfo.RecomWidth;
	*(UINT32 *)(&HtCapabilityTmp.TxBFCap) = cpu2le32(*(UINT32 *)(&HtCapabilityTmp.TxBFCap));
	*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
	{
		EXT_HT_CAP_INFO extHtCapInfo;

		NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
		*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
		NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
	}
#else
	*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */
	NdisMoveMemory(&addHTInfoTmp, addht, HtLen1);
	*(USHORT *)(&addHTInfoTmp.AddHtInfo2) = cpu2le16(*(USHORT *)(&addHTInfoTmp.AddHtInfo2));
	*(USHORT *)(&addHTInfoTmp.AddHtInfo3) = cpu2le16(*(USHORT *)(&addHTInfoTmp.AddHtInfo3));
	MakeOutgoingFrame(pBeaconFrame + FrameLen,         &TmpLen,
					  1,                                &HtCapIe,
					  1,                                &HtLen,
					  HtLen,                   &HtCapabilityTmp,
					  1,                                &AddHtInfoIe,
					  1,                                &HtLen1,
					  HtLen1,                   &addHTInfoTmp,
					  END_OF_ARGS);
#endif
	FrameLen += TmpLen;
#ifdef DOT11N_DRAFT3

	/*
	    P802.11n_D3.03, 7.3.2.60 Overlapping BSS Scan Parameters IE
	*/
	if (WMODE_CAP_2G(wdev->PhyMode) &&
		(ht_cap->HtCapInfo.ChannelWidth == 1)) {
		OVERLAP_BSS_SCAN_IE  OverlapScanParam;
		ULONG   TmpLen;
		UCHAR   OverlapScanIE, ScanIELen;

		OverlapScanIE = IE_OVERLAPBSS_SCAN_PARM;
		ScanIELen = 14;
		OverlapScanParam.ScanPassiveDwell = cpu2le16(pComCfg->Dot11OBssScanPassiveDwell);
		OverlapScanParam.ScanActiveDwell = cpu2le16(pComCfg->Dot11OBssScanActiveDwell);
		OverlapScanParam.TriggerScanInt = cpu2le16(pComCfg->Dot11BssWidthTriggerScanInt);
		OverlapScanParam.PassiveTalPerChannel = cpu2le16(pComCfg->Dot11OBssScanPassiveTotalPerChannel);
		OverlapScanParam.ActiveTalPerChannel = cpu2le16(pComCfg->Dot11OBssScanActiveTotalPerChannel);
		OverlapScanParam.DelayFactor = cpu2le16(pComCfg->Dot11BssWidthChanTranDelayFactor);
		OverlapScanParam.ScanActThre = cpu2le16(pComCfg->Dot11OBssScanActivityThre);
		MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
						  1,          &OverlapScanIE,
						  1,          &ScanIELen,
						  ScanIELen,  &OverlapScanParam,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}

#endif /* DOT11N_DRAFT3 */
	*pFrameLen = FrameLen;
}

#ifdef CONFIG_HOTSPOT
VOID MakeHotSpotIE(struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	ULONG FrameLen = *pFrameLen;
	ULONG TmpLen;
	BSS_STRUCT *pMbss = wdev->func_dev;

	if (pMbss->HotSpotCtrl.HotSpotEnable) {
		OS_SEM_LOCK(&pMbss->HotSpotCtrl.IeLock);
		/* Indication element */
		MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
						  pMbss->HotSpotCtrl.HSIndicationIELen,
						  pMbss->HotSpotCtrl.HSIndicationIE, END_OF_ARGS);
		FrameLen += TmpLen;
		/* P2P element */
		MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
						  pMbss->HotSpotCtrl.P2PIELen,
						  pMbss->HotSpotCtrl.P2PIE, END_OF_ARGS);
		FrameLen += TmpLen;
		OS_SEM_UNLOCK(&pMbss->HotSpotCtrl.IeLock);
	}

	*pFrameLen = FrameLen;
}
#endif /* CONFIG_HOTSPOT */

VOID MakeCountryIe(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	ULONG FrameLen = *pFrameLen;
	ULONG TmpLen, TmpLen2 = 0;
	UCHAR *TmpFrame = NULL;
	UCHAR CountryIe = IE_COUNTRY;
	UCHAR Environment = 0x20;

	if (pAd->CommonCfg.bCountryFlag ||
		(WMODE_CAP_5G(wdev->PhyMode) && pAd->CommonCfg.bIEEE80211H == TRUE)
#ifdef DOT11K_RRM_SUPPORT
		|| IS_RRM_ENABLE(wdev)
#endif /* DOT11K_RRM_SUPPORT */
	   ) {
		os_alloc_mem(NULL, (UCHAR **)&TmpFrame, 256);

		if (TmpFrame != NULL) {
			NdisZeroMemory(TmpFrame, 256);
			/* prepare channel information */
#ifdef EXT_BUILD_CHANNEL_LIST
			BuildBeaconChList(pAd, wdev, TmpFrame, &TmpLen2);
#else
			if (WMODE_CAP_6G(wdev->PhyMode)) {
				UINT i = 0;
				UCHAR OpExtIdentifier = 0xFE;
				UCHAR CoverageClass = 0;
				UCHAR reg_class_value[5] = {0};

				get_reg_class_list_for_6g(pAd, wdev->PhyMode, reg_class_value);

				if (reg_class_value[0] == 0) {
					MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
							"reg_class is NULL !!!\n");
					os_free_mem(TmpFrame);
					return;
				}

				for (i = 0; reg_class_value[i] != 0; i++) {
					MakeOutgoingFrame(TmpFrame + TmpLen2,
							&TmpLen,
							1,
							&OpExtIdentifier,
							1,
							&reg_class_value[i],
							1,
							&CoverageClass,
							END_OF_ARGS);
					TmpLen2 += TmpLen;
					if (i == 4)
						break;
				}
			} else {
				UINT i = 0;
				PCH_DESC pChDesc = NULL;
				UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
				UCHAR MaxTxPower = GetCuntryMaxTxPwr(pAd, wdev->PhyMode, wdev, op_ht_bw);

				MaxTxPower = MAX_TRANSMIT_POWER;
				/* do not change sequence due to 6GHz might include AC/GN then confused */
				if (WMODE_CAP_5G(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode)) {
					if (pAd->CommonCfg.pChDesc5G != NULL)
						pChDesc = (PCH_DESC)pAd->CommonCfg.pChDesc5G;
					else
						MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
								 "pChDesc5G is NULL .\n");
				}

				if (!pChDesc && WMODE_CAP_2G(wdev->PhyMode)) {
					if (pAd->CommonCfg.pChDesc2G != NULL)
						pChDesc = (PCH_DESC)pAd->CommonCfg.pChDesc2G;
					else
						MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
								 "pChDesc2G is NULL !!!\n");
				}

				if (pChDesc == NULL) {
					MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
							 "pChDesc is NULL !!!\n");
					os_free_mem(TmpFrame);
					return;
				}

				for (i = 0; pChDesc[i].FirstChannel != 0; i++) {
					MakeOutgoingFrame(TmpFrame + TmpLen2,
									  &TmpLen,
									  1,
									  &pChDesc[i].FirstChannel,
									  1,
									  &pChDesc[i].NumOfCh,
									  1,
									  &MaxTxPower,
									  END_OF_ARGS);
					TmpLen2 += TmpLen;
				}
			}
#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef DOT11K_RRM_SUPPORT


#endif /* DOT11K_RRM_SUPPORT */
#ifdef MBO_SUPPORT
			if (IS_MBO_ENABLE(wdev))
				Environment = MBO_AP_USE_GLOBAL_OPERATING_CLASS;
#endif /* MBO_SUPPORT */

			/* need to do the padding bit check, and concatenate it */
			if ((TmpLen2 % 2) == 0) {
				UCHAR TmpLen3 = TmpLen2 + 4;

				MakeOutgoingFrame(pBeaconFrame+FrameLen, &TmpLen,
					1, &CountryIe,
					1, &TmpLen3,
					1, &pAd->CommonCfg.CountryCode[0],
					1, &pAd->CommonCfg.CountryCode[1],
					1, &Environment,
					TmpLen2+1, TmpFrame,
								  END_OF_ARGS);
			} else {
				UCHAR TmpLen3 = TmpLen2 + 3;

				MakeOutgoingFrame(pBeaconFrame + FrameLen,
								  &TmpLen,
					1, &CountryIe,
					1, &TmpLen3,
					1, &pAd->CommonCfg.CountryCode[0],
					1, &pAd->CommonCfg.CountryCode[1],
					1, &Environment,
					TmpLen2, TmpFrame,
								  END_OF_ARGS);
			}

			FrameLen += TmpLen;
			os_free_mem(TmpFrame);
		} else
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
					 "Allocate memory fail!!!\n");
	}

	*pFrameLen = FrameLen;
}

VOID MakeChReportIe(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	ULONG FrameLen = *pFrameLen;
	USHORT PhyMode = wdev->PhyMode;
#ifdef DOT11K_RRM_SUPPORT
	/* UCHAR i; */
#else
	UCHAR APChannelReportIe = IE_AP_CHANNEL_REPORT;
	ULONG TmpLen;
#endif
#ifdef DOT11K_RRM_SUPPORT
	InsertChannelRepIE(pAd, pBeaconFrame + FrameLen, &FrameLen,
					   (RTMP_STRING *)pAd->CommonCfg.CountryCode,
					   get_regulatory_class(pAd, wdev->channel, wdev->PhyMode, wdev),
					   NULL, PhyMode, wdev->func_idx);
#else
	{
		/*
		    802.11n D2.0 Annex J, USA regulatory
			class 32, channel set 1~7
			class 33, channel set 5-11
		*/
		UCHAR rclass32[] = {32, 1, 2, 3, 4, 5, 6, 7};
		UCHAR rclass33[] = {33, 5, 6, 7, 8, 9, 10, 11};
		UCHAR rclasslen = 8; /*sizeof(rclass32); */

		if (PhyMode == (WMODE_B | WMODE_G | WMODE_GN)) {
			MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
							  1,                    &APChannelReportIe,
							  1,                    &rclasslen,
							  rclasslen,            rclass32,
							  1,                    &APChannelReportIe,
							  1,                    &rclasslen,
							  rclasslen,            rclass33,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}
#endif
	*pFrameLen = FrameLen;
}

VOID MakeExtSuppRateIe(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	struct legacy_rate *rate = &wdev->rate.legacy_rate;

	*pFrameLen += build_support_ext_rate_ie(wdev, rate->sup_rate_len,
		rate->ext_rate, rate->ext_rate_len, pBeaconFrame + *pFrameLen);
}

#ifdef HOSTAPD_OWE_SUPPORT
VOID MakeTransIe(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _BSS_STRUCT *pMbss, ULONG *pFrameLen, UCHAR *pFrame)
{
	ULONG FrameLen = *pFrameLen;

	if (pMbss->TRANSIE_Len) {
		ULONG TmpLen;

		MakeOutgoingFrame(pFrame+FrameLen, &TmpLen,
			pMbss->TRANSIE_Len, pMbss->TRANS_IE, END_OF_ARGS);
		FrameLen += TmpLen;
	}

	*pFrameLen = FrameLen;
}
#endif /* HOSTAPD_OWE_SUPPORT */

VOID MakePwrConstraintIe(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	COMMON_CONFIG *pComCfg = &pAd->CommonCfg;
	ULONG FrameLen = *pFrameLen;
	ULONG TmpLen = 0;
	USHORT PhyMode = wdev->PhyMode;

	/*
		Only APs that comply with 802.11h or 802.11k are required to include
		the Power Constraint element (IE=32) and
		the TPC Report element (IE=35) and
		the VHT Transmit Power Envelope element (IE=195)
		in beacon frames and probe response frames
	*/
	if ((WMODE_CAP_5G(PhyMode) && pComCfg->bIEEE80211H == TRUE)
#ifdef DOT11K_RRM_SUPPORT
		|| IS_RRM_ENABLE(wdev)
#endif /* DOT11K_RRM_SUPPORT */
	   ) {
		UINT8 PwrConstraintIE = IE_POWER_CONSTRAINT;
		UINT8 PwrConstraintLen = 1;
		UINT8 PwrConstraint = pComCfg->PwrConstraint;
		/* prepare power constraint IE */
		MakeOutgoingFrame(pBeaconFrame + FrameLen,    &TmpLen,
						  1,                          &PwrConstraintIE,
						  1,                          &PwrConstraintLen,
						  1,                          &PwrConstraint,
						  END_OF_ARGS);
		FrameLen += TmpLen;
		/* prepare TPC Report IE */
		InsertTpcReportIE(pAd,
				  pBeaconFrame + FrameLen,
				  &FrameLen,
				  GetMaxTxPwr(pAd),
				  0);

		if (WMODE_CAP_AX(PhyMode)
			|| WMODE_CAP_AC(PhyMode)
			|| WMODE_CAP_BE(PhyMode)) {
			const UINT8 txpwr_env_ie = IE_TPE;
			UINT8 ie_len;
			struct _tx_pwr_env_ie txpwr_env;
			UCHAR ucBand = wlan_config_get_ch_band(wdev);

			ie_len = build_txpwr_envelope(pAd, wdev, &txpwr_env);
			MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
						1, &txpwr_env_ie,
						1, &ie_len,
						ie_len, &txpwr_env,
						END_OF_ARGS);
			FrameLen += TmpLen;

			if ((ucBand == CMD_CH_BAND_6G)
#ifdef CONFIG_6G_AFC_SUPPORT
				&& (pAd->CommonCfg.AfcDeviceType != AFC_STANDARD_POWER_DEVICE)
#endif
				) {
				ie_len = build_txpwr_envelope_eirp_psd(pAd, wdev, &txpwr_env,
					TX_PWR_INTERPRET_REG_CLIENT_EIRP_PSD, TX_PWR_CATEGORY_SUBORDINATE_DEV);
				MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
							1, &txpwr_env_ie,
							1, &ie_len,
							ie_len, &txpwr_env,
							END_OF_ARGS);
				FrameLen += TmpLen;
			}
#ifdef CONFIG_6G_AFC_SUPPORT
			if (is_afc_in_run_state((struct _RTMP_ADAPTER *)wdev->sys_handle)) {
				if (pAd->CommonCfg.AfcDeviceType == AFC_LP_SP_COEX) {
					ie_len = build_txpwr_envelope_eirp_psd(pAd, wdev, &txpwr_env,
						TX_PWR_INTERPRET_ADDITIONAL_REG_CLIENT_EIRP_PSD, TX_PWR_CATEGORY_DEFAULT);
					MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
								1, &txpwr_env_ie,
								1, &ie_len,
								ie_len, &txpwr_env,
								END_OF_ARGS);
					FrameLen += TmpLen;
				}

				ie_len = build_txpwr_envelope_eirp(pAd, wdev, &txpwr_env);
				MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
							1, &txpwr_env_ie,
							1, &ie_len,
							ie_len, &txpwr_env,
							END_OF_ARGS);
				FrameLen += TmpLen;
			}
#endif
		}
	}

	*pFrameLen = FrameLen;
}

VOID ComposeBcnPktTail(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen,
	UCHAR *pBeaconFrame, BOOLEAN bcn)
{
	ULONG FrameLen = *pFrameLen;
	struct _build_ie_info vht_ie_info;
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = NULL;
	UCHAR apidx = 0;
	/* BOOLEAN HotSpotEnable = FALSE; */
#ifdef A_BAND_SUPPORT
	struct DOT11_H *pDot11h = wdev->pDot11_H;
#endif
#endif
	USHORT PhyMode = wdev->PhyMode;
#ifdef RT_CFG80211_SUPPORT
#ifdef MBO_SUPPORT
	P_MBO_CTRL pMboCtrl = &wdev->MboCtrl;
#endif /* MBO_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */

#ifdef CONFIG_AP_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		pMbss = wdev->func_dev;
		apidx = wdev->func_idx;
	}

	/* fix klockwork issue */
	if (pMbss == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				 "=unexpected pMbss NULL, please check\n");
		return;
	}

#endif /* CONFIG_AP_SUPPORT */
	vht_ie_info.frame_subtype = SUBTYPE_BEACON;
	vht_ie_info.channel = wdev->channel;
	vht_ie_info.phy_mode = PhyMode;
	vht_ie_info.wdev = wdev;

	MakeCountryIe(pAd, wdev, &FrameLen, pBeaconFrame);
#ifdef CONFIG_AP_SUPPORT
	MakePwrConstraintIe(pAd, wdev, &FrameLen, pBeaconFrame);
	if (pDot11h == NULL)
		return;

	if (bcn) {
		 /* init ie_offset */
		wdev->bcn_buf.CsaIELocationInBeacon = 0;
#ifdef DOT11_EHT_BE
		wdev->bcn_buf.csa.ml_csa_ie_offset = 0;
		wdev->bcn_buf.csa.ml_ie_offset = 0;

		wdev->bcn_buf.reconf_info.num_offset = 0;
#endif
	}

	MakeCSAIe(pAd, wdev, bcn, &FrameLen, pBeaconFrame);

#ifdef DOT11V_MBSSID_SUPPORT
	/* mbssid ie */
	mbss_11v_tim_ie_handle(pAd, wdev, pMbss, pBeaconFrame, &FrameLen);
#endif

#ifdef DOT11K_RRM_SUPPORT
	if (IS_RRM_ENABLE(wdev))
		RRM_InsertRRMEnCapIE(pAd, wdev, pBeaconFrame + FrameLen, &FrameLen, apidx);

#endif /* DOT11K_RRM_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	MakeChReportIe(pAd, wdev, &FrameLen, pBeaconFrame);
#ifdef DOT11R_FT_SUPPORT
	MakeFtMdIe(pAd, wdev, apidx, &FrameLen, pBeaconFrame);
#endif /* DOT11R_FT_SUPPORT */

	/* Update ERP */
	if ((wdev->rate.legacy_rate.ext_rate_len) && (PhyMode != WMODE_B)) {
		if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G)
			MakeErpIE(pAd, wdev, &FrameLen, pBeaconFrame);
	}

	MakeExtSuppRateIe(pAd, wdev, &FrameLen, pBeaconFrame);
	FrameLen += build_rsn_ie(pAd, wdev, (UCHAR *)(pBeaconFrame + FrameLen));
	ComposeWPSIE(pAd, wdev, &FrameLen, pBeaconFrame);

#ifdef HOSTAPD_OWE_SUPPORT
	MakeTransIe(pAd, wdev, pMbss, &FrameLen, pBeaconFrame);
#endif

#ifdef AP_QLOAD_SUPPORT
	MakeQBSSLoadIe(pAd, wdev, pMbss, apidx, &FrameLen, pBeaconFrame);
#endif /* AP_QLOAD_SUPPORT */
#if defined(CONFIG_HOTSPOT)

	if (pMbss->GASCtrl.b11U_enable)
		MakeHotSpotIE(wdev, &FrameLen, pBeaconFrame);

#endif /* CONFIG_HOTSPOT */

#ifdef CONFIG_DOT11U_INTERWORKING
	MakeRoamingConsortiumIe(pAd, wdev, &FrameLen, pBeaconFrame);
#endif /* CONFIG_DOT11U_INTERWORKING */

#ifdef DOT11_N_SUPPORT
	/* step 5. Update HT. Since some fields might change in the same BSS. */
	if (WMODE_CAP_N(PhyMode) && (wdev->DesiredHtPhyInfo.bHtEnable)) {
		MakeHTIe(pAd, wdev, &FrameLen, pBeaconFrame);
#ifdef DOT11_VHT_AC
		vht_ie_info.frame_buf = (UCHAR *)(pBeaconFrame + FrameLen);
		FrameLen += build_vht_ies(pAd, &vht_ie_info);
#endif /* DOT11_VHT_AC */
	}
#endif /* DOT11_N_SUPPORT */

#ifdef QOS_R2
	MakeWfaCapaIe(pAd, wdev, apidx, &FrameLen, pBeaconFrame);
#endif

#ifdef CONFIG_AP_SUPPORT
	/* 7.3.2.27 Extended Capabilities IE */
	vht_ie_info.frame_buf = (UCHAR *)(pBeaconFrame + FrameLen);
	FrameLen += build_extended_cap_ie(pAd, &vht_ie_info);

#ifdef DOT11K_RRM_SUPPORT

	if (IS_RRM_ENABLE(wdev)) {
#ifdef QUIET_SUPPORT
		PRRM_QUIET_CB pQuietCB = &pMbss->wdev.RrmCfg.QuietCB;

		RRM_InsertQuietIE(pAd, pBeaconFrame + FrameLen, &FrameLen,
						  pQuietCB->QuietCnt, pQuietCB->QuietPeriod,
						  pQuietCB->QuietDuration, pQuietCB->QuietOffset);
#endif
#ifndef APPLE_11K_IOT
		/* Insert BSS AC Access Delay IE. */
		RRM_InsertBssACDelayIE(pAd, pBeaconFrame + FrameLen, &FrameLen);
		/* Insert BSS Available Access Capacity IE. */
		RRM_InsertBssAvailableACIE(pAd, pBeaconFrame + FrameLen, &FrameLen);
#endif /* !APPLE_11K_IOT */
	}
#endif /* DOT11K_RRM_SUPPORT */
#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(wdev->PhyMode) && wdev->DesiredHtPhyInfo.bHtEnable)
		FrameLen += add_beacon_he_ies(wdev, pBeaconFrame, FrameLen);
#endif /* DOT11_HE_AX */

#if defined(MBO_SUPPORT) || defined(OCE_SUPPORT)
	if (IS_MBO_ENABLE(wdev) || IS_OCE_ENABLE(wdev)) {
#ifdef RT_CFG80211_SUPPORT
		if (pMboCtrl->MboIELen > 0 && pMboCtrl->MboIELen <= MBO_IE_MAX_LEN)
			MakeMboIE(pAd, wdev, &FrameLen, pBeaconFrame);
		else
#endif /* RT_CFG80211_SUPPORT */
			MakeMboOceIE(pAd, wdev, NULL, pBeaconFrame+FrameLen, &FrameLen, MBO_FRAME_TYPE_BEACON);
	}
#endif /* OCE_SUPPORT MBO_SUPPORT */

#endif /*CONFIG_AP_SUPPORT*/

#ifdef CONFIG_MAP_SUPPORT
#if defined(WAPP_SUPPORT)
	MakeMAPIe(pAd, wdev, &FrameLen, pBeaconFrame);
#endif /*WAPP_SUPPORT*/
#endif

#ifdef DPP_R2_SUPPORT
	MakeDPPVendorIe(pAd, wdev, &FrameLen, pBeaconFrame);
#endif
#ifdef RT_CFG80211_SUPPORT
#endif
/*Vendor IE should be final IE to be added, so we can determine the maximum length of Beacon*/
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	RTMP_SPIN_LOCK(&pAd->ApCfg.MBSSID[apidx].ap_vendor_ie.vendor_ie_lock);

	if (pAd->ApCfg.MBSSID[apidx].ap_vendor_ie.pointer != NULL) {
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
		ULONG TmpMaxBeaconLen;

#ifdef BCN_V2_SUPPORT
		if (apidx < cap->max_v2_bcn_num)
			TmpMaxBeaconLen = 1520 - cap->tx_hw_hdr_len;/*FW limitation*/
		else
			TmpMaxBeaconLen = 512 - cap->tx_hw_hdr_len;
#else
		TmpMaxBeaconLen = cap->BcnMaxLength - cap->tx_hw_hdr_len;
#endif

		if (FrameLen + pAd->ApCfg.MBSSID[apidx].ap_vendor_ie.length > TmpMaxBeaconLen)
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				"BCN is too long, can't add vendor ie!\n");
		else {

			ULONG TmpLen;

			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
				"BCN add vendor ie\n");
			MakeOutgoingFrame(pBeaconFrame + FrameLen,
					  &TmpLen,
					  pAd->ApCfg.MBSSID[apidx].ap_vendor_ie.length,
					  pAd->ApCfg.MBSSID[apidx].ap_vendor_ie.pointer,
					  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}
	RTMP_SPIN_UNLOCK(&pAd->ApCfg.MBSSID[apidx].ap_vendor_ie.vendor_ie_lock);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
/*Reduced neighbor report IE should be final IE to be added, so we can determine the maximum length of Beacon*/
#ifdef OCE_FILS_SUPPORT
		vht_ie_info.frame_buf = (UCHAR *)(pBeaconFrame + FrameLen);
		vht_ie_info.pos = FrameLen;
		FrameLen += oce_build_ies(pAd, &vht_ie_info, TRUE);
#endif

	FrameLen +=  build_rsnxe_ie(wdev, &wdev->SecConfig,
				    (UCHAR *)pBeaconFrame + FrameLen);
	/* rnr ie */
	FrameLen += bss_mngr_query_rnr_ie(wdev, (u8 *)(pBeaconFrame + FrameLen));

	/* ML IE */
#ifdef DOT11_EHT_BE
	if (WMODE_CAP_BE(wdev->PhyMode) &&
			wdev->DesiredHtPhyInfo.bHtEnable) {

		if (PD_CEHCK_MLO_V1_ENABLE(pAd->physical_dev)) {
			UCHAR mtk_vendor_ie_tlv1_type = BIT(MEDIATEK_TLV1_TYPE4) | BIT(MEDIATEK_TLV1_TYPE2);

			if (bcn)
				wdev->bcn_buf.csa.ml_ie_offset = FrameLen;
			/* add MTK MLO V1 IE */
			FrameLen += build_mtk_tlv1_vendor_ie(pAd, wdev, (pBeaconFrame + FrameLen), VIE_BEACON, mtk_vendor_ie_tlv1_type);
		} else {
			if (bcn)
				wdev->bcn_buf.csa.ml_ie_offset = FrameLen;
			FrameLen += eht_add_beacon_ies(wdev, pBeaconFrame, FrameLen);
			FrameLen += build_mtk_tlv1_vendor_ie(pAd, wdev, (pBeaconFrame + FrameLen),
				VIE_BEACON, BIT(MEDIATEK_TLV1_TYPE4));
		}

		if (bcn) {
			wdev->bcn_buf.csa.ml_csa_ie_offset = 0;

			if (wdev->bcn_buf.csa.csa_ie_offset_in_ml) {
				wdev->bcn_buf.csa.ml_csa_ie_offset = wdev->bcn_buf.csa.ml_ie_offset
													+ wdev->bcn_buf.csa.csa_ie_offset_in_ml;
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
					"ml_ie_offset(%d), csa_ie_offset_in_ml(%d), csa_bss_idx(%d).\n",
					wdev->bcn_buf.csa.ml_ie_offset,
					wdev->bcn_buf.csa.csa_ie_offset_in_ml,
					wdev->bcn_buf.csa.csa_bss_idx);
			}
		}
	}
#endif /* DOT11_EHT_BE */

#ifdef CFG_RSNO_SUPPORT
	/* rsn override ie */
	FrameLen += build_rsn_override_ie(
		pAd, wdev, NULL, SUBTYPE_BEACON, (UCHAR *)(pBeaconFrame + FrameLen));
#endif /* CFG_RSNO_SUPPORT */

	/* add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back */
	FrameLen += build_vendor_ie(pAd, wdev, (pBeaconFrame + FrameLen), VIE_BEACON);

	if (wdev->bWmmCapable) {
		vht_ie_info.frame_buf = (UCHAR *)(pBeaconFrame + FrameLen);
		FrameLen += build_wmm_cap_ie(pAd, &vht_ie_info);
	}
#ifdef MLR_SUPPORT
	if (pAd->CommonCfg.bMLREnable)
		FrameLen += build_mtk_tlv1_vendor_ie(pAd, wdev, (pBeaconFrame + FrameLen), VIE_BEACON, BIT(MEDIATEK_TLV1_TYPE1));
#endif



#ifdef BCN_PROTECTION_SUPPORT
	FrameLen +=  build_bcn_mmie(&wdev->SecConfig.bcn_prot_cfg,
	(UCHAR *)pBeaconFrame + FrameLen);/*mmie should be the last of the beacon*/
#endif

	*pFrameLen = FrameLen;
}
VOID updateBeaconRoutineCase(RTMP_ADAPTER *pAd, BOOLEAN UpdateAfterTim)
{
	INT     i;
	struct wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN FlgQloadIsAlarmIssued = FALSE;
	UCHAR cfg_ht_bw;
	UCHAR cfg_ext_cha;
	UCHAR op_ht_bw;
	UCHAR op_ext_cha;
#ifdef MBSS_DTIM_SUPPORT
	UINT bssidx;
	UCHAR minDtimCount = pAd->ApCfg.MBSSID[0].DtimCount;
#endif

	wdev = get_default_wdev(pAd);
	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"wdev = NULL\n");
		return;
	}

	cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	cfg_ext_cha = wlan_config_get_ext_cha(wdev);
	op_ht_bw = wlan_operate_get_ht_bw(wdev);
	op_ext_cha = wlan_operate_get_ext_cha(wdev);

#ifdef MBSS_DTIM_SUPPORT
	for (bssidx = 0; bssidx < pAd->ApCfg.BssidNum; bssidx++) {
		if (pAd->ApCfg.MBSSID[bssidx].DtimCount == 0)
			pAd->ApCfg.MBSSID[bssidx].DtimCount = pAd->ApCfg.MBSSID[bssidx].DtimPeriod - 1;
		else
			pAd->ApCfg.MBSSID[bssidx].DtimCount -= 1;

		if (pAd->ApCfg.MBSSID[bssidx].DtimCount < minDtimCount)
			minDtimCount = pAd->ApCfg.MBSSID[bssidx].DtimCount;
	}
#else
	if (pAd->ApCfg.DtimCount == 0)
		pAd->ApCfg.DtimCount = pAd->ApCfg.DtimPeriod - 1;
	else
		pAd->ApCfg.DtimCount -= 1;
#endif

#ifdef AP_QLOAD_SUPPORT
	FlgQloadIsAlarmIssued = QBSS_LoadIsAlarmIssued(pAd);
#endif /* AP_QLOAD_SUPPORT */

	if (
#ifdef MBSS_DTIM_SUPPORT
		(minDtimCount == 0)
#else
		(pAd->ApCfg.DtimCount == 0)
#endif
		&& (((pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_SYNC) &&
		  (pAd->CommonCfg.bForty_Mhz_Intolerant == FALSE)) ||
		 (FlgQloadIsAlarmIssued == TRUE)) &&
		 (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G)) {
		UCHAR   prevBW, prevExtChOffset;

		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				 "DTIM Period reached, BSS20WidthReq=%d, Intolerant40=%d!\n",
				  pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq,
				  pAd->CommonCfg.LastBSSCoexist2040.field.Intolerant40);
		pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_INFO_SYNC);
		prevBW = wlan_operate_get_ht_bw(wdev);
		prevExtChOffset = wlan_operate_get_ext_cha(wdev);

		if (pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq ||
			pAd->CommonCfg.LastBSSCoexist2040.field.Intolerant40 ||
			(pAd->MacTab->fAnyStaFortyIntolerant == TRUE) ||
			(FlgQloadIsAlarmIssued == TRUE)) {
			wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
		} else{
			wlan_operate_set_ht_bw(wdev, cfg_ht_bw, cfg_ext_cha);
		}
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				 "\tNow RecomWidth=%d, ExtChanOffset=%d, prevBW=%d, prevExtOffset=%d\n",
				  wlan_operate_get_ht_bw(wdev),
				  wlan_operate_get_ext_cha(wdev),
				  prevBW, prevExtChOffset);
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_NOTIFY;
	}

#endif /* CONFIG_AP_SUPPORT */

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev != NULL)
			MakeBeacon(pAd, wdev, UpdateAfterTim);
	}
}

VOID UpdateBeaconHandler(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BCN_UPDATE_REASON reason)
{
	struct DOT11_H *pDot11h = NULL;
	BOOLEAN is_in_interrupt = FALSE;

	is_in_interrupt = in_interrupt() ? TRUE:FALSE;
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR, "wdev = NULL, (caller:%pS)\n",
				 OS_TRACE);
		return;
	}


	/* replace as 11vt for 11vnt mbss */
#if defined(CONFIG_AP_SUPPORT) && defined(DOT11V_MBSSID_SUPPORT)
	if (BCN_REASON_EQUAL(reason, BCN_UPDATE_IE_CHG))
	{
		BOOLEAN bMakeBeacon;
		struct wifi_dev *orig_wdev;

		orig_wdev = wdev;
		if ((wdev->func_idx < 0) || (wdev->func_idx >= MAX_BEACON_NUM)) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				 "invalid wdev->func_idx(%d), (caller:%pS)!!\n",
				 wdev->func_idx, OS_TRACE);
			return;
		}
		wdev = mbss_11v_get_tx_wdev(pAd, wdev, &pAd->ApCfg.MBSSID[wdev->func_idx], &bMakeBeacon);
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			"11vT wdev = %d,11vNT wdev = %d\n",
			wdev->wdev_idx, orig_wdev->wdev_idx);
	}
#endif

#ifdef CONFIG_6G_AFC_SUPPORT
	if (WMODE_CAP_6G(wdev->PhyMode) && BCN_REASON_EQUAL(reason, BCN_UPDATE_INIT)) {
		if (!afc_beacon_init_handler(pAd, wdev))
			return;
		}
#endif /*CONFIG_6G_AFC_SUPPORT*/

#ifdef DFS_SLAVE_SUPPORT
	if (SLAVE_BEACON_STOPPED(pAd)
		&& (!BCN_REASON_EQUAL(reason, BCN_UPDATE_DISABLE_TX))) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"[DFS-SLAVE] %s(): beaconing off Update reason: %d\n", __func__, reason);
		return;
	}
#endif /* DFS_SLAVE_SUPPORT */

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			 "wdev(%d) devname:%s, reason = %d (caller:%pS)\n",
			 wdev->wdev_idx, RtmpOsGetNetDevName(pAd->net_dev), reason, OS_TRACE);

	if (!WDEV_WITH_BCN_ABILITY(wdev)) {
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				 "wdev(%d) beacon needless (type:%d, caller:%pS)\n",
				 wdev->wdev_idx, wdev->wdev_type, OS_TRACE);
		goto end;
	}

	if (WDEV_BSS_STATE(wdev) < BSS_READY) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				 "wdev(%d) bss not ready (state:%d, caller:%pS)!!\n",
				 wdev->wdev_idx, WDEV_BSS_STATE(wdev), OS_TRACE);
		goto end;
	}

	pDot11h = wdev->pDot11_H;
	if (pDot11h) {
		/* ignore non-CSA beacon update during CSA counting period */
		if ((pDot11h->csa_ap_bitmap != 0
#ifdef DOT11_EHT_BE
			|| (bss_mngr_is_in_mlo_csa(wdev) && (!BCN_REASON_EQUAL(reason, BCN_UPDATE_INIT)))
#endif
			)
			&& (!BCN_REASON_EQUAL(reason, BCN_UPDATE_CSA))
		) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
					 "wdev(%d) CSA counting, ignore!! (caller:%pS, reason:%d)!!\n",
					 wdev->wdev_idx, OS_TRACE, reason);
			goto end;
		}
	}

	if (BCN_REASON_EQUAL(reason, BCN_UPDATE_INIT)) {
		UCHAR bandidx = hc_get_hw_band_idx(pAd);
		PBCN_CHECK_INFO_STRUC pBcnCheckInfo = &pAd->BcnCheckInfo;

		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO, "BCN_UPDATE_INIT, OmacIdx = %x (%s)\n",
				 wdev->OmacIdx, RtmpOsGetNetDevName(pAd->net_dev));

		if (bcn_buf_init(pAd, wdev) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				"bcn_buf_init fail!!\n");
			goto end;
		}

		wdev->bcn_buf.bBcnSntReq = TRUE;

		/* record beacon active PeriodicRound */
		if (pBcnCheckInfo->BcnInitedRnd == 0) {
			pBcnCheckInfo->BcnInitedRnd = pAd->Mlme.PeriodicRound;
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
					 "\tBand%d BcnInitedRnd = %ld\n", bandidx, pBcnCheckInfo->BcnInitedRnd);
		}
	} else if (BCN_REASON_EQUAL(reason, BCN_UPDATE_ENABLE_TX)) {
		UCHAR bandidx = hc_get_hw_band_idx(pAd);
		PBCN_CHECK_INFO_STRUC pBcnCheckInfo = &pAd->BcnCheckInfo;

		if (wdev->bcn_buf.stop_tx) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
					 "wdev(%d) No_bcn is enabled, ignore!! (caller:%pS, reason:%d)!!\n",
					 wdev->wdev_idx, OS_TRACE, reason);
			goto end;
		}
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
				"BCN_UPDATE_ENABLE_TX, OmacIdx = %x (%s)\n",
				wdev->OmacIdx, RtmpOsGetNetDevName(pAd->net_dev));

		/* record beacon active PeriodicRound */
		pBcnCheckInfo->BcnInitedRnd = pAd->Mlme.PeriodicRound;
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
			"\tBand%d BcnInitedRnd = %ld\n", bandidx, pBcnCheckInfo->BcnInitedRnd);
	}

#ifdef CONVERTER_MODE_SWITCH_SUPPORT
	{
		BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];

		if (pMbss->APStartPseduState != AP_STATE_ALWAYS_START_AP_DEFAULT) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"StopBeaconing:\n");
			return;
		}
	}
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */

	/* Get bpcc lock before legacy bcn update, the purpose is to prevent race
		condition for bcn content update. If reason is BCN_UPDATE_CRIT_UPD,it
		should get lock before calling UpdateBeaconHandler*/
	if (BCN_REASON_EQUAL(reason, BCN_UPDATE_IE_CHG) && (wdev->BcnBPCC_Init)
		&& !IS_BCN_CRIT_UPD(reason)) {
		bcn_bpcc_ct_switch(pAd, wdev, BCN_BPCC_LEGACY_BCN, reason);
		return;
	}

	HW_BEACON_UPDATE(pAd, wdev, (UCHAR)reason);

end:
	return;
}

static BOOLEAN _bcn_bpcc_get_value_by_key(RTMP_STRING *key, RTMP_STRING *dest, UINT32 dest_size, RTMP_STRING *buffer)
{
	RTMP_STRING temp_buf1[100] = {0}, temp_buf2[100] = {0};
	RTMP_STRING *start_ptr, *end_ptr;
	RTMP_STRING *ptr;
	INT len;

	// tmpbuf1 = "key="
	strlcpy(temp_buf1, key, strlen(key) + 1);
	strncat(temp_buf1, "=", strlen("="));

	// search buffer, get start_ptr
	start_ptr = rtstrstr(buffer, temp_buf1);
	if (!start_ptr) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_DEBUG,
			"Error: start_ptr is null\n");
		return FALSE;
	}

	// find next , in buffer and get end_ptr
	end_ptr = rtstrstr(start_ptr, ",");
	if (!end_ptr)
		end_ptr = start_ptr + strlen(start_ptr);

	if (end_ptr < start_ptr) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"Error: end_ptr < start_ptr\n");
		return FALSE;
	}

	if (end_ptr - start_ptr >= 100) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"Error: ptr is too long\n");
		return FALSE;
	}

	// get tmpbuf2
	NdisMoveMemory(temp_buf2, start_ptr, end_ptr - start_ptr);
	temp_buf2[end_ptr - start_ptr] = '\0';

	// find = and get the string after =
	start_ptr = rtstrstr(temp_buf2, "=");
	if (!start_ptr)
		return FALSE;

	// remove =
	ptr = (start_ptr + 1);

	// get result
	len = strlen(start_ptr);
	NdisZeroMemory(dest, dest_size);
	strlcpy(dest, ptr, dest_size);

	return TRUE;
}

static VOID _bcn_bpcc_info_dump(struct wifi_dev *wdev, BOOLEAN detail, BOOLEAN reset, BOOLEAN ignore_hist)
{
	UINT8 i;
	UINT cmdq_size = 0;
	RTMP_ADAPTER *pAd;

	if (!wdev || !wdev->BcnBPCC_Init)
		return;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	if (!pAd)
		return;

	NdisAcquireSpinLock(&pAd->BPCC_CmdQLock);
	cmdq_size = pAd->BPCC_CmdQ.size;
	NdisReleaseSpinLock(&pAd->BPCC_CmdQLock);

	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	if (detail == FALSE) {
		UINT32 BcnBPCC_legacy_fail_cnt_INT_total = 0;
		UINT32 BcnBPCC_legacy_fail_cnt_CT_total = 0;
		UINT32 BcnBPCC_bssmngr_fail_cnt_INT_total = 0;
		UINT32 BcnBPCC_bssmngr_fail_cnt_CT_total = 0;

		for (i = 0; i < BCN_BPCC_MAX; i++) {
			if ((wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_INT[i] != 0) ||
				(wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_CT[i] != 0)) {
				if (i == BCN_BPCC_LEGACY_BCN) {
					BcnBPCC_legacy_fail_cnt_INT_total += wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_INT[i];
					BcnBPCC_legacy_fail_cnt_CT_total += wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_CT[i];
				} else {
					BcnBPCC_bssmngr_fail_cnt_INT_total += wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_INT[i];
					BcnBPCC_bssmngr_fail_cnt_CT_total += wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_CT[i];
				}
			}
		}

		MTWF_PRINT("[%s]I/P/L/D(%d/%d/%d/%d),Legacy[%u(%u|%u)/%lu/%lu/%lu/%lu] MLD[%u(%u|%u)/%lu/%lu/%lu/%lu] TO[%u],PR[%lu/%lu/%lu/%lu],hwQ[%u],bpccQ[%u], %x\n",
			RtmpOsGetNetDevName(wdev->if_dev),
			wdev->BcnBPCC_Init, !wdev->BcnBPCC_protect_disable, wdev->BcnBPCC_log_on, wdev->BcnBPCC_protect_dynctrl_cnt,
			wdev->BcnBPCC_op_lock_by_legacy_fail_cnt, BcnBPCC_legacy_fail_cnt_INT_total, BcnBPCC_legacy_fail_cnt_CT_total,
			wdev->BcnBPCC_op_lock_get_fail_legacy_max, wdev->BcnBPCC_op_lock_get_fail_legacy_avg,
			wdev->BcnBPCC_op_lock_get_legacy_max, wdev->BcnBPCC_op_lock_get_legacy_avg,
			wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt,  BcnBPCC_bssmngr_fail_cnt_INT_total, BcnBPCC_bssmngr_fail_cnt_CT_total,
			wdev->BcnBPCC_op_lock_get_fail_mld_max, wdev->BcnBPCC_op_lock_get_fail_mld_avg,
			wdev->BcnBPCC_op_lock_get_mld_max, wdev->BcnBPCC_op_lock_get_mld_avg,
			wdev->BcnBPCC_TO_cnt,
			wdev->BcnBPCC_op_lock_max, wdev->BcnBPCC_op_lock_avg,
			wdev->BcnBPCC_op_lock_mld_max, wdev->BcnBPCC_op_lock_mld_avg,
			wdev->BcnBPCC_hwcmd_max, cmdq_size,
			wdev->BcnBPCC_op_lock_refine_NG_per_IE_bitmap);
	} else {
		MTWF_PRINT("\x1b[42m [%s] BcnBPCC Information\x1b[m\n\r", RtmpOsGetNetDevName(wdev->if_dev));
		MTWF_PRINT("BcnBPCC(Init/protect/log/dyn_pro_cnt)=(%d/%d/%d/%d)\n",
			wdev->BcnBPCC_Init, !wdev->BcnBPCC_protect_disable,
			wdev->BcnBPCC_log_on, wdev->BcnBPCC_protect_dynctrl_cnt);
		if (wdev->BcnBPCC_op_lock_flag) {
			MTWF_PRINT("op lock get by caller:%s\n",
				wdev->dbg_BcnBPCC_op_lock_caller);
		}
		MTWF_PRINT("op lock stat, Legacy[%u/%lu/%lu/%lu/%lu] MLD[=%u/%lu/%lu/%lu/%lu]\n",
			wdev->BcnBPCC_op_lock_by_legacy_fail_cnt,
			wdev->BcnBPCC_op_lock_get_fail_legacy_max, wdev->BcnBPCC_op_lock_get_fail_legacy_avg,
			wdev->BcnBPCC_op_lock_get_legacy_max, wdev->BcnBPCC_op_lock_get_legacy_avg,
			wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt,
			wdev->BcnBPCC_op_lock_get_fail_mld_max, wdev->BcnBPCC_op_lock_get_fail_mld_avg,
			wdev->BcnBPCC_op_lock_get_mld_max, wdev->BcnBPCC_op_lock_get_mld_avg);

		MTWF_PRINT("TO[TO=%u], hwQ[max=%u],bpccQ[%u] Max=%u sec\n",
			wdev->BcnBPCC_TO_cnt,
			wdev->BcnBPCC_hwcmd_max, cmdq_size,
			bpcc_op_lock_max_time_adjust/OS_HZ);
		MTWF_PRINT("op lock period[max/avg]=[%lu/%lu/%lu/%lu] ms, max TO=%u ms\n",
			wdev->BcnBPCC_op_lock_max,
			wdev->BcnBPCC_op_lock_avg,
			wdev->BcnBPCC_op_lock_mld_max,
			wdev->BcnBPCC_op_lock_mld_avg,
			bpcc_op_lock_max_to_adjust);
		MTWF_PRINT("BcnBPCCIeType_bitmap_hist_idx=%d\n", wdev->BcnBPCCIeType_bitmap_hist_idx);
		if (ignore_hist == FALSE)
			for (i = 0; i < 20; i++) {
				if (wdev->BcnBPCCIeType_bitmap_hist[i] != 0)
					MTWF_PRINT("[%u]wdev->BcnBPCCIeType_bitmap_hist[%d]=0x%x\n",
						wdev->BcnBPCCIeType_bitmap_hist_TT[i] ?
						jiffies_to_msecs(abs(wdev->BcnBPCCIeType_bitmap_hist_TT[i]-wdev->BcnBPCC_Init_TT)):0,
						i, wdev->BcnBPCCIeType_bitmap_hist[i]);
			}
		for (i = 0; i < BCN_BPCC_MAX; i++) {
			if ((wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_CT[i] != 0) ||
				(wdev->BcnBPCC_TO_cnt_per_IE[i] != 0))
				MTWF_PRINT(
				"\x1b[41mie_type=[%u][%s],cnt=%d (INT/CT=%d/%d) TO(%u)\x1b[m\n\r",
					i, BCN_BPCC_IE_TYPE_STR[i], wdev->BcnBPCCIeType_cnt_hist[i],
					wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_INT[i],
					wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_CT[i],
					wdev->BcnBPCC_TO_cnt_per_IE[i]);
			else if (wdev->BcnBPCCIeType_cnt_hist[i] != 0)
				MTWF_PRINT("ie_type=[%u][%s],cnt=%d (INT/CT=%d/%d) TO(%u)\n",
					i, BCN_BPCC_IE_TYPE_STR[i], wdev->BcnBPCCIeType_cnt_hist[i],
					wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_INT[i],
					wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_CT[i],
					wdev->BcnBPCC_TO_cnt_per_IE[i]);
		}
	}
	if (reset == TRUE) {
		wdev->BcnBPCC_protect_dynctrl_cnt = 0;
		wdev->BcnBPCC_op_lock_by_legacy_fail_cnt = 0;
		wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt = 0;
		wdev->BcnBPCC_TO_cnt = 0;
		wdev->BcnBPCC_hwcmd_max = 0;
		wdev->BcnBPCC_op_lock_max = 0;
		wdev->BcnBPCC_op_lock_avg = 0;
		wdev->BcnBPCC_op_lock_mld_max = 0;
		wdev->BcnBPCC_op_lock_mld_avg = 0;
		wdev->BcnBPCC_op_lock_get_mld_max = 0;
		wdev->BcnBPCC_op_lock_get_mld_avg = 0;
		wdev->BcnBPCC_op_lock_get_legacy_max = 0;
		wdev->BcnBPCC_op_lock_get_legacy_avg = 0;
		wdev->BcnBPCC_op_lock_get_fail_mld_max = 0;
		wdev->BcnBPCC_op_lock_get_fail_mld_avg = 0;
		wdev->BcnBPCC_op_lock_get_fail_legacy_max = 0;
		wdev->BcnBPCC_op_lock_get_fail_legacy_avg = 0;
		memset(&wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_INT[0], 0x00, sizeof(wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_INT));
		memset(&wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_CT[0], 0x00, sizeof(wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_CT));
		memset(&wdev->BcnBPCC_TO_cnt_per_IE[0], 0x00, sizeof(wdev->BcnBPCC_TO_cnt_per_IE));
	}
	OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
}

static struct wifi_dev *_bcn_bpcc_available_tx_wdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	if (wdev) {
#if defined(CONFIG_AP_SUPPORT) && defined(DOT11V_MBSSID_SUPPORT)
		struct wifi_dev *orig_wdev = wdev;
		BOOLEAN bMakeBeacon;

		wdev = mbss_11v_get_tx_wdev(pAd, wdev, &pAd->ApCfg.MBSSID[wdev->func_idx], &bMakeBeacon);
		if (wdev && (wdev != orig_wdev)) {
			OS_SEM_LOCK(&wdev->BcnBPCCLock);
			if (wdev->BcnBPCC_log_on)
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_DEBUG,
				"\x1b[41m 11T(%s),11NT(%s),caller:%pS\x1b[m\n\r",
				RtmpOsGetNetDevName(wdev->if_dev),
				RtmpOsGetNetDevName(orig_wdev->if_dev),
				OS_TRACE);
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		}
#endif
	}
	if (wdev && !wdev->BcnBPCC_Init) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
			"\x1b[41m(%s/init=0),caller:%pS\x1b[m\n\r",
			RtmpOsGetNetDevName(wdev->if_dev), OS_TRACE);
		return NULL;
	}
	return wdev;
}

VOID bcn_bpcc_mld_id_log_sync(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 mld_id, BOOLEAN log)
{
	wdev = _bcn_bpcc_available_tx_wdev(pAd, wdev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
				"\x1b[41mcaller:%pS\x1b[m\n\r", OS_TRACE);
		return;
	}

	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	wdev->BcnBPCC_mld_grp_id = mld_id;
	if (mld_id <= 64)
		wdev->BcnBPCC_MLD_log_flag[wdev->BcnBPCC_mld_grp_id] = log;

	OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
}

static INT _bcn_bpcc_mld_log(struct wifi_dev *wdev, BOOLEAN log)
{
	struct query_mld_ap_basic bss_mld_info_basic;

	if (bss_mngr_query_mld_ap_basic(wdev, &bss_mld_info_basic) != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("Invalid query\n");
		return NDIS_STATUS_FAILURE;
	}
	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	if (bss_mld_info_basic.mld_grp_idx <= 64) {
		wdev->BcnBPCC_MLD_log_flag[bss_mld_info_basic.mld_grp_idx] = log;
		wdev->BcnBPCC_mld_grp_id = bss_mld_info_basic.mld_grp_idx;
		MTWF_PRINT("wdev->BcnBPCC_mld_grp_id=%d\n", wdev->BcnBPCC_mld_grp_id);
	} else {
		wdev->BcnBPCC_mld_grp_id = 0;
		MTWF_PRINT("wdev->BcnBPCC_mld_grp_id=%d\n", wdev->BcnBPCC_mld_grp_id);
	}
	OS_SEM_UNLOCK(&wdev->BcnBPCCLock);

	bss_mngr_mld_critical_update_op_lock(wdev,
		(log == TRUE) ? BMGR_BPCC_OP_MLD_ID_LOG_ON : BMGR_BPCC_OP_MLD_ID_LOG_OFF);

	return NDIS_STATUS_SUCCESS;
}

INT bcn_bpcc_dbg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE obj;
	RTMP_STRING *bpcc_dbg_op, *op_attr;
	RTMP_STRING *attrs;
	RTMP_STRING *tmpbuf = NULL;

	if (arg == NULL || strlen(arg) == 0)
		goto err;

	if (!pAd)
		goto err;

	obj = (POS_COOKIE) pAd->OS_Cookie;
	if ((obj->ioctl_if < 0) || (obj->ioctl_if >= MAX_BEACON_NUM))
		goto err;

	bpcc_dbg_op = arg;
	MTWF_PRINT("command: %s\n", bpcc_dbg_op);
	op_attr = rtstrchr(bpcc_dbg_op, ':');
	if (op_attr) {
		*op_attr = 0;
		op_attr++;
		attrs = op_attr;
		if (!attrs || !strlen(attrs))
			goto err;
		/* prepare tmpbuf for parameter parsing */
		os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);
		if (!tmpbuf)
			goto err;
	} else
		goto err;

	if (strcmp("protect", bpcc_dbg_op) == 0) {
		if (_bcn_bpcc_get_value_by_key("en", tmpbuf, 100, attrs)) {
			u8 protect_en;

			if (!kstrtou8(tmpbuf, 10, &protect_en)) {
				if (protect_en <= 1) {
					wdev = &pAd->ApCfg.MBSSID[obj->ioctl_if].wdev;
					OS_SEM_LOCK(&wdev->BcnBPCCLock);
					wdev->BcnBPCC_protect_disable = (protect_en == 0) ? TRUE : FALSE;
					wdev->BcnBPCC_protect_manual = TRUE;
					MTWF_PRINT("protect_en=%d\n", protect_en);
					OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
				} else if (protect_en == 100) {
					INT32 apidx;
					BOOLEAN Cancelled;

					for (apidx = 0; apidx < MAX_BEACON_NUM; apidx++) {
						wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
						if (!wdev->BcnBPCC_Init)
							continue;
						RTMPCancelTimer(&wdev->BcnBPCC_OP_Timer, &Cancelled);
						OS_SEM_LOCK(&wdev->BcnBPCCLock);
						wdev->BcnBPCC_protect_disable = TRUE;
						wdev->BcnBPCC_protect_manual = TRUE;
						wdev->BcnBPCC_op_lock_flag = FALSE;
						NdisZeroMemory(wdev->dbg_BcnBPCC_op_lock_caller, sizeof(wdev->dbg_BcnBPCC_op_lock_caller));
						OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
						RTMP_SEM_EVENT_UP(&wdev->BcnBPCC_op_lock);
					}
				} else if (protect_en == 200) {
					INT32 apidx;
					BOOLEAN Cancelled;

					for (apidx = 0; apidx < MAX_BEACON_NUM; apidx++) {
						wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
						if (!wdev->BcnBPCC_Init)
							continue;
						RTMPCancelTimer(&wdev->BcnBPCC_OP_Timer, &Cancelled);
						OS_SEM_LOCK(&wdev->BcnBPCCLock);
						wdev->BcnBPCC_protect_disable = FALSE;
						wdev->BcnBPCC_protect_manual = TRUE;
						wdev->BcnBPCC_op_lock_flag = FALSE;
						NdisZeroMemory(wdev->dbg_BcnBPCC_op_lock_caller, sizeof(wdev->dbg_BcnBPCC_op_lock_caller));
						OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
						RTMP_SEM_EVENT_UP(&wdev->BcnBPCC_op_lock);
					}
				} else if (protect_en == 255) {
					INT32 apidx;

					for (apidx = 0; apidx < MAX_BEACON_NUM; apidx++) {
						wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
						if (!wdev->BcnBPCC_Init)
							continue;
						OS_SEM_LOCK(&wdev->BcnBPCCLock);
						wdev->BcnBPCC_protect_manual = FALSE;
						OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
					}
				}
			} else
				goto err;
		} else if (_bcn_bpcc_get_value_by_key("oplock", tmpbuf, 100, attrs)) {
			u8 oplock;

			if (!kstrtou8(tmpbuf, 10, &oplock)) {
				UINT32 ret = 0;

				wdev = &pAd->ApCfg.MBSSID[obj->ioctl_if].wdev;
				if (oplock == 0)
					RTMP_SEM_EVENT_UP(&wdev->BcnBPCC_op_lock);
				else if (oplock == 1)
					RTMP_SEM_EVENT_TIMEOUT(&wdev->BcnBPCC_op_lock,
						bpcc_op_lock_max_time_adjust, ret);
				else if (oplock == 2)
					RTMP_SEM_EVENT_TRYLOCK(&wdev->BcnBPCC_op_lock, ret);
				MTWF_PRINT("BcnBPCC_op_lock=%d, ret=%d\n", oplock, ret);
			} else
				goto err;
		} else
			goto err;
	} else if (strcmp("info", bpcc_dbg_op) == 0) {
		if (_bcn_bpcc_get_value_by_key("dump", tmpbuf, 100, attrs)) {
			u8 dump;

			if (!kstrtou8(tmpbuf, 10, &dump)) {
				if (dump == 0) {
					INT32 apidx;

					for (apidx = 0; apidx < MAX_BEACON_NUM; apidx++) {
						wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
						_bcn_bpcc_info_dump(wdev, FALSE, FALSE, TRUE);
					}
				} else if (dump == 1) {
					wdev = &pAd->ApCfg.MBSSID[obj->ioctl_if].wdev;
					_bcn_bpcc_info_dump(wdev, TRUE, FALSE, FALSE);
				} else if (dump == 2) {
					INT32 apidx;

					for (apidx = 0; apidx < MAX_BEACON_NUM; apidx++) {
						wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
						_bcn_bpcc_info_dump(wdev, TRUE, FALSE, TRUE);
					}
				} else if (dump == 3) {
					INT32 apidx;

					for (apidx = 0; apidx < MAX_BEACON_NUM; apidx++) {
						wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
						_bcn_bpcc_info_dump(wdev, TRUE, TRUE, TRUE);
					}
				} else
					goto err;
			} else
				goto err;
		} else if (_bcn_bpcc_get_value_by_key("logoff", tmpbuf, 100, attrs)) {
			u8 logoff;

			if (!kstrtou8(tmpbuf, 10, &logoff)) {
				if (logoff < 2) {
					wdev = &pAd->ApCfg.MBSSID[obj->ioctl_if].wdev;
					OS_SEM_LOCK(&wdev->BcnBPCCLock);
					wdev->BcnBPCC_log_on = (logoff == 0) ? FALSE:TRUE;
					OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
					_bcn_bpcc_mld_log(wdev, wdev->BcnBPCC_log_on);
				} else if (logoff == 100) {
					INT32 apidx;

					for (apidx = 0; apidx < MAX_BEACON_NUM; apidx++) {
						wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
						if (!wdev->BcnBPCC_Init)
							continue;
						OS_SEM_LOCK(&wdev->BcnBPCCLock);
						wdev->BcnBPCC_log_on = FALSE;
						OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
						_bcn_bpcc_mld_log(wdev, wdev->BcnBPCC_log_on);
					}
				} else if (logoff == 200) {
					INT32 apidx;

					for (apidx = 0; apidx < MAX_BEACON_NUM; apidx++) {
						wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
						if (!wdev->BcnBPCC_Init)
							continue;
						OS_SEM_LOCK(&wdev->BcnBPCCLock);
						wdev->BcnBPCC_log_on = TRUE;
						OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
						_bcn_bpcc_mld_log(wdev, wdev->BcnBPCC_log_on);
					}
				}
			} else
				goto err;
		}
	} else if (strcmp("ie", bpcc_dbg_op) == 0) {
		if (_bcn_bpcc_get_value_by_key("type", tmpbuf, 100, attrs)) {
			u8 type;
			u8 i, j;

			if (!kstrtou8(tmpbuf, 10, &type)) {
				if (type <= 32) {
					wdev = &pAd->ApCfg.MBSSID[obj->ioctl_if].wdev;
					bcn_bpcc_ct_switch(pAd, wdev, BCN_BPCC_EHTOP, BCN_REASON(BCN_UPDATE_IE_CHG));
				} else if (type <= 64) {
					//send 16 times
					wdev = &pAd->ApCfg.MBSSID[obj->ioctl_if].wdev;
					for (i = 0; i < 16; i++)
						bcn_bpcc_ct_switch(pAd, wdev, BCN_BPCC_EHTOP, BCN_REASON(BCN_UPDATE_IE_CHG));
				} else if (type <= 96) {
					//send for all wdev
					for (j = 0; j < MAX_BEACON_NUM; j++) {
						wdev = &pAd->ApCfg.MBSSID[j].wdev;
						if (wdev->wdev_type != WDEV_TYPE_AP)
							continue;
						bcn_bpcc_ct_switch(pAd, wdev, BCN_BPCC_EHTOP, BCN_REASON(BCN_UPDATE_IE_CHG));
					}
				} else if (type <= 128) {
					//send 16 times for all wdev
					for (j = 0; j < MAX_BEACON_NUM; j++) {
						wdev = &pAd->ApCfg.MBSSID[j].wdev;
						if (wdev->wdev_type != WDEV_TYPE_AP)
							continue;
						for (i = 0; i < 16; i++)
							bcn_bpcc_ct_switch(pAd, wdev, BCN_BPCC_EHTOP, BCN_REASON(BCN_UPDATE_IE_CHG));
					}
				} else
					goto err;
			} else
				goto err;
		}
	} else if (strcmp("bcn", bpcc_dbg_op) == 0) {
		if (_bcn_bpcc_get_value_by_key("no_bcn", tmpbuf, 100, attrs)) {
			u8 no_bcn;

			if (!kstrtou8(tmpbuf, 10, &no_bcn)) {
				if (no_bcn <= 1) {
					wdev = &pAd->ApCfg.MBSSID[obj->ioctl_if].wdev;
					if (no_bcn)
						UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_DISABLE_TX));
					else
						UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_ENABLE_TX));
					MTWF_PRINT("(%s) Set no beacon as:%d\n",
						RtmpOsGetNetDevName(wdev->if_dev), (no_bcn ? 1 : 0));
				} else
					goto err;
			} else
				goto err;
		}
	} else if (strcmp("mlo", bpcc_dbg_op) == 0) {
		if (_bcn_bpcc_get_value_by_key("reconf", tmpbuf, 100, attrs)) {
			u8 reconf;

			if (!kstrtou8(tmpbuf, 10, &reconf)) {
				if (reconf == 1) {
					wdev = &pAd->ApCfg.MBSSID[obj->ioctl_if].wdev;
#ifdef RT_CFG80211_SUPPORT
					mtk_cfg80211_event_reconf_sm(wdev->if_dev, BSS_RECONFIG_COUNTDOWN_STAGE_END);
#endif
				} else
					goto err;
			} else
				goto err;
		}
	} else {
		MTWF_PRINT("Unknown op\n");
		goto err;
	}
	if (tmpbuf)
		os_free_mem(tmpbuf);

	return TRUE;
err:
	MTWF_PRINT
		("\n\tUsage: iwpriv $(inf_name) set bcnbpcc_dbg=");
	MTWF_PRINT
		("operation:param1=value1,[param2=value2,]...\n");
	MTWF_PRINT
		("\t\toperation: protect|info|ie|bcn\n");
	MTWF_PRINT
		("\t\tparam for protect:  en=(0/1/100(all disable)/200(all enable)/255(manual off))/oplock=(0/1)\n");
	MTWF_PRINT
		("\t\tparam for info:  dump=(0/1/2(all detail)/3)/logoff=(0/1/100(alloff)/200(allon))\n");
	MTWF_PRINT
		("\t\tparam for ie:  type=(32/64/96/128)\n");
	MTWF_PRINT
		("\t\tparam for bcn:  no_bcn=(0/1)\n");
	MTWF_PRINT
		("\t\tparam for mlo:  reconf=(1)\n");

	if (tmpbuf)
		os_free_mem(tmpbuf);
	return FALSE;
}

static VOID _bcn_bpcc_trigger(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3)
{
	struct wifi_dev *wdev, *BcnBPCC_trigger_wdev;
	RTMP_ADAPTER *pAd;
	BOOLEAN BcnBPCC_trigger_flag;
	UINT32 BcnBPCC_UpdScope;
	UINT8 BcnBPCC_UpdOriReason;

	wdev = (struct wifi_dev *)FunctionContext;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	if (wdev->BcnBPCC_log_on)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
		"\x1b[44m exec,trigger_wdev = %s, wdev_idx=%s, bitmap=0x%x \x1b[m\n\r",
		RtmpOsGetNetDevName(wdev->BcnBPCC_trigger_wdev->if_dev),
		RtmpOsGetNetDevName(wdev->if_dev), wdev->BcnBPCCIeType_bitmap);
	BcnBPCC_trigger_flag = wdev->BcnBPCC_trigger_flag;
	BcnBPCC_trigger_wdev = wdev->BcnBPCC_trigger_wdev;
	BcnBPCC_UpdScope = wdev->BcnBPCC_UpdScope;
	BcnBPCC_UpdOriReason = wdev->BcnBPCC_UpdOriReason;
	OS_SEM_UNLOCK(&wdev->BcnBPCCLock);

	if (BcnBPCC_trigger_flag == TRUE) {
#ifdef DOT11_EHT_BE
		bss_mngr_mld_critical_update_trigger(BcnBPCC_trigger_wdev, BcnBPCC_UpdScope, BcnBPCC_UpdOriReason);
#endif
		OS_SEM_LOCK(&wdev->BcnBPCCLock);
		wdev->BcnBPCC_trigger_wdev = NULL;
		wdev->BcnBPCCIeType_bitmap_hist[wdev->BcnBPCCIeType_bitmap_hist_idx] = wdev->BcnBPCCIeType_bitmap;
		RTMP_GetCurrentSystemTick(&wdev->BcnBPCCIeType_bitmap_hist_TT[wdev->BcnBPCCIeType_bitmap_hist_idx]);
		wdev->BcnBPCCIeType_bitmap_hist_idx = (wdev->BcnBPCCIeType_bitmap_hist_idx + 1) % 20;
		wdev->BcnBPCCIeType_bitmap_hist[wdev->BcnBPCCIeType_bitmap_hist_idx] = 0;
		wdev->BcnBPCC_trigger_flag = FALSE;
		wdev->BcnBPCC_UpdScope = 0;
		/* invalid reason */
		wdev->BcnBPCC_UpdOriReason = BCN_UPDATE_MAX;
		OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
	}
}

BUILD_TIMER_FUNCTION(_bcn_bpcc_trigger);

static VOID _bcn_bpcc_op_lock_to(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3)
{
	struct wifi_dev *wdev;
	RTMP_ADAPTER *pAd;
	ULONG temp;
	UINT32 ie_idx;

	wdev = (struct wifi_dev *)FunctionContext;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	RTMP_GetCurrentSystemTick(&temp);
	if (wdev->BcnBPCC_log_on)
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
			"\x1b[41m (%s). %d ms. ie=0x%x. The latest caller with the lock: %s\x1b[m\n\r",
			RtmpOsGetNetDevName(wdev->if_dev),
			jiffies_to_msecs(abs(temp-wdev->BcnBPCC_OP_Timer_TT_start)),
			wdev->BcnBPCCIeType_bitmap,
			wdev->dbg_BcnBPCC_op_lock_caller);
	wdev->BcnBPCC_TO_cnt++;
	ie_idx = wdev->BcnBPCCIeType_bitmap;
	if (wdev->BcnBPCCIeType_bitmap) {
		for (ie_idx = 0; ie_idx < BCN_BPCC_MAX; ie_idx++) {
			if ((wdev->BcnBPCCIeType_bitmap >> ie_idx) & 0x1)
				wdev->BcnBPCC_TO_cnt_per_IE[ie_idx]++;
		}
	}
	OS_SEM_UNLOCK(&wdev->BcnBPCCLock);

	/* check hwctl task status when Lock To happened */
	{
		struct physical_device *ph_dev = pAd->physical_dev;
		HW_CTRL_T *pHwCtrl = NULL;
		UINT32 HwCtrlQ_size = 0;

		if (ph_dev)
			pHwCtrl = &ph_dev->HwCtrl;

		if (pHwCtrl) {
			NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
			HwCtrlQ_size = pHwCtrl->HwCtrlQ.size;
			NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
			OS_SEM_LOCK(&wdev->BcnBPCCLock);
			if (HwCtrlQ_size > wdev->BcnBPCC_hwcmd_max)
				wdev->BcnBPCC_hwcmd_max = HwCtrlQ_size;
			if (wdev->BcnBPCC_log_on)
				MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
					"\x1b[41m QueSize: %d\x1b[m\n\r",
					wdev->BcnBPCC_hwcmd_max);
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		}
	}

	bcn_bpcc_op_unlock(pAd, wdev, FALSE);
}

BUILD_TIMER_FUNCTION(_bcn_bpcc_op_lock_to);

/*
   If the wdev belongs to the MLD device, it will decide critical
   scope and call bss_mngr_mld_critical_update_trigger to iterate
   all necessary MLD device to update beacon.
   If the wdev does not belong to the MLD device, it will call
   UpdateBeaconHandler directly.
*/
VOID UpdateBeaconHandler_BPCC(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BCN_UPDATE_REASON reason,
	enum BCN_BPCC_IE_TYPE ie_type,
	BOOLEAN ie_update
)
{
	INT32 upd_scope = BMGR_CRITICAL_UPDATE_ALL;
	BOOLEAN notify_crit_upd = ie_update ? TRUE : FALSE;
	BOOLEAN defer_crit_upd;
	UINT32 defer_ie_bitmap = (1 << BCN_BPCC_EDCA);
	UINT32 defer_cnt = 0;
	UINT32 dump_freq = 1;
	UINT32 ret = 0;
	struct wifi_dev *orig_wdev = wdev;
	BOOLEAN is_in_interrupt = FALSE;
	BOOLEAN is_bpcc_op_locked = FALSE;

	/* default clear high bits */
	reason = BCN_CLEAR_CRIT_UPD(reason);

	is_in_interrupt = in_interrupt() ? TRUE:FALSE;
	wdev = _bcn_bpcc_available_tx_wdev(pAd, wdev);
	if (!wdev) {
		if (orig_wdev && !orig_wdev->BcnBPCC_Init) {
			/* if bpcc init is not ready yet, always return TRUE to let bcn update can
				go ahead */
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
				"\x1b[41mie_type=%d, caller:%pS\x1b[m\n\r", ie_type, OS_TRACE);
			UpdateBeaconHandler(pAd, orig_wdev, reason);
			return;
		}
		return;
	}

	switch (ie_type) {
	case BCN_BPCC_CSA:
	case BCN_BPCC_ECSA:
		upd_scope = BMGR_CRITICAL_UPDATE_ALL;
		break;
	case BCN_BPCC_EDCA:/* per bss setting */
		/* defer it for wifi7 cert */
		upd_scope = BMGR_CRITICAL_UPDATE_SINGLE;
		break;
	case BCN_BPCC_QUIET:
	case BCN_BPCC_DSSS:
	case BCN_BPCC_HTOP:
	case BCN_BPCC_WBCS:
	case BCN_BPCC_CSW:
	case BCN_BPCC_OMN:
	case BCN_BPCC_QUIETC:
	case BCN_BPCC_HEOP:/* per bss setting */
	case BCN_BPCC_VHTOP:/* per bss setting */
		upd_scope = BMGR_CRITICAL_UPDATE_SINGLE;
		break;
	case BCN_BPCC_BTWT:
	case BCN_BPCC_BTWTSET:
	case BCN_BPCC_BSSCCA:
	case BCN_BPCC_SR:
	case BCN_BPCC_UORA:
		upd_scope = BMGR_CRITICAL_UPDATE_SINGLE;
		break;
	case BCN_BPCC_MUEDCA:/* per bss setting */
	case BCN_BPCC_EHTOP:
		dump_freq = 1;
		upd_scope = BMGR_CRITICAL_UPDATE_SINGLE;
		break;
	case BCN_BPCC_CH_CHANGED:
		upd_scope = BMGR_CRITICAL_UPDATE_ALL;
		break;
	case BCN_BPCC_RECONFIG:
	case BCN_BPCC_ADD_LINK:
	case BCN_BPCC_AT2LM:
		upd_scope = BMGR_CRITICAL_UPDATE_MLD_CUF;
		break;
	case BCN_BPCC_LEGACY_BCN:
		notify_crit_upd = FALSE;
		break;
	default:
		notify_crit_upd = FALSE;
		break;
	}

#ifdef DOT11_EHT_BE
	/* if wdev is not BE, there is no need to do critical update*/
	if (!WMODE_CAP_BE(wdev->PhyMode))
		notify_crit_upd = FALSE;
	if (bss_mngr_is_wdev_in_mlo_group(orig_wdev) != TRUE)
		notify_crit_upd = FALSE;
#endif

	defer_crit_upd = (defer_ie_bitmap & (1<<ie_type)) ? TRUE : FALSE;
	/*if it exists BPCC timer and ie_type is not EDCA ,it need to wait trigger done*/
	while (defer_cnt < 10) {
		OS_SEM_LOCK(&wdev->BcnBPCCLock);
		if ((wdev->BcnBPCC_trigger_flag == TRUE) &&
			!(wdev->BcnBPCCIeType_bitmap & defer_ie_bitmap & (1<<ie_type))) {
			if (wdev->BcnBPCC_log_on)
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
				"\x1b[41m Wait BcnBPCC_Trigger_Timer trigger done, bitmap=%x\x1b[m\n\r",
				wdev->BcnBPCCIeType_bitmap);
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
			defer_cnt++;
			if (is_in_interrupt) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
					"\x1b[41m wait defer in INT,immediately ,no wait(TBD)\x1b[m\n\r");
			}

			OS_WAIT(!pAd->CommonCfg.BeaconPeriod ? (100+10):(pAd->CommonCfg.BeaconPeriod+10));
		} else {
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
			break;
		}
	}

	/*check if get bcpp op locker before doing critical update */
	if (ie_type <= BCN_BPCC_LEGACY_BCN) {
		RTMP_SEM_EVENT_TRYLOCK(&wdev->BcnBPCC_op_lock, ret);
		if (!ret) {
			RTMP_SEM_EVENT_UP(&wdev->BcnBPCC_op_lock);
			OS_SEM_LOCK(&wdev->BcnBPCCLock);
			if (ie_type < BCN_BPCC_MAX)
				wdev->BcnBPCC_op_lock_refine_NG_per_IE_bitmap |= 1<<ie_type;
			if ((wdev->BcnBPCC_log_on) || (wdev->BcnBPCC_MLD_log_flag[wdev->BcnBPCC_mld_grp_id]))
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
					"\x1b[41m (%s)(orig:%s)ie_type=%d(%s). do not get bpcc op lock before calling %s. refine it \x1b[m\n\r",
					RtmpOsGetNetDevName(wdev->if_dev), RtmpOsGetNetDevName(orig_wdev->if_dev), ie_type, BCN_BPCC_IE_TYPE_STR[ie_type], __func__);
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		} else {
			OS_SEM_LOCK(&wdev->BcnBPCCLock);
			if ((wdev->BcnBPCC_log_on) || (wdev->BcnBPCC_MLD_log_flag[wdev->BcnBPCC_mld_grp_id]))
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
				"\x1b[44m (%s)(orig:%s)ie_type=%d(%s). get bpcc op lock before calling %s. refine done \x1b[m\n\r",
				RtmpOsGetNetDevName(wdev->if_dev), RtmpOsGetNetDevName(orig_wdev->if_dev), ie_type, BCN_BPCC_IE_TYPE_STR[ie_type], __func__);
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
			is_bpcc_op_locked = TRUE;
		}
	}

	if ((ie_type < BCN_BPCC_MAX) &&
		((wdev->BcnBPCCIeType_cnt_hist[ie_type] % dump_freq) == 0)) {
		OS_SEM_LOCK(&wdev->BcnBPCCLock);
		if ((wdev->BcnBPCC_log_on) || (wdev->BcnBPCC_MLD_log_flag[wdev->BcnBPCC_mld_grp_id]))
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
			"(%s) scope:%d, reason=%d, cap_be=%d, ie_type=%d, %d (caller:%pS)\n\r",
			RtmpOsGetNetDevName(wdev->if_dev), upd_scope, reason, WMODE_CAP_BE(wdev->PhyMode), ie_type,
			wdev->BcnBPCCIeType_cnt_hist[ie_type], OS_TRACE);
		OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
	}

	if (notify_crit_upd == TRUE) {
		if (defer_crit_upd == TRUE) {
			OS_SEM_LOCK(&wdev->BcnBPCCLock);
			if (wdev->BcnBPCC_trigger_flag == FALSE) {
				wdev->BcnBPCC_trigger_flag = TRUE;
				wdev->BcnBPCC_trigger_wdev = orig_wdev;
				/* carry original reason */
				wdev->BcnBPCC_UpdOriReason = reason;
				OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
				RTMPSetTimer(&wdev->BcnBPCC_Trigger_Timer, !pAd->CommonCfg.BeaconPeriod ? (100+10):(pAd->CommonCfg.BeaconPeriod+10));
			} else {
				OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
			}
			OS_SEM_LOCK(&wdev->BcnBPCCLock);
			if (wdev->BcnBPCC_UpdScope <= upd_scope)
				wdev->BcnBPCC_UpdScope = upd_scope;
			wdev->BcnBPCCIeType_bitmap |= (1 << ie_type);
			wdev->BcnBPCCIeType_cnt_hist[ie_type]++;
			if (wdev->BcnBPCC_log_on)
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
				"\x1b[44m Trigger %dms BcnBPCC_Trigger_Timer, ie_type=%d, upd_scope=%d \x1b[m\n\r",
				!pAd->CommonCfg.BeaconPeriod ? (100+10):(pAd->CommonCfg.BeaconPeriod+10),
				ie_type, upd_scope);
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		} else {
			OS_SEM_LOCK(&wdev->BcnBPCCLock);
			wdev->BcnBPCCIeType_bitmap_hist[wdev->BcnBPCCIeType_bitmap_hist_idx] = (1 << ie_type);
			RTMP_GetCurrentSystemTick(&wdev->BcnBPCCIeType_bitmap_hist_TT[wdev->BcnBPCCIeType_bitmap_hist_idx]);
			wdev->BcnBPCCIeType_bitmap_hist_idx = (wdev->BcnBPCCIeType_bitmap_hist_idx + 1) % 20;
			wdev->BcnBPCCIeType_bitmap_hist[wdev->BcnBPCCIeType_bitmap_hist_idx] = 0;
			wdev->BcnBPCCIeType_bitmap |= (1 << ie_type);
			wdev->BcnBPCCIeType_cnt_hist[ie_type]++;
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);

			bss_mngr_mld_critical_update_trigger(orig_wdev, upd_scope, reason);
		}
	} else if (notify_crit_upd == FALSE) {
		if (ie_type == BCN_BPCC_LEGACY_BCN) {
			OS_SEM_LOCK(&wdev->BcnBPCCLock);
			wdev->BcnBPCCIeType_bitmap |= (1 << ie_type);
			wdev->BcnBPCCIeType_cnt_hist[ie_type]++;
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);

			HW_BEACON_UPDATE(pAd, wdev, (UCHAR)reason);
		} else if (ie_type < BCN_BPCC_LEGACY_BCN) {
			if (is_bpcc_op_locked == TRUE)
				_bcn_bpcc_op_unlock(pAd, wdev);
			/* can not handle it, run UpdateBeaconHandler */
			UpdateBeaconHandler(pAd, orig_wdev, reason);
		} else
			UpdateBeaconHandler(pAd, orig_wdev, reason);
	}
}

static BOOLEAN _bcn_bpcc_op_lock(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN legacy)
{
	UINT32 ret = 0;
	BOOLEAN status = FALSE, is_in_interrupt = FALSE;
	INT rt = 0;
	BOOLEAN Cancelled;
	struct wifi_dev *orig_wdev = wdev;

	wdev = _bcn_bpcc_available_tx_wdev(pAd, wdev);
	if (!wdev) {
		if (orig_wdev && !orig_wdev->BcnBPCC_Init)
			/* if bpcc init is not ready yet, always return TRUE to let bcn update can
				go ahead */
			return TRUE;
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
				"\x1b[41mcaller:%pS\x1b[m\n\r", OS_TRACE);
		return status;
	}

	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	if (wdev->BcnBPCC_protect_disable) {
		OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		return TRUE;
	}
	OS_SEM_UNLOCK(&wdev->BcnBPCCLock);

	is_in_interrupt = in_interrupt() ? TRUE:FALSE;
	/* get lock directly */
	if (is_in_interrupt) {
		OS_SEM_LOCK(&wdev->BcnBPCCLock);
		if (wdev->BcnBPCC_log_on)
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_DEBUG,
				"\x1b[41mINT: (%s)use RTMP_SEM_EVENT_TRYLOCK\x1b[m\n\r", RtmpOsGetNetDevName(wdev->if_dev));
		OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		RTMP_SEM_EVENT_TRYLOCK(&wdev->BcnBPCC_op_lock, ret);
	} else {
		OS_SEM_LOCK(&wdev->BcnBPCCLock);
		if (wdev->BcnBPCC_log_on)
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_DEBUG,
				"\x1b[42mThread: (%s)use RTMP_SEM_EVENT_TIMEOUT\x1b[m\n\r", RtmpOsGetNetDevName(wdev->if_dev));
		OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		RTMP_SEM_EVENT_TIMEOUT(&wdev->BcnBPCC_op_lock, bpcc_op_lock_max_time_adjust, ret);
		OS_SEM_LOCK(&wdev->BcnBPCCLock);
		if (wdev->BcnBPCC_log_on)
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_DEBUG,
				"\x1b[42mThread: (%s) ret = %d\x1b[m\n\r", RtmpOsGetNetDevName(wdev->if_dev), ret);
		OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
	}
	if (!ret) {
		OS_SEM_LOCK(&wdev->BcnBPCCLock);
		rt = snprintf(wdev->dbg_BcnBPCC_op_lock_caller, sizeof(wdev->dbg_BcnBPCC_op_lock_caller), "%pS", OS_TRACE);
		if (os_snprintf_error(sizeof(wdev->dbg_BcnBPCC_op_lock_caller), rt)) {
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
					"final_name snprintf error!\n");
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
			return FALSE;
		}
		if (wdev->BcnBPCC_log_on)
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
			"\x1b[42m(%s): get lock.caller:%pS\x1b[m\n\r", RtmpOsGetNetDevName(wdev->if_dev), OS_TRACE);
		wdev->BcnBPCC_op_lock_flag = TRUE;
		RTMP_GetCurrentSystemTick(&wdev->BcnBPCC_OP_Timer_TT_start);
		RTMP_GetCurrentSystemTick(&wdev->BcnBPCC_op_lock_TT_start);
		OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		RTMPSetTimer(&wdev->BcnBPCC_OP_Timer, bpcc_op_lock_max_to_adjust);
		status = TRUE;
	} else {
		OS_SEM_LOCK(&wdev->BcnBPCCLock);
		if (legacy)
			wdev->BcnBPCC_op_lock_by_legacy_fail_cnt++;
		else
			wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt++;
		OS_SEM_UNLOCK(&wdev->BcnBPCCLock);

		OS_SEM_LOCK(&wdev->BcnBPCCLock);
		if (is_in_interrupt) {
			if (wdev->BcnBPCC_log_on)
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_DEBUG,
					"\x1b[41m(%s)in_interrupt and get lock fail caller:%pS\x1b[m\n\r",
					RtmpOsGetNetDevName(wdev->if_dev), OS_TRACE);
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		} else {
			if (wdev->BcnBPCC_log_on)
				MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
					"\x1b[41m(%s): get lock fail(BUG) caller:%pS.The latest caller with the lock: %s\x1b[m\n\r",
					RtmpOsGetNetDevName(wdev->if_dev), OS_TRACE, wdev->dbg_BcnBPCC_op_lock_caller);
			/* force unlock,  BcnBPCC_OP_Timer does not work for this case */
			RTMP_GetCurrentSystemTick(&wdev->BcnBPCC_OP_Timer_TT_cancel);
			wdev->BcnBPCC_op_lock_flag = FALSE;
			NdisZeroMemory(wdev->dbg_BcnBPCC_op_lock_caller, sizeof(wdev->dbg_BcnBPCC_op_lock_caller));
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
			RTMPCancelTimer(&wdev->BcnBPCC_OP_Timer, &Cancelled);
			RTMP_SEM_EVENT_UP(&wdev->BcnBPCC_op_lock);
		}
		status = FALSE;
	}

	return status;
}

static VOID _bcn_bpcc_op_lock_statistic_update(RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
	BOOLEAN query_bssmngr, enum BCN_BPCC_IE_TYPE ie_type,
	BOOLEAN lock_result, ULONG get_lock_time_period)
{
	BOOLEAN is_in_interrupt = FALSE;
	ULONG *BcnBPCC_op_lock_get_max;
	ULONG *BcnBPCC_op_lock_get_avg;
	ULONG *BcnBPCC_op_lock_get_fail_max;
	ULONG *BcnBPCC_op_lock_get_fail_avg;

	is_in_interrupt = in_interrupt() ? TRUE:FALSE;
	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	if (query_bssmngr) {
		BcnBPCC_op_lock_get_max = &wdev->BcnBPCC_op_lock_get_mld_max;
		BcnBPCC_op_lock_get_avg = &wdev->BcnBPCC_op_lock_get_mld_avg;
		BcnBPCC_op_lock_get_fail_max = &wdev->BcnBPCC_op_lock_get_fail_mld_max;
		BcnBPCC_op_lock_get_fail_avg = &wdev->BcnBPCC_op_lock_get_fail_mld_avg;
	} else {
		BcnBPCC_op_lock_get_max = &wdev->BcnBPCC_op_lock_get_legacy_max;
		BcnBPCC_op_lock_get_avg = &wdev->BcnBPCC_op_lock_get_legacy_avg;
		BcnBPCC_op_lock_get_fail_max = &wdev->BcnBPCC_op_lock_get_fail_legacy_max;
		BcnBPCC_op_lock_get_fail_avg = &wdev->BcnBPCC_op_lock_get_fail_legacy_avg;
	}
	if (lock_result == TRUE) {
		if (*BcnBPCC_op_lock_get_max < get_lock_time_period)
			*BcnBPCC_op_lock_get_max = get_lock_time_period;
		if (!(*BcnBPCC_op_lock_get_avg))
			(*BcnBPCC_op_lock_get_avg) = get_lock_time_period;
		if (get_lock_time_period > 1) {
			*BcnBPCC_op_lock_get_avg += get_lock_time_period;
			*BcnBPCC_op_lock_get_avg = *BcnBPCC_op_lock_get_avg >> 1;
		}
	} else {
		if (!is_in_interrupt) {
			if (*BcnBPCC_op_lock_get_fail_max < get_lock_time_period)
				*BcnBPCC_op_lock_get_fail_max = get_lock_time_period;
			if (!(*BcnBPCC_op_lock_get_fail_avg))
				*BcnBPCC_op_lock_get_fail_avg = get_lock_time_period;
			if (get_lock_time_period > 1) {
				*BcnBPCC_op_lock_get_fail_avg += get_lock_time_period;
				*BcnBPCC_op_lock_get_fail_avg = *BcnBPCC_op_lock_get_fail_avg >> 1;
			}
		}
		if ((wdev->BcnBPCC_Init == TRUE) && (ie_type < BCN_BPCC_MAX) &&
			wdev->wdev_type == WDEV_TYPE_AP) {
			if (is_in_interrupt)
				wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_INT[ie_type]++;
			else
				wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_CT[ie_type]++;
		}
	}
	if (wdev->BcnBPCC_log_on) {
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
			"\x1b[42m(%s): ie(%s) (get status:%s, %lu ms) by %s.\x1b[m\n\r",
			RtmpOsGetNetDevName(wdev->if_dev),
			BCN_BPCC_IE_TYPE_STR[ie_type],
			(lock_result == FALSE) ? "fail":"ok",
			get_lock_time_period,
			(query_bssmngr == TRUE) ? "MLD":"Legacy");
	}
	OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
}

BOOLEAN bcn_bpcc_op_lock(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN query_bssmngr, enum BCN_BPCC_IE_TYPE ie_type)
{
	BOOLEAN status = FALSE;
	ULONG get_lock_time_period, get_lock_time_start, get_lock_time_end;
	struct wifi_dev *orig_wdev = wdev;
	BOOLEAN is_in_interrupt = FALSE;

	is_in_interrupt = in_interrupt() ? TRUE:FALSE;
	wdev = _bcn_bpcc_available_tx_wdev(pAd, orig_wdev);
	if (!wdev) {
		if (orig_wdev && !orig_wdev->BcnBPCC_Init)
			/* if bpcc init is not ready yet, always return TRUE to let bcn update can
				go ahead */
			return TRUE;
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
				"\x1b[41mcaller:%pS\x1b[m\n\r", OS_TRACE);
		return status;
	}
	if (query_bssmngr == TRUE) {
		if (ie_type >= BCN_BPCC_MAX)
			return status;
		/* get lock by bssmngr */
		if (WMODE_CAP_BE(orig_wdev->PhyMode) && (bss_mngr_is_wdev_in_mlo_group(orig_wdev) == TRUE)) {
			RTMP_GetCurrentSystemTick(&get_lock_time_start);
			OS_SEM_LOCK(&wdev->BcnBPCCLock);
			if (wdev->BcnBPCC_log_on) {
				MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
					"\x1b[42m(%s):(orig:%s) ie(%s)\x1b[m\n\r",
					RtmpOsGetNetDevName(wdev->if_dev), RtmpOsGetNetDevName(orig_wdev->if_dev),
					BCN_BPCC_IE_TYPE_STR[ie_type]);
			}
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
			if ((ie_type == BCN_BPCC_CSA) || (ie_type == BCN_BPCC_ECSA) || (ie_type == BCN_BPCC_CH_CHANGED))
				status = bss_mngr_mld_critical_update_op_lock(orig_wdev, BMGR_BPCC_OP_GET_LOCK_ALL);
			else
				status = bss_mngr_mld_critical_update_op_lock(orig_wdev, BMGR_BPCC_OP_GET_LOCK);
			RTMP_GetCurrentSystemTick(&get_lock_time_end);
			get_lock_time_period = jiffies_to_msecs(abs(get_lock_time_end-get_lock_time_start));
			_bcn_bpcc_op_lock_statistic_update(pAd, wdev, TRUE, ie_type, status, get_lock_time_period);

			OS_SEM_LOCK(&wdev->BcnBPCCLock);
			if (wdev->BcnBPCC_log_on) {
				if (is_in_interrupt && (status == FALSE))
					MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_DEBUG,
						"\x1b[42m(%s):(orig:%s) ie(%s) get lock (%s) by query_bssmngr.caller:%pS\x1b[m\n\r",
						RtmpOsGetNetDevName(wdev->if_dev), RtmpOsGetNetDevName(orig_wdev->if_dev),
						BCN_BPCC_IE_TYPE_STR[ie_type], (status == FALSE) ? "fail":"ok", OS_TRACE);
				else
					MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
						"\x1b[42m(%s):(orig:%s) ie(%s) get lock (%s) by query_bssmngr.caller:%pS\x1b[m\n\r",
						RtmpOsGetNetDevName(wdev->if_dev), RtmpOsGetNetDevName(orig_wdev->if_dev),
						BCN_BPCC_IE_TYPE_STR[ie_type], (status == FALSE) ? "fail":"ok", OS_TRACE);
			}
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
			return status;
		}
	}
	RTMP_GetCurrentSystemTick(&get_lock_time_start);
	status = _bcn_bpcc_op_lock(pAd, orig_wdev, TRUE);
	RTMP_GetCurrentSystemTick(&get_lock_time_end);
	get_lock_time_period = jiffies_to_msecs(abs(get_lock_time_end-get_lock_time_start));
	_bcn_bpcc_op_lock_statistic_update(pAd, wdev, FALSE, ie_type, status, get_lock_time_period);

	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	if (wdev->BcnBPCC_log_on)
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
		"\x1b[42m(%s):(orig:%s) get lock (%s) by legacy.caller:%pS\x1b[m\n\r",
		RtmpOsGetNetDevName(wdev->if_dev), RtmpOsGetNetDevName(orig_wdev->if_dev),
		(status == FALSE) ? "fail":"ok", OS_TRACE);
	OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
	return status;
}

BOOLEAN bcn_bpcc_op_lock_by_bssmngr(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	return _bcn_bpcc_op_lock(pAd, wdev, FALSE);
}

void bcn_bpcc_op_lock_set_timer_by_bssmngr(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct wifi_dev *orig_wdev = wdev;

	wdev = _bcn_bpcc_available_tx_wdev(pAd, orig_wdev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
				"\x1b[41mcaller:%pS\x1b[m\n\r", OS_TRACE);
		return;
	}

	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	if (wdev->BcnBPCC_protect_disable) {
		if (wdev->BcnBPCC_op_lock_flag == TRUE) {
			/* force unlock for dynamic protect */
			BOOLEAN Cancelled;

			RTMP_GetCurrentSystemTick(&wdev->BcnBPCC_OP_Timer_TT_cancel);
			wdev->BcnBPCC_op_lock_flag = FALSE;
			NdisZeroMemory(wdev->dbg_BcnBPCC_op_lock_caller, sizeof(wdev->dbg_BcnBPCC_op_lock_caller));
			wdev->BcnBPCCIeType_bitmap = 0;
			if (wdev->BcnBPCC_log_on)
				MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
				"\x1b[42m(%s): BcnBPCC_protect_disable.caller:%pS\x1b[m\n\r", RtmpOsGetNetDevName(wdev->if_dev), OS_TRACE);
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
			RTMPCancelTimer(&wdev->BcnBPCC_OP_Timer, &Cancelled);
			RTMP_SEM_EVENT_UP(&wdev->BcnBPCC_op_lock);
		} else
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		return;
	}
	OS_SEM_UNLOCK(&wdev->BcnBPCCLock);

	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	if (wdev->BcnBPCC_op_lock_flag == TRUE) {
		if (wdev->BcnBPCC_log_on)
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
			"\x1b[42m(%s): modify lock timer.caller:%pS\x1b[m\n\r", RtmpOsGetNetDevName(wdev->if_dev), OS_TRACE);
		RTMP_GetCurrentSystemTick(&wdev->BcnBPCC_OP_Timer_TT_start);
		OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		RTMPModTimer(&wdev->BcnBPCC_OP_Timer, bpcc_op_lock_max_to_adjust);
	} else {
		if (wdev->BcnBPCC_log_on)
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
			"\x1b[41m(%s): BUG.caller:%pS\x1b[m\n\r", RtmpOsGetNetDevName(wdev->if_dev), OS_TRACE);
		RTMP_GetCurrentSystemTick(&wdev->BcnBPCC_OP_Timer_TT_start);
		OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		RTMPSetTimer(&wdev->BcnBPCC_OP_Timer, bpcc_op_lock_max_to_adjust);
	}
}

static BOOLEAN _bcn_bpcc_op_unlock(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	BOOLEAN Cancelled;
	BOOLEAN ret = TRUE;
	struct wifi_dev *orig_wdev = wdev;

	wdev = _bcn_bpcc_available_tx_wdev(pAd, wdev);
	if (!wdev) {
		if (orig_wdev && !orig_wdev->BcnBPCC_Init)
			/* if bpcc init is not ready yet, always return TRUE to let bcn update can
				go ahead */
			return TRUE;
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
				"\x1b[41mcaller:%pS\x1b[m\n\r", OS_TRACE);
		return FALSE;
	}

	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	if (wdev->BcnBPCC_protect_disable) {
		if (wdev->BcnBPCC_op_lock_flag == TRUE) {
			/* force unlock for dynamic protect */
			wdev->BcnBPCC_op_lock_flag = FALSE;
			NdisZeroMemory(wdev->dbg_BcnBPCC_op_lock_caller, sizeof(wdev->dbg_BcnBPCC_op_lock_caller));
			wdev->BcnBPCCIeType_bitmap = 0;
			if (wdev->BcnBPCC_log_on)
				MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
				"\x1b[42m(%s): BcnBPCC_protect_disable.caller:%pS\x1b[m\n\r", RtmpOsGetNetDevName(wdev->if_dev), OS_TRACE);
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
			RTMPCancelTimer(&wdev->BcnBPCC_OP_Timer, &Cancelled);
			RTMP_SEM_EVENT_UP(&wdev->BcnBPCC_op_lock);
		} else {
			OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		}
		return TRUE;
	}
	OS_SEM_UNLOCK(&wdev->BcnBPCCLock);

	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	/* release lock directly */
	if (wdev->BcnBPCC_op_lock_flag == TRUE) {
		ULONG lock_time_period;

		RTMP_GetCurrentSystemTick(&wdev->BcnBPCC_OP_Timer_TT_cancel);
		wdev->BcnBPCC_op_lock_TT_end = wdev->BcnBPCC_OP_Timer_TT_cancel;
		lock_time_period = jiffies_to_msecs(abs(wdev->BcnBPCC_op_lock_TT_end-wdev->BcnBPCC_op_lock_TT_start));
		if (wdev->BcnBPCCIeType_bitmap != 0) {
			if (wdev->BcnBPCC_op_lock_max < lock_time_period)
				wdev->BcnBPCC_op_lock_max = lock_time_period;
			if (!wdev->BcnBPCC_op_lock_avg)
				wdev->BcnBPCC_op_lock_avg = lock_time_period;
			if (lock_time_period > 1) {
				wdev->BcnBPCC_op_lock_avg += lock_time_period;
				wdev->BcnBPCC_op_lock_avg = wdev->BcnBPCC_op_lock_avg >> 1;
			}
		}
		if ((wdev->BcnBPCCIeType_bitmap != 0) &&
			(wdev->BcnBPCCIeType_bitmap != (1<<BCN_BPCC_LEGACY_BCN))) {
			if (wdev->BcnBPCC_op_lock_mld_max < lock_time_period)
				wdev->BcnBPCC_op_lock_mld_max = lock_time_period;
			if (!wdev->BcnBPCC_op_lock_mld_avg)
				wdev->BcnBPCC_op_lock_mld_avg = lock_time_period;
			if (lock_time_period > 1) {
				wdev->BcnBPCC_op_lock_mld_avg += lock_time_period;
				wdev->BcnBPCC_op_lock_mld_avg = wdev->BcnBPCC_op_lock_mld_avg >> 1;
			}
		}

		if (wdev->BcnBPCC_log_on)
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
			"\x1b[42m(%s) %lu ms! caller:%pS\x1b[m\n\r",
			RtmpOsGetNetDevName(wdev->if_dev), lock_time_period,
			OS_TRACE);
		wdev->BcnBPCC_op_lock_flag = FALSE;
		NdisZeroMemory(wdev->dbg_BcnBPCC_op_lock_caller, sizeof(wdev->dbg_BcnBPCC_op_lock_caller));
		wdev->BcnBPCCIeType_bitmap = 0;
		OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		RTMPCancelTimer(&wdev->BcnBPCC_OP_Timer, &Cancelled);
		RTMP_SEM_EVENT_UP(&wdev->BcnBPCC_op_lock);
	} else {
		if (wdev->BcnBPCC_log_on)
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
			"\x1b[41m(%s): release lock(BUG).caller:%pS\x1b[m\n\r",
			RtmpOsGetNetDevName(wdev->if_dev), OS_TRACE);
		OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		ret = FALSE;
	}

	return ret;
}

BOOLEAN bcn_bpcc_op_unlock(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN query_bssmngr)
{
	BOOLEAN status;
	struct wifi_dev *orig_wdev = wdev;

	if (query_bssmngr == TRUE) {
		/* release lock by bssmngr */
		if (WMODE_CAP_BE(orig_wdev->PhyMode) && (bss_mngr_is_wdev_in_mlo_group(orig_wdev) == TRUE)) {
			bss_mngr_mld_critical_update_op_lock(orig_wdev, BMGR_BPCC_OP_RELEASE_LOCK);
			return TRUE;
		}
	}

	wdev = _bcn_bpcc_available_tx_wdev(pAd, orig_wdev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
				"\x1b[41mcaller:%pS\x1b[m\n\r", OS_TRACE);
		return FALSE;
	}

	status = _bcn_bpcc_op_unlock(pAd, orig_wdev);

	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	if (wdev->BcnBPCC_log_on) {
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
		"\x1b[42m(%s) unlock (%s) caller:%pS\x1b[m\n\r",
		RtmpOsGetNetDevName(wdev->if_dev),
		(status == FALSE) ? "fail":"ok",
		OS_TRACE);
	}
	OS_SEM_UNLOCK(&wdev->BcnBPCCLock);

	return status;
}

VOID bcn_bpcc_op_unlock_by_hwctrl(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN query_bssmngr, UCHAR UpdateReason)
{
	BOOLEAN status;
	struct wifi_dev *orig_wdev = wdev;

	if (query_bssmngr == TRUE) {
		/* release lock by bssmngr */
		if (WMODE_CAP_BE(orig_wdev->PhyMode) && (bss_mngr_is_wdev_in_mlo_group(orig_wdev) == TRUE)) {
			bss_mngr_mld_critical_update_op_lock(orig_wdev, BMGR_BPCC_OP_RELEASE_LOCK);
			return;
		}
	}

	wdev = _bcn_bpcc_available_tx_wdev(pAd, orig_wdev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
				"\x1b[41mcaller:%pS\x1b[m\n\r", OS_TRACE);
		return;
	}

	status = _bcn_bpcc_op_unlock(pAd, orig_wdev);

	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	if (wdev->BcnBPCC_log_on) {
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
		"\x1b[42m(%s) unlock (%s) (UpdateReason:%d) caller:%pS\x1b[m\n\r",
		RtmpOsGetNetDevName(wdev->if_dev),
		(status == FALSE) ? "fail":"ok",
		UpdateReason, OS_TRACE);
	}
	OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
}

VOID bcn_bpcc_op_unlock_by_bssmngr(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	BOOLEAN status;
	struct wifi_dev *orig_wdev = wdev;

	wdev = _bcn_bpcc_available_tx_wdev(pAd, orig_wdev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
				"\x1b[41mcaller:%pS\x1b[m\n\r", OS_TRACE);
		return;
	}

	status = _bcn_bpcc_op_unlock(pAd, orig_wdev);

	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	if (wdev->BcnBPCC_log_on) {
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
		"\x1b[42m(%s) unlock (%s) caller:%pS\x1b[m\n\r",
		RtmpOsGetNetDevName(wdev->if_dev),
		(status == FALSE) ? "fail":"ok",
		OS_TRACE);
	}
	OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
}

static void _bcn_bpcc_deinit_all(RTMP_ADAPTER *pAd)
{
	INT i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];
		if (!wdev)
			continue;
		bcn_bpcc_deinit(pAd, wdev);
	}
}

static NTSTATUS _bcn_bpcc_ct_switch_handler(RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt)
{
	struct BPCC_BEACON_CT_SWITCH *prMtUpdateBeacon = (struct BPCC_BEACON_CT_SWITCH *)CMDQelmt->buffer;
	struct wifi_dev *wdev = prMtUpdateBeacon->wdev;
	UINT32 switch_type = prMtUpdateBeacon->switch_type;

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
		"\x1b[43m --->switch_type=%d\x1b[m\n\r", switch_type);
	switch (switch_type) {
	case BCN_BPCC_HEOP:
		{
			UINT32 UpdateReason;

			prMtUpdateBeacon->UpdateReason = (UINT32 *)&prMtUpdateBeacon->content[0];
			UpdateReason = *prMtUpdateBeacon->UpdateReason;
			if (UpdateReason == BCN_BPCC_HEOP_BSS_COLOR)
				bss_color_event_handler(wdev);
		}
		break;
	case BCN_BPCC_BTWT:
	case BCN_BPCC_BTWTSET:
		if (bcn_bpcc_op_lock(pAd, wdev, TRUE, switch_type) == FALSE)
			MTWF_PRINT("%s(%d): bcn_bpcc_op_lock fail!\n", __func__, __LINE__);
		twt_build_btwt_ie(wdev, switch_type);
		break;
	case BCN_BPCC_MUEDCA:
		{
			struct _BCN_MUEDCA_UPDATE_PARAM update_param;
			bool isLockSuccess = TRUE;

			memcpy(&update_param, &prMtUpdateBeacon->content[1], sizeof(struct _BCN_MUEDCA_UPDATE_PARAM));
			if (bcn_bpcc_op_lock(pAd, wdev, TRUE, BCN_BPCC_MUEDCA) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
					"%s: MUEDCA bcn_bpcc_op_lock fail!\n", __func__);
				isLockSuccess = FALSE;
			}
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
				"MUEDCA update reason = %d\n", update_param.MUEDCAUpdateReason);
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
				"MUEDCA bcn update parameters: value = %d, param = %d, aci = %d\n",
				update_param.value, update_param.param, update_param.aci);
			if (update_param.MUEDCAUpdateReason == RSN_AP_MUEDCA) {
				struct mu_edca_cfg *mu_edca = wlan_config_get_he_mu_edca(wdev);
				struct _EDCA_PARM *pBssEdca = wlan_config_get_ht_edca(wdev);

				if (mu_edca == NULL || pBssEdca == NULL) {
					if (isLockSuccess)
						bcn_bpcc_op_unlock(pAd, wdev, TRUE);
					break;
				}

				pBssEdca->EdcaUpdateCount++;
				if ((mu_edca->mu_ac_rec[ACI_AC_BE].mu_edca_timer != update_param.value) ||
				(mu_edca->mu_ac_rec[ACI_AC_BK].mu_edca_timer != update_param.value) ||
				(mu_edca->mu_ac_rec[ACI_AC_VI].mu_edca_timer != update_param.value) ||
				(mu_edca->mu_ac_rec[ACI_AC_VO].mu_edca_timer != update_param.value)) {
					mu_edca->mu_ac_rec[ACI_AC_BE].mu_edca_timer = update_param.value;
					mu_edca->mu_ac_rec[ACI_AC_BK].mu_edca_timer = update_param.value;
					mu_edca->mu_ac_rec[ACI_AC_VI].mu_edca_timer = update_param.value;
					mu_edca->mu_ac_rec[ACI_AC_VO].mu_edca_timer = update_param.value;
				} else {
					if (isLockSuccess)
						bcn_bpcc_op_unlock(pAd, wdev, TRUE);
					break;
				}
			} else if (update_param.MUEDCAUpdateReason == RSN_RFEATURE_DIS_MUEDCA) {
				RTMP_CHIP_CAP *chip_cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
				/*update IE*/
				if (update_param.value)
					chip_cap->phy_caps &= ~fPHY_CAP_HE_UL_MUOFDMA;
				else
					chip_cap->phy_caps |= fPHY_CAP_HE_UL_MUOFDMA;

			} else if (update_param.MUEDCAUpdateReason == RSN_CFG_SET) {
				struct _EDCA_PARM *edca_param = wlan_config_get_ht_edca(wdev);
				struct mu_edca_cfg *mu_edca = wlan_config_get_he_mu_edca(wdev);

				if (mu_edca == NULL || edca_param == NULL) {
					if (isLockSuccess)
						bcn_bpcc_op_unlock(pAd, wdev, TRUE);
					break;
				}

				if (update_param.param == MU_EDCA_ECW_MIN)
					mu_edca->mu_ac_rec[update_param.aci].ecw_min = update_param.value;
				else if (update_param.param == MU_EDCA_ECW_MAX)
					mu_edca->mu_ac_rec[update_param.aci].ecw_max = update_param.value;
				else if (update_param.param == MU_EDCA_AIFSN)
					mu_edca->mu_ac_rec[update_param.aci].aifsn = update_param.value;
				else if (update_param.param == MU_EDCA_TIMER)
					mu_edca->mu_ac_rec[update_param.aci].mu_edca_timer = update_param.value;

				edca_param->EdcaUpdateCount++;
			}
			UpdateBeaconHandler_BPCC(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG),
					BCN_BPCC_MUEDCA, TRUE);

		}
		break;
	case BCN_BPCC_BSSCCA:
		{
			struct _BCN_BSSCCA_UPDATE_PARAM update_param;
			struct _BSS_INFO_ARGUMENT_T *bssinfo = &wdev->bss_info_argument;
			struct bss_color_ctrl *bss_color = &bssinfo->bss_color;

			/* get transmited parameter */
			memcpy(&update_param, &prMtUpdateBeacon->content[1], sizeof(struct _BCN_BSSCCA_UPDATE_PARAM));
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
				"BSSCCA update reason = %d\n", update_param.BSSCAUpdateReason);

			if (bcn_bpcc_op_lock(pAd, wdev, TRUE, BCN_BPCC_BSSCCA) == FALSE)
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
					"%s: BSSCCA bcn_bpcc_op_lock fail!\n", __func__);

			bss_color->disabled = TRUE;

			/* update state on firmware side */
			deliver_bss_color(pAd, bssinfo, bss_color);

			/* update wlan_operation state */
			wlan_operate_set_he_bss_color(wdev, bss_color->color, TRUE);
			if (update_param.BSSCAUpdateReason == RSN_CFG80211_PATH
				|| update_param.BSSCAUpdateReason == RSN_DBG_PATH) {
				bss_color->next_color = update_param.next_color;
			} else if (update_param.BSSCAUpdateReason == RSN_COLLISION_DETECT) {
				UINT8 next_color = 0, band_idx = 0;

				band_idx = HcGetBandByWdev(wdev);
				/*If manual bss color configured update the configured bss color */
				if (IS_BSS_COLOR_MANUAL_ACTIVE(pAd, band_idx)) {

					if (!pAd->ApCfg.bss_color_cfg.bss_color_next) {
						if (bss_color_acquire(wdev, &next_color) == FALSE) {
							/* error handling to add */
							break;
						}
						MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO, "Got new color after collision:%d\n", next_color);
						pAd->ApCfg.bss_color_cfg.bss_color_next = next_color;
						pAd->ApCfg.bss_color_cfg.bss_color_enable = next_color;
						bss_color->next_color = next_color;
						update_param.next_color = next_color;
					} else {
						MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO, "Update color to MBSS(%d):%d\n",
								wdev->wdev_idx, pAd->ApCfg.bss_color_cfg.bss_color_next);
						bss_color->next_color = pAd->ApCfg.bss_color_cfg.bss_color_next;
						update_param.next_color = pAd->ApCfg.bss_color_cfg.bss_color_next;
					}
				} else {
					/* acquire next BSS color which is used in BSS Color Change
					* Announcement IE and update it to wlan_opertaion module
					*/
					if (bss_color_acquire(wdev, &next_color) == FALSE) {
						/* error handling to add */
						break;
					}
					bss_color->next_color = next_color;
					update_param.next_color = next_color;
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO, "Got new color after collision:%d\n", next_color);
				}
			}
			wlan_operate_set_he_bss_next_color(wdev, update_param.next_color, update_param.count);

			UpdateBeaconHandler_BPCC(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG),
								BCN_BPCC_BSSCCA, TRUE);
#ifdef DOT11_EHT_BE
			if (bss_mngr_is_wdev_in_mlo_group(wdev))
				bss_mngr_mld_sync_ml_probe_rsp(wdev);
#endif/*DOT11_EHT_BE*/
#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
			if (update_param.BSSCAUpdateReason == RSN_COLLISION_DETECT) {
				/* Notify SR module */
				if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd))
					SrMeshSelfSrgBMChangeEvent(pAd, wdev, FALSE);
			}
#endif
		}
		break;
	case BCN_BPCC_VHTOP:
		{
			if (bcn_bpcc_op_lock(pAd, wdev, TRUE, switch_type) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
					"%s: VHTOP bcn_bpcc_op_lock fail!\n", __func__);
			}
			OS_SEM_LOCK(&wdev->bpcc_vhtop_info.vht_opinfo_bpcc_lock);
			wdev->bpcc_vhtop_info.status = VHTOP_INFO_UPDATE_SUC;
			OS_SEM_UNLOCK(&wdev->bpcc_vhtop_info.vht_opinfo_bpcc_lock);
			UpdateBeaconHandler_BPCC(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG), switch_type, TRUE);
		}
		break;
	case BCN_BPCC_LEGACY_BCN:
	case BCN_BPCC_EHTOP:
		{
			UINT32 reason;

			memcpy(&reason, &prMtUpdateBeacon->content[0], sizeof(UINT32));
			if (switch_type == BCN_BPCC_EHTOP) {
				if (bcn_bpcc_op_lock(pAd, wdev, TRUE, switch_type) == FALSE)
					MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
						"EHTOP bcn_bpcc_op_lock fail!\n");
				UpdateBeaconHandler_BPCC(pAd, wdev, BCN_REASON(reason), BCN_BPCC_EHTOP, TRUE);
			} else {
				if (bcn_bpcc_op_lock(pAd, wdev, FALSE, switch_type) == FALSE)
					MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
						"LEGACY_BCN bcn_bpcc_op_lock fail!\n");
				UpdateBeaconHandler_BPCC(pAd, wdev, BCN_REASON(reason), BCN_BPCC_LEGACY_BCN, TRUE);
			}
		}
		break;
	case BCN_BPCC_CSA:
		{
			BOOLEAN ret = TRUE;

			if (bcn_bpcc_op_lock(pAd, wdev, TRUE, BCN_BPCC_CSA) == TRUE) {
				if (!prepare_bpcc_csa(pAd))
					bcn_bpcc_op_unlock(pAd, wdev, TRUE);
			} else {
				ret = prepare_bpcc_csa(pAd);
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_NOTICE,
					"%s: bcn_bpcc_op_lock fail, ret=%d\n", __func__, ret);
			}
		}
		break;
	default:
		break;
	}
	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
		"\x1b[43m <---\x1b[m\n\r");


	return NDIS_STATUS_SUCCESS;
}

VOID bcn_bpcc_ct_switch(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 ie_type, ...)
{
	UINT32 ret;
	struct BPCC_BEACON_CT_SWITCH rMtUpdateBeacon = {0};
	va_list Args;

	if (!wdev)
		return;

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_DEBUG,
		"\x1b[43m --->(%s)(%d)\x1b[m\n\r", RtmpOsGetNetDevName(wdev->if_dev), ie_type);
	rMtUpdateBeacon.wdev = wdev;
	rMtUpdateBeacon.switch_type = ie_type;
	switch (ie_type) {
	case BCN_BPCC_EDCA:
		break;
	case BCN_BPCC_HEOP:
		{
			UINT32 temp;

			va_start(Args, ie_type);
			temp = va_arg(Args, UINT32);
			va_end(Args);
			memcpy(rMtUpdateBeacon.content, &temp, sizeof(UINT32));
			rMtUpdateBeacon.UpdateReason = (UINT32 *)&rMtUpdateBeacon.content[0];
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				"BCN_BPCC_HEOP: wdev(%d)(%d), Update reason = %d\n\r",
				wdev->wdev_idx, wdev->channel, *rMtUpdateBeacon.UpdateReason);
		}
		break;
	case BCN_BPCC_BTWT:
	case BCN_BPCC_BTWTSET:
		break;
	case BCN_BPCC_MUEDCA:
		{
			UINT8 UpdateReason;
			struct _BCN_MUEDCA_UPDATE_PARAM temp;

			va_start(Args, ie_type);
			UpdateReason = (UINT8)va_arg(Args, UINT32);
			memcpy(rMtUpdateBeacon.content, &UpdateReason, sizeof(UINT8));
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				"wdev(%d)(%d), Update reason = %d\n\r",
				wdev->wdev_idx, wdev->channel, UpdateReason);

			temp = va_arg(Args, struct _BCN_MUEDCA_UPDATE_PARAM);
			memcpy(&(rMtUpdateBeacon.content[1]), &temp, sizeof(struct _BCN_MUEDCA_UPDATE_PARAM));
			va_end(Args);

			rMtUpdateBeacon.APMUEdcaParam = (struct _BCN_MUEDCA_UPDATE_PARAM *)&rMtUpdateBeacon.content[1];
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				"wdev(%d)(%d), MUEDCA bcn update reason = %d\n",
				wdev->wdev_idx, wdev->channel, rMtUpdateBeacon.APMUEdcaParam->MUEDCAUpdateReason);
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				"wdev(%d)(%d), APMUEdcaParam: value=%d, param=%d, aci=%d\n",
				wdev->wdev_idx, wdev->channel, rMtUpdateBeacon.APMUEdcaParam->value,
				rMtUpdateBeacon.APMUEdcaParam->param, rMtUpdateBeacon.APMUEdcaParam->aci);
		}
		break;
	case BCN_BPCC_BSSCCA:
		{
			UINT8 UpdateReason;
			struct _BCN_BSSCCA_UPDATE_PARAM temp;

			va_start(Args, ie_type);
			UpdateReason = (UINT8)va_arg(Args, UINT32);
			memcpy(rMtUpdateBeacon.content, &UpdateReason, sizeof(UINT8));
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				"wdev(%d)(%d), Update reason = %d\n\r",
				wdev->wdev_idx, wdev->channel, UpdateReason);
			temp = va_arg(Args, struct _BCN_BSSCCA_UPDATE_PARAM);
			memcpy(&(rMtUpdateBeacon.content[1]), &temp, sizeof(struct _BCN_BSSCCA_UPDATE_PARAM));
			va_end(Args);

			rMtUpdateBeacon.APBSSCCAParam = (struct _BCN_BSSCCA_UPDATE_PARAM *)&rMtUpdateBeacon.content[1];
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				"wdev(%d)(%d), BSSCCA bcn update reason = %d\n",
				wdev->wdev_idx, wdev->channel, rMtUpdateBeacon.APBSSCCAParam->BSSCAUpdateReason);
		}
		break;
	case BCN_BPCC_VHTOP:
	case BCN_BPCC_LEGACY_BCN:
	case BCN_BPCC_EHTOP:
		{
			UINT32 temp;

			va_start(Args, ie_type);
			temp = va_arg(Args, UINT32);
			va_end(Args);
			memcpy(rMtUpdateBeacon.content, &temp, sizeof(UINT32));
			rMtUpdateBeacon.UpdateReason = (UINT32 *)&rMtUpdateBeacon.content[0];
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				"wdev(%d)(%d), ie_type = %u, Update reason = %d\n\r",
				wdev->wdev_idx, wdev->channel, ie_type, *rMtUpdateBeacon.UpdateReason);
		}
		break;
	case BCN_BPCC_CSA:
		{
			UINT32 temp;

			va_start(Args, ie_type);
			temp = va_arg(Args, UINT32);
			va_end(Args);
			memcpy(rMtUpdateBeacon.content, &temp, sizeof(UINT32));
			rMtUpdateBeacon.UpdateReason = (UINT32 *)&rMtUpdateBeacon.content[0];
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
				"wdev(%d)(%d), ie_type = %u, Update reason = %d\n\r",
				wdev->wdev_idx, wdev->channel, ie_type, *rMtUpdateBeacon.UpdateReason);
		}
		break;
	default:
		break;
	}
	ret = _bcn_bpcc_RTEnqueueInternalCmd(pAd, BPCC_CMDTHREAD_BPCC_CT_SWITCH,
		&rMtUpdateBeacon, sizeof(struct BPCC_BEACON_CT_SWITCH));

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_DEBUG,
		"\x1b[43m <---ret=%d\x1b[m\n\r", ret);
}

INT bcn_bpcc_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
		"\x1b[42m Init BPCC protection mechanism,wdev_idx(%s),init=%d\x1b[m\n\r",
		RtmpOsGetNetDevName(wdev->if_dev), wdev->BcnBPCC_Init);

	if (wdev->BcnBPCC_Init) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
			"\x1b[42m (%s)already init\x1b[m\n\r", RtmpOsGetNetDevName(wdev->if_dev));
		return Status;
	}
	NdisAllocateSpinLock(pAd, &wdev->BcnBPCCLock);
	OS_SEM_LOCK(&wdev->BcnBPCCLock);
	wdev->BcnBPCC_op_lock_flag = FALSE;
	wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt = 0;
	memset(&wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_INT[0], 0x00, sizeof(wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_INT));
	memset(&wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_CT[0], 0x00, sizeof(wdev->BcnBPCC_op_lock_by_bssmngr_fail_cnt_per_IE_CT));
	memset(&wdev->BcnBPCC_TO_cnt_per_IE[0], 0x00, sizeof(wdev->BcnBPCC_TO_cnt_per_IE));
	wdev->BcnBPCC_op_lock_refine_NG_per_IE_bitmap = 0x0;
	wdev->BcnBPCC_op_lock_by_legacy_fail_cnt = 0;
	wdev->BcnBPCC_TO_cnt = 0;
	wdev->BcnBPCC_hwcmd_max = 0;
	wdev->BcnBPCC_OP_Timer_TT_start = 0;
	wdev->BcnBPCC_op_lock_TT_start = 0;
	wdev->BcnBPCC_op_lock_TT_end = 0;
	wdev->BcnBPCC_op_lock_max = 0;
	wdev->BcnBPCC_op_lock_avg = 0;
	wdev->BcnBPCC_op_lock_get_mld_max = 0;
	wdev->BcnBPCC_op_lock_get_mld_avg = 0;
	wdev->BcnBPCC_op_lock_get_legacy_max = 0;
	wdev->BcnBPCC_op_lock_get_legacy_avg = 0;
	wdev->BcnBPCC_op_lock_get_fail_mld_max = 0;
	wdev->BcnBPCC_op_lock_get_fail_mld_avg = 0;
	wdev->BcnBPCC_op_lock_get_fail_legacy_max = 0;
	wdev->BcnBPCC_op_lock_get_fail_legacy_avg = 0;
	wdev->BcnBPCC_OP_Timer_TT_cancel = 0;
	wdev->BcnBPCC_trigger_wdev = NULL;
	wdev->BcnBPCC_trigger_flag = FALSE;
	wdev->BcnBPCC_UpdScope = 0;
	wdev->BcnBPCCIeType_bitmap = 0;
	wdev->BcnBPCC_protect_disable = FALSE;
	wdev->BcnBPCC_protect_dynctrl = FALSE;
	wdev->BcnBPCC_protect_dynctrl_cnt = 0;
	wdev->BcnBPCC_protect_manual = FALSE;
	NdisZeroMemory(wdev->dbg_BcnBPCC_op_lock_caller, sizeof(wdev->dbg_BcnBPCC_op_lock_caller));
	NdisZeroMemory(&wdev->BcnBPCCIeType_bitmap_hist[0], sizeof(wdev->BcnBPCCIeType_bitmap_hist));
	NdisZeroMemory(&wdev->BcnBPCCIeType_bitmap_hist_TT[0], sizeof(wdev->BcnBPCCIeType_bitmap_hist_TT));
	NdisZeroMemory(&wdev->BcnBPCCIeType_cnt_hist[0], sizeof(wdev->BcnBPCCIeType_cnt_hist));
	wdev->BcnBPCCIeType_bitmap_hist_idx = 0;
	RTMP_GetCurrentSystemTick(&wdev->BcnBPCC_Init_TT);
	wdev->BcnBPCC_log_on = FALSE;
	wdev->BcnBPCC_mld_grp_id = 0;
	NdisZeroMemory(&wdev->BcnBPCC_MLD_log_flag[0], sizeof(wdev->BcnBPCC_MLD_log_flag));
	OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
	RTMPInitTimer(pAd, &wdev->BcnBPCC_OP_Timer, GET_TIMER_FUNCTION(_bcn_bpcc_op_lock_to), wdev, FALSE);
	RTMPInitTimer(pAd, &wdev->BcnBPCC_Trigger_Timer, GET_TIMER_FUNCTION(_bcn_bpcc_trigger), wdev, FALSE);
	RTMP_SEM_EVENT_INIT(&wdev->BcnBPCC_op_lock, &pAd->RscSemMemList);
	if (pAd->ApCfg.BssidNum >= 8) {
		bpcc_op_lock_max_time_adjust = BCN_BPCC_OP_LOCK_MAX_TIME << 1;
		bpcc_op_lock_max_to_adjust = BCN_BPCC_OP_TO_MAX_TIME << 1;
	} else {
		if (bpcc_op_lock_max_time_adjust < (BCN_BPCC_OP_LOCK_MAX_TIME << 1))
			bpcc_op_lock_max_time_adjust = BCN_BPCC_OP_LOCK_MAX_TIME;
		if (bpcc_op_lock_max_to_adjust < (BCN_BPCC_OP_TO_MAX_TIME << 1))
			bpcc_op_lock_max_to_adjust = BCN_BPCC_OP_TO_MAX_TIME;
	}
	NdisAllocateSpinLock(pAd, &wdev->bpcc_vhtop_info.vht_opinfo_bpcc_lock);
	OS_SEM_LOCK(&wdev->bpcc_vhtop_info.vht_opinfo_bpcc_lock);
	wdev->bpcc_vhtop_info.status = VHTOP_INFO_UPDATE_SUC;
	NdisZeroMemory(&wdev->bpcc_vhtop_info.vht_opinfo_orig, sizeof(struct vht_opinfo));
	OS_SEM_UNLOCK(&wdev->bpcc_vhtop_info.vht_opinfo_bpcc_lock);
	wdev->BcnBPCC_Init = TRUE;

	return Status;
}

INT bcn_bpcc_deinit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	BOOLEAN Cancelled;

	if (wdev->BcnBPCC_Init) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
			"\x1b[42m Deinit ,wdev_idx(%s),init=%d \x1b[m\n\r", RtmpOsGetNetDevName(wdev->if_dev),
			wdev->BcnBPCC_Init);
		OS_SEM_LOCK(&wdev->BcnBPCCLock);
		wdev->BcnBPCC_trigger_wdev = NULL;
		OS_SEM_UNLOCK(&wdev->BcnBPCCLock);
		RTMPReleaseTimer(&wdev->BcnBPCC_Trigger_Timer, &Cancelled);
		RTMPReleaseTimer(&wdev->BcnBPCC_OP_Timer, &Cancelled);
		RTMP_SEM_EVENT_DESTORY(&wifi_dev->BcnBPCC_op_lock);
		NdisFreeSpinLock(&wdev->BcnBPCCLock);
		NdisFreeSpinLock(&wdev->bpcc_vhtop_info.vht_opinfo_bpcc_lock);
		wdev->BcnBPCC_Init = FALSE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
			"\x1b[42m (%s)already deinit\x1b[m\n\r", RtmpOsGetNetDevName(wdev->if_dev));
	}

	return Status;
}

static NDIS_STATUS _bcn_bpcc_RTEnqueueInternalCmd(
	IN PRTMP_ADAPTER	pAd,
	IN NDIS_OID			Oid,
	IN PVOID			pInformationBuffer,
	IN UINT32			InformationBufferLength)
{
	NDIS_STATUS	status;
	ULONG	flag = 0;
	PCmdQElmt	cmdqelmt = NULL;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
			"---> NIC is not exist!!\n");
		return NDIS_STATUS_FAILURE;
	}

	status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt, sizeof(CmdQElmt));

	if ((status != NDIS_STATUS_SUCCESS) || (cmdqelmt == NULL))
		return NDIS_STATUS_RESOURCES;

	NdisZeroMemory(cmdqelmt, sizeof(CmdQElmt));

	if (InformationBufferLength > 0) {
		status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt->buffer, InformationBufferLength);

		if ((status != NDIS_STATUS_SUCCESS) || (cmdqelmt->buffer == NULL)) {
			os_free_mem(cmdqelmt);
			return NDIS_STATUS_RESOURCES;
		}
		NdisMoveMemory(cmdqelmt->buffer, pInformationBuffer, InformationBufferLength);
		cmdqelmt->bufferlength = InformationBufferLength;
	} else {
		cmdqelmt->buffer = NULL;
		cmdqelmt->bufferlength = 0;
	}

	cmdqelmt->command = Oid;
	cmdqelmt->CmdFromNdis = FALSE;

	if (cmdqelmt != NULL) {
		RTMP_SPIN_LOCK_IRQSAVE(&pAd->BPCC_CmdQLock, &flag);

		if ((pAd->BPCC_CmdQ.size < MAX_LEN_OF_CMD_QUEUE) &&
			(pAd->BPCC_CmdQ.CmdQState & RTMP_TASK_CAN_DO_INSERT)) {
#ifdef DBG_STARVATION
			starv_dbg_init(&pAd->BPCC_CmdQ.block, &cmdqelmt->starv);
			starv_dbg_get(&cmdqelmt->starv);
#endif /*DBG_STARVATION*/
			EnqueueCmd((&pAd->BPCC_CmdQ), cmdqelmt);
			status = NDIS_STATUS_SUCCESS;
		} else
			status = NDIS_STATUS_FAILURE;

		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->BPCC_CmdQLock, &flag);

		if (status == NDIS_STATUS_FAILURE) {
			if (cmdqelmt->buffer)
				os_free_mem(cmdqelmt->buffer);

			os_free_mem(cmdqelmt);
		} else
			RTCMDUp(&pAd->BPCC_cmdQTask);
	}

	return status;
}

static inline BCN_BPCC_CMDHdlr _bcn_bpcc_ValidCMD(IN PCmdQElmt CMDQelmt)
{
	SHORT CMDIndex = CMDQelmt->command;
	SHORT CurIndex = 0;
	UINT32 CMDHdlrTableLength = sizeof(_bcn_bpcc_CMDHdlrTable) / sizeof(struct MT_BCN_BPCC_CMD_TABL_T);
	BCN_BPCC_CMDHdlr Handler = NULL;

	if (CMDIndex > BPCC_CMDTHREAD_END_CMD_ID) {
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
			"CMD(%x) is out of boundary\n", CMDQelmt->command);
		return NULL;
	}

	for (CurIndex = 0; CurIndex < CMDHdlrTableLength; CurIndex++) {
		if (_bcn_bpcc_CMDHdlrTable[CurIndex].CmdID == CMDIndex) {
			Handler = _bcn_bpcc_CMDHdlrTable[CurIndex].CmdHdlr;
			break;
		}
	}

	if (Handler == NULL)
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
			"No corresponding CMDHdlr for this CMD(%x)\n",  CMDQelmt->command);

	return Handler;
}

static VOID _bcn_bpcc_CMDHandler(RTMP_ADAPTER *pAd)
{
	PCmdQElmt		cmdqelmt;
	NTSTATUS		ntStatus;
	BCN_BPCC_CMDHdlr		Handler = NULL;
	UINT32		process_cnt = 0;
	UINT		cmdq_size = 0;

	if (!pAd) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"pAd is NULL.\n");
		return;
	}
	NdisAcquireSpinLock(&pAd->BPCC_CmdQLock);
	cmdq_size = pAd->BPCC_CmdQ.size;
	NdisReleaseSpinLock(&pAd->BPCC_CmdQLock);
	while (cmdq_size > 0) {

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
				"System halted, exit CMDHandler!(CmdQ.size = %d)\n",
				cmdq_size);
			break;
		}

		/* For worst case, avoid process CmdQ too long which cause RCU_sched stall */
		process_cnt++;
		if ((!in_interrupt()) && (process_cnt >= CMD_QUEUE_SCH)) {/*process_cnt-16*/
			process_cnt = 0;
			OS_SCHEDULE();
		}

		NdisAcquireSpinLock(&pAd->BPCC_CmdQLock);
		RTThreadDequeueCmd(&pAd->BPCC_CmdQ, &cmdqelmt);
		cmdq_size = pAd->BPCC_CmdQ.size;
		NdisReleaseSpinLock(&pAd->BPCC_CmdQLock);

		if (cmdqelmt == NULL)
			break;


		Handler = _bcn_bpcc_ValidCMD(cmdqelmt);

		if (Handler)
			ntStatus = Handler(pAd, cmdqelmt);

#ifdef DBG_STARVATION
		starv_dbg_put(&cmdqelmt->starv);
#endif /*DBG_STARVATION*/

		if (cmdqelmt->CmdFromNdis == TRUE) {
			if (cmdqelmt->buffer != NULL)
				os_free_mem(cmdqelmt->buffer);

			os_free_mem(cmdqelmt);
		} else {
			if ((cmdqelmt->buffer != NULL) && (cmdqelmt->bufferlength != 0))
				os_free_mem(cmdqelmt->buffer);

			os_free_mem(cmdqelmt);
		}
	}	/* end of while */
}

static void _bcn_bpcc_CmdQExit(RTMP_ADAPTER *pAd)
{
#ifdef DBG_STARVATION
	unregister_starv_block(&pAd->BPCC_CmdQ.block);
#endif /*DBG_STARVATION*/
}

static void _bcn_bpcc_CmdQInit(RTMP_ADAPTER *pAd)
{
	/* Init the CmdQ and CmdQLock*/
	NdisAllocateSpinLock(pAd, &pAd->BPCC_CmdQLock);
	NdisAcquireSpinLock(&pAd->BPCC_CmdQLock);
	RTInitializeCmdQ(&pAd->BPCC_CmdQ);
	NdisReleaseSpinLock(&pAd->BPCC_CmdQLock);
#ifdef DBG_STARVATION
	cmdq_starv_block_init(&pAd->starv_log_ctrl, &pAd->BPCC_CmdQLock);
#endif /*DBG_STARVATION*/
}

/*
*
*/
static INT _bcn_bpcc_cmd_thread(ULONG context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *pTask;
	int status;
	UINT32 CmdQState;

	status = 0;
	pTask = (RTMP_OS_TASK *)context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);
	RtmpOSTaskCustomize(pTask);
	NdisAcquireSpinLock(&pAd->BPCC_CmdQLock);
	pAd->BPCC_CmdQ.CmdQState = RTMP_TASK_STAT_RUNNING;
	CmdQState = pAd->BPCC_CmdQ.CmdQState;
	NdisReleaseSpinLock(&pAd->BPCC_CmdQLock);
	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO, "\x1b[43m--->\x1b[m\n\r\n");

	while (CmdQState == RTMP_TASK_STAT_RUNNING) {
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		NdisAcquireSpinLock(&pAd->BPCC_CmdQLock);
		if (pAd->BPCC_CmdQ.CmdQState == RTMP_TASK_STAT_STOPED) {
			NdisReleaseSpinLock(&pAd->BPCC_CmdQLock);
			break;
		}
		CmdQState = pAd->BPCC_CmdQ.CmdQState;
		NdisReleaseSpinLock(&pAd->BPCC_CmdQLock);

		if (!pAd->PM_FlgSuspend)
			_bcn_bpcc_CMDHandler(pAd);
	}

	if (!pAd->PM_FlgSuspend) {
		/* Clear the CmdQElements. */
		CmdQElmt	*pCmdQElmt = NULL;

		NdisAcquireSpinLock(&pAd->BPCC_CmdQLock);
		pAd->BPCC_CmdQ.CmdQState = RTMP_TASK_STAT_STOPED;

		while (pAd->BPCC_CmdQ.size) {
			RTThreadDequeueCmd(&pAd->BPCC_CmdQ, &pCmdQElmt);

			if (pCmdQElmt) {
				if (pCmdQElmt->CmdFromNdis == TRUE) {
					if (pCmdQElmt->buffer != NULL)
						os_free_mem(pCmdQElmt->buffer);

					os_free_mem((PUCHAR)pCmdQElmt);
				} else {
					if ((pCmdQElmt->buffer != NULL) && (pCmdQElmt->bufferlength != 0))
						os_free_mem(pCmdQElmt->buffer);

					os_free_mem((PUCHAR)pCmdQElmt);
				}
			}
		}

		NdisReleaseSpinLock(&pAd->BPCC_CmdQLock);
	}

	/* notify the exit routine that we're actually exiting now
	 *
	 * complete()/wait_for_completion() is similar to up()/down(),
	 * except that complete() is safe in the case where the structure
	 * is getting deleted in a parallel mode of execution (i.e. just
	 * after the down() -- that's necessary for the thread-shutdown
	 * case.
	 *
	 * complete_and_exit() goes even further than this -- it is safe in
	 * the case that the thread of the caller is going away (not just
	 * the structure) -- this is necessary for the module-remove case.
	 * This is important in preemption kernels, which transfer the flow
	 * of execution immediately upon a complete().
	 */
	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO, "\x1b[43m<---\x1b[m\n\r\n");
	RtmpOSTaskNotifyToExit(pTask);

	return 0;
}

VOID bcn_bpcc_TaskExit(
	IN RTMP_ADAPTER *pAd)
{
	INT			ret;
	RTMP_OS_TASK	*pTask;

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO, "\n pAd=%p\n", pAd);

	_bcn_bpcc_CmdQExit(pAd);
	/* Terminate cmdQ thread */
	pTask = &pAd->BPCC_cmdQTask;
	if (RTMP_OS_TASK_LEGALITY(pTask)) {
		NdisAcquireSpinLock(&pAd->BPCC_CmdQLock);
		pAd->BPCC_CmdQ.CmdQState = RTMP_TASK_STAT_STOPED;
		NdisReleaseSpinLock(&pAd->BPCC_CmdQLock);
		ret = RtmpOSTaskKill(pTask);

		if (ret == NDIS_STATUS_FAILURE)
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
				"kill command task failed!\n");

		NdisAcquireSpinLock(&pAd->BPCC_CmdQLock);
		pAd->BPCC_CmdQ.CmdQState = RTMP_TASK_STAT_UNKNOWN;
		NdisReleaseSpinLock(&pAd->BPCC_CmdQLock);
	}

	_bcn_bpcc_deinit_all(pAd);
}

NDIS_STATUS	 bcn_bpcc_TaskInit(
	IN RTMP_ADAPTER *pAd)
{
	RTMP_OS_TASK *pTask;
	NDIS_STATUS status;
	char task_name[TASK_NAME_LEN] = {0};
	int ret = 0;
	u8 band_idx = hc_get_hw_band_idx(pAd);

	if (band_idx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
			"invalid band index(=%d)\n", band_idx);
		return NDIS_STATUS_FAILURE;
	}

	/* WCNCR00034259: init CmdQ resources before run thread */
	_bcn_bpcc_CmdQInit(pAd);
	/* Creat Command Thread */
	pTask = &pAd->BPCC_cmdQTask;
	os_zero_mem(task_name, TASK_NAME_LEN);
	ret = snprintf(task_name, TASK_NAME_LEN,
		"BPCCCmdQTask_%d%d", PD_GET_DEVICE_IDX(pAd->physical_dev), band_idx);
	if (os_snprintf_error(TASK_NAME_LEN, ret))
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
			"(line.%d) snprintf error!\n", __LINE__);
	MTWF_PRINT("%s(%d): task_name is %s\n", __func__, __LINE__, task_name);
	RTMP_OS_TASK_INIT(pTask, task_name, pAd);
	status = RtmpOSTaskAttach(pTask, _bcn_bpcc_cmd_thread, (ULONG)pTask);

	if (status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_ERROR,
				 KERN_WARNING "%s: unable to start BPCCCmdQThread\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
		return NDIS_STATUS_FAILURE;
	}

	return status;
}

BOOLEAN UpdateBeaconProc(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BOOLEAN UpdateRoutine,
	UCHAR UpdatePktType,
	BOOLEAN bMakeBeacon,
	UCHAR UpdateReason)
{
	BCN_BUF_STRUCT *pbcn_buf = NULL;

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			 "bMakeBeacon: %d\n", bMakeBeacon);

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				 "no wdev\n");
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11V_MBSSID_SUPPORT
	wdev = mbss_11v_get_tx_wdev(pAd, wdev, &pAd->ApCfg.MBSSID[wdev->func_idx], &bMakeBeacon);
#endif /* DOT11V_MBSSID_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	if ((WDEV_BSS_STATE(wdev) < BSS_READY) || !WDEV_WITH_BCN_ABILITY(wdev)) {
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				 "wdev(%d) bss not ready (%d) or not required (%d) !!\n",
				 wdev->wdev_idx, WDEV_BSS_STATE(wdev), wdev->wdev_type);
		return FALSE;
	}

	pbcn_buf = &wdev->bcn_buf;

	if (BeaconTransmitRequired(pAd, wdev) == FALSE) {
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			"wdev(%d) NO BeaconTransmitRequired\n",
			wdev->wdev_idx);
		/* change bcn update function to ensure bcn can disable Normally. */
		AsicUpdateBeacon(pAd, wdev, FALSE, UpdateReason);
		return FALSE;
	}

	if (bMakeBeacon)
		pbcn_buf->FrameLen = MakeBeacon(pAd, wdev, UpdateRoutine);

#ifdef CONFIG_6G_SUPPORT
	if (WMODE_CAP_6G(wdev->PhyMode)) {
		ap_6g_build_discovery_frame(pAd, wdev);
		ap_6g_set_frame_offload(pAd, wdev);
	}
#endif

	/* set Beacon to Asic/Mcu */
	/* if Beacon offload, FW will help to pause BcnQ */
	AsicUpdateBeacon(pAd, wdev, TRUE, UpdateReason);

	return TRUE;
}

INT BcnTimAdd(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *ptr)
{
	INT tim_len = 0;
	UCHAR TimFirst, TimLast;

	*ptr = IE_TIM;
#ifdef MBSS_DTIM_SUPPORT
	*(ptr + 2) = pAd->ApCfg.MBSSID[wdev->func_idx].DtimCount;
	*(ptr + 3) = pAd->ApCfg.MBSSID[wdev->func_idx].DtimPeriod;
#else
	*(ptr + 2) = pAd->ApCfg.DtimCount;
	*(ptr + 3) = pAd->ApCfg.DtimPeriod;
#endif

	/* find the smallest AID (PS mode) */
	TimFirst = 0; /* record first TIM byte != 0x00 */
	TimLast = 0;  /* record last  TIM byte != 0x00 */

	*(ptr + 1) = 3 + (TimLast - TimFirst + 1); /* TIM IE length */
	*(ptr + 4) = TimFirst;

	/* adjust BEACON length according to the new TIM */
	tim_len = (2 + *(ptr + 1));
	return tim_len;
}

ULONG ComposeBcnPktHead(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pBeaconFrame)
{
	ULONG FrameLen = 0;
	ULONG TmpLen;
	UCHAR DsLen = 1, SsidLen = 0;
	HEADER_802_11 BcnHdr;
	LARGE_INTEGER FakeTimestamp;
	USHORT PhyMode;
	BSS_STRUCT *pMbss = NULL;
	struct DOT11_H *pDot11h = NULL;
	/* INT apidx = wdev->func_idx; */
	UCHAR *Addr2 = NULL, *Addr3 = NULL, *pSsid = NULL;
	USHORT CapabilityInfo, *pCapabilityInfo = &CapabilityInfo;
	BOOLEAN ess = FALSE;
#ifdef CONFIG_STA_SUPPORT
	UCHAR IbssLen = 2;
	BOOLEAN Privacy, ibss = FALSE, need_ibss_ie = FALSE;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
#endif /* CONFIG_STA_SUPPORT */
	struct legacy_rate *rate;
	UCHAR Channel;
	UCHAR DefaultAddr[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	if (wdev == NULL)
		return FALSE;

	PhyMode = wdev->PhyMode;
	Channel = wdev->channel;
	Addr2 = Addr3 = pSsid = DefaultAddr;

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		pMbss = wdev->func_dev;

		if (pMbss == NULL)
			return FALSE;

		SsidLen = (pMbss->bHideSsid) ? 0 : pMbss->SsidLen;
		Addr2 = wdev->if_addr;
		Addr3 = wdev->bssid;
		pSsid = pMbss->Ssid;
		ess = TRUE;
		pCapabilityInfo = &pMbss->CapabilityInfo;

		/*for 802.11H in Switch mode should take current channel*/
		pDot11h = wdev->pDot11_H;
		if (pDot11h == NULL)
			return FALSE;
		if (pAd->CommonCfg.bIEEE80211H && pDot11h->ChannelMode == CHAN_SWITCHING_MODE)
			Channel = (pDot11h->org_ch != 0) ? pDot11h->org_ch : Channel;
	}
#ifdef CONFIG_STA_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_STA) {
		pStaCfg = GetStaCfgByWdev(pAd, wdev);

		if (pStaCfg == NULL)
			return FALSE;

		SsidLen = pStaCfg->SsidLen;
		Addr2 = wdev->if_addr;
		Addr3 = pStaCfg->Bssid;
		pSsid = pStaCfg->Ssid;
		ibss = TRUE;
		need_ibss_ie = TRUE;
		Privacy = IS_SECURITY(&wdev->SecConfig);
		CapabilityInfo = CAP_GENERATE(
							 ess, ibss, Privacy,
							 (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1),
							 FALSE, FALSE);
		/*TODO: Carter, I think the capability of Adhoc could be decided in earlier,
			why it is decided here?? Whose masterpiece?
		*/
	}

#endif
	MgtMacHeaderInit(pAd,
					 &BcnHdr,
					 SUBTYPE_BEACON,
					 0,
					 BROADCAST_ADDR,
					 Addr2,
					 Addr3);
	MakeOutgoingFrame(
		pBeaconFrame,           &FrameLen,
		sizeof(HEADER_802_11),  &BcnHdr,
		TIMESTAMP_LEN,          &FakeTimestamp,
		2,                      &pAd->CommonCfg.BeaconPeriod,
		2,                      pCapabilityInfo,
		1,                      &SsidIe,
		1,                      &SsidLen,
		SsidLen,                pSsid,
		END_OF_ARGS);
	/*
	  if wdev is AP, SupRateLen is global setting,
	  shall check each's wdev setting to update SupportedRate.
	*/
	rate = &wdev->rate.legacy_rate;

	FrameLen += build_support_rate_ie(wdev, rate->sup_rate, rate->sup_rate_len, pBeaconFrame + FrameLen);
	TmpLen = 0;

	if (!WMODE_CAP_AX_6G(wdev->PhyMode)) {
		MakeOutgoingFrame(pBeaconFrame + FrameLen,        &TmpLen,
						  1,                              &DsIe,
						  1,                              &DsLen,
						  1,                              &Channel,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (!pStaCfg)
			pStaCfg = GetStaCfgByWdev(pAd, wdev);

		if (need_ibss_ie == TRUE) {
			TmpLen = 0;
			MakeOutgoingFrame(pBeaconFrame + FrameLen,        &TmpLen,
							  1,                              &IbssIe,
							  1,                              &IbssLen,
							  2,                              &pStaCfg->StaActive.AtimWin,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}
#endif
	return FrameLen;
}

static BOOLEAN is_beacon_active(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	UINT32 index;
	struct wifi_dev *wdev;
	BOOLEAN bcnactive = FALSE;

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY) ||
		RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)))
		return FALSE;

#ifdef ERR_RECOVERY
	if (IsStopingPdma(&pAd->ErrRecoveryCtl))
		return FALSE;
#endif

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (pAd->CommonCfg.bOverlapScanning)
		return FALSE;
#endif
#endif

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
		return FALSE;

	for (index = 0; index < PD_GET_BAND_NUM(pAd->physical_dev); index++) {
		RTMP_ADAPTER *ad = NULL;

		ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, index);
		if (ad && ATE_ON(ad))
			return FALSE;
	}
#endif

#ifdef MT_DFS_SUPPORT
	if (pAd->Dot11_H.ChannelMode != CHAN_NORMAL_MODE)
		return FALSE;
#endif

#ifdef WDS_SUPPORT
	/*
	 * WDS is bound on main wifi dev which should not issue Beacons
	 * when system operates as bridge mode
	 */
	if (pAd->WdsTab.Mode == WDS_BRIDGE_MODE)
		return FALSE;
#endif/* WDS_SUPPORT */

	for (index = 0; index < WDEV_NUM_MAX; index++) {
		wdev = pAd->wdev_list[index];
		if (wdev == NULL)
			continue;
		if (wdev->bcn_buf.stop_tx)
			continue;
		if (HcIsRadioAcq(wdev)
			&& (!IsHcRadioCurStatOffByWdev(wdev))
			&& (HcGetBandByWdev(wdev) == BandIdx)
			&& (WDEV_BSS_STATE(wdev) == BSS_READY)
			&& (wdev->bcn_buf.bBcnSntReq)) {
			bcnactive = TRUE;
			break;
		}
	}

	return bcnactive;
}

#define BCN_CHECK_PERIOD		50 /* 5s */
#define PRE_BCN_CHECK_PERIOD	25 /* 2.5s */

VOID BcnCheck(RTMP_ADAPTER *pAd)
{
	UCHAR bandidx;
	ULONG PeriodicRound = pAd->Mlme.PeriodicRound;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	/* In Slave mode can trigger false bcn loss on BCN disable so return */
#ifdef DFS_SLAVE_SUPPORT
	if (SLAVE_MODE_EN(pAd) && SLAVE_BEACON_STOPPED(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"[DFS-SLAVE][%s]!!!Slave mode on, return!!!\n", __func__);
		return;
	}
#endif /* DFS_SLAVE_SUPPORT */

	if ((PeriodicRound % PRE_BCN_CHECK_PERIOD) == 0) {
		PBCN_CHECK_INFO_STRUC pBcnCheckInfo = &pAd->BcnCheckInfo;
		UINT32 *nobcncnt, *prebcncnt, *totalbcncnt;
		UINT32 bcn_cnt = 0;
		UINT32 recoverext = 0;
		UINT32 bcnTimeoutPeriod = 5;

		bandidx = hc_get_hw_band_idx(pAd);
		if (is_beacon_active(pAd, bandidx) == FALSE)
			return;

		/* start checking after a while (5s) to avoid nobcn false alarm */
		if (PeriodicRound < (pBcnCheckInfo->BcnInitedRnd + BCN_CHECK_PERIOD)) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_WARN,
				"start after %ld00 ms (%s)\n",
				(pBcnCheckInfo->BcnInitedRnd + BCN_CHECK_PERIOD) - PeriodicRound,
				RtmpOsGetNetDevName(pAd->net_dev));
			return;
		}

		nobcncnt = &pBcnCheckInfo->nobcncnt;
		prebcncnt = &pBcnCheckInfo->prebcncnt;
		totalbcncnt = &pBcnCheckInfo->totalbcncnt;

		NdisAcquireSpinLock(&pAd->physical_dev->bcn_check_lock);
		bcn_cnt = asic_get_bcn_tx_cnt(pAd, bandidx);
		NdisReleaseSpinLock(&pAd->physical_dev->bcn_check_lock);
		*totalbcncnt += bcn_cnt;	/* Save total bcn count for MibInfo query */

		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
			 "bcn_cnt = %d (%s)\n",
			 bcn_cnt, RtmpOsGetNetDevName(pAd->net_dev));
		if ((PeriodicRound % BCN_CHECK_PERIOD) == 0) {
			bcn_cnt += *prebcncnt;
			*prebcncnt = 0;
		} else {
			*prebcncnt = bcn_cnt;
			return;
		}


		if (bcn_cnt == 0) {
			(*nobcncnt)++;

			if (*nobcncnt >= bcnTimeoutPeriod) {
				if (*nobcncnt % (bcnTimeoutPeriod+1) == 0) /* 6*5=30s */
					MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_WARN,
							 "nobcn still occur within %d sec for band %d (%s)!!\n",
							  (*nobcncnt) * 5, bandidx,
							  RtmpOsGetNetDevName(pAd->net_dev));

				if (*nobcncnt == bcnTimeoutPeriod
				) {
					MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_WARN,
							 "Beacon Timeout(period: %d) occurred!!!", bcnTimeoutPeriod);
#ifdef WIFI_UNIFIED_COMMAND
					if (cap->uni_cmd_support)
						RTMP_FWLog2Host(pAd, HOST2N9, ENUM_CMD_FW_LOG_2_HOST_CTRL_2_UART);
					else
#endif /* WIFI_UNIFIED_COMMAND */
						RTMP_FWLog2Host(pAd, 0, 0);
				}

				return;
			}
		} else if (*nobcncnt != 0) {
			recoverext = 1;
			*nobcncnt = 0;
		} else {
			*nobcncnt = 0;
			return;
		}

		if ((*nobcncnt != 0 || recoverext == 1) && DebugLevel >= DBG_LVL_ERROR
			) {
			if (recoverext == 1) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_WARN,
					 "bcn recover for band %d (%s)!!\n",
					  bandidx, RtmpOsGetNetDevName(pAd->net_dev));
			} else {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
					 "nobcn occurs within %d sec for band %d (%s)!!\n",
					  (*nobcncnt) * 5, bandidx,
					  RtmpOsGetNetDevName(pAd->net_dev));
			}


		}
	}
}

VOID BcnStopHandle(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN stop)
{
	BCN_BUF_STRUCT *bcn_info = &wdev->bcn_buf;

	if (stop)
		bcn_info->stop_tx |= STOP_BCN_TX_NO_BCN;
	else
		bcn_info->stop_tx &= ~STOP_BCN_TX_NO_BCN;

	if (stop) {
		bcn_info->bBcnSntReq = FALSE;
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_DISABLE_TX));
	} else {
		bcn_info->bBcnSntReq = TRUE;
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_DISABLE_TX));
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_ENABLE_TX));
	}
}
#ifdef BCN_EXTCAP_VAR_LEN
INT32 set_BeaconExtCapLength_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	long Length;
	UINT ifIndex = pObj->ioctl_if;

	if ((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) {
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATAP_BCN, DBG_LVL_ERROR,
					"invalid mbss id(%d)\n", ifIndex);
			return FALSE;
		}
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	} else {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				"ioctl_if_type:%d error.\n", pObj->ioctl_if_type);
		return FALSE;
	}

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR, "wdev is NULL\n");
		return FALSE;
	}

	if (wdev->wdev_type != WDEV_TYPE_AP) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR, "wdev is not ap\n");
		return FALSE;
	}

	if (kstrtol(arg, 10, &Length)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"<--Length(=%u) is not received\n", Length);
		return FALSE;
	}
	if (Length > sizeof(EXT_CAP_INFO_ELEMENT) || Length < 0) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR, "Length is error!\n");
		return FALSE;
	}

	wdev->BcnExtCapLen = Length;
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE, "wdev(%d) Bcn Ext cap Length is %u!\n",
			wdev->func_idx, wdev->BcnExtCapLen);
	return TRUE;
}
#endif /* BCN_EXTCAP_VAR_LEN */
#endif /* CONFIG_AP_SUPPORT */
