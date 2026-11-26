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
 ***************************************************************************/

/****************************************************************************
 *	Abstract:
 *
 *	All related CFG80211 function body.
 *
 *	History:
 *
 ***************************************************************************/

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"
#include "mgmt/be_internal.h"

#define BSSID_WCID_TO_REMOVE 1 /* Pat:TODO */

extern struct notifier_block cfg80211_netdev_notifier;

extern INT RtmpIoctl_rt_ioctl_siwauth(
	IN      RTMP_ADAPTER * pAd,
	IN      VOID * pData,
	IN      ULONG                            Data);

extern INT RtmpIoctl_rt_ioctl_siwauth(
	IN      RTMP_ADAPTER * pAd,
	IN      VOID * pData,
	IN      ULONG                            Data);


INT CFG80211DRV_IoctlHandle(
	IN	VOID * pAdSrc,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID * pData,
	IN	ULONG					Data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	switch (cmd) {
	case CMD_RTPRIV_IOCTL_80211_START:
	case CMD_RTPRIV_IOCTL_80211_END:
		/* nothing to do */
		break;

	case CMD_RTPRIV_IOCTL_80211_CHAN_SET:
		if (CFG80211DRV_OpsSetChannel(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;
	case CMD_RTPRIV_IOCTL_80211_AP_CHAN_SET:
		if (CFG80211DRV_AP_OpsSetChannel(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;
		break;

	case CMD_RTPRIV_IOCTL_80211_SCAN:
		if (CFG80211DRV_OpsScanCheckStatus(pAd, Data) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_80211_SCAN_STATUS_LOCK_INIT:
		CFG80211_ScanStatusLockInit(pAd, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_IBSS_JOIN:
		CFG80211DRV_OpsJoinIbss(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_STA_LEAVE:
		CFG80211DRV_OpsLeave(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_STA_GET:
		if (CFG80211DRV_StaGet(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;

#ifdef RT_CFG80211_SUPPORT
		case CMD_RTPRIV_IOCTL_80211_AP_STA_GET:
			if (CFG80211DRV_Ap_StaGet(pAd, pData) != TRUE)
				return NDIS_STATUS_FAILURE;
			break;
#endif /* RT_CFG80211_SUPPORT */

	case CMD_RTPRIV_IOCTL_80211_STA_KEY_ADD:
#ifdef APCLI_CFG80211_SUPPORT
		CFG80211DRV_ApClientKeyAdd(pAd, pData);
#else
		CFG80211DRV_StaKeyAdd(pAd, pData);
#endif /* APCLI_CFG80211_SUPPORT */
		break;
#ifdef CONFIG_STA_SUPPORT

	case CMD_RTPRIV_IOCTL_80211_STA_KEY_DEFAULT_SET:
		CFG80211_setStaDefaultKey(pAd, Data);
		break;
#endif /*CONFIG_STA_SUPPORT*/

	case CMD_RTPRIV_IOCTL_80211_CONNECT_TO:
		CFG80211DRV_Connect(pAd, pData);
		break;

#ifdef SUPP_SAE_SUPPORT
	case CMD_RTPRIV_IOCTL_80211_EXT_CONNECT: {
		UINT8 staIndex = Data;
		SECURITY_CONFIG *pSecConfig;
		struct wifi_dev *pWdev;
		STA_ADMIN_CONFIG *pstacfg = &pAd->StaCfg[staIndex];
		CMD_RTPRIV_IOCTL_80211_CONNECT_PARAM *pConnParam;
		pConnParam = (CMD_RTPRIV_IOCTL_80211_CONNECT_PARAM *)pData;
		pWdev = &pAd->StaCfg[staIndex].wdev;
		pSecConfig = &pWdev->SecConfig;

		if (pConnParam->psk_len < 65) {
			os_move_mem(pSecConfig->PSK, pConnParam->psk, pConnParam->psk_len);
			pstacfg->sae_cfg_group = (UCHAR)pConnParam->sae_group;
			pSecConfig->PSK[pConnParam->psk_len] = '\0';
		}
	}
		break;
#endif
	case CMD_RTPRIV_IOCTL_80211_REG_NOTIFY_TO:
		CFG80211DRV_RegNotify(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_UNREGISTER:

		/* Only main net_dev needs to do CFG80211_UnRegister. */
		if (pAd->net_dev == pData)
			CFG80211_UnRegister(pAd, pData);

		break;

	case CMD_RTPRIV_IOCTL_80211_BANDINFO_GET: {
		CFG80211_BAND *pBandInfo = (CFG80211_BAND *)pData;
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

		CFG80211_BANDINFO_FILL(pAd, wdev, pBandInfo);

		if (IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_24G))
			pBandInfo->RFICType |= RFIC_24GHZ;

		if (IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_5G))
			pBandInfo->RFICType |= RFIC_5GHZ;

		if (IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_6G))
			pBandInfo->RFICType |= RFIC_6GHZ;

		pBandInfo->phy_caps = cap->phy_caps;
	}
	break;

	case CMD_RTPRIV_IOCTL_80211_SURVEY_GET:
		CFG80211DRV_SurveyGet(pAd, pData);
		break;

#ifdef APCLI_CFG80211_SUPPORT
	case CMD_RTPRIV_IOCTL_APCLI_SITE_SURVEY:
		CFG80211DRV_ApcliSiteSurvey(pAd, pData);
		break;
#endif

	case CMD_RTPRIV_IOCTL_AP_SITE_SURVEY:
		CFG80211DRV_ApSiteSurvey(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_EXTRA_IES_SET:
		CFG80211DRV_OpsScanExtraIesSet(pAd);
		break;

	/* CFG_TODO */
	case CMD_RTPRIV_IOCTL_80211_MGMT_FRAME_REG:
		CFG80211DRV_OpsMgmtFrameProbeRegister(pAd, pData, Data);
		break;

	/* CFG_TODO */
	case CMD_RTPRIV_IOCTL_80211_ACTION_FRAME_REG:
		CFG80211DRV_OpsMgmtFrameActionRegister(pAd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_CHANNEL_LOCK:
		CFG80211_SwitchTxChannel(pAd, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_CHANNEL_RESTORE:
		break;

	case CMD_RTPRIV_IOCTL_80211_MGMT_FRAME_SEND:
		CFG80211_SendMgmtFrame(pAd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_CHANNEL_LIST_SET:
		return CFG80211DRV_OpsScanSetSpecifyChannel(pAd, pData, Data);
#ifdef CONFIG_AP_SUPPORT
#ifdef HOSTAPD_HS_R2_SUPPORT
		case CMD_RTPRIV_IOCTL_SET_QOS_PARAM:
			CFG80211DRV_SetQosParam(pAd, pData, Data);
			break;
#endif

	case CMD_RTPRIV_IOCTL_80211_BEACON_DEL: {
#ifndef DISABLE_HOSTAPD_BEACON
			if (pAd->cfg80211_ctrl.beacon_tail_buf != NULL) {
				os_free_mem(pAd->cfg80211_ctrl.beacon_tail_buf);
				pAd->cfg80211_ctrl.beacon_tail_buf = NULL;
			}
			pAd->cfg80211_ctrl.beacon_tail_len = 0;
#endif
	}
	break;

	case CMD_RTPRIV_IOCTL_80211_AP_KEY_ADD:
		CFG80211DRV_ApKeyAdd(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_RTS_THRESHOLD_ADD: {
		UCHAR i = 0;

		for (i = 0; i < WDEV_NUM_MAX; i++) {
			if (pAd->wdev_list[i]) {
				wdev = pAd->wdev_list[i];
				CFG80211DRV_RtsThresholdAdd(pAd, wdev, Data);
			}
		}
	}
		break;

	case CMD_RTPRIV_IOCTL_80211_FRAG_THRESHOLD_ADD: {
		UCHAR i = 0;

		for (i = 0; i < WDEV_NUM_MAX; i++) {
			if (pAd->wdev_list[i]) {
				wdev = pAd->wdev_list[i];
				CFG80211DRV_FragThresholdAdd(pAd, wdev, Data);
			}
		}
	}
		break;

#ifdef ACK_CTS_TIMEOUT_SUPPORT
	case CMD_RTPRIV_IOCTL_80211_ACK_THRESHOLD_ADD:
	if (CFG80211DRV_AckThresholdAdd(pAd, wdev, Data) != TRUE)
		return NDIS_STATUS_FAILURE;
	break;
#endif/*ACK_CTS_TIMEOUT_SUPPORT*/
	case CMD_RTPRIV_IOCTL_80211_AP_KEY_DEL:
		CFG80211DRV_ApKeyDel(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_AP_KEY_DEFAULT_SET:
		CFG80211_setApDefaultKey(pAd, pData, Data);
		break;
#ifdef BCN_PROTECTION_SUPPORT
	case CMD_RTPRIV_IOCTL_80211_AP_BEACON_KEY_DEFAULT_SET:
		CFG80211_setApBeaconDefaultKey(pAd, pData, Data);
		break;
#endif /*BCN_PROTECTION_SUPPORT*/

#ifdef DOT11W_PMF_SUPPORT
	case CMD_RTPRIV_IOCTL_80211_AP_KEY_DEFAULT_MGMT_SET:
		CFG80211_setApDefaultMgmtKey(pAd, pData, Data);
		break;
#endif /*DOT11W_PMF_SUPPORT*/


	case CMD_RTPRIV_IOCTL_80211_PORT_SECURED:
		CFG80211_StaPortSecured(pAd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_AP_STA_DEL:
		CFG80211_ApStaDel(pAd, pData, Data);
		break;

#ifdef HOSTAPD_PMKID_IN_DRIVER_SUPPORT
	case CMD_RTPRIV_IOCTL_80211_AP_UPDATE_STA_PMKID:
		CFG80211_ApUpdateStaPmkid(pAd, pData);
		break;
#endif /*HOSTAPD_PMKID_IN_DRIVER_SUPPORT*/

#endif /* CONFIG_AP_SUPPORT */

	case CMD_RTPRIV_IOCTL_80211_AP_PROBE_RSP_EXTRA_IE:
		break;

	case CMD_RTPRIV_IOCTL_80211_BITRATE_SET:
		CFG80211DRV_OpsBitRateParm(pAd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_MCAST_RATE_SET:
		CFG80211DRV_OpsMcastRateParm(pAd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_BSS_COLOR_CHANGE:
		CFG80211DRV_OpsBssColorParm(pAd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_FT_IES_UPDATE:
		CFG80211DRV_OpsFtIesParm(pAd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_RESET:
		CFG80211_reSetToDefault(pAd);
		break;

	case CMD_RTPRIV_IOCTL_80211_NETDEV_EVENT: {
		/*
		 * CFG_TODO: For Scan_req per netdevice
		 * PNET_DEV pNetDev = (PNET_DEV) pData;
		 * struct wireless_dev *pWdev = pAd->pCfg80211_CB->pCfg80211_Wdev;
		 * if (RTMPEqualMemory(pNetDev->dev_addr, pNewNetDev->dev_addr, MAC_ADDR_LEN))
		 */
		if (CFG80211DRV_OpsScanRunning(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"CFG_SCAN: close the scan cmd in device close phase\n");
			CFG80211OS_ScanEnd(pAd, TRUE);
		}
	}
	break;
#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)

	case CMD_RTPRIV_IOCTL_80211_P2PCLI_ASSSOC_IE_SET: {
		CMD_RTPRIV_IOCTL_80211_ASSOC_IE *pAssocIe;

		pAssocIe = (CMD_RTPRIV_IOCTL_80211_ASSOC_IE *)pData;
#ifdef APCLI_CFG80211_SUPPORT
		CFG80211DRV_SetApCliAssocIe(pAd, pAssocIe->pNetDev, pAssocIe->ie, pAssocIe->ie_len);
#else
		RTMP_DRIVER_80211_GEN_IE_SET(pAd, pAssocIe->ie, pAssocIe->ie_len);
#endif
		break;
	}
#endif /*CONFIG_STA_SUPPORT || APCLI_CFG80211_SUPPORT*/
#if defined(IWCOMMAND_CFG80211_SUPPORT) || defined(HOSTAPD_MBSS_SUPPORT)
	case CMD_RTPRIV_IOCTL_80211_VIF_ADD:
		if (CFG80211DRV_OpsVifAdd(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_80211_VIF_DEL:
		RTMP_CFG80211_VirtualIF_Remove(pAd, pData, Data);
		break;
#endif /* IWCOMMAND_CFG80211_SUPPORT */

#ifdef RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT
	case CMD_RTPRIV_IOCTL_80211_ANDROID_PRIV_CMD:
		/* rt_android_private_command_entry(pAd, ); */
		break;
#endif /* RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT */
#ifdef RFKILL_HW_SUPPORT

	case CMD_RTPRIV_IOCTL_80211_RFKILL: {
		UINT32 data = 0;
		BOOLEAN active;
		/* Read GPIO pin2 as Hardware controlled radio state */
		RTMP_IO_READ32(pAd->hdev_ctrl, GPIO_CTRL_CFG, &data);
		active = !!(data & 0x04);

		if (!active) {
			RTMPSetLED(pAd, LED_RADIO_OFF, DBDC_BAND0);
			*(UINT8 *)pData = 0;
		} else
			*(UINT8 *)pData = 1;
	}
	break;
#endif /* RFKILL_HW_SUPPORT */

	case CMD_RTPRIV_IOCTL_80211_ANTENNA_CTRL:
	{
		BOOLEAN is_get = (Data == 1) ? TRUE : FALSE;
		CMD_RTPRIV_IOCTL_80211_ANTENNA *ant_cnt = (CMD_RTPRIV_IOCTL_80211_ANTENNA *)pData;
		struct mcs_nss_caps *nss_cap = MCS_NSS_CAP(pAd);

		if (is_get) {
			if (OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED) &&
				(wdev != NULL) && (wdev->if_up_down_state == TRUE)) {
#ifdef DOT11_EHT_BE
				if (WMODE_CAP_BE(wdev->PhyMode)) {
					ant_cnt->rx_ant = wlan_config_get_eht_rx_nss(wdev);
					ant_cnt->tx_ant = wlan_config_get_eht_tx_nss(wdev);
				} else
#endif /* DOT11_EHT_BE */
#ifdef DOT11_HE_AX
				if (WMODE_CAP_AX(wdev->PhyMode)) {
					ant_cnt->rx_ant = wlan_config_get_he_rx_nss(wdev);
					ant_cnt->tx_ant = wlan_config_get_he_tx_nss(wdev);
				} else
#endif /* DOT11_HE_AX */
				{
					ant_cnt->tx_ant = wlan_config_get_tx_stream(wdev);
					ant_cnt->rx_ant = wlan_config_get_rx_stream(wdev);
				}
			} else {
				UINT tx_max_nss;
				UINT rx_max_nss;

				tx_max_nss = nss_cap->max_nss;
				rx_max_nss = nss_cap->max_nss;
				ant_cnt->tx_ant = tx_max_nss;
				ant_cnt->rx_ant = rx_max_nss;
			}
		} else {
			UINT8 Txstream = ant_cnt->tx_ant;
			UINT8 Rxstream = ant_cnt->rx_ant;

			if (wdev == NULL) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
						"wdev is null, return!!!\n");
				return NDIS_STATUS_FAILURE;
			}

			if (Txstream != Rxstream) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
						"Wrong Input: Tx & Rx Antenna number different!\n");
				return NDIS_STATUS_FAILURE;
			}

			if (Txstream > nss_cap->max_nss) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
						"Wrong Input: More than max ant cap (%d)!!\n", nss_cap->max_nss);
				return NDIS_STATUS_FAILURE;
			}
#ifdef ANTENNA_CONTROL_SUPPORT
			pAd->TxStream  = Txstream;
			pAd->RxStream  = Rxstream;
			pAd->bAntennaSetAPEnable = 1;
#endif /* ANTENNA_CONTROL_SUPPORT */

			wlan_config_set_tx_stream(wdev, min(Txstream, nss_cap->max_nss));
			wlan_config_set_rx_stream(wdev, min(Rxstream, nss_cap->max_nss));
			wlan_operate_set_tx_stream(wdev, min(Txstream, nss_cap->max_nss));
			wlan_operate_set_rx_stream(wdev, min(Rxstream, nss_cap->max_nss));
#ifdef DOT11_HE_AX
			wlan_config_set_he_tx_nss(wdev, min(Txstream, nss_cap->max_nss));
			wlan_config_set_he_rx_nss(wdev, min(Rxstream, nss_cap->max_nss));
#endif /* DOT11_HE_AX */
#ifdef DOT11_EHT_BE
			wlan_config_set_eht_tx_nss(wdev, min(Txstream, nss_cap->max_nss));
			wlan_config_set_eht_rx_nss(wdev, min(Rxstream, nss_cap->max_nss));
#endif /* DOT11_EHT_BE */

			SetCommonHtVht(pAd, wdev);
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				APStop(pAd, NULL, AP_BSS_OPER_ALL);
				APStartUp(pAd, NULL, AP_BSS_OPER_ALL);
			}
#endif /* CONFIG_AP_SUPPORT */
		}
	}
	break;

	default:
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

VOID CFG80211DRV_OpsMgmtFrameProbeRegister(
	VOID                                            *pAdOrg,
	VOID                                            *pData,
	BOOLEAN                                          isReg)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) pAdOrg;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;
#if defined(APCLI_CFG80211_SUPPORT)
	PNET_DEV net_dev = (PNET_DEV)pData;
	struct wireless_dev *wdev = net_dev->ieee80211_ptr;

	if (wdev->iftype == NL80211_IFTYPE_STATION) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_DEBUG,
			"Do not register or unreg probe Req for station intf\n");
		return;
	}
#endif /* APCLI_CFG80211_SUPPORT */

	/* IF Not Exist on VIF List, the device must be MAIN_DEV */
	if (isReg) {
		if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount < 255)
			pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount++;
	} else {
		if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount > 0)
			pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount--;
	}

	if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount > 0)
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = TRUE;
	else {
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = FALSE;
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount = 0;
	}

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_DEBUG,
		"[%d] pAd->Cfg80211RegisterProbeReqFrame=%d[%d]\n",
			 isReg, pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame,
			 pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount);
}

VOID CFG80211DRV_OpsMgmtFrameActionRegister(
	VOID                                            *pAdOrg,
	VOID                                            *pData,
	BOOLEAN                                          isReg)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) pAdOrg;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

	/* IF Not Exist on VIF List, the device must be MAIN_DEV */
	if (isReg) {
		if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount < 255)
			pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount++;
	} else {
		if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount > 0)
			pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount--;
	}

	if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount > 0)
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = TRUE;
	else {
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = FALSE;
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount = 0;
	}

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_DEBUG,
		"[%d] TYPE pAd->Cfg80211RegisterActionFrame=%d[%d]\n",
			 isReg, pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame,
			 pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount);
}


VOID CFG80211DRV_OpsBitRateParm(
	VOID                                            *pAdOrg,
	VOID                                            *pData,
	INT												apidx)
{
	struct cfg80211_bitrate_mask *mask;
	UCHAR band = 0;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	INT i = 0;
	UCHAR rate[] = { 0x82, 0x84, 0x8b, 0x96, 0x8C, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6c};
	struct legacy_rate *eap_legacy_rate = NULL;
	UCHAR tx_stream = 0;
	UCHAR ch_band;
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

	if (apidx < 0 || apidx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"invalid apidx:%d\n", apidx);
		return;
	}

	mask = (struct cfg80211_bitrate_mask *)pData;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (pAd->CommonCfg.wifi_cert) {
		u8 he_gi = CAPI_HE_GI_NUM;
		u8 he_ltf = CAPI_HE_LTF_NUM;

		/* 0-2G; 1-5G; 2-6G */
		band = wlan_operate_get_ch_band(wdev);

		if (band > CMD_CH_BAND_6G) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
				"invalid value: band = %d\n", band);
			return;
		}

#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		/* Remapping to correct 6G BAND */
		if (band == CMD_CH_BAND_6G)
			band = NL80211_BAND_6GHZ;
#endif

#if KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE || defined(BACKPORT_NOSTDINC)
		switch (mask->control[band].he_gi) {
		case NL80211_RATE_INFO_HE_GI_0_8:
			he_gi = CAPI_HE_08_GI;
			break;
		case NL80211_RATE_INFO_HE_GI_1_6:
			he_gi = CAPI_HE_16_GI;
			break;
		case NL80211_RATE_INFO_HE_GI_3_2:
			he_gi = CAPI_HE_32_GI;
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
				"Wrong Input: band = %d, he_gi = %d!\n",
				band, mask->control[band].he_gi);
			break;
		}
		if (he_gi < CAPI_HE_GI_NUM) {
			ap_set_rfeature_he_gi(pAd, (VOID *)&he_gi);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
				"band = %d, update to he_gi = %d\n",
				band, he_gi);
		}

		switch (mask->control[band].he_ltf) {
		case NL80211_RATE_INFO_HE_1XLTF:
			he_ltf = CAPI_HE_1x_LTF;
			break;
		case NL80211_RATE_INFO_HE_2XLTF:
			he_ltf = CAPI_HE_2x_LTF;
			break;
		case NL80211_RATE_INFO_HE_4XLTF:
			he_ltf = CAPI_HE_4x_LTF;
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
				"Wrong Input: band = %d, he_ltf = %d!\n",
				band, mask->control[band].he_ltf);
			break;
		}
		if (he_ltf < CAPI_HE_LTF_NUM) {
			ap_set_rfeature_he_ltf(pAd, (VOID *)&he_ltf);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
				"band = %d, update to he_ltf = %d\n",
				band, he_ltf);
		}
#endif
	}
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	else {
		eap_legacy_rate = &wdev->eap.eap_legacy_rate;
		ch_band = wlan_config_get_ch_band(wdev);

		if (ch_band == CMD_CH_BAND_5G || ch_band == CMD_CH_BAND_6G) {
			if (ch_band == CMD_CH_BAND_5G)
				band = 1;
			else if (ch_band == CMD_CH_BAND_6G)
				band = 3;
			/* legacy mode*/
			if ((mask->control[band].legacy == 0) || (mask->control[band].legacy == 0xff)) {
				wdev->eap.eap_suprate_en = FALSE;
				RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);
			} else {
				wdev->eap.eap_suprate_en = TRUE;
				eap_legacy_rate->sup_rate_len = 0;
				eap_legacy_rate->ext_rate_len = 0;

				for (i = 0; i < 8; i++) {
					if (mask->control[band].legacy & (1 << i)) {
						eap_legacy_rate->sup_rate[eap_legacy_rate->sup_rate_len] = rate[i+4];
						eap_legacy_rate->sup_rate_len++;
						wdev->eap.eapsupportofdmmcs |= (1 << i);
						wdev->eap.eapsupportratemode |= SUPPORT_OFDM_MODE;
					}
				}
				rtmpeapupdaterateinfo(wdev->PhyMode, &wdev->rate, &wdev->eap);
			}
		} else {
			band = 0;
			if ((mask->control[band].legacy == 0) || (mask->control[band].legacy == 0xfff)) {
				wdev->eap.eap_suprate_en = FALSE;
				RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);
			} else {
				wdev->eap.eap_suprate_en = TRUE;
				eap_legacy_rate->sup_rate_len = 0;
				eap_legacy_rate->ext_rate_len = 0;
				for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
					if (mask->control[band].legacy & (1 << i)) {
						if (WMODE_EQUAL(wdev->PhyMode, WMODE_B) && (wdev->channel <= 14)) {
							eap_legacy_rate->sup_rate[eap_legacy_rate->sup_rate_len] = rate[i];
							eap_legacy_rate->sup_rate_len++;
							wdev->eap.eapsupportcckmcs |= (1 << i);
							wdev->eap.eapsupportratemode |= SUPPORT_CCK_MODE;
						} else {
							if ((i < 4) || (i == 5) || (i == 7) || (i == 9) || (i == 11)) {
								eap_legacy_rate->sup_rate[eap_legacy_rate->sup_rate_len]
														= rate[i];
								eap_legacy_rate->sup_rate_len++;
								if (i < 4) {
									wdev->eap.eapsupportcckmcs |= (1 << i);
									wdev->eap.eapsupportratemode |= SUPPORT_CCK_MODE;
								} else {
									wdev->eap.eapsupportofdmmcs |= (1 << (i - 4));
									wdev->eap.eapsupportratemode |= SUPPORT_OFDM_MODE;
								}
							} else {
								eap_legacy_rate->ext_rate[eap_legacy_rate->ext_rate_len]
													= rate[i] & 0x7f;
								eap_legacy_rate->ext_rate_len++;
								wdev->eap.eapsupportofdmmcs |= (1 << (i - 4));
								wdev->eap.eapsupportratemode |= SUPPORT_OFDM_MODE;
							}
						}
				    }
				}
				rtmpeapupdaterateinfo(wdev->PhyMode, &wdev->rate, &wdev->eap);
			}
		}

		tx_stream = wlan_config_get_tx_stream(wdev);
		if (tx_stream == 1) {
			/* HT mode*/
			if ((mask->control[band].ht_mcs[0] == 0) || (mask->control[band].ht_mcs[0] == 0xff)) {
				wdev->eap.eap_htsuprate_en = FALSE;
			} else {
				wdev->eap.eap_htsuprate_en = TRUE;
				wdev->eap.eapmcsset[0] = mask->control[band].ht_mcs[0];
				wdev->eap.eapmcsset[1] = 0;
				wdev->eap.eapmcsset[2] = 0;
				wdev->eap.eapmcsset[3] = 0;
			}
			/* VHT mode*/
			if ((mask->control[band].vht_mcs[0] == 0) || (mask->control[band].vht_mcs[0] == 0x3ff)) {
				wdev->eap.eap_vhtsuprate_en = FALSE;
			} else {
				wdev->eap.eap_vhtsuprate_en = TRUE;
				if (((mask->control[band].vht_mcs[0] & 0x00000300) >> 8) == 3)
					wdev->eap.rx_mcs_map.mcs_ss1 = 2;
				else
					wdev->eap.rx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;
				wdev->eap.rx_mcs_map.mcs_ss2 = 0;
				wdev->eap.rx_mcs_map.mcs_ss3 = 0;
				wdev->eap.rx_mcs_map.mcs_ss4 = 0;
				if (((mask->control[band].vht_mcs[0] & 0x00000300) >> 8) == 3)
					wdev->eap.tx_mcs_map.mcs_ss1 = 2;
				else
					wdev->eap.tx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;
				wdev->eap.tx_mcs_map.mcs_ss2 = 0;
				wdev->eap.tx_mcs_map.mcs_ss3 = 0;
				wdev->eap.tx_mcs_map.mcs_ss4 = 0;
			}
		} else if (tx_stream == 2) {
			/* HT mode*/
			if (((mask->control[band].ht_mcs[0] == 0) && (mask->control[band].ht_mcs[1] == 0))
				|| ((mask->control[band].ht_mcs[0] == 0xff) && (mask->control[band].ht_mcs[1] == 0xff))) {
				wdev->eap.eap_htsuprate_en = FALSE;
			} else {
				wdev->eap.eap_htsuprate_en = TRUE;
				wdev->eap.eapmcsset[0] = mask->control[band].ht_mcs[0];
				wdev->eap.eapmcsset[1] = mask->control[band].ht_mcs[1];
				wdev->eap.eapmcsset[2] = 0;
				wdev->eap.eapmcsset[3] = 0;
			}
			/* VHT mode*/
			if (((mask->control[band].vht_mcs[0] == 0) && (mask->control[band].vht_mcs[1] == 0))
					|| ((mask->control[band].vht_mcs[0] == 0x3ff)
								&& (mask->control[band].vht_mcs[1] == 0x3ff))) {
				wdev->eap.eap_vhtsuprate_en = FALSE;
			} else {
				wdev->eap.eap_vhtsuprate_en = TRUE;
				if (((mask->control[band].vht_mcs[0] & 0x00000300) >> 8) == 3)
					wdev->eap.rx_mcs_map.mcs_ss1 = 2;
				else
					wdev->eap.rx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;

				if (((mask->control[band].vht_mcs[1] & 0x00000300) >> 8) == 3)
					wdev->eap.rx_mcs_map.mcs_ss2 = 2;
				else
					wdev->eap.rx_mcs_map.mcs_ss2 = (mask->control[band].vht_mcs[1] & 0x00000300) >> 8;
				wdev->eap.rx_mcs_map.mcs_ss3 = 0;
				wdev->eap.rx_mcs_map.mcs_ss4 = 0;
				if (((mask->control[band].vht_mcs[0] & 0x00000300) >> 8) == 3)
					wdev->eap.tx_mcs_map.mcs_ss1 = 2;
				else
					wdev->eap.tx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;

				if (((mask->control[band].vht_mcs[1] & 0x00000300) >> 8) == 3)
					wdev->eap.tx_mcs_map.mcs_ss2 = 2;
				else
					wdev->eap.tx_mcs_map.mcs_ss2 = (mask->control[band].vht_mcs[1] & 0x00000300) >> 8;
				wdev->eap.tx_mcs_map.mcs_ss3 = 0;
				wdev->eap.tx_mcs_map.mcs_ss4 = 0;
			}
		} else if (tx_stream == 3) {
			/* HT mode*/
			if (((mask->control[band].ht_mcs[0] == 0) && (mask->control[band].ht_mcs[1] == 0)
				&& (mask->control[band].ht_mcs[2] == 0))
				|| ((mask->control[band].ht_mcs[0] == 0xff) && (mask->control[band].ht_mcs[1] == 0xff)
				&& (mask->control[band].ht_mcs[2] == 0xff))) {
				wdev->eap.eap_htsuprate_en = FALSE;
			} else {
				wdev->eap.eap_htsuprate_en = TRUE;
				wdev->eap.eapmcsset[0] = mask->control[band].ht_mcs[0];
				wdev->eap.eapmcsset[1] = mask->control[band].ht_mcs[1];
				wdev->eap.eapmcsset[2] = mask->control[band].ht_mcs[2];
				wdev->eap.eapmcsset[3] = 0;
			}
			/* VHT mode*/
			if (((mask->control[band].vht_mcs[0] == 0) && (mask->control[band].vht_mcs[1] == 0)
				 && (mask->control[band].vht_mcs[2] == 0))
				|| ((mask->control[band].vht_mcs[0] == 0x3ff) && (mask->control[band].vht_mcs[1] == 0x3ff)
				 && (mask->control[band].vht_mcs[2] == 0x3ff))) {
				wdev->eap.eap_vhtsuprate_en = FALSE;
			} else {
				wdev->eap.eap_vhtsuprate_en = TRUE;
				if (((mask->control[band].vht_mcs[0] & 0x00000300) >> 8) == 3) {
					wdev->eap.rx_mcs_map.mcs_ss1 = 2;
					wdev->eap.tx_mcs_map.mcs_ss1 = 2;
				} else {
					wdev->eap.rx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;
					wdev->eap.tx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;
				}

				if (((mask->control[band].vht_mcs[1] & 0x00000300) >> 8) == 3) {
					wdev->eap.rx_mcs_map.mcs_ss2 = 2;
					wdev->eap.tx_mcs_map.mcs_ss2 = 2;
				} else {
					wdev->eap.rx_mcs_map.mcs_ss2 = (mask->control[band].vht_mcs[1] & 0x00000300) >> 8;
					wdev->eap.tx_mcs_map.mcs_ss2 = (mask->control[band].vht_mcs[1] & 0x00000300) >> 8;
				}

				if (((mask->control[band].vht_mcs[2] & 0x00000300) >> 8) == 3) {
					wdev->eap.rx_mcs_map.mcs_ss3 = 2;
					wdev->eap.tx_mcs_map.mcs_ss3 = 2;
				} else {
					wdev->eap.rx_mcs_map.mcs_ss3 = (mask->control[band].vht_mcs[2] & 0x00000300) >> 8;
					wdev->eap.tx_mcs_map.mcs_ss3 = (mask->control[band].vht_mcs[2] & 0x00000300) >> 8;
				}

				wdev->eap.rx_mcs_map.mcs_ss4 = 0;
				wdev->eap.tx_mcs_map.mcs_ss4 = 0;
			}
		} else if (tx_stream == 4) {
			/* HT mode*/
			if (((mask->control[band].ht_mcs[0] == 0) && (mask->control[band].ht_mcs[1] == 0)
				  && (mask->control[band].ht_mcs[2] == 0) && (mask->control[band].ht_mcs[3] == 0))
				|| ((mask->control[band].ht_mcs[0] == 0xff) && (mask->control[band].ht_mcs[1] == 0xff)
				  && (mask->control[band].ht_mcs[2] == 0xff) && (mask->control[band].ht_mcs[3] == 0xff))) {
				wdev->eap.eap_htsuprate_en = FALSE;
			} else {
				wdev->eap.eap_htsuprate_en = TRUE;
				wdev->eap.eapmcsset[0] = mask->control[band].ht_mcs[0];
				wdev->eap.eapmcsset[1] = mask->control[band].ht_mcs[1];
				wdev->eap.eapmcsset[2] = mask->control[band].ht_mcs[2];
				wdev->eap.eapmcsset[3] = mask->control[band].ht_mcs[3];
			}
			/* VHT mode*/
			if (((mask->control[band].vht_mcs[0] == 0) && (mask->control[band].vht_mcs[1] == 0)
				  && (mask->control[band].vht_mcs[2] == 0) && (mask->control[band].vht_mcs[3] == 0))
				|| ((mask->control[band].vht_mcs[0] == 0x3ff) && (mask->control[band].vht_mcs[1] == 0x3ff)
				  && (mask->control[band].vht_mcs[2] == 0x3ff) && (mask->control[band].vht_mcs[3] == 0x3ff))) {
				wdev->eap.eap_vhtsuprate_en = FALSE;
			} else {
				wdev->eap.eap_vhtsuprate_en = TRUE;
				if (((mask->control[band].vht_mcs[0] & 0x00000300) >> 8) == 3) {
					wdev->eap.rx_mcs_map.mcs_ss1 = 2;
					wdev->eap.tx_mcs_map.mcs_ss1 = 2;
				} else {
					wdev->eap.rx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;
					wdev->eap.tx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;
				}

				if (((mask->control[band].vht_mcs[1] & 0x00000300) >> 8) == 3) {
					wdev->eap.rx_mcs_map.mcs_ss2 = 2;
					wdev->eap.tx_mcs_map.mcs_ss2 = 2;
				} else {
					wdev->eap.rx_mcs_map.mcs_ss2 = (mask->control[band].vht_mcs[1] & 0x00000300) >> 8;
					wdev->eap.tx_mcs_map.mcs_ss2 = (mask->control[band].vht_mcs[1] & 0x00000300) >> 8;
				}

				if (((mask->control[band].vht_mcs[2] & 0x00000300) >> 8) == 3) {
					wdev->eap.rx_mcs_map.mcs_ss3 = 2;
					wdev->eap.tx_mcs_map.mcs_ss3 = 2;
				} else {
					wdev->eap.rx_mcs_map.mcs_ss3 = (mask->control[band].vht_mcs[2] & 0x00000300) >> 8;
					wdev->eap.tx_mcs_map.mcs_ss3 = (mask->control[band].vht_mcs[2] & 0x00000300) >> 8;
				}

				if (((mask->control[band].vht_mcs[3] & 0x00000300) >> 8) == 3) {
					wdev->eap.rx_mcs_map.mcs_ss4 = 2;
					wdev->eap.tx_mcs_map.mcs_ss4 = 2;
				} else {
					wdev->eap.rx_mcs_map.mcs_ss4 = (mask->control[band].vht_mcs[3] & 0x00000300) >> 8;
					wdev->eap.tx_mcs_map.mcs_ss4 = (mask->control[band].vht_mcs[3] & 0x00000300) >> 8;
				}
			}
		}
#ifdef DOT11_N_SUPPORT
		SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */

		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));
	}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
}

VOID CFG80211DRV_OpsMcastRateParm(
	VOID *pAdOrg,
	VOID *pData,
	INT apidx)
{
#ifdef MCAST_RATE_SPECIFIC
	PRTMP_ADAPTER pAd = NULL;
	struct wifi_dev *wdev = NULL;
	union _HTTRANSMIT_SETTING *transmit, *pMcastTransmit;
	BSS_INFO_ARGUMENT_T bss_info_argument;

	if (apidx < 0 || apidx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"invalid apidx:%d\n", apidx);
		return;
	}

	pAd = (PRTMP_ADAPTER)pAdOrg;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	transmit = (union _HTTRANSMIT_SETTING *)pData;

	pMcastTransmit = &wdev->rate.mcastphymode;
	pMcastTransmit->field.MODE = transmit->field.MODE;
	pMcastTransmit->field.MCS = transmit->field.MCS;
	pMcastTransmit->field.BW = transmit->field.BW;

	NdisZeroMemory(&bss_info_argument, sizeof(BSS_INFO_ARGUMENT_T));
	bss_info_argument.bss_state = BSS_ACTIVE;
	bss_info_argument.ucBssIndex = wdev->bss_info_argument.ucBssIndex;
	bss_info_argument.u8BssInfoFeature = BSS_INFO_BROADCAST_INFO_FEATURE;
	memmove(&bss_info_argument.BcTransmit, pMcastTransmit, sizeof(union _HTTRANSMIT_SETTING));
	memmove(&bss_info_argument.McTransmit, pMcastTransmit, sizeof(union _HTTRANSMIT_SETTING));

	if (AsicBssInfoUpdate(pAd, &bss_info_argument) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"Fail to apply the bssinfo, BSSID=%d!\n", apidx);
		return;
	}
#endif /* MCAST_RATE_SPECIFIC */
}

VOID CFG80211DRV_OpsBssColorParm(
	VOID *pAdOrg,
	VOID *pData,
	INT apidx)
{
#ifdef DOT11_HE_AX
	PRTMP_ADAPTER pAd = NULL;
	struct wifi_dev *wdev = NULL;
	struct cfg80211_color_change_settings *params;
	UCHAR Color, Count;
	struct _BSS_INFO_ARGUMENT_T *bssinfo;
	struct bss_color_ctrl *bss_color;

	if (apidx < 0 || apidx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"invalid apidx:%d\n", apidx);
		return;
	}

	pAd = (PRTMP_ADAPTER)pAdOrg;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	params = (struct cfg80211_color_change_settings *)pData;

	if (WMODE_CAP_AX(wdev->PhyMode)) {
		bssinfo = &wdev->bss_info_argument;
		bss_color = &bssinfo->bss_color;

		/* Don't Care other params? beacon data? */
		Color = params->color;
		Count = params->count;

		/* check range */
		if (Color < BSS_COLOR_VALUE_MIN || Color > BSS_COLOR_VALUE_MAX)
			return;

		/* ignore it if desired color is same as current setting */
		if (bss_color->color == Color)
			return;

		/*acquire BPCC lock*/
		if (bcn_bpcc_op_lock(pAd, wdev, TRUE, BCN_BPCC_BSSCCA) == FALSE) {
			if (in_interrupt()) {
				struct _BCN_BSSCCA_UPDATE_PARAM update_param = {0};

				update_param.BSSCAUpdateReason = RSN_CFG80211_PATH;
				update_param.next_color = Color;
				update_param.count = Count;
				/* defer it to workqueue */
				bcn_bpcc_ct_switch(pAd, wdev, BCN_BPCC_BSSCCA, BCN_REASON(BCN_UPDATE_IE_CHG), update_param);
				return;
			} else
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
					"%s: BSSCCA bcn_bpcc_op_lock fail!\n", __func__);
		}

		bss_color->disabled = TRUE;

		/* update state on firmware side */
		deliver_bss_color(pAd, bssinfo, bss_color);

		/* update wlan_operation state */
		wlan_operate_set_he_bss_color(wdev, bss_color->color, TRUE);
		bss_color->next_color = Color;
		wlan_operate_set_he_bss_next_color(wdev, Color, Count);

		/* update the Beacon content */
		UpdateBeaconHandler_BPCC(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG),
								BCN_BPCC_BSSCCA, TRUE);
	}
#endif
}

VOID CFG80211DRV_OpsFtIesParm(
	VOID *pAdOrg,
	VOID *pData,
	INT apidx)
{
#ifdef DOT11R_FT_SUPPORT
	PRTMP_ADAPTER pAd = NULL;
	struct wifi_dev *wdev = NULL;
	struct cfg80211_update_ft_ies_params *ftie;
	const u8 *end, *pos;
	struct _SECURITY_CONFIG *pSecConfig;
	UCHAR rsne_idx, ftcapflag;
	PFT_CFG pFtCfg;

	if (apidx < 0 || apidx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"invalid apidx:%d\n", apidx);
		return;
	}

	pAd = (PRTMP_ADAPTER)pAdOrg;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	ftie = (struct cfg80211_update_ft_ies_params *)pData;

	pos = ftie->ie;
	end = ftie->ie + ftie->ie_len;

	while (end - pos >= 2) {
		u8 id, len;

		id = *pos++;
		len = *pos++;
		if (len > end - pos)
			break;

		switch (id) {
		case IE_RSN:
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE, "FT: RSN\n");
			pSecConfig = &wdev->SecConfig;
			for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
				if (pSecConfig->RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
					continue;

				pSecConfig->RSNE_EID[rsne_idx][0] = IE_RSN;
				pSecConfig->RSNE_Len[rsne_idx] = len;
				if (len)
					NdisMoveMemory(
						&pSecConfig->RSNE_Content[rsne_idx][0], pos, len);
			}
			break;

		case IE_FT_MDIE:
			if (len <= FT_MDID_LEN)
				break;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE, "FT: MDE\n");
			pFtCfg = &wdev->FtCfg;
			NdisMoveMemory(pFtCfg->FtMdId, pos, FT_MDID_LEN);
			ftcapflag = *(pos + FT_MDID_LEN);
			pFtCfg->FtCapFlag.FtOverDs = ftcapflag & BIT0;
			pFtCfg->FtCapFlag.RsrReqCap = (ftcapflag >> 1) & BIT0;
			pFtCfg->FtCapFlag.Dot11rFtEnable = (ftcapflag >> 2) & BIT0;
			break;

		case IE_FT_FTIE:
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE, "FT: FTE\n");
			//refer to wpa_ft_parse_ftie ?

			break;

		case IE_FT_RIC_DATA:
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE, "FT: RDE\n");

			break;

		case IE_RSNXE:
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE, "FT: RSNXE\n");

			break;
		}
		pos += len;
	}
#endif /* DOT11R_FT_SUPPORT */
}

void CFG8021DRV_AP_PHYINIT(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 channel)
{
	CHANNEL_CTRL *pchctrl = NULL;

	/*phy mode setting */
	if (channel > 14) {
		wdev->PhyMode = (WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G);
	} else {
		wdev->PhyMode = (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G);
	}
	/*change channel state to NONE*/
	pchctrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	hc_set_ChCtrlChListStat(pchctrl, CH_LIST_STATE_NONE);
#ifdef EXT_BUILD_CHANNEL_LIST
	BuildChannelListEx(pAd, wdev);
#else
	BuildChannelList(pAd, wdev);
#endif
	RTMPSetPhyMode(pAd, wdev, wdev->PhyMode);

	return;
}

BOOLEAN CFG80211DRV_AP_SetChanBw_byWdev(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	CMD_RTPRIV_IOCTL_80211_CHAN *pchan_info)
{
	UCHAR bw = 0;
	UCHAR ext_cha = EXTCHA_BELOW;
	INT32 ret = FALSE, success = FALSE;

	if (!pAd || !wdev || !pchan_info) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"ERROR! invalid parameter. pAd=%p,wdev=%p, pchan_info=%p\n",
				pAd, wdev, pchan_info);
		return FALSE;
	}

	/*kernel4.4 set bw*/
	if (MTK_NL80211_CHAN_WIDTH_20_NOHT == pchan_info->ChanType) {
		/*bw and ht check*/
		bw = wlan_operate_get_ht_bw(wdev);
		if (HT_BW_20 == (enum ht_bw_def)bw && 1 == pAd->CommonCfg.HT_Disable) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"bw 20 is already set.\n");
			goto set_channel;
		}

		CFG8021DRV_AP_PHYINIT(pAd, wdev, pchan_info->ChanId);
		wlan_config_set_ht_bw(wdev, BW_20);
		if (WLAN_OPER_OK == wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE)) {
			pAd->CommonCfg.HT_Disable = 1;/*HT disable*/
			SetCommonHtVht(pAd, wdev);
			ret = TRUE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"ERROR! wlan_operate_set_ht_bw 20 FAIL\n");
			ret = FALSE;
		}
	} else if (MTK_NL80211_CHAN_WIDTH_20 == pchan_info->ChanType) {
		/*bw and ht check*/
		bw = wlan_operate_get_ht_bw(wdev);
		if (HT_BW_20 == (enum ht_bw_def)bw && 0 == pAd->CommonCfg.HT_Disable) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"bw 20 is already set.\n");
			goto set_channel;
		}
		CFG8021DRV_AP_PHYINIT(pAd, wdev, pchan_info->ChanId);
		wlan_config_set_ht_bw(wdev, BW_20);
		if (WLAN_OPER_OK == wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE)) {
			pAd->CommonCfg.HT_Disable = 0;/*HT enable*/
			SetCommonHtVht(pAd, wdev);
			ret = TRUE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"ERROR! wlan_operate_set_ht_bw 20 FAIL\n");
			ret = FALSE;
		}
	} else if (MTK_NL80211_CHAN_WIDTH_40 == pchan_info->ChanType) {
		/*extra_bw*/
		if (pchan_info->CenterChanId > pchan_info->ChanId)
			ext_cha = EXTCHA_ABOVE;
		else
			ext_cha = EXTCHA_BELOW;

		/*bw and ht check*/
		bw = wlan_operate_get_ht_bw(wdev);

		if (HT_BW_40 == (enum ht_bw_def)bw && ext_cha == wlan_operate_get_ext_cha(wdev)
			&& 0 == pAd->CommonCfg.HT_Disable) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"bw 40 ext_bw(%s) is already set.\n",
				ext_cha == EXTCHA_ABOVE ? "above":"below");
			goto set_channel;
		}
		CFG8021DRV_AP_PHYINIT(pAd, wdev, pchan_info->ChanId);

		/*set bw*/
		wlan_config_set_ht_bw(wdev, BW_40);
		if (WLAN_OPER_OK == wlan_operate_set_ht_bw(wdev, HT_BW_40, ext_cha)) {
			pAd->CommonCfg.HT_Disable = 0;/*HT enable*/
			SetCommonHtVht(pAd, wdev);
			ret = TRUE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"ERROR! wlan_operate_set_ht_bw 40 FAIL\n");
			ret = FALSE;
		}
	} else if (MTK_NL80211_CHAN_WIDTH_80 == pchan_info->ChanType) {
		/*bw and ht check*/
		bw = wlan_operate_get_vht_bw(wdev);
		if (VHT_BW_80 == (enum vht_config_bw)bw && 0 == pAd->CommonCfg.HT_Disable) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"bw 80 is already set.\n");
			goto set_channel;
		}
		CFG8021DRV_AP_PHYINIT(pAd, wdev, pchan_info->ChanId);
		/*set bw*/
		if (WLAN_OPER_OK == wlan_operate_set_vht_bw(wdev, VHT_BW_80)) {
			pAd->CommonCfg.HT_Disable = 0;/*HT disable*/
			ret = TRUE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"ERROR! wlan_operate_set_ht_bw 80 FAIL\n");
			ret = FALSE;
		}

	} else if (MTK_NL80211_CHAN_WIDTH_80P80 == pchan_info->ChanType) {
		/*bw and vht check*/
		bw = wlan_operate_get_vht_bw(wdev);
		if (VHT_BW_8080 == (enum vht_config_bw)bw && 0 == pAd->CommonCfg.HT_Disable) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"bw 8080 is already set.\n");
			goto set_channel;
		}
		CFG8021DRV_AP_PHYINIT(pAd, wdev, pchan_info->ChanId);
		/*set bw*/
		if (WLAN_OPER_OK == wlan_operate_set_vht_bw(wdev, VHT_BW_8080)) {
			pAd->CommonCfg.HT_Disable = 0;/*HT disable*/
			wlan_config_set_he_bw(wdev, HE_BW_8080);
			ret = TRUE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
				"ERROR! wlan_operate_set_ht_bw 8080 FAIL\n");
			ret = FALSE;
		}
	} else if (MTK_NL80211_CHAN_WIDTH_160 == pchan_info->ChanType) {
		/*bw and ht check*/
		bw = wlan_operate_get_vht_bw(wdev);
		if (VHT_BW_160 == (enum vht_config_bw)bw && 0 == pAd->CommonCfg.HT_Disable) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"bw 160 is already set.\n");
			goto set_channel;
		}
		CFG8021DRV_AP_PHYINIT(pAd, wdev, pchan_info->ChanId);
		/*set vbw*/
		if (WLAN_OPER_OK == wlan_operate_set_vht_bw(wdev, VHT_BW_160)) {
			pAd->CommonCfg.HT_Disable = 0;/*HT disable*/
			wlan_config_set_he_bw(wdev, HE_BW_160);
			ret = TRUE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"ERROR! wlan_operate_set_ht_bw 160 FAIL\n");
			ret = FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
			"WARN, bw=%d not support\n", pchan_info->ChanType);
		CFG8021DRV_AP_PHYINIT(pAd, wdev, pchan_info->ChanId);
		ret = FALSE;
	}

set_channel:
	/*bw and channel is not same as input check*/
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"ret:%d (FALSE:%d), wdev->channel:%d, pchan_info->ChanId:%d\n",
		ret, FALSE, wdev->channel, pchan_info->ChanId);

	/*set channel*/
	if (wdev->channel != pchan_info->ChanId) {
		/*To do set channel, need TakeChannelOpCharge first*/
		if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN, TRUE)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"TakeChannelOpCharge fail for SET channel!!\n");
			return FALSE;
		}
		pAd->ApCfg.iwpriv_event_flag = TRUE;

		RTMP_OS_REINIT_COMPLETION(&pAd->ApCfg.set_ch_aync_done);
		success = rtmp_set_channel(pAd, wdev, pchan_info->ChanId);

		if (pAd->ApCfg.set_ch_async_flag == TRUE) {
			ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ApCfg.set_ch_aync_done, ((80*100*OS_HZ)/1000));/*Wait 8s.*/
			if (ret)
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
					"wait channel setting success.\n");
			else {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					"wait channel setting timeout.\n");
				pAd->ApCfg.set_ch_async_flag = FALSE;
			}
		}
		pAd->ApCfg.iwpriv_event_flag = FALSE;

		/*if channel setting is DONE, release ChannelOpCharge here*/
		ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN);

		if (success == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
				"ERROR! channel(%d) set fail\n", pchan_info->ChanId);
			return FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"channel(%d) is already set!\n", pchan_info->ChanId);
	}
	return TRUE;

}


BOOLEAN CFG80211DRV_AP_OpsSetChannel(RTMP_ADAPTER *pAd, VOID *pData)
{
	CMD_RTPRIV_IOCTL_80211_CHAN *pchan_info = NULL;
	struct wifi_dev *pwdev = NULL;

	if (!pData || !pAd) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"ERROR! invalid parametr: pAd = %p, pData=%p\n",
			pAd, pData);
		return FALSE;
	}

	pchan_info = (CMD_RTPRIV_IOCTL_80211_CHAN *)pData;
	/*get wifi_dev*/
	if (pchan_info->pWdev && pchan_info->pWdev->netdev)
	pwdev = RTMP_OS_NETDEV_GET_WDEV(pchan_info->pWdev->netdev);


	if (TRUE != CFG80211DRV_AP_SetChanBw_byWdev(pAd, pwdev, pchan_info)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"ERROR! CFG80211DRV_AP_SetChanBw_byWdev FAIL!\n");
		return FALSE;
	}

	return TRUE;
}


BOOLEAN CFG80211DRV_OpsSetChannel(RTMP_ADAPTER *pAd, VOID *pData)
{
	CMD_RTPRIV_IOCTL_80211_CHAN *pChan;
	UINT8 ChanId, IfType, ChannelType;

	UCHAR ht_bw = 0, vht_bw = 0, eht_bw = 0;
	UCHAR ext_cha = EXTCHA_NONE;
	struct wifi_dev *wdev = NULL, *tdev = NULL;
	BOOLEAN b_ch_change = FALSE, b_bw_change = FALSE;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	UCHAR i = 0;
	struct wlan_config *cfg = NULL;

	wdev = &pMbss->wdev;

	/*
	 *  enum nl80211_channel_type {
	 *	NL80211_CHAN_NO_HT,
	 *	NL80211_CHAN_HT20,
	 *	NL80211_CHAN_HT40MINUS,
	 *	NL80211_CHAN_HT40PLUS
	 *  };
	 */
	/* init */
	pChan = (CMD_RTPRIV_IOCTL_80211_CHAN *)pData;
	ChanId = pChan->ChanId;
	IfType = pChan->IfType;
	ChannelType = pChan->ChanType;

#ifdef HOSTAPD_AUTO_CH_SUPPORT
	printk("HOSTAPD AUTO_CH_SUPPORT Ignore Channel %d from HostAPD \n", pChan->ChanId);
	return TRUE;
#endif

	if (IfType != RT_CMD_80211_IFTYPE_MONITOR) {
		if (ChanId != wdev->channel)
			b_ch_change = TRUE;
	}

	cfg = (struct wlan_config *)wdev->wpf_cfg;
	if (!cfg) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"80211> null wdev->wpf_cfg!\n");
		return FALSE;
	}

	ht_bw = cfg->ht_conf.ht_bw;
	vht_bw = cfg->vht_conf.vht_bw;
	eht_bw = cfg->eht_conf.bw;

	switch (ChannelType) {
	case RT_CMD_80211_CHANTYPE_NOHT:
		ht_bw = HT_BW_20;
		vht_bw = VHT_BW_2040;
		eht_bw = EHT_BW_20;
		break;
	case RT_CMD_80211_CHANTYPE_HT20:
		ht_bw = HT_BW_20;
		vht_bw = VHT_BW_2040;
		eht_bw = EHT_BW_20;
		break;
	case RT_CMD_80211_CHANTYPE_HT40MINUS:
		ht_bw = HT_BW_40;
		ext_cha = EXTCHA_BELOW;
		vht_bw = VHT_BW_2040;
		eht_bw = EHT_BW_2040;
		break;
	case RT_CMD_80211_CHANTYPE_HT40PLUS:
		ht_bw = HT_BW_40;
		ext_cha = EXTCHA_ABOVE;
		vht_bw = VHT_BW_2040;
		eht_bw = EHT_BW_2040;
		break;
	case RT_CMD_80211_CHANTYPE_VHT80:
		ht_bw = HT_BW_40;
		vht_bw = VHT_BW_80;
		eht_bw = EHT_BW_80;
		break;
	case RT_CMD_80211_CHANTYPE_VHT80P80:
		ht_bw = HT_BW_40;
		vht_bw = VHT_BW_8080;
		eht_bw = EHT_BW_160;
		break;
	case RT_CMD_80211_CHANTYPE_VHT160:
		ht_bw = HT_BW_40;
		vht_bw = VHT_BW_160;
		eht_bw = EHT_BW_160;
		break;
	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"80211> ChannelType(%d) not match!\n", ChannelType);
		break;
	}

	if (b_ch_change) {
		if (IfType != RT_CMD_80211_IFTYPE_MONITOR) {
			wdev->channel = ChanId;
			wlan_config_set_ht_bw(wdev, ht_bw);
			wlan_config_set_vht_bw(wdev, vht_bw);
			wlan_config_set_eht_bw(wdev, eht_bw);
			wlan_operate_set_prim_ch(wdev, wdev->channel);
		}
	} else {
		b_bw_change = FALSE;

		for (i = 0; i < WDEV_NUM_MAX; i++) {
			tdev = pAd->wdev_list[i];
			if (tdev) {
				struct wlan_config *cfg = (struct wlan_config *)tdev->wpf_cfg;

				if (cfg) {

					if (cfg->ht_conf.ht_bw != ht_bw) {
						wlan_config_set_ht_bw(tdev, ht_bw);
						cfg->ht_conf.ext_cha = ext_cha;
						b_bw_change = TRUE;
					}
					if (cfg->vht_conf.vht_bw != vht_bw) {
						wlan_config_set_vht_bw(tdev, vht_bw);
						b_bw_change = TRUE;
					}
					if (cfg->eht_conf.bw != eht_bw) {
						wlan_config_set_eht_bw(tdev, eht_bw);
						b_bw_change = TRUE;
					}

				}
			}
		}

		if (b_bw_change) {
			struct freq_cfg freq_cfg;

			/* Reset ht coex result */
			pAd->CommonCfg.BssCoexScanLastResult.bNeedFallBack = FALSE;

			os_zero_mem(&freq_cfg, sizeof(freq_cfg));
			phy_freq_get_cfg(wdev, &freq_cfg);
			operate_loader_phy(wdev, &freq_cfg);

			for (i = 0; i < WDEV_NUM_MAX; i++) {
				tdev = pAd->wdev_list[i];
				if (tdev)
					SetCommonHtVht(pAd, tdev);
			}
		}
	}

	if ((b_ch_change || b_bw_change) && pAd->CommonCfg.bBssCoexEnable) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE,
			"Start 20/40 BSSCoex Channel Scan\n");
		/* Reset ht coex result */
		pAd->CommonCfg.BssCoexScanLastResult.LastScanTime = 0;
		pAd->CommonCfg.BssCoexScanLastResult.bNeedFallBack = FALSE;
		pAd->CommonCfg.bRcvBSSWidthTriggerEvents = FALSE;
		APOverlappingBSSScan(pAd, wdev);
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE,
		"80211> b_ch_change=%d, b_bw_change=%d, New Chan=%d with Ext[%d], ht_bw=%d, vht_bw=%d, eht_bw=%d\n",
		b_ch_change, b_bw_change, ChanId, ext_cha, ht_bw, vht_bw, eht_bw);

	return TRUE;
}

BOOLEAN CFG80211DRV_OpsJoinIbss(
	VOID						*pAdOrg,
	VOID						*pData)
{
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_IBSS *pIbssInfo;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;

	pIbssInfo = (CMD_RTPRIV_IOCTL_80211_IBSS *)pData;
	pAd->StaCfg[0].bAutoReconnect = TRUE;
	pAd->CommonCfg.BeaconPeriod = pIbssInfo->BeaconInterval;

	if (pIbssInfo->privacy) {
		SET_AKM_OPEN(wdev->SecConfig.AKMMap);
		SET_CIPHER_WEP(wdev->SecConfig.PairwiseCipher);
		SET_CIPHER_WEP(wdev->SecConfig.GroupCipher);
		SET_CIPHER_WEP(pAd->StaCfg[0].GroupCipher);
		SET_CIPHER_WEP(pAd->StaCfg[0].PairwiseCipher);
	}

	if (pIbssInfo->BeaconExtraIeLen > 0) {
		const UCHAR *ie = NULL;

		if (pCfg80211_ctrl->BeaconExtraIe != NULL) {
			os_free_mem(pCfg80211_ctrl->BeaconExtraIe);
			pCfg80211_ctrl->BeaconExtraIe = NULL;
		}

		os_alloc_mem(NULL, (UCHAR **)&pCfg80211_ctrl->BeaconExtraIe, pIbssInfo->BeaconExtraIeLen);

		if (pCfg80211_ctrl->BeaconExtraIe != NULL) {
			NdisCopyMemory(pCfg80211_ctrl->BeaconExtraIe, pIbssInfo->BeaconExtraIe, pIbssInfo->BeaconExtraIeLen);
			pCfg80211_ctrl->BeaconExtraIeLen = pIbssInfo->BeaconExtraIeLen;
		} else {
			pCfg80211_ctrl->BeaconExtraIeLen = 0;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
				"CFG80211: MEM ALLOC ERROR\n");
			return FALSE;
		}

		ie = pCfg80211_ctrl->BeaconExtraIe;

		if ((ie[0] == WLAN_EID_VENDOR_SPECIFIC) &&
			(ie[1] >= 4) &&
			(ie[2] == 0x00) && (ie[3] == 0x50) && (ie[4] == 0xf2) && (ie[5] == 0x01)) {
			/* skip wpa_version [6][7] */
			if ((ie[8] == 0x00) && (ie[9] == 0x50) && (ie[10] == 0xf2) && (ie[11] == 0x04)) {
				SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
				SET_CIPHER_CCMP128(wdev->SecConfig.GroupCipher);
				SET_CIPHER_CCMP128(pAd->StaCfg[0].GroupCipher);
				SET_CIPHER_CCMP128(pAd->StaCfg[0].PairwiseCipher);
			} else {
				SET_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher);
				SET_CIPHER_TKIP(wdev->SecConfig.GroupCipher);
				SET_CIPHER_TKIP(pAd->StaCfg[0].GroupCipher);
				SET_CIPHER_TKIP(pAd->StaCfg[0].PairwiseCipher);
			}

			SET_AKM_WPANONE(wdev->SecConfig.AKMMap);
			pAd->StaCfg[0].WpaState = SS_NOTUSE;
		}
	}
	Set_SSID_Proc(pAd, (RTMP_STRING *)pIbssInfo->Ssid);
#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}

BOOLEAN CFG80211DRV_OpsLeave(VOID *pAdOrg, PNET_DEV pNetDev)
{
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;

	/*check if net dev corresponding to Apcli entry */
	if (pAd->StaCfg[0].wdev.if_dev == pNetDev) {
		pAd->cfg80211_ctrl.FlgCfg80211Connecting = FALSE;
		SetApCliEnableByWdev(pAd, &pAd->StaCfg[0].wdev, FALSE);
	}
#endif /* defined(CONFIG_STA_SUPPORT) */
	return TRUE;
}

static VOID CFG80211DRV_GetTxRxRate(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, struct  cfg80211_rate_info *txrate, struct cfg80211_rate_info *rxrate)
{
	ULONG DataRate = 0, DataRate_r = 0;
	struct cfg80211_rate_info *prate_info = NULL;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif

	getRate(pEntry->HTPhyMode, &DataRate);

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	if (cap->fgRateAdaptFWOffload == TRUE) {
		UCHAR phy_mode, rate, bw, sgi, stbc;
		UCHAR phy_mode_r, rate_r, bw_r, sgi_r, stbc_r;
		UCHAR nss;
		UCHAR nss_r;
		UINT32 RawData;
		UINT32 lastTxRate;
		UINT32 lastRxRate = pEntry->LastRxRate;
		UCHAR ucBand = HcGetBandByWdev(pEntry->wdev);

		EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
		EXT_EVENT_PHY_STATE_RX_RATE rRxStatResult;
		union _HTTRANSMIT_SETTING LastTxRate;
		union _HTTRANSMIT_SETTING LastRxRate;

		NdisZeroMemory(&rRxStatResult, sizeof(rRxStatResult));
		NdisZeroMemory(&rTxStatResult, sizeof(rTxStatResult));

		MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
		LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
		LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
		LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
		LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
		LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

		if (LastTxRate.field.MODE >= MODE_VHT)
			LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
		else if (LastTxRate.field.MODE == MODE_OFDM)
			LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
		else
			LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

		lastTxRate = (UINT32)(LastTxRate.word);
		LastRxRate.word = (USHORT)lastRxRate;
		RawData = lastTxRate;
		phy_mode = rTxStatResult.rEntryTxRate.MODE;
		rate = RawData & 0x3F;
		bw = (RawData >> 7) & 0x7;
		sgi = rTxStatResult.rEntryTxRate.ShortGI;
		stbc = ((RawData >> 10) & 0x1);
		nss = rTxStatResult.rEntryTxRate.VhtNss;

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			"Tx: phy_mode=%d, rate=%d, bw=%d, sgi=%d, stbc=%d, nss=%d\n",
			phy_mode, rate, bw, sgi, stbc, nss);

		MtCmdPhyGetRxRate(pAd, CMD_PHY_STATE_CONTENTION_RX_PHYRATE, ucBand, pEntry->wcid, (UINT32 *)&rRxStatResult);
		LastRxRate.field.MODE = rRxStatResult.u1RxMode;
		LastRxRate.field.BW = rRxStatResult.u1BW;
		LastRxRate.field.ldpc = rRxStatResult.u1Coding;
		LastRxRate.field.ShortGI = rRxStatResult.u1Gi ? 1 : 0;
		LastRxRate.field.STBC = rRxStatResult.u1Stbc;

		if (LastRxRate.field.MODE >= MODE_VHT)
			LastRxRate.field.MCS = ((rRxStatResult.u1RxNsts & 0x3) << 4) + rRxStatResult.u1RxRate;
		else if (LastRxRate.field.MODE == MODE_OFDM)
			LastRxRate.field.MCS = getLegacyOFDMMCSIndex(rRxStatResult.u1RxRate) & 0x0000003F;
		else
			LastRxRate.field.MCS = rRxStatResult.u1RxRate;

		phy_mode_r = rRxStatResult.u1RxMode;
		rate_r = rRxStatResult.u1RxRate & 0x3F;
		bw_r = rRxStatResult.u1BW;
		sgi_r = rRxStatResult.u1Gi;
		stbc_r = rRxStatResult.u1Stbc;
		nss_r = rRxStatResult.u1RxNsts + 1;

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			"Rx: phy_mode_r=%d, rate_r=%d, bw_r=%d, sgi_r=%d, stbc_r=%d, nss_r=%d\n",
			phy_mode_r, rate_r, bw_r, sgi_r, stbc_r, nss_r);
/*TX MCS*/
#ifdef DOT11_VHT_AC
		if (phy_mode >= MODE_VHT)
			rate = rate & 0xF;
#endif /* DOT11_VHT_AC */


/*RX MCS*/
#ifdef DOT11_VHT_AC
		if (phy_mode_r >= MODE_VHT)
			rate_r = rate_r & 0xF;
		else
#endif /* DOT11_VHT_AC */
#if DOT11_N_SUPPORT
			if (phy_mode_r >= MODE_HTMIX)
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
					"MODE_HTMIX(%d >= %d): MCS=%d\n",
					phy_mode_r, MODE_HTMIX, rate_r);
		else
#endif
			if (phy_mode_r == MODE_OFDM) {
				if (rate_r == TMI_TX_RATE_OFDM_6M)
					LastRxRate.field.MCS = 0;
				else if (rate_r == TMI_TX_RATE_OFDM_9M)
					LastRxRate.field.MCS = 1;
				else if (rate_r == TMI_TX_RATE_OFDM_12M)
					LastRxRate.field.MCS = 2;
				else if (rate_r == TMI_TX_RATE_OFDM_18M)
					LastRxRate.field.MCS = 3;
				else if (rate_r == TMI_TX_RATE_OFDM_24M)
					LastRxRate.field.MCS = 4;
				else if (rate_r == TMI_TX_RATE_OFDM_36M)
					LastRxRate.field.MCS = 5;
				else if (rate_r == TMI_TX_RATE_OFDM_48M)
					LastRxRate.field.MCS = 6;
				else if (rate_r == TMI_TX_RATE_OFDM_54M)
					LastRxRate.field.MCS = 7;
				else
					LastRxRate.field.MCS = 0;

				rate_r = LastRxRate.field.MCS;
				rxrate->mcs = rate_r;/*rx mcs: ofdm*/
		} else if (phy_mode_r == MODE_CCK) {
			if (rate_r == TMI_TX_RATE_CCK_1M_LP)
				LastRxRate.field.MCS = 0;
			else if (rate_r == TMI_TX_RATE_CCK_2M_LP)
				LastRxRate.field.MCS = 1;
			else if (rate_r == TMI_TX_RATE_CCK_5M_LP)
				LastRxRate.field.MCS = 2;
			else if (rate_r == TMI_TX_RATE_CCK_11M_LP)
				LastRxRate.field.MCS = 3;
			else if (rate_r == TMI_TX_RATE_CCK_2M_SP)
				LastRxRate.field.MCS = 1;
			else if (rate_r == TMI_TX_RATE_CCK_5M_SP)
				LastRxRate.field.MCS = 2;
			else if (rate_r == TMI_TX_RATE_CCK_11M_SP)
				LastRxRate.field.MCS = 3;
			else
				LastRxRate.field.MCS = 0;

			rate_r = LastRxRate.field.MCS;
			rxrate->mcs = LastRxRate.field.MCS;/*rx_mcs:cck*/
		}

		/*tx_gi*/
		if (sgi)
			txrate->flags |= RATE_INFO_FLAGS_SHORT_GI;
		/*rx_gi*/
		if (sgi_r)
			rxrate->flags |= RATE_INFO_FLAGS_SHORT_GI;

		if (phy_mode >= MODE_HE) {
#ifdef DOT11_EHT_BE
			/* be tx */
			if ((phy_mode == MODE_EHT) || (phy_mode == MODE_EHT_ER_SU) || (phy_mode == MODE_EHT_TB) || (phy_mode == MODE_EHT_MU))
				get_rate_eht((rate & 0xf), bw, nss, 0, &DataRate);
			else
#endif
			/* ax tx */
				get_rate_he((rate & 0xf), bw, nss, 0, &DataRate);
			if (sgi == 1)
				DataRate = (DataRate * 967) >> 10;
			if (sgi == 2)
				DataRate = (DataRate * 870) >> 10;

#ifdef DOT11_EHT_BE
			/* be rx */
			if ((phy_mode == MODE_EHT) || (phy_mode == MODE_EHT_ER_SU) || (phy_mode == MODE_EHT_TB) || (phy_mode == MODE_EHT_MU))
				get_rate_eht((rate_r & 0xf), bw_r, nss_r, 0, &DataRate_r);
			else
#endif
			/* ax rx */
				get_rate_he((rate_r & 0xf), bw_r, nss_r, 0, &DataRate_r);
			if (sgi == 1)
				DataRate_r = (DataRate_r * 967) >> 10;
			if (sgi == 2)
				DataRate_r = (DataRate_r * 870) >> 10;
			/*tx rate infos*/
			prate_info = txrate;
#if (KERNEL_VERSION(4, 19, 0) > LINUX_VERSION_CODE)
			/*linux4.4 not support wifi6, set as leagacy*/
			prate_info->flags = 0;
#else
#ifdef DOT11_EHT_BE
			/* be */
			if ((phy_mode == MODE_EHT) || (phy_mode == MODE_EHT_ER_SU) || (phy_mode == MODE_EHT_TB) || (phy_mode == MODE_EHT_MU))
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 0, 0) <= CFG_CFG80211_VERSION)
				prate_info->flags |= RATE_INFO_FLAGS_EHT_MCS;
#else
				prate_info->flags |= BIT(7);   //RATE_INFO_FLAGS_EHT_MCS;
#endif /* CFG_CFG80211_VERSION && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION) */
			else
#endif /* DOT11_EHT_BE */
				prate_info->flags |= RATE_INFO_FLAGS_HE_MCS;
#endif /* KERNEL_VERSION(4, 19, 0) > LINUX_VERSION_CODE */
			prate_info->mcs = rate;
			prate_info->legacy = (UINT16)DataRate;
			prate_info->nss = nss;
			prate_info->bw = bw;

			/*rx rate infos*/
			prate_info = rxrate;
#if (KERNEL_VERSION(4, 19, 0) > LINUX_VERSION_CODE)
			/*linux4.4 not support wifi6, set as leagacy*/
			prate_info->flags = 0;
#else
#ifdef DOT11_EHT_BE
			/* be */
			if ((phy_mode == MODE_EHT) || (phy_mode == MODE_EHT_ER_SU) || (phy_mode == MODE_EHT_TB) || (phy_mode == MODE_EHT_MU))
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 0, 0) <= CFG_CFG80211_VERSION)
				prate_info->flags |= RATE_INFO_FLAGS_EHT_MCS;
#else
				prate_info->flags |= BIT(7);   //RATE_INFO_FLAGS_EHT_MCS;
#endif /* CFG_CFG80211_VERSION && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION) */
			else
#endif
				prate_info->flags |= RATE_INFO_FLAGS_HE_MCS;
#endif /* KERNEL_VERSION(4, 19, 0) > LINUX_VERSION_CODE */
			prate_info->mcs = rate_r;
			prate_info->legacy = (UINT16)DataRate_r;
			prate_info->nss = nss_r;
			prate_info->bw = bw_r;

		} else {
			getRate(LastTxRate, &DataRate);
			getRate(LastRxRate, &DataRate_r);
			/*tx rate infos (for phymode is low than HE)*/
			prate_info = txrate;
			if (phy_mode >= MODE_VHT)
				prate_info->flags |= RATE_INFO_FLAGS_VHT_MCS;
			else if (phy_mode >= MODE_HTMIX)
				prate_info->flags |= RATE_INFO_FLAGS_MCS;
			else
				prate_info->flags = 0;/*other as legacy*/
			prate_info->mcs = rate;
			prate_info->legacy = (UINT16)DataRate;
			prate_info->nss = nss;
			prate_info->bw = bw;

			/*rx rate infos (for phymode is low than HE)*/
			prate_info = rxrate;
			if (phy_mode_r >= MODE_VHT) {
				prate_info->flags |= RATE_INFO_FLAGS_VHT_MCS;
			} else if (phy_mode_r >= MODE_HTMIX) {
				prate_info->flags |= RATE_INFO_FLAGS_MCS;
			} else {
				/*other as legacy*/
				prate_info->flags = 0;
			}
				prate_info->mcs = rate_r;
				prate_info->legacy = (UINT16)DataRate_r;
				prate_info->nss = nss_r;
				prate_info->bw = bw_r;
		}
		/*tx rate infos*/
		prate_info = txrate;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			"Tx: flag=%u, mcs=%u, legacy=%u, nss=%u, bw=%u\n",
			prate_info->flags, prate_info->mcs, prate_info->legacy, prate_info->nss, prate_info->bw);

		/*rx rate infos*/
		prate_info = rxrate;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			"Rx: flag=%u, mcs=%u, legacy=%u, nss=%u, bw=%u\n",
			prate_info->flags, prate_info->mcs, prate_info->legacy, prate_info->nss, prate_info->bw);

	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

}
BOOLEAN CFG80211DRV_StaGet(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_STA *pIbssInfo;
	STA_ADMIN_CONFIG *pstacfg = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	UINT8 i;
	RSSI_SAMPLE rx_rssi_sample;
	UINT8 RxPath;
#ifdef VOW_SUPPORT
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);
#endif


	pIbssInfo = (CMD_RTPRIV_IOCTL_80211_STA *)pData;
	pEntry = MacTableLookup(pAd, pIbssInfo->MAC);

	if (pEntry == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"sta not found\n");
		return FALSE;
	}
	if (!pEntry->wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"ERROR! invalid parameter! <wdev>\n");
		return FALSE;
	}
	if (pEntry->Sst != SST_ASSOC) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"idx: %d, mac: "MACSTR" is disassociated\n",
			pEntry->wcid, MAC2STR(pEntry->Addr));
		return FALSE;
	}

#ifdef CONFIG_STA_SUPPORT
	pstacfg = (STA_ADMIN_CONFIG *)GetStaCfgByWdev(pAd, pEntry->wdev);
#endif

	/* fill tx_bytes count */
	pIbssInfo->tx_bytes = pEntry->TxBytes;

	/* fill tx_packets count */
	pIbssInfo->tx_packets = pEntry->TxPackets.u.LowPart;

	/* fill tx_retries */
	pIbssInfo->tx_retries = (UINT32)pEntry->mpdu_retries.QuadPart;

	/* fill tx_failed  */
	pIbssInfo->tx_failed = pEntry->TxFailCount;

	/* fill rx_bytes */
	pIbssInfo->rx_bytes = pEntry->RxBytes;

	/* fill rx_packets count */
	pIbssInfo->rx_packets = pEntry->RxPackets.u.LowPart;

#ifdef VOW_SUPPORT
	if (chip_dbg->get_sta_airtime) {
		UCHAR ac;

		for (ac = 0; ac < WMM_NUM; ac++) {
			/* fill tx_duration  */
			pIbssInfo->tx_duration += chip_dbg->get_sta_airtime(pAd, pEntry->wcid, ac, TRUE);
			/* fill rx_duration  */
			pIbssInfo->rx_duration += chip_dbg->get_sta_airtime(pAd, pEntry->wcid, ac, FALSE);
		}
	}
#endif /* VOW_SUPPORT */

	/* fill inactive time */
	pIbssInfo->InactiveTime = pEntry->NoDataIdleCount * 1000; /* unit: ms */

	/* fill tx/rx rate */
	CFG80211DRV_GetTxRxRate(pAd, pEntry, &pIbssInfo->txrate, &pIbssInfo->rxrate);

	/* fill signal */
	pIbssInfo->Signal = RTMPAvgRssi(pAd, &pEntry->RssiSample);

	/* fill chains */
	RxPath = pAd->Antenna.field.RxPath;
	RxPath = min(RxPath, (UINT8)(sizeof(pEntry->RssiSample.AvgRssi) / sizeof(CHAR)));
	for (i = 0; i < RxPath; i++) {
		if ((pEntry->RssiSample.AvgRssi[i] > -127) && (pEntry->RssiSample.AvgRssi[i] < 0)) {
			/* fill chains signal*/
			pIbssInfo->chain_signal[i] = pEntry->RssiSample.AvgRssi[i];
			pIbssInfo->chains |= BIT(i);
		}
	}
	/*fill ack signal*/
	if (chip_dbg->get_sta_rx_rcpi) {
		UINT32 rx_rcpi;

		rx_rcpi = chip_dbg->get_sta_rx_rcpi(pAd, pEntry->wcid);

		for (i = 0; i < RxPath; i++) {
			rx_rssi_sample.AvgRssi[i] = (rx_rcpi & (0xff << (i * 8))) >> (i * 8);
			rx_rssi_sample.AvgRssi[i] = RCPI_TO_RSSI(rx_rssi_sample.AvgRssi[i]);
		}
		pIbssInfo->ack_signal = RTMPAvgRssi(pAd, &rx_rssi_sample);
	}

	/* fill associated at */
	pIbssInfo->assoc_at = pEntry->assoc_at;

	/* fill authorized */
	tr_entry = tr_entry_get(pAd, pEntry->wcid);
	pIbssInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_AUTHORIZED);
	if (tr_entry && tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
		pIbssInfo->sta_flags.set |= BIT(NL80211_STA_FLAG_AUTHORIZED);

	/* fill authenticated */
	pIbssInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_AUTHENTICATED);
	if (pEntry->Sst >= SST_AUTH)
		pIbssInfo->sta_flags.set |= BIT(NL80211_STA_FLAG_AUTHENTICATED);

	/* fill associated */
	pIbssInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_ASSOCIATED);
	if (pEntry->Sst == SST_ASSOC)
		pIbssInfo->sta_flags.set |= BIT(NL80211_STA_FLAG_ASSOCIATED);

	/* fill WME */
	pIbssInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_WME);
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
		pIbssInfo->sta_flags.set |= BIT(NL80211_STA_FLAG_WME);

	/* fill MFP */
	pIbssInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_MFP);
	if (pEntry->SecConfig.PmfCfg.UsePMFConnect)
		pIbssInfo->sta_flags.set |= BIT(NL80211_STA_FLAG_MFP);

	/* fill preamble */
	pIbssInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_SHORT_PREAMBLE);
		if (CAP_IS_SHORT_PREAMBLE_ON(pEntry->CapabilityInfo))
			pIbssInfo->sta_flags.set  |= BIT(NL80211_STA_FLAG_SHORT_PREAMBLE);

	/* fill TDLS peer */
	pIbssInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_TDLS_PEER);
	/*not support*/
	pIbssInfo->sta_flags.set  &= ~(BIT(NL80211_STA_FLAG_TDLS_PEER));



	if (IS_ENTRY_PEER_AP(pEntry) && pstacfg && pstacfg->ApcliInfStat.Valid) {
		/*fill connected time*/
		ULONG time_now = 0;

		/* fill connected Time */
		NdisGetSystemUpTime(&time_now);
		time_now -= pstacfg->ApcliInfStat.ApCliLinkUpTime;
		pIbssInfo->connected_time = (UINT32)(time_now/OS_HZ);


#ifdef WIFI_IAP_BCN_STAT_FEATURE
		/*add beacon infos here*/
		pIbssInfo->beacon_mask |= BIT(NL80211_STA_INFO_BEACON_LOSS);
		pIbssInfo->beacon_loss_count = pstacfg->beacon_loss_count;
		pIbssInfo->beacon_mask |= BIT(NL80211_STA_INFO_BEACON_RX);
		pIbssInfo->rx_beacon = pstacfg->rx_beacon;
#endif/*WIFI_IAP_BCN_STAT_FEATURE*/

		for (i = 0; i < MAX_RSSI_LEN; i++)
			rx_rssi_sample.AvgRssi[i] = pstacfg->BcnRssiAvg[i];
		pIbssInfo->rx_beacon_signal_avg = RTMPAvgRssi(pAd, &rx_rssi_sample);

		/* fill bss param */
		if (CAP_IS_SHORT_PREAMBLE_ON(pstacfg->StaActive.CapabilityInfo))
			pIbssInfo->bss_param.flags |= BSS_PARAM_FLAGS_SHORT_PREAMBLE;
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED))
			pIbssInfo->bss_param.flags |= BSS_PARAM_FLAGS_SHORT_SLOT_TIME;
		pIbssInfo->bss_param.dtim_period  = pstacfg->DtimPeriod;
		pIbssInfo->bss_param.beacon_interval = pstacfg->BeaconPeriod;

	} else {
		BSS_STRUCT *pmbss = &pAd->ApCfg.MBSSID[pEntry->wdev->func_idx];

		/* fill connected time */
		pIbssInfo->connected_time = pEntry->StaConnectTime;

		/* fill bss param */
		if (CAP_IS_SHORT_PREAMBLE_ON(pmbss->CapabilityInfo))
			pIbssInfo->bss_param.flags |= BSS_PARAM_FLAGS_SHORT_PREAMBLE;
		if (pAd->CommonCfg.bUseShortSlotTime)
			pIbssInfo->bss_param.flags |= BSS_PARAM_FLAGS_SHORT_SLOT_TIME;
		pIbssInfo->bss_param.dtim_period = pmbss->DtimPeriod;
		pIbssInfo->bss_param.beacon_interval = pAd->CommonCfg.BeaconPeriod;
	}
	return TRUE;
}

BOOLEAN CFG80211DRV_FILL_STAInfo(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry,
	CMD_RTPRIV_IOCTL_80211_STA *pApStaInfo,
	BOOLEAN bReptCli)
{
	INT i = 0;
	ULONG DataRate = 0;
	ULONG DataRate_r = 0;
	ADD_HT_INFO_IE *addht;
	struct cfg80211_rate_info *prate_info = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	struct wifi_dev *wdev = NULL;
	BSS_STRUCT *mbss = NULL;
	STA_ADMIN_CONFIG *pstacfg = NULL;
	UINT rssi_idx = 0;
	INT Rssi_temp = 0;
	UINT32	rx_stream;
	RSSI_SAMPLE RssiSample;
	USHORT bss_capability;
	BOOLEAN bss_shorttime_en;
	UINT8 bss_dtim;
	UINT16 bss_bcn_interval;
	ULONG time_now = 0;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"ERROR! invalid parameter! <pEntry>\n");
		return FALSE;
	}
	if (!pEntry->wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"ERROR! invalid parameter! <wdev>\n");
		return FALSE;
	}
	if (!pApStaInfo) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"ERROR! invalid parameter! <pApStaInfo>\n");
		return FALSE;
	}
#ifdef MAC_REPEATER_SUPPORT
	if (bReptCli == FALSE && IS_ENTRY_REPEATER(pEntry)) {
		/* only dump the apcli entry which not a RepeaterCli */
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
		"REPEATER not Support\n");
		return TRUE;
	}
#endif /* MAC_REPEATER_SUPPORT */

	/*init data*/
	wdev = pEntry->wdev;
#ifdef CONFIG_STA_SUPPORT
	pstacfg = (STA_ADMIN_CONFIG *) GetStaCfgByWdev(pAd, wdev);
#endif /*CONFIG_STA_SUPPORT*/


	if (pEntry->Sst != SST_ASSOC) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"idx: %d, mac: "MACSTR" is disassociated\n",
			pEntry->wcid, MAC2STR(pEntry->Addr));
		return FALSE;
	}

#ifdef MAC_REPEATER_SUPPORT
	if (bReptCli == FALSE) {
		/* only dump the apcli entry which not a RepeaterCli */
		if (IS_REPT_LINK_UP(pEntry->pReptCli)) {
			MTWF_PRINT("%d, mac: "MACSTR" REPT linkup\n",
				pEntry->wcid, MAC2STR(pEntry->Addr));
			return FALSE;
		}
	}
#endif /* MAC_REPEATER_SUPPORT */

	addht = wlan_operate_get_addht(pEntry->wdev);
	DataRate = 0;
	getRate(pEntry->HTPhyMode, &DataRate);

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	if (cap->fgRateAdaptFWOffload == TRUE) {
		UCHAR phy_mode, rate, bw, sgi, stbc;
		UCHAR phy_mode_r, rate_r, bw_r, sgi_r, stbc_r;
		UCHAR nss;
		UCHAR nss_r;
		UINT32 RawData;
		UINT32 lastTxRate;
		UINT32 lastRxRate = pEntry->LastRxRate;
		UCHAR ucBand = HcGetBandByWdev(pEntry->wdev);

		EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
		EXT_EVENT_PHY_STATE_RX_RATE rRxStatResult;
		union _HTTRANSMIT_SETTING LastTxRate;
		union _HTTRANSMIT_SETTING LastRxRate;

		NdisZeroMemory(&rRxStatResult, sizeof(rRxStatResult));
		NdisZeroMemory(&rTxStatResult, sizeof(rTxStatResult));

			MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
			LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
			LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
			LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
			LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
			LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

			if (LastTxRate.field.MODE >= MODE_VHT)
				LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
			else if (LastTxRate.field.MODE == MODE_OFDM)
				LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
			else
				LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

			lastTxRate = (UINT32)(LastTxRate.word);
			LastRxRate.word = (USHORT)lastRxRate;
			RawData = lastTxRate;
			phy_mode = rTxStatResult.rEntryTxRate.MODE;
			rate = RawData & 0x3F;
			bw = (RawData >> 7) & 0x7;
			sgi = rTxStatResult.rEntryTxRate.ShortGI;
			stbc = ((RawData >> 10) & 0x1);
			nss = rTxStatResult.rEntryTxRate.VhtNss;

			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
				"Tx: phy_mode=%d, rate=%d, bw=%d, sgi=%d, stbc=%d, nss=%d\n",
				phy_mode, rate, bw, sgi, stbc, nss);

			MtCmdPhyGetRxRate(pAd, CMD_PHY_STATE_CONTENTION_RX_PHYRATE, ucBand, pEntry->wcid, (UINT32 *)&rRxStatResult);
			LastRxRate.field.MODE = rRxStatResult.u1RxMode;
			LastRxRate.field.BW = rRxStatResult.u1BW;
			LastRxRate.field.ldpc = rRxStatResult.u1Coding;
			LastRxRate.field.ShortGI = rRxStatResult.u1Gi ? 1 : 0;
			LastRxRate.field.STBC = rRxStatResult.u1Stbc;

			if (LastRxRate.field.MODE >= MODE_VHT)
				LastRxRate.field.MCS = ((rRxStatResult.u1RxNsts & 0x3) << 4) + rRxStatResult.u1RxRate;
			else if (LastRxRate.field.MODE == MODE_OFDM)
				LastRxRate.field.MCS = getLegacyOFDMMCSIndex(rRxStatResult.u1RxRate) & 0x0000003F;
			else
				LastRxRate.field.MCS = rRxStatResult.u1RxRate;

			phy_mode_r = rRxStatResult.u1RxMode;
			rate_r = rRxStatResult.u1RxRate & 0x3F;
			bw_r = rRxStatResult.u1BW;
			sgi_r = rRxStatResult.u1Gi;
			stbc_r = rRxStatResult.u1Stbc;
			nss_r = rRxStatResult.u1RxNsts + 1;

			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
				"Rx: phy_mode_r=%d, rate_r=%d, bw_r=%d, sgi_r=%d, stbc_r=%d, nss_r=%d\n",
				phy_mode_r, rate_r, bw_r, sgi_r, stbc_r, nss_r);
/*TX MCS*/
#ifdef DOT11_VHT_AC
			if (phy_mode >= MODE_VHT) {
				rate = rate & 0xF;
			}
#endif /* DOT11_VHT_AC */


/*RX MCS*/
#ifdef DOT11_VHT_AC
			if (phy_mode_r >= MODE_VHT) {
				rate_r = rate_r & 0xF;
			} else
#endif /* DOT11_VHT_AC */
#if DOT11_N_SUPPORT
			if (phy_mode_r >= MODE_HTMIX) {
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
					"MODE_HTMIX(%d >= %d): MCS=%d\n",
					phy_mode_r, MODE_HTMIX, rate_r);
			} else
#endif
			if (phy_mode_r == MODE_OFDM) {
				if (rate_r == TMI_TX_RATE_OFDM_6M)
					LastRxRate.field.MCS = 0;
				else if (rate_r == TMI_TX_RATE_OFDM_9M)
					LastRxRate.field.MCS = 1;
				else if (rate_r == TMI_TX_RATE_OFDM_12M)
					LastRxRate.field.MCS = 2;
				else if (rate_r == TMI_TX_RATE_OFDM_18M)
					LastRxRate.field.MCS = 3;
				else if (rate_r == TMI_TX_RATE_OFDM_24M)
					LastRxRate.field.MCS = 4;
				else if (rate_r == TMI_TX_RATE_OFDM_36M)
					LastRxRate.field.MCS = 5;
				else if (rate_r == TMI_TX_RATE_OFDM_48M)
					LastRxRate.field.MCS = 6;
				else if (rate_r == TMI_TX_RATE_OFDM_54M)
					LastRxRate.field.MCS = 7;
				else
					LastRxRate.field.MCS = 0;

				rate_r = LastRxRate.field.MCS;
				pApStaInfo->rxrate.mcs = rate_r;/*rx mcs: ofdm*/
			} else if (phy_mode_r == MODE_CCK) {
				if (rate_r == TMI_TX_RATE_CCK_1M_LP)
					LastRxRate.field.MCS = 0;
				else if (rate_r == TMI_TX_RATE_CCK_2M_LP)
					LastRxRate.field.MCS = 1;
				else if (rate_r == TMI_TX_RATE_CCK_5M_LP)
					LastRxRate.field.MCS = 2;
				else if (rate_r == TMI_TX_RATE_CCK_11M_LP)
					LastRxRate.field.MCS = 3;
				else if (rate_r == TMI_TX_RATE_CCK_2M_SP)
					LastRxRate.field.MCS = 1;
				else if (rate_r == TMI_TX_RATE_CCK_5M_SP)
					LastRxRate.field.MCS = 2;
				else if (rate_r == TMI_TX_RATE_CCK_11M_SP)
					LastRxRate.field.MCS = 3;
				else
					LastRxRate.field.MCS = 0;

				rate_r = LastRxRate.field.MCS;
				pApStaInfo->rxrate.mcs = LastRxRate.field.MCS;/*rx_mcs:cck*/
			}

			/*tx_gi*/
			if (sgi)
				pApStaInfo->txrate.flags |= RATE_INFO_FLAGS_SHORT_GI;
			/*rx_gi*/
			if (sgi_r)
				pApStaInfo->rxrate.flags |= RATE_INFO_FLAGS_SHORT_GI;

			if (phy_mode >= MODE_HE) {
#ifdef DOT11_EHT_BE
				/* be tx */
				if ((phy_mode == MODE_EHT) || (phy_mode == MODE_EHT_ER_SU) || (phy_mode == MODE_EHT_TB) || (phy_mode == MODE_EHT_MU)) {
					get_rate_eht((rate & 0xf), bw, nss, 0, &DataRate);
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 0, 0) <= CFG_CFG80211_VERSION)
					pApStaInfo->txrate.flags |= RATE_INFO_FLAGS_EHT_MCS;
#else
					pApStaInfo->txrate.flags |= BIT(7);//RATE_INFO_FLAGS_EHT_MCS
#endif /* CFG_CFG80211_VERSION && (KERNEL_VERSION(6, 0, 0) <= CFG_CFG80211_VERSION) */
				} else
#endif
				{
				/* ax tx */
					get_rate_he((rate & 0xf), bw, nss, 0, &DataRate);
					pApStaInfo->txrate.flags |= RATE_INFO_FLAGS_HE_MCS;
				}
				if (sgi == 1)
					DataRate = (DataRate * 967) >> 10;
				if (sgi == 2)
					DataRate = (DataRate * 870) >> 10;

#ifdef DOT11_EHT_BE
				/* be rx */
				if ((phy_mode == MODE_EHT) || (phy_mode == MODE_EHT_ER_SU) || (phy_mode == MODE_EHT_TB) || (phy_mode == MODE_EHT_MU)) {
					get_rate_eht((rate_r & 0xf), bw_r, nss_r, 0, &DataRate_r);
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 0, 0) <= CFG_CFG80211_VERSION)
					pApStaInfo->rxrate.flags |= RATE_INFO_FLAGS_EHT_MCS;
#else
					pApStaInfo->rxrate.flags |= BIT(7);//RATE_INFO_FLAGS_EHT_MCS
#endif /* CFG_CFG80211_VERSION && (KERNEL_VERSION(6, 0, 0) <= CFG_CFG80211_VERSION) */
				} else
#endif
				{
				/* ax rx */
					get_rate_he((rate_r & 0xf), bw_r, nss_r, 0, &DataRate_r);
					pApStaInfo->rxrate.flags |= RATE_INFO_FLAGS_HE_MCS;
				}
				if (sgi == 1)
					DataRate_r = (DataRate_r * 967) >> 10;
				if (sgi == 2)
					DataRate_r = (DataRate_r * 870) >> 10;
				/*tx rate infos*/
				prate_info = &pApStaInfo->txrate;
				prate_info->mcs = rate;
				prate_info->legacy = (UINT16)DataRate;
				prate_info->nss = nss;
				prate_info->bw = bw;

				/*rx rate infos*/
				prate_info = &pApStaInfo->rxrate;
				prate_info->mcs = rate_r;
				prate_info->legacy = (UINT16)DataRate_r;
				prate_info->nss = nss_r;
				prate_info->bw = bw_r;

			} else {
				getRate(LastTxRate, &DataRate);
				getRate(LastRxRate, &DataRate_r);
				/*tx rate infos (for phymode is low than HE)*/
				prate_info = &pApStaInfo->txrate;
				if (phy_mode >= MODE_VHT) {
					prate_info->flags |= RATE_INFO_FLAGS_VHT_MCS;
				} else if (phy_mode >= MODE_HTMIX) {
					prate_info->flags |= RATE_INFO_FLAGS_MCS;
				} else {
					prate_info->flags = 0;/*other as legacy*/
				}
				prate_info->mcs = rate;
				prate_info->legacy = (UINT16)DataRate;
				prate_info->nss = nss;
				prate_info->bw = bw;

			/*rx rate infos (for phymode is low than HE)*/
			prate_info = &pApStaInfo->rxrate;
			if (phy_mode_r >= MODE_VHT) {
				prate_info->flags |= RATE_INFO_FLAGS_VHT_MCS;
			} else if (phy_mode_r >= MODE_HTMIX) {
				prate_info->flags |= RATE_INFO_FLAGS_MCS;
			} else {
				/*other as legacy*/
				prate_info->flags = 0;
			}
			prate_info->mcs = rate_r;
			prate_info->legacy = (UINT16)DataRate_r;
			prate_info->nss = nss_r;
			prate_info->bw = bw_r;
		}
		/*tx rate infos*/
		prate_info = &pApStaInfo->txrate;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			"\nTx: flag=%u, mcs=%u, legacy=%u, nss=%u, bw=%u\n",
			prate_info->flags, prate_info->mcs, prate_info->legacy, prate_info->nss, prate_info->bw);

		/*rx rate infos*/
		prate_info = &pApStaInfo->rxrate;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			"\nRx: flag=%u, mcs=%u, legacy=%u, nss=%u, bw=%u\n",
			prate_info->flags, prate_info->mcs, prate_info->legacy, prate_info->nss, prate_info->bw);

	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
		/*apcli entry infos*/
		if (IS_ENTRY_PEER_AP(pEntry) && pstacfg && pstacfg->ApcliInfStat.Valid) {

			/*Signal_avg*/
			pApStaInfo->signal_avg = RTMPAvgRssi(pAd, &pstacfg->RssiSample);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
					"\npApStaInfo->signal_avg:%d\n",
					pApStaInfo->signal_avg);

			/*Signal*/
			rx_stream = pAd->Antenna.field.RxPath;
			rx_stream = min(rx_stream, (UINT32)(sizeof(RssiSample.LastRssi) / sizeof(CHAR)));
			RssiSample = pstacfg->RssiSample;
			for (rssi_idx = 0; rssi_idx < rx_stream; rssi_idx++) {
				Rssi_temp += RssiSample.LastRssi[rssi_idx];
			}
			pApStaInfo->Signal = (CHAR)(Rssi_temp/rx_stream);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
					"\nRssi_temp:%d, rx_stream:%d, pApStaInfo->Signal:%d\n",
					Rssi_temp, rssi_idx, pApStaInfo->Signal);
			/*InactiveTime*/
			pApStaInfo->InactiveTime = pEntry->NoDataIdleCount * 1000; /* unit: ms */

			/*connectedTime*/
			NdisGetSystemUpTime(&time_now);
			time_now -= pstacfg->ApcliInfStat.ApCliLinkUpTime;
			pApStaInfo->connected_time = (UINT32)(time_now/OS_HZ);/*sec*/
			/*tx_packets*/
			pApStaInfo->tx_packets = (UINT32)pstacfg->StaStatistic.TxCount;
			/*tx_bytes*/
			pApStaInfo->tx_bytes = (UINT32)pstacfg->StaStatistic.TransmittedByteCount;
			/*tx_failed*/
			pApStaInfo->tx_failed = (UINT)pEntry->TxFailCount;
			/*rx_packets*/
			pApStaInfo->rx_packets = (UINT32)pstacfg->StaStatistic.RxCount;
			/*rx_bytes*/
			pApStaInfo->rx_bytes = (UINT32)pstacfg->StaStatistic.ReceivedByteCount;
			/*tx_retries*/
			#ifdef EAP_STATS_SUPPORT
			pApStaInfo->tx_retries = (UINT32)pEntry->mpdu_retries.QuadPart;
			#endif
			#ifdef WIFI_IAP_BCN_STAT_FEATURE
			/*add beacon infos here*/
			pApStaInfo->beacon_mask |= BIT(NL80211_STA_INFO_BEACON_LOSS);
			pApStaInfo->beacon_loss_count = pstacfg->beacon_loss_count;
			pApStaInfo->beacon_mask |= BIT(NL80211_STA_INFO_BEACON_RX);
			pApStaInfo->rx_beacon = pstacfg->rx_beacon;

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
				"\nbeacon_loss_count  = %u, rx_beacon = %lu;\n",
				pstacfg->beacon_loss_count, pstacfg->rx_beacon);
			#endif/*WIFI_IAP_BCN_STAT_FEATURE*/
		}  else {

			/*Signal_avg*/
			pApStaInfo->signal_avg = RTMPAvgRssi(pAd, &pEntry->RssiSample);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
					"\npApStaInfo->signal_avg:%d\n",
					pApStaInfo->signal_avg);

			/*Signal*/
			rx_stream = pAd->Antenna.field.RxPath;
			rx_stream = min(rx_stream, (UINT32)(sizeof(RssiSample.LastRssi) / sizeof(CHAR)));
			RssiSample = pEntry->RssiSample;
			for (rssi_idx = 0; rssi_idx < rx_stream; rssi_idx++) {
				Rssi_temp += RssiSample.LastRssi[rssi_idx];
			}
			pApStaInfo->Signal = (CHAR)(Rssi_temp/rx_stream);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
					"\nRssi_temp:%d, rx_stream:%d, pApStaInfo->Signal:%d\n",
					Rssi_temp, rssi_idx, pApStaInfo->Signal);
			/*InactiveTime*/
			pApStaInfo->InactiveTime = pEntry->NoDataIdleCount * 1000; /* unit: ms */
			/*connectedtime*/
			pApStaInfo->connected_time = pEntry->StaConnectTime;/*sec*/
			/*tx_packets*/
			pApStaInfo->tx_packets = (UINT32)pEntry->TxPackets.u.LowPart;
			/*tx_bytes*/
			pApStaInfo->tx_bytes = (UINT64)pEntry->TxBytes;
			/*tx_failed*/
			pApStaInfo->tx_failed = (UINT32)pAd->WlanCounters.FailedCount.u.LowPart;
			/*rx_packets*/
			pApStaInfo->rx_packets = (UINT32)pEntry->RxPackets.u.LowPart;
			/*rx_bytes*/
			pApStaInfo->rx_bytes = (UINT64)pEntry->RxBytes;
		#ifdef EAP_STATS_SUPPORT
			/*tx_retries*/
			pApStaInfo->tx_retries = (UINT32)pEntry->mpdu_retries.QuadPart;
		#endif
}

			/*sta_flages*/
			pApStaInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_WME);/*wmm*/
			if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) {
				pApStaInfo->sta_flags.set  |= BIT(NL80211_STA_FLAG_WME);
			} else {
				pApStaInfo->sta_flags.set &= ~(BIT(NL80211_STA_FLAG_WME));
			}

			pApStaInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_MFP);/*pmf*/
#ifdef DOT11W_PMF_SUPPORT
			if (pEntry->SecConfig.PmfCfg.UsePMFConnect) {
				pApStaInfo->sta_flags.set  |= BIT(NL80211_STA_FLAG_MFP);
			} else
#endif
			{
				pApStaInfo->sta_flags.set &= ~(BIT(NL80211_STA_FLAG_MFP));
			}

			pApStaInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_SHORT_PREAMBLE);/*sta preamble*/
			if (CAP_IS_SHORT_PREAMBLE_ON(pEntry->CapabilityInfo)) {
				pApStaInfo->sta_flags.set  |= BIT(NL80211_STA_FLAG_SHORT_PREAMBLE);
			} else {
				pApStaInfo->sta_flags.set &= ~(BIT(NL80211_STA_FLAG_SHORT_PREAMBLE));
			}

	pApStaInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_AUTHENTICATED);/*AUTHENTICATED*/
	pApStaInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_ASSOCIATED);/*assoc*/
	if (pEntry->Sst == SST_ASSOC) {
		pApStaInfo->sta_flags.set  |= BIT(NL80211_STA_FLAG_AUTHENTICATED);
		pApStaInfo->sta_flags.set  |= BIT(NL80211_STA_FLAG_ASSOCIATED);
	} else {
		pApStaInfo->sta_flags.set &= ~(BIT(NL80211_STA_FLAG_ASSOCIATED));
		if (pEntry->Sst == SST_AUTH) {
			pApStaInfo->sta_flags.set  |= BIT(NL80211_STA_FLAG_AUTHENTICATED);
		} else {
			pApStaInfo->sta_flags.set &= ~(BIT(NL80211_STA_FLAG_AUTHENTICATED));
		}
	}

	/*802.x authorized*/
	pApStaInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_AUTHORIZED);
	if (IS_WCID_VALID(pAd, pEntry->wcid)) {
		tr_entry = tr_entry_get(pAd, pEntry->wcid);
		if (tr_entry && tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) {
			pApStaInfo->sta_flags.set  |= BIT(NL80211_STA_FLAG_AUTHORIZED);
		} else {
			pApStaInfo->sta_flags.set &= ~(BIT(NL80211_STA_FLAG_AUTHORIZED));
		}

	}

/*apcli or mbss infos fill*/

	/*fill bss from apcli*/
	if (IS_ENTRY_PEER_AP(pEntry) && pstacfg && pstacfg->ApcliInfStat.Valid)  {
		bss_capability = pstacfg->StaActive.CapabilityInfo;
		bss_shorttime_en  = OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
		bss_dtim = pstacfg->DtimPeriod;
		bss_bcn_interval = pstacfg->BeaconPeriod;
	} else {
		for (i = 0; i < MAX_BEACON_NUM ; i++) {
			if (wdev == &pAd->ApCfg.MBSSID[i].wdev) {
				mbss = &pAd->ApCfg.MBSSID[i];
				bss_capability = mbss->CapabilityInfo;
				bss_shorttime_en = pAd->CommonCfg.bUseShortSlotTime;
				bss_dtim = mbss->DtimPeriod;
				bss_bcn_interval = pAd->CommonCfg.BeaconPeriod;
				break;
			}
		}

		if (i >= MAX_BEACON_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
			"\nERROR ! CAN'T FIND BSS INFOS\n");
			return TRUE;
		}
	}

	/*linux4.4 only check ht protection */
	if (wlan_config_get_ht_protect_en(wdev)) {
		pApStaInfo->bss_param.flags |= BSS_PARAM_FLAGS_CTS_PROT;
	} else {
		pApStaInfo->bss_param.flags &= ~BSS_PARAM_FLAGS_CTS_PROT;
	}
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			"\nBSS_PARAM_FLAGS_CTS_PROT(%d)\n",
			wlan_config_get_ht_protect_en(wdev));
	/*short preamble*/
	if (CAP_IS_SHORT_PREAMBLE_ON(bss_capability)) {
		pApStaInfo->bss_param.flags |= BSS_PARAM_FLAGS_SHORT_PREAMBLE;
	} else {
		pApStaInfo->bss_param.flags &= ~BSS_PARAM_FLAGS_SHORT_PREAMBLE;
	}
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			"\nBSS_PARAM_FLAGS_SHORT_PREAMBLE(%d)\n",
			CAP_IS_SHORT_PREAMBLE_ON(bss_capability));

	/*slot time*/
	if (bss_shorttime_en) {
		pApStaInfo->bss_param.flags |= BSS_PARAM_FLAGS_SHORT_SLOT_TIME;
	} else {
		pApStaInfo->bss_param.flags &= ~BSS_PARAM_FLAGS_SHORT_SLOT_TIME;
	}
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			"\nBSS_PARAM_FLAGS_SHORT_SLOT_TIME(%d)\n",
			bss_shorttime_en);

	/*dtim and beancon_interval value */
	if (bss_dtim > 0) {
		pApStaInfo->bss_param.dtim_period = bss_dtim;/*dtim*/
	} else {
		pApStaInfo->bss_param.dtim_period = 0;
	}
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			"\nNL80211_STA_BSS_PARAM_DTIM_PERIOD(%d)\n",
			pApStaInfo->bss_param.dtim_period);

	/*beacon_interval*/
	if (bss_bcn_interval > 0) {
		/* dbdc*/
		pApStaInfo->bss_param.beacon_interval = bss_bcn_interval;
	} else {
		pApStaInfo->bss_param.beacon_interval = 0;
	}
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
		"\nNL80211_STA_BSS_PARAM_BEACON_INTERVAL:%d\n",
		pApStaInfo->bss_param.beacon_interval);
	return TRUE;
}



BOOLEAN CFG80211DRV_Ap_StaGet(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_STA *pApStaInfo = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	pApStaInfo = (CMD_RTPRIV_IOCTL_80211_STA *)pData;

	pEntry = MacTableLookup(pAd, pApStaInfo->MAC);

	if (FALSE == CFG80211DRV_FILL_STAInfo(pAd, pEntry, pApStaInfo, FALSE)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
				"ERROR! CFG80211DRV_FILL_STAInfo Fail.\n");
		return FALSE;
	}
	return TRUE;
}


BOOLEAN CFG80211DRV_StaKeyAdd(
	VOID						*pAdOrg,
	VOID						*pData)
{
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_KEY *pKeyInfo;
	pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;

	if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40 || pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP104) {
		RT_CMD_STA_IOCTL_SECURITY IoctlSec;
		MAC_TABLE_ENTRY *pEntry = NULL;
		INT groupWcid = 0;
		os_zero_mem(&IoctlSec, sizeof(RT_CMD_STA_IOCTL_SECURITY));

		if (ADHOC_ON(pAd))
			groupWcid = pAd->StaCfg[0].wdev.tr_tb_idx;

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO, "RT_CMD_80211_KEY_WEP\n");
		pEntry = entry_get(pAd, BSSID_WCID_TO_REMOVE);
		IoctlSec.KeyIdx = pKeyInfo->KeyId;
		IoctlSec.pData = pKeyInfo->KeyBuf;
		IoctlSec.length = pKeyInfo->KeyLen;
		IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_WEP;
		IoctlSec.flags = RT_CMD_STA_IOCTL_SECURITY_ENABLED;
		RTMP_STA_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_STA_SIOCSIWENCODEEXT, 0,
							 &IoctlSec, 0, INT_MAIN);
#ifdef MT_MAC

		if (pKeyInfo->bPairwise == FALSE)
		{
			if (IS_HIF_TYPE(pAd, HIF_MT)) {
				struct _ASIC_SEC_INFO *info = NULL;

				if (ADHOC_ON(pAd)) {
					UINT i = 0;

					for (i = BSSID_WCID_TO_REMOVE; i < groupWcid; i++) {
						pEntry = entry_get(pAd, i);

						if (pEntry->wcid == 0)
							continue;

						if (IS_ENTRY_ADHOC(pEntry)) {
							/* Set key material to Asic */
							os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
							if (info) {
								os_zero_mem(info, sizeof(ASIC_SEC_INFO));
								info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
								info->Direction = SEC_ASIC_KEY_BOTH;
								info->Wcid = pEntry->wcid;
								info->BssIndex = BSS0;
								info->Cipher = pEntry->SecConfig.PairwiseCipher;
								info->KeyIdx = pKeyInfo->KeyId;
								os_move_mem(&info->PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
								os_move_mem(&info->Key, &pEntry->SecConfig.WepKey[info->KeyIdx], sizeof(SEC_KEY_INFO));
								HW_ADDREMOVE_KEYTABLE(pAd, info);
								os_free_mem(info);
							}
						} else
							MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
									 "=====> can't add to [%d]Wcid %d, type=%d\n", i,
									  pEntry->wcid, pEntry->EntryType);
					}
				}

				/* Set key material to Asic */
				os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
				if (info) {
					os_zero_mem(info, sizeof(ASIC_SEC_INFO));
					info->Operation = SEC_ASIC_ADD_GROUP_KEY;
					info->Direction = SEC_ASIC_KEY_RX;
					info->Wcid = groupWcid;
					info->BssIndex = BSS0;
					info->Cipher = pEntry->SecConfig.GroupCipher;
					info->KeyIdx = pKeyInfo->KeyId;
					os_move_mem(&info->PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
					os_move_mem(&info->Key, &pEntry->SecConfig.WepKey[info->KeyIdx], sizeof(SEC_KEY_INFO));
					HW_ADDREMOVE_KEYTABLE(pAd, info);
					os_free_mem(info);
				}
			}
		} else {
			struct _ASIC_SEC_INFO *info = NULL;
			/* Set key material to Asic */
			os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
			if (info) {
				os_zero_mem(info, sizeof(ASIC_SEC_INFO));
				info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
				info->Direction = SEC_ASIC_KEY_BOTH;
				info->Wcid = pEntry->wcid;
				info->BssIndex = BSS0;
				info->Cipher = pEntry->SecConfig.PairwiseCipher;
				info->KeyIdx = pKeyInfo->KeyId;
				os_move_mem(&info->PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
				os_move_mem(&info->Key, &pEntry->SecConfig.WepKey[info->KeyIdx], sizeof(SEC_KEY_INFO));
				HW_ADDREMOVE_KEYTABLE(pAd, info);
				os_free_mem(info);
			}
		}

#endif /* MT_MAC */
	} else {
		RT_CMD_STA_IOCTL_SECURITY IoctlSec;
		os_zero_mem(&IoctlSec, sizeof(RT_CMD_STA_IOCTL_SECURITY));

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			"Set_WPAPSK_Proc ==> id:%d, type:%d, len:%d\n",
				 pKeyInfo->KeyId, pKeyInfo->KeyType, (INT32)strlen(pKeyInfo->KeyBuf));
		IoctlSec.KeyIdx = pKeyInfo->KeyId;
		IoctlSec.pData = pKeyInfo->KeyBuf;
		IoctlSec.length = pKeyInfo->KeyLen;

		/* YF@20120327: Due to WepStatus will be set in the cfg connect function.*/
		if (IS_CIPHER_TKIP_Entry(&pAd->StaCfg[0].wdev))
			IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_TKIP;
		else if (IS_CIPHER_AES_Entry(&pAd->StaCfg[0].wdev))
			IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_CCMP;

		IoctlSec.flags = RT_CMD_STA_IOCTL_SECURITY_ENABLED;

		if (pKeyInfo->bPairwise == FALSE)
		{
			if (IS_CIPHER_TKIP(pAd->StaCfg[0].GroupCipher))
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_TKIP;
			else if (IS_CIPHER_CCMP128(pAd->StaCfg[0].GroupCipher))
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_CCMP;

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO, "Install GTK: %d\n", IoctlSec.Alg);
			IoctlSec.ext_flags = RT_CMD_STA_IOCTL_SECURTIY_EXT_GROUP_KEY;
		} else {
			if (IS_CIPHER_TKIP(pAd->StaCfg[0].PairwiseCipher))
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_TKIP;
			else if (IS_CIPHER_CCMP128(pAd->StaCfg[0].PairwiseCipher))
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_CCMP;

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO, "Install PTK: %d\n", IoctlSec.Alg);
			IoctlSec.ext_flags = RT_CMD_STA_IOCTL_SECURTIY_EXT_SET_TX_KEY;
		}

		/*Set_GroupKey_Proc(pAd, &IoctlSec) */
		RTMP_STA_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_STA_SIOCSIWENCODEEXT, 0,
							 &IoctlSec, 0, INT_MAIN);

		if (IS_AKM_WPANONE(pAd->StaCfg[0].wdev.SecConfig.AKMMap)) {
			if (IS_CIPHER_TKIP(pAd->StaCfg[0].PairwiseCipher))
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_TKIP;
			else if (IS_CIPHER_CCMP128(pAd->StaCfg[0].PairwiseCipher))
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_CCMP;

			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO, "Install ADHOC PTK: %d\n", IoctlSec.Alg);
			IoctlSec.ext_flags = RT_CMD_STA_IOCTL_SECURTIY_EXT_SET_TX_KEY;
			RTMP_STA_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_STA_SIOCSIWENCODEEXT, 0,
								 &IoctlSec, 0, INT_MAIN);
		}
	}

#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}

BOOLEAN CFG80211DRV_Connect(
	VOID	*pAdOrg,
	VOID	*pData)
{
#ifdef APCLI_CFG80211_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) pAdOrg;
	CMD_RTPRIV_IOCTL_80211_CONNECT *pConnInfo;
	UCHAR SSID[NDIS_802_11_LENGTH_SSID + 1]; /* Add One for SSID_Len == 32 */
	UINT32 SSIDLen;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	INT staidx;
#ifdef DOT11W_PMF_SUPPORT
	struct _PMF_CFG *pPmfCfg = NULL;
#endif
	struct wifi_dev *wdev;
	struct _SECURITY_CONFIG *sec_cfg;


	pConnInfo = (CMD_RTPRIV_IOCTL_80211_CONNECT *) pData;

	SSIDLen = pConnInfo->SsidLen;
	if (SSIDLen > NDIS_802_11_LENGTH_SSID)
		SSIDLen = NDIS_802_11_LENGTH_SSID;

	memset(&SSID, 0, sizeof(SSID));
	memcpy(SSID, pConnInfo->pSsid, SSIDLen);

	staidx = CFG80211_FindStaIdxByNetDevice(pAd, pConnInfo->pNetDev);
	if (staidx == WDEV_NOT_FOUND) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			"STATION Interface for connection not found\n");
		return TRUE;
	}

	wdev = &(pAd->StaCfg[staidx].wdev);
	sec_cfg = &wdev->SecConfig;
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
#ifdef DOT11W_PMF_SUPPORT
	pPmfCfg = &sec_cfg->PmfCfg;
#endif
	pStaCfg->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE;

	/* Check the connection is WPS or not */
	if (pConnInfo->bWpsConnection) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			"AP_CLI WPS Connection onGoing.....\n");
		pStaCfg->wpa_supplicant_info.WpaSupplicantUP |= WPA_SUPPLICANT_ENABLE_WPS;
	}

	sec_cfg->AuthType = pConnInfo->AuthType;

	switch (pConnInfo->AkmSuite) {
	case WLAN_AKM_SUITE_8021X:
		if (pConnInfo->WpaVer == 2)
			Set_ApCli_AuthMode(pAd, staidx, "WPA2");
		else
			Set_ApCli_AuthMode(pAd, staidx, "WPA");
		break;
	case WLAN_AKM_SUITE_PSK:
		if (pConnInfo->WpaVer == 2)
			Set_ApCli_AuthMode(pAd, staidx, "WPA2PSK");
		else
			Set_ApCli_AuthMode(pAd, staidx, "WPAPSK");
		break;
	case WLAN_AKM_SUITE_8021X_SHA256:
		Set_ApCli_AuthMode(pAd, staidx, "WPA3");
		break;
	case WLAN_AKM_SUITE_PSK_SHA256:
		Set_ApCli_AuthMode(pAd, staidx, "WPA2PSK");
		break;
#ifdef SUPP_SAE_SUPPORT
	case WLAN_AKM_SUITE_SAE:
		Set_ApCli_AuthMode(pAd, staidx, "WPA3PSK");
		break;
#endif /* SUPP_SAE_SUPPORT */
#ifdef HOSTAPD_SUITEB_SUPPORT
	case WLAN_AKM_SUITE_8021X_SUITE_B_192:
		Set_ApCli_AuthMode(pAd, staidx, "WPA3-192");
		break;
#endif /* HOSTAPD_SUITEB_SUPPORT */
#ifdef SUPP_OWE_SUPPORT
	case WLAN_AKM_SUITE_OWE:
		Set_ApCli_AuthMode(pAd, staidx, "OWE");
		break;
#endif /* SUPP_OWE_SUPPORT */
#if defined(MTK_HOSTAPD_SUPPORT) && defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
	case WLAN_AKM_SUITE_WFA_DPP:
		// if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd))
		Set_ApCli_AuthMode(pAd, staidx, "DPP");
		break;
#endif /* MTK_HOSTAPD_SUPPORT && CONFIG_MAP_SUPPORT && MAP_R3 */
#ifdef SUPP_SAE_SUPPORT
	case WLAN_AKM_SUITE_SAE_EXT:
		Set_ApCli_AuthMode(pAd, staidx, "WPA3PSK_EXT");
		break;
#endif /* SUPP_SAE_SUPPORT */
	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"Unknown AkmSuite: %08x\n", pConnInfo->AkmSuite);
		if (pConnInfo->AuthType == Ndis802_11AuthModeShared)
			Set_ApCli_AuthMode(pAd, staidx, "SHARED");
		else if (pConnInfo->AuthType == Ndis802_11AuthModeOpen)
			Set_ApCli_AuthMode(pAd, staidx, "OPEN");
		else
			Set_ApCli_AuthMode(pAd, staidx, "WEPAUTO");
	}

	switch (pConnInfo->Pairwise) {
	case WLAN_CIPHER_SUITE_WEP40:
	case WLAN_CIPHER_SUITE_WEP104:
		Set_ApCli_EncrypType(pAd, staidx, "WEP");

		if (pConnInfo->pKey) {
			UCHAR KeyBuf[50] = {0};

			sec_cfg->PairwiseKeyId = pConnInfo->KeyIdx;/* base 0 */

			if (pConnInfo->KeyLen >= sizeof(KeyBuf))
				return FALSE;

			memset(KeyBuf, 0, sizeof(KeyBuf));
			memcpy(KeyBuf, pConnInfo->pKey, pConnInfo->KeyLen);
			KeyBuf[pConnInfo->KeyLen] = 0x00;

			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
				"sec_cfg->DefaultKeyId = %d\n",
				 sec_cfg->PairwiseKeyId);

			Set_Wep_Key_Proc(pAd, (RTMP_STRING *) KeyBuf, (INT) pConnInfo->KeyLen,
					(INT) pConnInfo->KeyIdx);
		}

		break;
	case WLAN_CIPHER_SUITE_TKIP:
		Set_ApCli_EncrypType(pAd, staidx, "TKIP");
		break;
	case WLAN_CIPHER_SUITE_CCMP:
		Set_ApCli_EncrypType(pAd, staidx, "AES");
		break;
	case WLAN_CIPHER_SUITE_GCMP:
		Set_ApCli_EncrypType(pAd, staidx, "GCMP128");
		break;
#ifdef HOSTAPD_SUITEB_SUPPORT
	case WLAN_CIPHER_SUITE_GCMP_256:
		Set_ApCli_EncrypType(pAd, staidx, "GCMP256");
		break;
#endif /* HOSTAPD_SUITEB_SUPPORT */
	case WLAN_CIPHER_SUITE_CCMP_256:
		Set_ApCli_EncrypType(pAd, staidx, "CCMP256");
		break;
	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"Unknown Pairwise cipher: %08x\n", pConnInfo->Pairwise);
		Set_ApCli_EncrypType(pAd, staidx, "NONE");
	}

	if (pConnInfo->pBssid != NULL) {
		os_zero_mem(pStaCfg->CfgApCliBssid, MAC_ADDR_LEN);
		NdisCopyMemory(pStaCfg->CfgApCliBssid, pConnInfo->pBssid, MAC_ADDR_LEN);
	}

#ifdef DOT11W_PMF_SUPPORT
	pPmfCfg->Desired_MFPC = pConnInfo->mfpc;
	pPmfCfg->Desired_MFPR = pConnInfo->mfpr;

	pPmfCfg->MFPC = pPmfCfg->Desired_MFPC;
	pPmfCfg->MFPR = pPmfCfg->Desired_MFPR;
	pPmfCfg->PMFSHA256 = pPmfCfg->MFPR;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO,
		"[PMF]: MFPC=%d, MFPR=%d, SHA256=%d\n",
		pPmfCfg->MFPC, pPmfCfg->MFPR, pPmfCfg->PMFSHA256);
#endif /* DOT11W_PMF_SUPPORT */

	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

	pAd->cfg80211_ctrl.FlgCfg80211Connecting = TRUE;
	Set_ApCli_Ssid(pAd, staidx, (RTMP_STRING *) SSID);

	Set_ApCli_Enable(pAd, staidx, "1");

	upSpecificApCliIf(pAd, staidx);

	MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_ERROR,
		"80211> APCLI CONNECTING SSID = %s\n", SSID);
#endif /* APCLI_CFG80211_SUPPORT */

	return TRUE;
}


VOID CFG80211DRV_RegNotify(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_REG_NOTIFY *pRegInfo;

	pRegInfo = (CMD_RTPRIV_IOCTL_80211_REG_NOTIFY *)pData;
	/* keep Alpha2 and we can re-call the function when interface is up */
	pAd->cfg80211_ctrl.Cfg80211_Alpha2[0] = pRegInfo->Alpha2[0];
	pAd->cfg80211_ctrl.Cfg80211_Alpha2[1] = pRegInfo->Alpha2[1];

	/* apply the new regulatory rule only when cert mode */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		/* interface is up */
		if (pAd->CommonCfg.wifi_cert)
			CFG80211_RegRuleApply(pAd, pRegInfo->pWiphy, (UCHAR *)pRegInfo->Alpha2);
	} else
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"crda> interface is down!\n");
}


VOID CFG80211DRV_SurveyGet(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_SURVEY *pSurveyInfo;
#ifdef AP_QLOAD_SUPPORT
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);
#endif
	pSurveyInfo = (CMD_RTPRIV_IOCTL_80211_SURVEY *)pData;
	pSurveyInfo->pCfg80211 = pAd->pCfg80211_CB;
#ifdef AP_QLOAD_SUPPORT
	pSurveyInfo->ChannelTimeBusy = pQloadCtrl->QloadLatestChannelBusyTimePri;
	pSurveyInfo->ChannelTimeExtBusy = pQloadCtrl->QloadLatestChannelBusyTimeSec;
#endif /* AP_QLOAD_SUPPORT */
}


VOID CFG80211_UnRegister(
	IN VOID						*pAdOrg,
	IN VOID						*pNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

	/* sanity check */
	if (pAd->pCfg80211_CB == NULL)
		return;

	CFG80211OS_UnRegister(pAd->pCfg80211_CB, pNetDev);
	RTMP_DRIVER_80211_SCAN_STATUS_LOCK_INIT(pAd, FALSE);
	unregister_netdevice_notifier(&cfg80211_netdev_notifier);
	/* Reset CFG80211 Global Setting Here */
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
		"==========> TYPE Reset CFG80211 Global Setting Here <==========\n");
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = FALSE,
									pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount = 0;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = FALSE;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount = 0;
	pAd->pCfg80211_CB = NULL;
	pAd->CommonCfg.HT_Disable = 0;

	/* It should be free when ScanEnd, */
	/*  But Hit close the device in Scanning */
	if (pCfg80211_ctrl->pCfg80211ChanList != NULL) {
		os_free_mem(pCfg80211_ctrl->pCfg80211ChanList);
		pCfg80211_ctrl->pCfg80211ChanList = NULL;
	}

	pCfg80211_ctrl->Cfg80211ChanListLen = 0;
	pCfg80211_ctrl->Cfg80211CurChanIndex = 0;

	if (pCfg80211_ctrl->pExtraIe) {
		os_free_mem(pCfg80211_ctrl->pExtraIe);
		pCfg80211_ctrl->pExtraIe = NULL;
	}

	pCfg80211_ctrl->ExtraIeLen = 0;
	/*
	 * CFG_TODO
	 *    if (pAd->pTxStatusBuf != NULL)
	 *    {
	 *	 os_free_mem(pAd->pTxStatusBuf);
	 *	 pAd->pTxStatusBuf = NULL;
	 *   }
	 *	 pAd->TxStatusBufLen = 0;
	 */
#ifdef CONFIG_AP_SUPPORT

	if (pAd->cfg80211_ctrl.beacon_tail_buf != NULL) {
		os_free_mem(pAd->cfg80211_ctrl.beacon_tail_buf);
		pAd->cfg80211_ctrl.beacon_tail_buf = NULL;
	}

	pAd->cfg80211_ctrl.beacon_tail_len = 0;
#endif /* CONFIG_AP_SUPPORT */

	if (pAd->cfg80211_ctrl.BeaconExtraIe != NULL) {
		os_free_mem(pAd->cfg80211_ctrl.BeaconExtraIe);
		pAd->cfg80211_ctrl.BeaconExtraIe = NULL;
	}

	pAd->cfg80211_ctrl.BeaconExtraIeLen = 0;
}


/*
 * ========================================================================
 * Routine Description:
 *	Parse and handle country region in beacon from associated AP.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pVIE			- Beacon elements
 *	LenVIE			- Total length of Beacon elements
 *
 * Return Value:
 *	NONE
 * ========================================================================
 */
VOID CFG80211_BeaconCountryRegionParse(
	IN VOID						*pAdCB,
	IN NDIS_802_11_VARIABLE_IEs * pVIE,
	IN UINT16					LenVIE)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	UCHAR *pElement = (UCHAR *)pVIE;
	UINT32 LenEmt;

	while (LenVIE > 1) {
		pVIE = (NDIS_802_11_VARIABLE_IEs *)pElement;

		LenEmt = pVIE->Length + 2;
		if (pVIE->ElementID == IE_COUNTRY
			&& (LenEmt <= LenVIE)) {
			/* send command to do regulation hint only when associated */
			/* RT_CFG80211_CRDA_REG_HINT11D(pAd, pVIE->data, pVIE->Length); */
			RTEnqueueInternalCmd(pAd, CMDTHREAD_REG_HINT_11D,
								 pVIE->data, pVIE->Length);
			break;
		}

		if (LenVIE <= LenEmt)
			break; /* length is not enough */

		pElement += LenEmt;
		LenVIE -= LenEmt;
	}
} /* End of CFG80211_BeaconCountryRegionParse */

/*
 * ========================================================================
 * Routine Description:
 *	Hint to the wireless core a regulatory domain from driver.
 *
 * Arguments:
 *	pAd				- WLAN control block pointer
 *	pCountryIe		- pointer to the country IE
 *	CountryIeLen	- length of the country IE
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	Must call the function in kernel thread.
 * ========================================================================
 */
VOID CFG80211_RegHint(
	IN VOID						*pAdCB,
	IN UCHAR					*pCountryIe,
	IN ULONG					CountryIeLen)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;

	CFG80211OS_RegHint(CFG80211CB, pCountryIe, CountryIeLen);
}


/*
 * ========================================================================
 * Routine Description:
 *	Hint to the wireless core a regulatory domain from country element.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pCountryIe		- pointer to the country IE
 *	CountryIeLen	- length of the country IE
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	Must call the function in kernel thread.
 * ========================================================================
 */
VOID CFG80211_RegHint11D(
	IN VOID						*pAdCB,
	IN UCHAR					*pCountryIe,
	IN ULONG					CountryIeLen)
{
	/* no regulatory_hint_11d() in 2.6.32 */
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;

	CFG80211OS_RegHint11D(CFG80211CB, pCountryIe, CountryIeLen);
}


/*
 * ========================================================================
 * Routine Description:
 *	Apply new regulatory rule.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pWiphy			- Wireless hardware description
 *	pAlpha2			- Regulation domain (2B)
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	Can only be called when interface is up.
 *
 *	For general mac80211 device, it will be set to new power by Ops->config()
 *	In rt2x00/, the settings is done in rt2x00lib_config().
 * ========================================================================
 */
VOID CFG80211_RegRuleApply(
	IN VOID						*pAdCB,
	IN VOID						*pWiphy,
	IN UCHAR					*pAlpha2)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	VOID *pBand24G, *pBand5G, *pBand6G;
	UINT32 IdBand, IdChan, IdPwr;
	UINT32 ChanNum, ChanId, Power, RecId, DfsType;
	BOOLEAN FlgIsRadar;
	ULONG IrqFlags;
	CHANNEL_CTRL *pChCtrl = NULL;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
		"crda> ==>\n");
	/* init */
	pBand24G = NULL;
	pBand5G = NULL;
	pBand6G = NULL;

	if (pAd == NULL)
		return;

	RTMP_IRQ_LOCK(&PD_GET_DEVICE_IRQ(pAd->physical_dev), IrqFlags);
	/* zero first */
	/* 2.4GHZ & 5GHz */
	RecId = 0;
	/* find the DfsType */
	DfsType = CE;
	pBand24G = NULL;
	pBand5G = NULL;
	pBand6G = NULL;

	if (CFG80211OS_BandInfoGet(CFG80211CB, pWiphy, &pBand24G, &pBand5G, &pBand6G) == FALSE) {
		RTMP_IRQ_UNLOCK(&PD_GET_DEVICE_IRQ(pAd->physical_dev), IrqFlags);
		return;
	}

#ifdef AUTO_CH_SELECT_ENHANCE
#ifdef EXT_BUILD_CHANNEL_LIST

	if ((pAlpha2[0] != '0') && (pAlpha2[1] != '0')) {
		UINT32 IdReg;

		if (pBand5G != NULL) {
			for (IdReg = 0;; IdReg++) {
				if (ChRegion[IdReg].CountReg[0] == 0x00)
					break;

				if ((pAlpha2[0] == ChRegion[IdReg].CountReg[0]) &&
					(pAlpha2[1] == ChRegion[IdReg].CountReg[1])) {
					if (pAd->CommonCfg.DfsType != MAX_RD_REGION)
						DfsType = pAd->CommonCfg.DfsType;
					else
						DfsType = ChRegion[IdReg].op_class_region;

					MTWF_DBG(NULL,
						DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
						"crda> find region %c%c, DFS Type %d\n",
						pAlpha2[0], pAlpha2[1], DfsType);
					break;
				}
			}
		}
	}

#endif /* EXT_BUILD_CHANNEL_LIST */
#endif /* AUTO_CH_SELECT_ENHANCE */

	pAd->CommonCfg.CountryCode[0] = pAlpha2[0];
	pAd->CommonCfg.CountryCode[1] = pAlpha2[1];
	pAd->CommonCfg.bCountryFlag = TRUE;

	for (IdBand = 0; IdBand < IEEE80211_NUM_BANDS; IdBand++) {
		if (((IdBand == NL80211_BAND_2GHZ) && (pBand24G == NULL)) ||
			((IdBand == NL80211_BAND_5GHZ) && (pBand5G == NULL)))
		continue;

		if (IdBand == NL80211_BAND_2GHZ)
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
				"crda> reset chan/power for 2.4GHz\n");
		else
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
				"crda> reset chan/power for 5GHz\n");

		ChanNum = CFG80211OS_ChanNumGet(CFG80211CB, pWiphy, IdBand);

		for (IdChan = 0; IdChan < ChanNum; IdChan++) {
			if (CFG80211OS_ChanInfoGet(CFG80211CB, pWiphy, IdBand, IdChan,
									   &ChanId, &Power, &FlgIsRadar) == FALSE) {
				/* the channel is not allowed in the regulatory domain */
				/* get next channel information */
				continue;
			}

			if (!WMODE_CAP_2G(pAd->CommonCfg.cfg_wmode) && !WMODE_CAP_6G(pAd->CommonCfg.cfg_wmode)) {
				/* 5G-only mode */
				if (ChanId <= CFG80211_NUM_OF_CHAN_2GHZ)
					continue;
			}

			if (!WMODE_CAP_5G(pAd->CommonCfg.cfg_wmode) && !WMODE_CAP_6G(pAd->CommonCfg.cfg_wmode)) {
				/* 2.4G-only mode */
				if (ChanId > CFG80211_NUM_OF_CHAN_2GHZ)
					continue;
			}
			pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

			/* zero first */
			os_zero_mem(pChCtrl->ChList, MAX_NUM_OF_CHANNELS * sizeof(CHANNEL_TX_POWER));
			for (IdPwr = 0; IdPwr < MAX_NUM_OF_CHANNELS; IdPwr++) {
				/* sachin - TODO */
				/* if (ChanId == pAd->TxPower[IdPwr].Channel) */
				{
					/* sachin - TODO */
					/* init the channel info. */
					/* os_move_mem(&pAd->ChannelList[RecId],&pAd->TxPower[IdPwr],sizeof(CHANNEL_TX_POWER)); */
					/* keep channel number */
					pChCtrl->ChList[RecId].Channel = ChanId;
					/* keep maximum tranmission power */
					pChCtrl->ChList[RecId].MaxTxPwr = Power;

					/* keep DFS flag */
					if (FlgIsRadar == TRUE)
						pChCtrl->ChList[RecId].DfsReq = TRUE;
					else
						pChCtrl->ChList[RecId].DfsReq = FALSE;

					/* keep DFS type */
					pChCtrl->ChList[RecId].RegulatoryDomain = DfsType;
					/* re-set DFS info. */
					pAd->CommonCfg.RDDurRegion = DfsType;
					MTWF_DBG(NULL,
						DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
						"Chan %03d:\tpower %d dBm, DFS %d, DFS Type %d\n",
						ChanId, Power,
						((FlgIsRadar == TRUE) ? 1 : 0),
						DfsType);
					/* change to record next channel info. */
					RecId++;
					/*break;*/
				}
			}
		}
	}
	if (pChCtrl) {
		pChCtrl->ChListNum = RecId;
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
			"pChCtrl->ChListNum = %d\n", pChCtrl->ChListNum);
	}
	RTMP_IRQ_UNLOCK(&PD_GET_DEVICE_IRQ(pAd->physical_dev), IrqFlags);
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
		"crda> Number of channels = %d\n", RecId);
} /* End of CFG80211_RegRuleApply */

/*
 * ========================================================================
 * Routine Description:
 *	Inform CFG80211 about association status.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pBSSID			- the BSSID of the AP
 *	pReqIe			- the element list in the association request frame
 *	ReqIeLen		- the request element length
 *	pRspIe			- the element list in the association response frame
 *	RspIeLen		- the response element length
 *	FlgIsSuccess	- 1: success; otherwise: fail
 *
 * Return Value:
 *	None
 * ========================================================================
 */
VOID CFG80211_ConnectResultInform(
	IN VOID *pAdCB, IN UCHAR *pBSSID, IN UCHAR ifIndex, IN UCHAR *pReqIe, IN UINT32 ReqIeLen,
	IN UCHAR *pRspIe, IN UINT32 RspIeLen, IN UCHAR FlgIsSuccess)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	struct wifi_dev *wdev = &pAd->StaCfg[ifIndex].wdev;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
		"80211> ==>\n");

	if (CFG80211DRV_OpsScanRunning(pAd) ||
		RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"Abort running scan\n");
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
		CFG80211OS_ScanEnd(pAd, TRUE);
	}

	if (wdev && (pAd->cfg80211_ctrl.FlgCfg80211Connecting))
		CFG80211OS_ConnectResultInform(wdev->if_dev, pBSSID,
			pReqIe, ReqIeLen, pRspIe, RspIeLen, FlgIsSuccess);

	pAd->cfg80211_ctrl.FlgCfg80211Connecting = FALSE;
} /* End of CFG80211_ConnectResultInform */

/*
 * ========================================================================
 * Routine Description:
 *	Re-Initialize wireless channel/PHY in 2.4GHZ and 5GHZ.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *
 * Return Value:
 *	TRUE			- re-init successfully
 *	FALSE			- re-init fail
 *
 * Note:
 *	CFG80211_SupBandInit() is called in xx_probe().
 *	But we do not have complete chip information in xx_probe() so we
 *	need to re-init bands in xx_open().
 * ========================================================================
 */
BOOLEAN CFG80211_SupBandReInit(VOID *pAdCB, VOID *wdev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	struct wifi_dev *curr_wdev = (struct wifi_dev *)wdev;
	CFG80211_BAND BandInfo;
#ifdef CFG80211_FULL_OPS_SUPPORT
	CFG80211_CB *pCfg80211_CB = CFG80211CB;
#endif

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
		"80211> re-init bands...\n");
	/* re-init bands */
	os_zero_mem(&BandInfo, sizeof(BandInfo));

	CFG80211_BANDINFO_FILL(pAd, curr_wdev, &BandInfo);
#ifdef CFG80211_FULL_OPS_SUPPORT
	if (pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport == TRUE &&
		pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault == TRUE)
		wiphy_ext_feature_set(pCfg80211_CB->pCfg80211_Wdev->wiphy,
			NL80211_EXT_FEATURE_RADAR_BACKGROUND);
#endif
	return CFG80211OS_SupBandReInit(pAd, CFG80211CB, &BandInfo);
} /* End of CFG80211_SupBandReInit */

#ifdef CONFIG_STA_SUPPORT
INT CFG80211_setStaDefaultKey(
	IN VOID                     *pAdCB,
	IN UINT					Data
)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO, "Set Sta Default Key: %d\n", Data);
	pAd->StaCfg[0].wdev.SecConfig.PairwiseKeyId = Data; /* base 0 */
	return 0;
}
#endif /*CONFIG_STA_SUPPORT*/

INT CFG80211_reSetToDefault(
	IN VOID                                         *pAdCB)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO, "\n");
#ifdef CONFIG_STA_SUPPORT
	/* Driver Internal Parm */
	pAd->StaCfg[0].bAutoConnectByBssid = FALSE;
#endif /*CONFIG_STA_SUPPORT*/
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = FALSE;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = FALSE;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount = 0;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount = 0;
	pCfg80211_ctrl->Cfg80211RocTimerInit = FALSE;
	pCfg80211_ctrl->Cfg80211RocTimerRunning = FALSE;
	pCfg80211_ctrl->FlgCfg80211Scanning = FALSE;
	/* pCfg80211_ctrl->isMccOn = FALSE; */
	return TRUE;
}

/* initList(&pAd->Cfg80211VifDevSet.vifDevList); */
/* initList(&pAd->cfg80211_ctrl.cfg80211TxPacketList); */
#if defined(APCLI_CFG80211_SUPPORT)
BOOLEAN CFG80211_checkScanResInKernelCache(
	IN VOID *pAdCB,
	IN UCHAR *pBSSID,
	IN UCHAR *pSsid,
	IN INT ssidLen)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;
	struct wiphy *pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	struct cfg80211_bss *bss;

	bss = cfg80211_get_bss(pWiphy, NULL, pBSSID,
						   pSsid, ssidLen,
						   WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);

	if (bss) {
		CFG80211OS_PutBss(pWiphy, bss);
		return TRUE;
	}

	return FALSE;
}

BOOLEAN CFG80211_checkScanTable(
	IN VOID *pAdCB)
{
#ifndef APCLI_CFG80211_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;
	struct wiphy *pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	ULONG bss_idx = BSS_NOT_FOUND;
	struct cfg80211_bss *bss;
	struct ieee80211_channel *chan;
	UINT32 CenFreq;
	UINT64 timestamp;
	struct timeval tv;
	UCHAR *ie, ieLen = 0;
	BOOLEAN isOk = FALSE;
	BSS_ENTRY *pBssEntry;
	USHORT ifIndex = 0;
	PSTA_ADMIN_CONFIG pApCliEntry = NULL;
	struct wifi_dev *wdev = NULL;
	BSS_TABLE *ScanTab = NULL;

	pApCliEntry = &pAd->StaCfg[ifIndex];
	wdev = &pApCliEntry->wdev;
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	if (MAC_ADDR_EQUAL(pApCliEntry->MlmeAux.Bssid, ZERO_MAC_ADDR)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"pAd->ApCliMlmeAux.Bssid ==> ZERO_MAC_ADDR\n");
		/* ToDo: pAd->StaCfg[0].CfgApCliBssid */
		return FALSE;
	}

	/* Fake TSF */
	do_gettimeofday(&tv);
	timestamp = ((UINT64)tv.tv_sec * 1000000) + tv.tv_usec;
	bss = cfg80211_get_bss(pWiphy, NULL, pApCliEntry->MlmeAux.Bssid,
						   pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen,
						IEEE80211_BSS_TYPE_ESS, IEEE80211_PRIVACY_ANY);

	if (bss) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO, "Found %s in Kernel_ScanTable with CH[%d]\n", pApCliEntry->MlmeAux.Ssid, bss->channel->center_freq);
		CFG80211OS_PutBss(pWiphy, bss);
		return TRUE;
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
		"Can't Found %s in Kernel_ScanTable & Try Fake it\n", pApCliEntry->MlmeAux.Ssid);

	bss_idx = BssSsidTableSearchBySSID(ScanTab, pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen);

	if (bss_idx != BSS_NOT_FOUND) {
		/* Since the cfg80211 kernel scanTable not exist this Entry,
		 * Build an Entry for this connect inform event.
			 */
		pBssEntry = &ScanTab->BssEntry[bss_idx];

		if (ScanTab->BssEntry[bss_idx].Channel > 14)
			CenFreq = ieee80211_channel_to_frequency(pBssEntry->Channel, IEEE80211_BAND_5GHZ);
		else
			CenFreq = ieee80211_channel_to_frequency(pBssEntry->Channel, IEEE80211_BAND_2GHZ);

		chan = ieee80211_get_channel(pWiphy, CenFreq);
		ieLen = 2 + pApCliEntry->MlmeAux.SsidLen + pBssEntry->VarIeFromProbeRspLen;
		os_alloc_mem(NULL, (UCHAR **)&ie, ieLen);

		if (!ie) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
				"Memory Allocate Fail\n");
			return FALSE;
		}

		ie[0] = WLAN_EID_SSID;
		ie[1] = pApCliEntry->MlmeAux.SsidLen;
		NdisCopyMemory(ie + 2, pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen);
		NdisCopyMemory(ie + 2 + pApCliEntry->MlmeAux.SsidLen, pBssEntry->pVarIeFromProbRsp,
					   pBssEntry->VarIeFromProbeRspLen);
		bss = cfg80211_inform_bss(pWiphy, chan,
								  pApCliEntry->MlmeAux.Bssid, timestamp, WLAN_CAPABILITY_ESS, pApCliEntry->MlmeAux.BeaconPeriod,
								  ie, ieLen,
#ifdef CFG80211_SCAN_SIGNAL_AVG
								  (pBssEntry->AvgRssi * 100),
#else
								  (pBssEntry->Rssi * 100),
#endif
								  GFP_KERNEL);

		if (bss) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
					 "Fake New %s("MACSTR") in Kernel_ScanTable with CH[%d][%d] BI:%d len:%d\n",
					  pApCliEntry->MlmeAux.Ssid,
					  MAC2STR(pApCliEntry->MlmeAux.Bssid), bss->channel->center_freq, pBssEntry->Channel,
					  pApCliEntry->MlmeAux.BeaconPeriod, pBssEntry->VarIeFromProbeRspLen);
			CFG80211OS_PutBss(pWiphy, bss);
			isOk = TRUE;
		}

		if (ie != NULL)
			os_free_mem(ie);

		if (isOk)
			return TRUE;
	} else
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
				 "%s Not In Driver Scan Table\n", pApCliEntry->MlmeAux.Ssid);

	return FALSE;
#else
	return TRUE;
#endif /* APCLI_CFG80211_SUPPORT */
}
#endif /* APCLI_CFG80211_SUPPORT */

/* CFG_TODO */
UCHAR CFG80211_getCenCh(RTMP_ADAPTER *pAd, UCHAR prim_ch)
{
	UCHAR ret_channel = 0;
	struct wifi_dev *wdev = NULL;
	UCHAR ht_bw = 0;
	UCHAR ext_cha = 0;

	wdev = get_default_wdev(pAd);
	if (wdev) {
		ht_bw = wlan_operate_get_ht_bw(wdev);
		ext_cha = wlan_operate_get_ext_cha(wdev);
	}

	if (ht_bw == BW_40) {
		if (ext_cha == EXTCHA_ABOVE)
			ret_channel = prim_ch + 2;
		else {
			if (prim_ch == 14)
				ret_channel = prim_ch - 1;
			else
				ret_channel = prim_ch - 2;
		}
	} else
		ret_channel = prim_ch;

	return ret_channel;
}


#ifdef MT_MAC
VOID CFG80211_InitTxSCallBack(RTMP_ADAPTER *pAd)
{
	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"80211> Only MT_MAC support this feature.\n");
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO, "\n");
}
#endif /* MT_MAC */
#ifdef CONFIG_VLAN_GTK_SUPPORT
struct wifi_dev *CFG80211_GetWdevByVlandev(PRTMP_ADAPTER pAd, PNET_DEV vlan_dev)
{
	UINT8 apidx;
	struct wifi_dev *wdev = NULL;

	if (!pAd)
		return NULL;

	for (apidx = MAIN_MBSSID; apidx < pAd->ApCfg.BssidNum; apidx++) {
		PNET_DEV tmp_ndev;
		UINT8 ifname_len;
		char *pch;

		tmp_ndev = pAd->ApCfg.MBSSID[apidx].wdev.if_dev;
		if (!tmp_ndev)
			continue;

		pch = strchr(vlan_dev->name, '.');
		ifname_len = pch - tmp_ndev->name;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
				"ifname=%s, vlan_ifname=%s\n",
				tmp_ndev->name, vlan_dev->name);
		if (strncmp(tmp_ndev->name, vlan_dev->name, ifname_len) == 0) {
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			break;
		}
	}

	return wdev;
}

BOOLEAN CFG80211_MatchVlandev(struct wifi_dev *wdev, PNET_DEV vlan_dev)
{
	struct list_head *listptr;
	struct vlan_gtk_info *vg_info;

	if (!wdev)
		return FALSE;

	if (wdev->vlan_cnt == 0)
		return FALSE;

	list_for_each(listptr, &wdev->vlan_gtk_list) {
		vg_info = list_entry(listptr, struct vlan_gtk_info, list);
		if (vg_info && vg_info->vlan_dev == vlan_dev)
			return TRUE;
	}

	return FALSE;
}

struct vlan_gtk_info *CFG80211_GetVlanInfoByVlandev(struct wifi_dev *wdev, PNET_DEV vlan_dev)
{
	struct list_head *listptr;
	struct vlan_gtk_info *vg_info;

	if (!wdev)
		return NULL;

	list_for_each(listptr, &wdev->vlan_gtk_list) {
		vg_info = list_entry(listptr, struct vlan_gtk_info, list);
		if (vg_info && vg_info->vlan_dev == vlan_dev)
			return vg_info;
	}

	return NULL;
}

struct vlan_gtk_info *CFG80211_GetVlanInfoByVlanid(struct wifi_dev *wdev, UINT16 vlan_id)
{
	struct list_head *listptr;
	struct vlan_gtk_info *vg_info;

	if (!wdev)
		return NULL;

	list_for_each(listptr, &wdev->vlan_gtk_list) {
		vg_info = list_entry(listptr, struct vlan_gtk_info, list);
		if (vg_info && vg_info->vlan_id == vlan_id)
			return vg_info;
	}

	return NULL;
}

/* return VLAN ID on success */
INT16 CFG80211_IsVlanPkt(PNDIS_PACKET pkt)
{
	UINT16 vlan_id = 0;
	struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);
	UCHAR *data = skb->data;

	if (ntohs(skb->protocol) == ETH_TYPE_VLAN) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO, "%s() get vlan pkt\n", __func__);
		vlan_id = ((data[14] << 8) + data[15]) & 0xfff;
	}

	return vlan_id;
}
#endif
#endif /* RT_CFG80211_SUPPORT */
