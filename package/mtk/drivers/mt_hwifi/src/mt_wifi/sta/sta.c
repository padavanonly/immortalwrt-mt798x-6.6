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
	sta.c

	Abstract:
	initialization for STA module

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"

#ifdef LINUX
NET_DEV_STATS *RT28xx_get_ether_stats(PNET_DEV net_dev);
#endif

/* Initialize STA and the MAIN STA interface. */
INT STAInitialize(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#ifdef ETH_CONVERT_SUPPORT
	extern UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN];
#endif
	ASSERT(pStaCfg);

	if (!pStaCfg)
		return 0;

#ifdef ETH_CONVERT_SUPPORT

	if (NdisEqualMemory(&pAd->EthConvert.EthCloneMac[0], &ZERO_MAC_ADDR[0], MAC_ADDR_LEN)) {
		NdisMoveMemory(&pAd->EthConvert.EthCloneMac[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_MLME, DBG_LVL_INFO, "Read EEPROM, EthCloneMac is "MACSTR"!\n",
				 MAC2STR(pAd->EthConvert.EthCloneMac));
	}

#endif /* ETH_CONVERT_SUPPORT */
	pStaCfg->OriDevType = RTMP_OS_NETDEV_GET_TYPE(pAd->net_dev);
	pAd->MSTANum = 1;

	if (pAd->MaxMSTANum == 0) /* This mean profile has no setting for MaxMSTANum */
		pAd->MaxMSTANum = 1;

	return 0;
}



VOID rtmp_sta_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	UCHAR if_addr[MAC_ADDR_LEN] = { 0 };

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	STAInitialize(pAd, wdev);
	/* Set up the Mac address*//* Here is to set MAIN_MSTA_ID STA, Extend MSTA is set in MSTA_Open */
	NdisMoveMemory(&if_addr[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
	if_addr[5] ^= wdev->func_idx << BIT(1);

	NdisMoveMemory(&pStaCfg->wdev.if_addr[0], &if_addr[0], MAC_ADDR_LEN);
	MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_MLME, DBG_LVL_ERROR,
		"wdev: 0x%p, if_addr: %pM\n",
		wdev, if_addr);
	RtmpOSNetDevAddrSet(pAd->OpMode, pAd->net_dev, &pStaCfg->wdev.if_addr[0], (PUCHAR)(pStaCfg->dev_name));
#ifdef EXT_BUILD_CHANNEL_LIST
	BuildChannelListEx(pAd, wdev);
#else
	BuildChannelList(pAd, wdev);
#endif
	RTMPSetPhyMode(pAd, &pStaCfg->wdev, pStaCfg->wdev.PhyMode);
	sta_os_completion_initialize(pStaCfg);
}

VOID RT28xx_MSTA_Init(VOID *pAd, PNET_DEV main_dev_p)
{
	RTMP_OS_NETDEV_OP_HOOK netDevHook;

	NdisZeroMemory(&netDevHook, sizeof(netDevHook));
	netDevHook.open = msta_virtual_if_open;  /* device opem hook point */
	netDevHook.stop = msta_virtual_if_close; /* device close hook point */
	netDevHook.xmit = rt28xx_send_packets;  /* hard transmit hook point */
	netDevHook.ioctl = rt28xx_ioctl;    /* ioctl hook point */
#ifdef LINUX
	netDevHook.get_stats = RT28xx_get_ether_stats;
#endif
	RTMP_STA_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_MSTA_INIT,
						 0, &netDevHook, 0, 0);
}

VOID RT28xx_MSTA_Remove(VOID *pAd)
{
	RTMP_STA_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_MSTA_REMOVE, 0, NULL, 0, 0);
}

INT msta_virtual_if_open(PNET_DEV pDev)
{
	VOID *pAd = NULL;
#ifdef DOT11_EHT_BE
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pDev);
#endif

	MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_INFO, " ===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(pDev));

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);

	if (VIRTUAL_IF_INIT(pAd, pDev) != 0)
		goto err;
#ifdef DOT11_EHT_BE
	if (wdev && wdev->wdev_type == WDEV_TYPE_STA && !IS_APCLI_DISABLE_MLO(wdev)) {
		if (sta_mld_link_mgr_reg_dev(wdev) < 0)
			goto err;
	}
#endif
	if (VIRTUAL_IF_UP(pAd, pDev) != 0)
		goto err2;

	/* increase MODULE use count */
	RT_MOD_INC_USE_COUNT();
	RT_MOD_HNAT_REG(pDev);
	RTMP_OS_NETDEV_START_QUEUE(pDev);
#ifdef MTFWD
	RTMP_OS_NETDEV_CARRIER_OFF(pDev);
#endif

	return 0;
err2:
	if (wdev && wdev->wdev_type == WDEV_TYPE_STA && !IS_APCLI_DISABLE_MLO(wdev))
		sta_mld_link_mgr_dereg_dev(wdev);
err:
	return -1;
}

INT msta_virtual_if_close(PNET_DEV pDev)
{
	VOID *pAd = NULL;
#ifdef DOT11_EHT_BE
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pDev);
#endif

	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_INFO, " ===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(pDev));

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	RTMP_OS_NETDEV_STOP_QUEUE(pDev);

	VIRTUAL_IF_DOWN(pAd, pDev);
#ifdef DOT11_EHT_BE
	if (wdev && wdev->wdev_type == WDEV_TYPE_STA && !IS_APCLI_DISABLE_MLO(wdev))
		sta_mld_link_mgr_dereg_dev(wdev);
#endif

	VIRTUAL_IF_DEINIT(pAd, pDev);

	RT_MOD_HNAT_DEREG(pDev);
	RT_MOD_DEC_USE_COUNT();
	return 0;
}

INT32 MSTA_IdxGet(RTMP_ADAPTER *pAd, PNET_DEV pDev)
{
	INT32 ret = -1;
	INT32 i;

	if (!pAd || !pDev)
		return -1;

	for (i = 0; i < pAd->MSTANum; i++) {
		if (pAd->StaCfg[i].wdev.if_dev == pDev) {
			ret = i;
			break;
		}
	}

	return ret;
}

extern struct wifi_dev_ops sta_wdev_ops;

VOID MSTA_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps)
{
	UINT32 idx = 0;
	UINT32 sta_start_id = (MAIN_MSTA_ID + 1);
	UINT32 max_num_sta = pAd->MaxMSTANum;
	UINT32 inf_type = INT_MSTA;
	struct wifi_dev_ops *wdev_ops = &sta_wdev_ops;
	PNET_DEV pDevNew = NULL;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
	INT status;
	struct wifi_dev *wdev;
#if defined(MULTIPLE_CARD_SUPPORT) || defined(CONFIG_APSTA_MIXED_SUPPORT)
	UINT32 MC_RowID = 0, IoctlIF = 0;
#endif
	char *dev_name;
	INT32 Ret;
#ifdef MULTIPLE_CARD_SUPPORT
	MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef CONFIG_APSTA_MIXED_SUPPORT

	if (IF_COMBO_HAVE_AP_STA(pAd)) {
		sta_start_id = 0;
		max_num_sta = min(pAd->ApCfg.ApCliNum, (UCHAR)MAX_APCLI_NUM);
		inf_type = INT_APCLI;
		wdev_ops = &apcli_wdev_ops;

		if (pAd->flg_msta_init != FALSE) {
			pAd->MSTANum = max_num_sta; /* re-assign new actual number of total MSTA, including MAIN STA*/
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_MLME, DBG_LVL_INFO,
				" re-assign MSTANum=%d\n", pAd->MSTANum);
			for (idx = 0; idx < pAd->ApCfg.ApCliNum; idx++) {
				pStaCfg = &pAd->StaCfg[idx];
				wdev = &pStaCfg->wdev;
				wlan_config_set_ext_cha(wdev, EXTCHA_NOASSIGN);
				phy_sync_wdev(pAd, wdev);
			}

			return;
		}
	} else
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
	{
		if (pAd->flg_msta_init != FALSE)
			return;
	}

	MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_MLME, DBG_LVL_INFO, " (%d) ---> %s\n",
			 max_num_sta, IF_COMBO_HAVE_AP_STA(pAd) ? "ApCli" : "STA");

	/* create virtual network interface */
	for (idx = sta_start_id; idx < max_num_sta && idx < MAX_MULTI_STA; idx++) {
		pStaCfg = &pAd->StaCfg[idx];
		wdev = &pStaCfg->wdev;
		dev_name = get_dev_name_prefix(pAd, inf_type);
		if (dev_name == NULL) {
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_MLME, DBG_LVL_ERROR,
					 "apcli interface name is null,apcli idx=%d!\n",
					  idx);
			break;
		}

#ifdef CONFIG_APSTA_MIXED_SUPPORT
		pDevNew = RtmpOSNetDevCreate(
			MC_RowID,
			&IoctlIF,
			inf_type,
			idx,
			sizeof(struct mt_dev_priv),
			dev_name,
			TRUE);
#endif /*CONFIG_APSTA_MIXED_SUPPORT*/

		if (!pDevNew) {
			break;
		}
		pAd->MSTANum = idx + 1; /* re-assign new actual number of total MSTA, including MAIN STA*/
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_MLME, DBG_LVL_INFO,
			"Register MSTA IF (%s) , pAd->MSTANum = %d\n",
			RTMP_OS_NETDEV_GET_DEVNAME(pDevNew), pAd->MSTANum);

#ifdef CONFIG_OWE_SUPPORT
		pStaCfg->curr_owe_group = ECDH_GROUP_256;
#endif

		Ret = wdev_init(pAd, wdev, WDEV_TYPE_STA, pDevNew, idx,
						(VOID *)pStaCfg, (VOID *)pAd);

		if (!Ret) {
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_MLME, DBG_LVL_ERROR,
				"Assign wdev idx for %s failed, free net device!\n",
				RTMP_OS_NETDEV_GET_DEVNAME(pDevNew));
			RtmpOSNetDevFree(pDevNew);
			break;
		}

		Ret = wdev_ops_register(wdev, WDEV_TYPE_STA, wdev_ops,
								cap->qos.wmm_detect_method);

		if (!Ret) {
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_MLME, DBG_LVL_ERROR,
					 "register wdev_ops %s failed, free net device!\n", RTMP_OS_NETDEV_GET_DEVNAME(pDevNew));
			RtmpOSNetDevFree(pDevNew);
			break;
		}

		/* init operation functions and flags */
		NdisCopyMemory(&netDevHook, pNetDevOps, sizeof(netDevHook));
		netDevHook.priv_flags = inf_type;
		netDevHook.needProtcted = TRUE;
		netDevHook.wdev = wdev;
#ifdef CONFIG_APSTA_MIXED_SUPPORT

		if ((IF_COMBO_HAVE_AP_STA(pAd))) {
			phy_sync_wdev(pAd, wdev);
			/*update rate info*/
			SetCommonHtVht(pAd, wdev);
			RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);
			AsicSetWdevIfAddr(pAd, wdev, OPMODE_STA);
		} else
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
		{
			COPY_MAC_ADDR(&pStaCfg->wdev.if_addr[0], pAd->CurrentAddress);
			pStaCfg->wdev.if_addr[0] |= 0x2;
			/* default choose bit[31:28], if there is no assigned mac from profile. */
			pStaCfg->wdev.if_addr[3] = pStaCfg->wdev.if_addr[3] & 0xef;
			pStaCfg->wdev.if_addr[3] = (pStaCfg->wdev.if_addr[3] | (idx << 4));
		}

		RTMP_OS_NETDEV_SET_PRIV(pDevNew, pAd);
		RTMP_OS_NETDEV_SET_WDEV(pDevNew, wdev);
		NdisMoveMemory(&netDevHook.devAddr[0], pStaCfg->wdev.if_addr, MAC_ADDR_LEN);


#ifdef APCLI_CFG80211_SUPPORT
		if (IF_COMBO_HAVE_AP_STA(pAd)) {
			struct wireless_dev *pWdev;
			CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
			UINT32 DevType = RT_CMD_80211_IFTYPE_STATION;

			pWdev = &wdev->cfg80211_wdev;
			os_zero_mem(pWdev, sizeof(struct wireless_dev));

			if (pWdev) {
				pDevNew->ieee80211_ptr = pWdev;
				pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
				SET_NETDEV_DEV(pDevNew, wiphy_dev(pWdev->wiphy));
				pWdev->netdev = pDevNew;
				pWdev->iftype = DevType;
				pWdev->use_4addr = true;
			} else {
				MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_MLME, DBG_LVL_ERROR,
						"Failed to allocate pWdev Memory.\n");
				return;
			}
		}
#endif /* APCLI_CFG80211_SUPPORT */


		/* register this device to OS */
		status = RtmpOSNetDevAttach(pAd->OpMode, pDevNew, &netDevHook);
		pStaCfg->ApcliInfStat.ApCliInit = TRUE;
		pAd->flg_msta_init = TRUE;
	}

#ifdef MAC_REPEATER_SUPPORT
	CliLinkMapInit(pAd);
#endif
}


VOID MSTAStop(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	if (!pStaCfg)
		return;
	/* Clear PMKID cache.*/
	pStaCfg->SavedPMKNum = 0;
	RTMPZeroMemory(pStaCfg->SavedPMK, (PMKID_NO * sizeof(BSSID_INFO)));

	/* Link down first if any association exists*/
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		if (INFRA_ON(pStaCfg) || ADHOC_ON(pAd)) {
#ifdef MAC_REPEATER_SUPPORT
			repeater_disconnect_by_band(pAd, HcGetBandByWdev(&pStaCfg->wdev));
#endif /* MAC_REPEATER_SUPPORT */
			RTMP_OS_INIT_COMPLETION(&pStaCfg->linkdown_complete);
#ifdef APCLI_SUPPORT
			pStaCfg->ApcliInfStat.Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCLI_IF_DOWN;
#endif
			/**
			 * Interface is going to be closed.
			 * Send de-auth frame & link down directly here.
			 */
			sta_auth_send_deauth_frame(pAd, wdev);
			LinkDown(pAd, 0, wdev, NULL);
		} else
			sta_auth_send_deauth_frame(pAd, wdev);
		sta_ifdown_fsm_reset(pStaCfg);
	}

	/*==========================================*/
	/* Clean up old bss table*/
#ifndef ANDROID_SUPPORT
	/* because abdroid will get scan table when interface down, so we not clean scan table */
	BssTableInit(ScanTab);
#endif /* ANDROID_SUPPORT */
}


VOID MSTA_Remove(RTMP_ADAPTER *pAd)
{
	struct wifi_dev *wdev = NULL;
	UINT IdSta = 0;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;

	if (!pAd)
		return;

#ifdef CONFIG_APSTA_MIXED_SUPPORT

	if ((IF_COMBO_HAVE_AP_STA(pAd))) {
		for (IdSta = 0; IdSta < MAX_APCLI_NUM; IdSta++) {
			wdev = &pAd->StaCfg[IdSta].wdev;

			if (wdev->if_dev) {
				RtmpOSNetDevProtect(1);
				RtmpOSNetDevDetach(wdev->if_dev);
				RtmpOSNetDevProtect(0);
				wdev_deinit(pAd, wdev);
#ifndef RT_CFG80211_SUPPORT
				RtmpOSNetDevFree(wdev->if_dev);
#endif
				/* Clear it as NULL to prevent latter access error. */
				pAd->StaCfg[IdSta].ApcliInfStat.ApCliInit = FALSE;
				wdev->if_dev = NULL;
			}
		}
	} else
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
	{
		for (IdSta = 0; IdSta < MAX_MULTI_STA; IdSta++) {
			wdev = &pAd->StaCfg[IdSta].wdev;
			pStaCfg = &pAd->StaCfg[IdSta];

			if (pStaCfg)
				return;

			if (wdev->if_dev) {
				RtmpOSNetDevProtect(1);
				RtmpOSNetDevDetach(wdev->if_dev);
				RtmpOSNetDevProtect(0);
				wdev_deinit(pAd, wdev);
#ifndef RT_CFG80211_SUPPORT
				RtmpOSNetDevFree(wdev->if_dev);
#endif
				wdev->if_dev = NULL;
			}
		}
	}

}

/*
* MSTA_Open
*/
INT sta_inf_open(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;


#ifdef CONFIG_APSTA_MIXED_SUPPORT
	if ((IF_COMBO_HAVE_AP_STA(pAd))) {
		if (wdev->func_idx >= MAX_APCLI_NUM)
			return FALSE;

#ifdef IWCOMMAND_CFG80211_SUPPORT
		AsicSetWdevIfAddr(pAd, wdev, OPMODE_STA);
		if (wdev->if_dev) {
			NdisMoveMemory(RTMP_OS_NETDEV_GET_PHYADDR(wdev->if_dev),
				wdev->if_addr, MAC_ADDR_LEN);
		}
#endif /* IWCOMMAND_CFG80211_SUPPORT */

		/* Sync the Channel information */
		phy_sync_wdev(pAd, wdev);
	}

#endif /* CONFIG_APSTA_MIXED_SUPPORT */
	if ((wdev->func_idx >= 0) && (wdev->func_idx < MAX_MULTI_STA))
		pStaCfg = &pAd->StaCfg[wdev->func_idx];
	else
		return FALSE;
	if (wifi_sys_open(wdev) != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR, "open fail!!!\n");
		return FALSE;
	}

#ifdef CONFIG_APSTA_MIXED_SUPPORT
	if (IF_COMBO_HAVE_AP_STA(pAd))
		RTMPSetPhyMode(pAd, wdev, wdev->PhyMode);
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

#ifdef WSC_INCLUDED
	/* WSC parameters initialization required in case of ApCli mode as well */
	WscUUIDInit(pAd, wdev->func_idx, TRUE);
#endif /* WSC_INCLUDED */
#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd))
		map_a4_init(pAd, wdev->func_idx, FALSE);
#endif
#ifdef MWDS
	if (wdev->bDefaultMwdsStatus == TRUE)
		MWDSEnable(pAd, wdev->func_idx, FALSE, TRUE);
#endif /* MWDS */

#ifdef PRE_CFG_SUPPORT
	pre_cfg_interface_init(pAd, wdev->func_idx, FALSE);
#endif /* PRE_CFG_SUPPORT */

	sta_os_completion_initialize(pStaCfg);

	MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_INFO,
		"MSTA interface up for %s%x func_idx=%d OmacIdx=%d\n",
			 (IF_COMBO_HAVE_AP_STA(pAd)) ? "apcli" : "ra",
			 wdev->func_idx,
			 pStaCfg->wdev.func_idx,
			 pStaCfg->wdev.OmacIdx);

#ifdef DPP_SUPPORT
	DlListInit(&wdev->dpp_frame_event_list);
#endif /* DPP_SUPPORT */


	return TRUE;
}

/*
* MSTA_Close + RTMPInfClose
*/
INT sta_inf_close(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;

	if ((wdev->func_idx >= 0) && (wdev->func_idx < MAX_MULTI_STA))
		pStaCfg = &pAd->StaCfg[wdev->func_idx];
	else
		return FALSE;
#if defined(DOT11_SAE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
	if (IS_AKM_SAE(wdev->SecConfig.AKMMap)) {
		sae_pt_list_deinit(&wdev->SecConfig.pt_list);
		sae_pk_deinit(&wdev->SecConfig.sae_pk);
	}
#endif


	/* Disconnet from AP of this interface */
	/* Pat: TODO: */

	DoActionBeforeDownIntf(pAd, wdev);

#ifdef APCLI_AUTO_CONNECT_SUPPORT
	pStaCfg->ApCliAutoConnectRunning = FALSE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd))
			map_a4_deinit(pAd, wdev->func_idx, FALSE);
#endif

#ifdef MWDS
		MWDSDisable(pAd, wdev->func_idx, FALSE, TRUE);
#endif /* MWDS */

#ifdef PRE_CFG_SUPPORT
	pre_cfg_interface_deinit(pAd, wdev->func_idx, FALSE);
#endif /* PRE_CFG_SUPPORT */

#ifdef APCLI_CFG80211_SUPPORT
		if (pStaCfg->wpa_supplicant_info.pWpsProbeReqIe) {
			os_free_mem(pStaCfg->wpa_supplicant_info.pWpsProbeReqIe);
			pStaCfg->wpa_supplicant_info.pWpsProbeReqIe = NULL;
			pStaCfg->wpa_supplicant_info.WpsProbeReqIeLen = 0;
		}

		if (pStaCfg->wpa_supplicant_info.pWpaAssocIe) {
			os_free_mem(pStaCfg->wpa_supplicant_info.pWpaAssocIe);
			pStaCfg->wpa_supplicant_info.pWpaAssocIe = NULL;
			pStaCfg->wpa_supplicant_info.WpaAssocIeLen = 0;
		}
#endif /* defined(APCLI_CFG80211_SUPPORT) */

	MSTAStop(pAd, wdev);
	MacTableResetWdev(pAd, wdev);
	if (wifi_sys_close(wdev) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_INFO, "close fail!!!\n");

		DoActionAfterDownIntf(pAd, wdev);
		return FALSE;
	}

	DoActionAfterDownIntf(pAd, wdev);


	return TRUE;
}

/*
 * wifi system layer api for ADHOC
 */
INT adhoc_link_up(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) wdev->sys_handle;

	UpdateBeaconHandler(
		ad,
		wdev,
		BCN_REASON(BCN_UPDATE_IF_STATE_CHG));

	if (wifi_sys_linkup(wdev, entry) != TRUE) {
		MTWF_DBG(ad, DBG_CAT_CLIENT, CATCLIENT_ADHOC, DBG_LVL_ERROR,
				 "linkup fail!\n");
	}

	return TRUE;
}

UINT32 bssinfo_sta_feature_decision(struct wifi_dev *wdev, UINT16 wcid, UINT64 *feature)
{
	UINT64 features = 0;

	features |= BSS_INFO_PM_FEATURE;
#ifdef UAPSD_SUPPORT
	features |= BSS_INFO_UAPSD_FEATURE;
#endif /* UAPSD_SUPPORT */

#ifdef DOT11V_MBSSID_SUPPORT
	features |= BSS_INFO_11V_MBSSID_FEATURE;
#endif

	*feature |= features;
	return TRUE;
}

VOID sta_deauth_act(struct wifi_dev *wdev)
{
	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
		"caller:%pS,wdev(if)addr="MACSTR"\n", OS_TRACE, MAC2STR(wdev->if_addr));

	RTEnqueueInternalCmd(wdev->sys_handle, CMDTHRED_STA_DEAUTH_ACT, (VOID *) wdev, sizeof(struct wifi_dev));
}

VOID sta_deassoc_act(struct wifi_dev *wdev)
{
	RTEnqueueInternalCmd(wdev->sys_handle, CMDTHRED_STA_DEASSOC_ACT, (VOID *) wdev, sizeof(struct wifi_dev));
}

BOOLEAN sta_media_state_connected(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad;
	STA_ADMIN_CONFIG *sta_cfg;

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	sta_cfg = GetStaCfgByWdev(ad, wdev);
	if (!sta_cfg)
		return FALSE;
	return STA_STATUS_TEST_FLAG(sta_cfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
}

VOID sta_handle_mic_error_event(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *entry, struct _RX_BLK *pRxBlk)
{
	CIPHER_KEY *pWpaKey = NULL;
	PSTA_ADMIN_CONFIG sta_cfg = NULL;
	struct wifi_dev *wdev = entry->wdev;

	if (pRxBlk->key_idx < 4)
		pWpaKey = &pAd->SharedKey[BSS0][pRxBlk->key_idx];

	if (!pWpaKey)
		return;

	sta_cfg = GetStaCfgByWdev(pAd, wdev);
	ASSERT(sta_cfg);

	if (sta_cfg && INFRA_ON(sta_cfg)) {
		RTMPReportMicError(pAd, sta_cfg, pWpaKey);

		RTMPSendWirelessEvent(pAd, IW_MIC_ERROR_EVENT_FLAG,
							  pRxBlk->Addr2, BSS0, 0);
	}
}



/* for STA/APCLI - main thread to wait for mlme completes LinkDown */
VOID sta_os_completion_initialize(STA_ADMIN_CONFIG *pStaCfg)
{
	RTMP_OS_INIT_COMPLETION(&pStaCfg->linkdown_complete);
}

VOID sta_link_down_complete(STA_ADMIN_CONFIG *pStaCfg)
{
	RTMP_OS_COMPLETE(&pStaCfg->linkdown_complete);
}

VOID sta_wait_link_down(STA_ADMIN_CONFIG *pStaCfg)
{
	if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pStaCfg->linkdown_complete, STA_OS_WAIT_TIMEOUT)) {
		struct wifi_dev *wdev = &pStaCfg->wdev;
		RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;
		UINT linkdown_type = 0;

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_ERROR,
				 "sta idx [%d] can't linkdown within 500ms, do linkdown in main thread\n"
				  , pStaCfg->wdev.func_idx);
		LinkDown(pAd, linkdown_type, wdev, NULL);
		if (pStaCfg->ApcliInfStat.Valid == TRUE) {
			pStaCfg->ApcliInfStat.Valid = FALSE;
			/* clear MlmeAux.Ssid and Bssid. */
			NdisZeroMemory(pStaCfg->MlmeAux.Bssid, MAC_ADDR_LEN);
			pStaCfg->MlmeAux.SsidLen = 0;
			NdisZeroMemory(pStaCfg->MlmeAux.Ssid, MAX_LEN_OF_SSID);
			pStaCfg->MlmeAux.Rssi = 0;
		}
	}
}

VOID sta_ifdown_fsm_reset(STA_ADMIN_CONFIG *pStaCfg)
{
	struct wifi_dev *wdev = &pStaCfg->wdev;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_NOTICE,
			 "sta idx [%d].\n", pStaCfg->wdev.func_idx);
	cntl_fsm_reset(wdev);
	auth_fsm_reset(wdev);
	assoc_fsm_reset(wdev);
}


VOID sta_fsm_ops_hook(struct wifi_dev *wdev)
{
	sta_cntl_init(wdev);
	sta_auth_init(wdev);
	sta_assoc_init(wdev);
}

#ifdef CONFIG_APSTA_MIXED_SUPPORT

VOID ApCliPeerCsaAction(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BCN_IE_LIST *ie_list)
{
	struct DOT11_H *pDot11h = NULL;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
#if defined(CONFIG_MAP_SUPPORT)
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
#endif
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
#endif
#endif
#ifdef DOT11W_PMF_SUPPORT
	MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)NULL;

	pEntry = MacTableLookup(pAd, ie_list->Addr2);
	if (pEntry == NULL)
		return;
#endif /* DOT11W_PMF_SUPPORT */

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;

	if ((pAd->CommonCfg.bIEEE80211H == 1) &&
		ie_list->NewChannel != 0 &&
		wdev->channel != ie_list->NewChannel &&
		pDot11h->ChannelMode != CHAN_SWITCHING_MODE) {
#ifdef DOT11_VHT_AC
		{
			struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

			if (IS_CAP_BW160(cap)) {
				if (ie_list->cmm_ies.wb_info.new_ch_width == 1)
					wlan_config_set_cen_ch_2(wdev, ie_list->cmm_ies.wb_info.center_freq_2);
			}
		}
#endif /* DOT11_VHT_AC */
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				 "[APCLI]  Following root AP to switch channel to ch%u\n",
				  ie_list->NewChannel);

#if defined(WAPP_SUPPORT) && defined(CONFIG_MAP_SUPPORT)

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"Channel Change due to csa\n");
#endif

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)

		if ((pDfsParam->bDedicatedZeroWaitDefault == TRUE) &&
			(wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G) &&
			(RadarChannelCheck(pAd, ie_list->NewChannel))) {
			*ch_stat = DFS_OUTB_CH_CAC;
			if (ie_list->cmm_ies.wb_info.center_freq_2 == 0 && IS_CH_BETWEEN(ie_list->NewChannel, 36, 48))
				*ch_stat = DFS_INB_DFS_OUTB_CH_CAC;
			else
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
					"[APCLI] Following root AP to switch to DFS channel\n");
		}
#endif
#endif

#if defined(CONFIG_MAP_SUPPORT)
				if (IS_MAP_TURNKEY_ENABLE(pAd) && (pAd->bMAPAvoidScanDuringCac == 1)) {
						BssTableInit(ScanTab);
				}
#endif
		/*To do set channel for PEER CSA, need TakeChannelOpCharge first*/
		if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_PEER_CSA, FALSE)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"TakeChannelOpCharge fail for PEER CSA!!\n");
			return;
		}

#ifdef DFS_ADJ_BW_ZERO_WAIT
		if (IS_ADJ_BW_ZERO_WAIT_TX80RX160(pDfsParam->BW160ZeroWaitState) && (RadarChannelCheck(pAd, ie_list->NewChannel)) && ie_list->cmm_ies.wb_info.center_freq_2 != 0) {

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"[APCLI] Recieve CSA to DFS channel, do CAC\n");

			pAd->CommonCfg.DfsParameter.band_ch = ie_list->NewChannel;
			pDot11h->RDCount = 0;

			rtmp_set_channel(pAd, wdev, 36);
		} else
#endif

		rtmp_set_channel(pAd, wdev, ie_list->NewChannel);

		/*if no need CSA, just release ChannelOpCharge here*/
		if (pAd->ApCfg.set_ch_async_flag == FALSE)
			ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_PEER_CSA);

#ifdef DOT11W_PMF_SUPPORT
		if (pEntry->SecConfig.ocv_support)
			pDot11h->ChChangeCSA = TRUE;
#endif /* DOT11W_PMF_SUPPORT */

#if defined(WAPP_SUPPORT) && defined(CONFIG_MAP_SUPPORT)
		if (IS_MAP_ENABLE(pAd))
			pDot11h->ChChangeCSA = TRUE;
		wapp_send_csa_event(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), wdev->channel);
#endif
	}
}

NTSTATUS ApcliMloCsaSwitchChannelHandler(RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt)
{
	struct wifi_dev *wdev;
	STA_ADMIN_CONFIG *pStaCfg = NULL;
	struct MLO_CSA_INFO *pMlo_csa_info = &pAd->Dot11_H.mlo_csa_info;
	UCHAR apidx;
	UCHAR newCh;
	struct DOT11_H *pDot11h = NULL;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
#if defined(CONFIG_MAP_SUPPORT)
	PBSS_TABLE ScanTab = NULL;
#endif
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
#endif
#endif
	NdisMoveMemory(&apidx, CMDQelmt->buffer, sizeof(UCHAR));
	newCh = pMlo_csa_info->new_channel;
	pStaCfg = &pAd->StaCfg[apidx];
	wdev = &pStaCfg->wdev;
	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "cmd> get wdev fail!\n");
		return NDIS_STATUS_FAILURE;
	}
#if defined(CONFIG_MAP_SUPPORT)
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);
#endif
	pDot11h = wdev->pDot11_H;

	if (pDot11h == NULL)
		return NDIS_STATUS_FAILURE;

	if ((pAd->CommonCfg.bIEEE80211H == 1) &&
		newCh != 0 && wdev->channel != newCh &&
		pDot11h->ChannelMode != CHAN_SWITCHING_MODE) {
#ifdef DOT11_VHT_AC
		{
			struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

			if (IS_CAP_BW160(cap)) {
				if (pMlo_csa_info->wb_info.new_ch_width == 1)
					wlan_config_set_cen_ch_2(wdev, pMlo_csa_info->wb_info.center_freq_2);
			}
		}
#endif /* DOT11_VHT_AC */
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				 "[APCLI]  MLO CSA info receive Following root AP to switch channel to ch%u\n",
				  newCh);

#if defined(WAPP_SUPPORT) && defined(CONFIG_MAP_SUPPORT)

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"Channel Change due to csa\n");
#endif

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)

		if ((pDfsParam->bDedicatedZeroWaitDefault == TRUE) &&
			(wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G) &&
			(RadarChannelCheck(pAd, newCh))) {
			*ch_stat = DFS_OUTB_CH_CAC;
			if (pMlo_csa_info->wb_info.center_freq_2 == 0 && IS_CH_BETWEEN(newCh, 36, 48))
				*ch_stat = DFS_INB_DFS_OUTB_CH_CAC;
			else
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
					"[APCLI] Following root AP to switch to DFS channel\n");
		}
#endif
#endif

#if defined(CONFIG_MAP_SUPPORT)
		if (IS_MAP_TURNKEY_ENABLE(pAd) && (pAd->bMAPAvoidScanDuringCac == 1))
			BssTableInit(ScanTab);
#endif
		/*To do set channel for PEER CSA, need TakeChannelOpCharge first*/
		if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_PEER_CSA, FALSE)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"TakeChannelOpCharge fail for PEER CSA!!\n");
			return NDIS_STATUS_FAILURE;
		}

#ifdef DFS_ADJ_BW_ZERO_WAIT
		if ((pDfsParam->BW160ZeroWaitSupport == TRUE && IS_ADJ_BW_ZERO_WAIT_TX80RX160(pDfsParam->BW160ZeroWaitState))
		&& (RadarChannelCheck(pAd, newCh)) && pMlo_csa_info->wb_info.center_freq_2 != 0) {

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"[APCLI] Receive CSA to DFS channel, do CAC\n");

			pAd->CommonCfg.DfsParameter.band_ch = newCh;
			pDot11h->RDCount = 0;

			rtmp_set_channel(pAd, wdev, 36);
		} else
#endif
			if (rtmp_set_channel(pAd, wdev, newCh)) {
				/*if no need CSA, just release ChannelOpCharge here*/
				if (pAd->ApCfg.set_ch_async_flag == FALSE)
					ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_PEER_CSA);

#if defined(WAPP_SUPPORT) && defined(CONFIG_MAP_SUPPORT)
				if (IS_MAP_ENABLE(pAd))
					pDot11h->ChChangeCSA = TRUE;
				wapp_send_csa_event(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), wdev->channel);
#endif
			} else
				pMlo_csa_info->Ch_Switching = FALSE;
	}
	return NDIS_STATUS_SUCCESS;
}

/*
	==========================================================================
	Description:
		Check the Apcli Entry is valid or not.
	==========================================================================
 */
static inline BOOLEAN ValidApCliEntry(RTMP_ADAPTER *pAd, INT apCliIdx)
{
	BOOLEAN result;
	PMAC_TABLE_ENTRY pMacEntry;
	STA_ADMIN_CONFIG *pApCliEntry;

	do {
		if ((apCliIdx < 0) || (apCliIdx >= MAX_APCLI_NUM)) {
			result = FALSE;
			break;
		}

		pApCliEntry = (STA_ADMIN_CONFIG *)&pAd->StaCfg[apCliIdx];

		if (pApCliEntry->ApcliInfStat.Valid != TRUE) {
			result = FALSE;
			break;
		}

		if (pApCliEntry->ApcliInfStat.Enable != TRUE) {
			result = FALSE;
			break;
		}

		if (!VALID_UCAST_ENTRY_WCID(pAd, pApCliEntry->MacTabWCID)) {
			result = FALSE;
			break;
		}

		pMacEntry = entry_get(pAd, pApCliEntry->MacTabWCID);

		if (!IS_ENTRY_PEER_AP(pMacEntry)) {
			result = FALSE;
			break;
		}

		result = TRUE;
	} while (FALSE);

	return result;
}

INT apcli_fp_tx_pkt_allowed(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pkt)
{
	UCHAR idx;
	INT	allowed = NDIS_STATUS_FAILURE;
	STA_ADMIN_CONFIG *apcli_entry;
	STA_TR_ENTRY *tr_entry = NULL;
#ifdef WSC_INCLUDED
	BOOLEAN do_wsc_now = FALSE;
#endif /* WSC_INCLUDED */
	UINT16 wcid = RTMP_GET_PACKET_WCID(pkt);
	MAC_TABLE_ENTRY *pTmpEntry = NULL;
	UCHAR frag_nums;
#ifdef MAP_TS_TRAFFIC_SUPPORT
	MAC_TABLE_ENTRY *peer_entry = NULL;
#endif
	UINT16 pktType = 0;
	UCHAR *pSrcBuf = NULL;

	switch (wdev->wdev_type) {
	case WDEV_TYPE_STA:
		for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
			apcli_entry = &pAd->StaCfg[idx];

			if (&apcli_entry->wdev == wdev) {
				if (ValidApCliEntry(pAd, idx) == FALSE)
					break;

#ifdef MAC_REPEATER_SUPPORT

				if ((pAd->ApCfg.bMACRepeaterEn == TRUE) && repeater_enable_by_any_band(pAd)
#ifdef A4_CONN
					&& (IS_APCLI_A4(apcli_entry) == FALSE)
#endif /* A4_CONN */
				   ) {
					UINT Ret = 0;
					Ret = ReptTxPktCheckHandler(pAd, wdev, pkt, &wcid);
					if (Ret == REPEATER_ENTRY_EXIST)
						allowed = NDIS_STATUS_SUCCESS;
					else if (Ret == INSERT_REPT_ENTRY) {
						MTWF_DBG(NULL, DBG_CAT_TX, CATTX_SEC_TX, DBG_LVL_DEBUG,
								 "apcli_fp_tx_pkt_allowed: return FALSE as ReptTxPktCheckHandler indicated INSERT_REPT_ENTRY\n");
						allowed = NDIS_STATUS_FAILURE;
					} else if (Ret == INSERT_REPT_ENTRY_AND_ALLOW)
						allowed = NDIS_STATUS_SUCCESS;
					else if (Ret == USE_CLI_LINK_INFO) {
						wcid = apcli_entry->MacTabWCID;
						allowed = NDIS_STATUS_SUCCESS;
					}
				} else
#endif /* MAC_REPEATER_SUPPORT */
				{
					pAd->RalinkCounters.PendingNdisPacketCount++;
					RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
					wcid = apcli_entry->MacTabWCID;
					allowed = NDIS_STATUS_SUCCESS;
				}

				break;
			}
		}

		break;

#ifdef MAC_REPEATER_SUPPORT
	case WDEV_TYPE_REPEATER:
	{
		UCHAR MaxNumChipRept = GET_MAX_REPEATER_ENTRY_NUM(pChipCap);
		REPEATER_CLIENT_ENTRY *rept = NULL;
		/* Case EAPOL */
		if ((wdev->func_idx >= 0) && (wdev->func_idx < MaxNumChipRept))
			rept = &pAd->ApCfg.pRepeaterCliPool[wdev->func_idx];

		if (IS_REPT_LINK_UP(rept))
			allowed = NDIS_STATUS_SUCCESS;
	}
		break;
#endif /* MAC_REPEATER_SUPPORT */

	default:
		allowed = NDIS_STATUS_FAILURE;
		break;

	}

	if (!IS_WCID_VALID(pAd, wcid))
		return NDIS_STATUS_FAILURE;

	if (allowed == NDIS_STATUS_SUCCESS) {
		RTMP_SET_PACKET_WCID(pkt, wcid);
		frag_nums = get_frag_num(pAd, wdev, pkt);
		RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);

		tr_entry = tr_entry_get(pAd, wcid);
		/*  ethertype check is not offload to mcu for fragment frame*/
		if ((frag_nums > 1)
			|| RTMP_TEST_FLAG(pAd->physical_dev, PH_DEV_CHECK_ETH_TYPE)) {
			if (!RTMPCheckEtherType(pAd, pkt, tr_entry, wdev))
				return NDIS_STATUS_FAILURE;
		}

#ifdef WSC_INCLUDED
		if ((wdev->WscControl.WscConfMode != WSC_DISABLE)
			&& wdev->WscControl.bWscTrigger)
			do_wsc_now = TRUE;
#endif /* WSC_INCLUDED */

		if (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED) {
			pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
			if (pSrcBuf)
				pktType = (pSrcBuf[12] << 8) | pSrcBuf[13];

			if (!(IS_AKM_WPA_CAPABILITY_Entry(wdev)
#ifdef DOT1X_SUPPORT
				|| (IS_IEEE8021X_Entry(wdev))
#endif /* DOT1X_SUPPORT */
#ifdef WSC_INCLUDED
				|| do_wsc_now
#endif /* WSC_INCLUDED */
				)) {
				MTWF_DBG(pAd, DBG_CAT_RX, CATRX_SEC_RX, DBG_LVL_INFO,
					"Drop PKT before 4-Way Handshake done! wcid = %d.\n", wcid);
				return NDIS_STATUS_FAILURE;
			}
			if (pktType != ETH_TYPE_EAPOL && !RTMP_GET_PACKET_WAI(pkt)) {
				MTWF_DBG(pAd, DBG_CAT_RX, CATRX_SEC_RX, DBG_LVL_INFO,
					"Drop PKT before 4-Way Handshake done! pktType:0x%x.\n", pktType);
				return NDIS_STATUS_FAILURE;
			}
		}
		pTmpEntry = entry_get(pAd, wcid);
#ifdef RT_CFG80211_SUPPORT
		pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
		if (pSrcBuf)
			pktType = (pSrcBuf[12] << 8) | pSrcBuf[13];
		if (pktType == ETH_TYPE_EAPOL) {
			if (pTmpEntry->SecConfig.is_eapol_encrypted && tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
				RTMP_SET_PACKET_CLEAR_EAP_FRAME(pkt, 0);
			else
				RTMP_SET_PACKET_CLEAR_EAP_FRAME(pkt, 1);

			if (pAd->FragFrame.wcid == wcid) {
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_SEC_TX, DBG_LVL_INFO,
					"\nClear Wcid = %d FragBuffer !!!!!\n", wcid);
				RESET_FRAGFRAME(pAd->FragFrame);
			}
		}
#endif
		/* if sta rec isn't valid, don't allow pkt tx */
		if (!pTmpEntry) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_SEC_TX, DBG_LVL_INFO,
				"Drop PKT due to no StaRec! wcid = %d.\n", wcid);
			allowed = NDIS_STATUS_FAILURE;
		} else if (!pTmpEntry->sta_rec_valid) {
			struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);

			if (tr_entry->delay_queue.Number < SQ_ENQ_DELAYQ_MAX) {
				qm_ops->enq_delayq_pkt(pAd, wdev, tr_entry, pkt);
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
					"Requeue PKT until StaRec ready! wcid = %d.\n",  wcid);
				return NDIS_STATUS_PKT_REQUEUE;
			}

			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
				"Drop PKT due to delay queue full! wcid = %d.\n", wcid);
			return NDIS_STATUS_FAILURE;
		}

#ifdef MAP_TS_TRAFFIC_SUPPORT
		if (pAd->bTSEnable) {
			if ((wdev->func_idx >= 0) && IS_WCID_VALID(pAd, wcid))
				peer_entry = entry_get(pAd, wcid);

			if (peer_entry && !map_ts_tx_process(pAd, wdev, pkt, peer_entry))
				return NDIS_STATUS_FAILURE;
		}
#endif
	}

	return allowed;
}

INT apcli_tx_pkt_allowed(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pkt)
{
	UCHAR idx;
	BOOLEAN	allowed = NDIS_STATUS_FAILURE;
	STA_ADMIN_CONFIG *apcli_entry;
	STA_TR_ENTRY *tr_entry = NULL;
	UINT16 wcid = RTMP_GET_PACKET_WCID(pkt);
	UCHAR frag_nums;
	MAC_TABLE_ENTRY *pTmpEntry = NULL;
#ifdef WSC_INCLUDED
	BOOLEAN do_wsc_now = FALSE;
#endif /* WSC_INCLUDED */
#ifdef MAP_TS_TRAFFIC_SUPPORT
	MAC_TABLE_ENTRY *peer_entry = NULL;
#endif
	UINT16 pktType = 0;
	UCHAR *pSrcBuf = NULL;

	switch (wdev->wdev_type) {
	case WDEV_TYPE_STA:
		for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
			apcli_entry = &pAd->StaCfg[idx];

			if (&apcli_entry->wdev == wdev) {
				if (ValidApCliEntry(pAd, idx) == FALSE)
					break;

#ifdef MAC_REPEATER_SUPPORT
				if ((pAd->ApCfg.bMACRepeaterEn == TRUE) && repeater_enable_by_any_band(pAd)
#ifdef A4_CONN
					&& (IS_APCLI_A4(apcli_entry) == FALSE)
#endif /* A4_CONN */

				   ) {
					UINT Ret = 0;
					Ret = ReptTxPktCheckHandler(pAd, wdev, pkt, &wcid);

					if (Ret == REPEATER_ENTRY_EXIST) {
						allowed = NDIS_STATUS_SUCCESS;
					} else if (Ret == INSERT_REPT_ENTRY) {
						MTWF_DBG(NULL, DBG_CAT_TX, CATTX_SEC_TX, DBG_LVL_DEBUG,
								 "apcli_tx_pkt_allowed: return FALSE as ReptTxPktCheckHandler indicated INSERT_REPT_ENTRY\n");
						allowed = NDIS_STATUS_FAILURE;
					} else if (Ret == INSERT_REPT_ENTRY_AND_ALLOW) {
						allowed = NDIS_STATUS_SUCCESS;
					} else if (Ret == USE_CLI_LINK_INFO) {
						wcid = apcli_entry->MacTabWCID;
						allowed = NDIS_STATUS_SUCCESS;
					}
				} else
#endif /* MAC_REPEATER_SUPPORT */
				{
					pAd->RalinkCounters.PendingNdisPacketCount++;
					RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
					wcid = apcli_entry->MacTabWCID;
					allowed = NDIS_STATUS_SUCCESS;
				}

				break;
			}
		}
		break;

#ifdef MAC_REPEATER_SUPPORT
	case WDEV_TYPE_REPEATER:
	{
		/* Case EAPOL */
		REPEATER_CLIENT_ENTRY *rept = \
			&pAd->ApCfg.pRepeaterCliPool[wdev->func_idx];
		if (IS_REPT_LINK_UP(rept))
			allowed = NDIS_STATUS_SUCCESS;
	}
		break;
#endif /* MAC_REPEATER_SUPPORT */

	default:
		allowed = NDIS_STATUS_FAILURE;
		break;

	}

	if (allowed == NDIS_STATUS_SUCCESS) {
		RTMP_SET_PACKET_WCID(pkt, wcid);
		frag_nums = get_frag_num(pAd, wdev, pkt);
		RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);

#ifdef WSC_INCLUDED
		if ((wdev->WscControl.WscConfMode != WSC_DISABLE)
			&& wdev->WscControl.bWscTrigger)
			do_wsc_now = TRUE;
#endif /* WSC_INCLUDED */
		if (wcid >= WTBL_MAX_NUM(pAd))
			return NDIS_STATUS_FAILURE;

		tr_entry = tr_entry_get(pAd, wcid);
		if (!RTMPCheckEtherType(pAd, pkt, tr_entry, wdev))
			allowed = NDIS_STATUS_FAILURE;

		if (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED) {
			pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
			if (pSrcBuf)
				pktType = (pSrcBuf[12] << 8) | pSrcBuf[13];
			if (!(
#ifdef WSC_INCLUDED
				do_wsc_now ||
#endif /* WSC_INCLUDED */
				IS_AKM_WPA_CAPABILITY_Entry(wdev)
#ifdef DOT1X_SUPPORT
				|| (IS_IEEE8021X_Entry(wdev))
#endif /* DOT1X_SUPPORT */
				  )) {
				MTWF_DBG(pAd, DBG_CAT_RX, CATRX_SEC_RX, DBG_LVL_INFO,
					"Drop PKT before 4-Way Handshake done! wcid = %d.\n", wcid);
				allowed = NDIS_STATUS_FAILURE;
			}
			if (pktType != ETH_TYPE_EAPOL && !RTMP_GET_PACKET_WAI(pkt)) {
				MTWF_DBG(pAd, DBG_CAT_RX, CATRX_SEC_RX, DBG_LVL_INFO,
					"Drop PKT before 4-Way Handshake done! pktType:0x%x.\n", pktType);
				allowed = NDIS_STATUS_FAILURE;
			}
		}

		/* if sta rec isn't valid, don't allow pkt tx */
		pTmpEntry = entry_get(pAd, wcid);
		if (!pTmpEntry) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_SEC_TX, DBG_LVL_INFO,
				"Drop PKT due to no StaRec! wcid = %d.\n", wcid);
			allowed = NDIS_STATUS_FAILURE;
		} else if (!pTmpEntry->sta_rec_valid) {
			struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);

			if (tr_entry->delay_queue.Number < SQ_ENQ_DELAYQ_MAX) {
				qm_ops->enq_delayq_pkt(pAd, wdev, tr_entry, pkt);
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
					"Requeue PKT until StaRec ready! wcid = %d.\n",  wcid);
				return NDIS_STATUS_PKT_REQUEUE;
			}

			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
				"Drop PKT due to delay queue full! wcid = %d.\n", wcid);
			return NDIS_STATUS_FAILURE;
		}

#ifdef MAP_TS_TRAFFIC_SUPPORT
		if (pAd->bTSEnable) {
			if (IS_WCID_VALID(pAd, wcid))
				peer_entry = entry_get(pAd, wcid);

			if (peer_entry && !map_ts_tx_process(pAd, wdev, pkt, peer_entry))
				return NDIS_STATUS_FAILURE;
		}
#endif
	}

	return allowed;
}


BOOLEAN ApCliMsgTypeSubst(
	IN PRTMP_ADAPTER pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType) {
	USHORT Seq;
	UCHAR EAPType;
	BOOLEAN Return = FALSE;
#ifdef WSC_AP_SUPPORT
	UCHAR EAPCode;
	PMAC_TABLE_ENTRY pEntry;
#endif /* WSC_AP_SUPPORT */
	unsigned char hdr_len = LENGTH_802_11;
#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
	USHORT Alg;
#endif
#ifdef A4_CONN
	if ((pFrame->Hdr.FC.FrDs == 1) && (pFrame->Hdr.FC.ToDs == 1))
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif


	/* only PROBE_REQ can be broadcast, all others must be unicast-to-me && is_mybssid; otherwise, */
	/* ignore this frame */

	/* WPA EAPOL PACKET */
	if (pFrame->Hdr.FC.Type == FC_TYPE_DATA) {
#ifdef WSC_AP_SUPPORT
		/*WSC EAPOL PACKET */
		pEntry = MacTableLookup(pAd, pFrame->Hdr.Addr2);

		if (pEntry && IS_ENTRY_PEER_AP(pEntry) && pAd->StaCfg[pEntry->func_tb_idx].wdev.WscControl.WscConfMode == WSC_ENROLLEE) {
			*Machine = WSC_STATE_MACHINE;
			EAPType = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 1);
			EAPCode = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 4);
			Return = WscMsgTypeSubst(EAPType, EAPCode, MsgType);
		}

		if (!Return)
#endif /* WSC_AP_SUPPORT */
		{
			*Machine = WPA_STATE_MACHINE;
			EAPType = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 1);
			Return = WpaMsgTypeSubst(EAPType, MsgType);
		}

		return Return;
	} else if (pFrame->Hdr.FC.Type == FC_TYPE_MGMT) {
		switch (pFrame->Hdr.FC.SubType) {
		case SUBTYPE_ASSOC_RSP:
			*Machine = ASSOC_FSM;
			*MsgType = ASSOC_FSM_PEER_ASSOC_RSP;
			break;

		case SUBTYPE_REASSOC_RSP:
			*Machine = ASSOC_FSM;
			*MsgType = ASSOC_FSM_PEER_REASSOC_RSP;
			break;

		case SUBTYPE_DISASSOC:
			*Machine = ASSOC_FSM;
			*MsgType = ASSOC_FSM_PEER_DISASSOC_REQ;
			break;

		case SUBTYPE_DEAUTH:
			*Machine = AUTH_FSM;
			*MsgType = AUTH_FSM_PEER_DEAUTH;
			break;

		case SUBTYPE_AUTH:
#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
			NdisMoveMemory(&Alg, &pFrame->Octet[0], sizeof(USHORT));
#endif /* DOT11_SAE_SUPPORT */
			/* get the sequence number from payload 24 Mac Header + 2 bytes algorithm */
			NdisMoveMemory(&Seq, &pFrame->Octet[2], sizeof(USHORT));
#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
			if (Alg == AUTH_MODE_SAE) {
				*Machine = AUTH_FSM;
				*MsgType = AUTH_FSM_SAE_AUTH_RSP;
			} else
#endif /* DOT11_SAE_SUPPORT */
			if (Seq == 2 || Seq == 4) {
				*Machine = AUTH_FSM;
				*MsgType = AUTH_FSM_PEER_AUTH_EVEN;
			} else
				return FALSE;

			break;

		case SUBTYPE_ACTION:
			*Machine = ACTION_STATE_MACHINE;

			/*  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support */
			if ((pFrame->Octet[0] & 0x7F) > MAX_PEER_CATE_MSG)
				*MsgType = MT2_ACT_INVALID;
			else
				*MsgType = (pFrame->Octet[0] & 0x7F);

			break;

		default:
			return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}



BOOLEAN preCheckMsgTypeSubset(
	IN PRTMP_ADAPTER  pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType)
{
	if (pFrame->Hdr.FC.Type == FC_TYPE_MGMT) {
		switch (pFrame->Hdr.FC.SubType) {
		/* Beacon must be processed by AP Sync state machine. */
		case SUBTYPE_BEACON:
			*Machine = SYNC_FSM;
			*MsgType = SYNC_FSM_PEER_BEACON;
			break;

		/* Only Sta have chance to receive Probe-Rsp. */
		case SUBTYPE_PROBE_RSP:
			*Machine = SYNC_FSM;
			*MsgType = SYNC_FSM_PEER_PROBE_RSP;
			break;

		default:
			return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}

BOOLEAN  ApCliHandleRxBroadcastFrame(
	IN RTMP_ADAPTER *pAd,
	IN struct _RX_BLK *pRxBlk,
	IN MAC_TABLE_ENTRY *pEntry) {
	STA_ADMIN_CONFIG *pApCliEntry = NULL;
	/*
		It is possible to receive the multicast packet when in AP Client mode
		ex: broadcast from remote AP to AP-client,
				addr1=ffffff, addr2=remote AP's bssid, addr3=sta_mac_addr
	*/

	pApCliEntry = GetStaCfgByWdev(pAd, pEntry->wdev);

	/* Filter out Bcast frame which AP relayed for us */
	/* Multicast packet send from AP1 , received by AP2 and send back to AP1, drop this frame */

	if (MAC_ADDR_EQUAL(pRxBlk->Addr3, pApCliEntry->wdev.if_addr))
		return FALSE;

#ifdef MAC_REPEATER_SUPPORT
	if (lookup_rept_entry(pAd, pRxBlk->Addr3))
		return FALSE;
#endif /* MAC_REPEATER_SUPPORT */

	if (pEntry->PrivacyFilter != Ndis802_11PrivFilterAcceptAll)
		return FALSE;

	return TRUE;
}

BOOLEAN apcli_fill_non_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk)
{
	PNDIS_PACKET pPacket;
	MAC_TABLE_ENTRY *pMacEntry = NULL, *tmp_entry;
	struct wifi_dev_ops *ops = wdev->wdev_ops;

	pPacket = pTxBlk->pPacket;
	pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
	pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	pTxBlk->wmm_set = HcGetWmmIdx(pAd, wdev);
	pTxBlk->UserPriority = RTMP_GET_PACKET_UP(pPacket);
	pTxBlk->pMbss = NULL;
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;

	if (IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) {
		if ((pTxBlk->TxFrameType == TX_LEGACY_FRAME) ||
			(pTxBlk->TxFrameType == TX_AMSDU_FRAME) ||
			(pTxBlk->TxFrameType == TX_BMC_FRAME))
			TX_BLK_SET_FLAG(pTxBlk, fTX_HDR_TRANS);
	}

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pTxBlk->pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bClearEAPFrame);


	if (pTxBlk->tr_entry->EntryType == ENTRY_CAT_MCAST) {
		pTxBlk->pMacEntry = NULL;
		TX_BLK_SET_FLAG(pTxBlk, fTX_ForceRate);
		{
			{
				tmp_entry = entry_get(pAd, MCAST_WCID_TO_REMOVE);
				pTxBlk->pTransmit = &tmp_entry->HTPhyMode;

				pTxBlk->pTransmit->field.MODE = MODE_OFDM;
				pTxBlk->pTransmit->field.MCS = MCS_RATE_6;
			}
		}
		/* AckRequired = FALSE, when broadcast packet in Adhoc mode.*/
		TX_BLK_CLEAR_FLAG(pTxBlk, (fTX_bAckRequired | fTX_bAllowFrag | fTX_bWMM));

		if (RTMP_GET_PACKET_MOREDATA(pPacket))
			TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);
	} else {
		if IS_WCID_VALID(pAd, pTxBlk->Wcid)
			pTxBlk->pMacEntry = entry_get(pAd, pTxBlk->Wcid);

		pTxBlk->pTransmit = &pTxBlk->pMacEntry->HTPhyMode;
		pMacEntry = pTxBlk->pMacEntry;

		if (!pMacEntry)
			MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
				"Err!! pMacEntry is NULL!!\n");
		else
			pTxBlk->pMbss = MBSS_GET(pMacEntry->pMbss);

#ifdef MULTI_WMM_SUPPORT

		if (pMacEntry && IS_ENTRY_PEER_AP(pMacEntry))
			pTxBlk->QueIdx = EDCA_WMM1_AC0_PIPE;

#endif /* MULTI_WMM_SUPPORT */
		/* For all unicast packets, need Ack unless the Ack Policy is not set as NORMAL_ACK.*/
#ifdef MULTI_WMM_SUPPORT

		if (pTxBlk->QueIdx >= EDCA_WMM1_AC0_PIPE) {
			if (((pTxBlk->QueIdx - EDCA_WMM1_AC0_PIPE) >= 0) &&
					((pTxBlk->QueIdx - EDCA_WMM1_AC0_PIPE) < WMM_NUM_OF_AC) &&
					pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx - EDCA_WMM1_AC0_PIPE] != NORMAL_ACK)
				TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
			else
				TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);
		} else
#endif /* MULTI_WMM_SUPPORT */
		{
			if ((pTxBlk->QueIdx < WMM_NUM_OF_AC) &&
					pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx] != NORMAL_ACK)
				TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
			else
				TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);
		}

		{
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef A4_CONN
				if (pMacEntry && IS_ENTRY_A4(pMacEntry)) {
					pTxBlk->pMacEntry = pMacEntry;
					pTxBlk->pApCliEntry = &pAd->StaCfg[pMacEntry->func_tb_idx];
					TX_BLK_SET_FLAG(pTxBlk, fTX_bA4Frame);
				} else
#endif /* A4_CONN */
				if (pMacEntry && (IS_ENTRY_PEER_AP(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry)) &&
					((pTxBlk->TxFrameType != TX_BMC_FRAME) &&
					 (pTxBlk->TxFrameType != TX_MLME_DATAQ_FRAME) &&
					 (pTxBlk->TxFrameType != TX_MLME_MGMTQ_FRAME))) {
#ifdef MAT_SUPPORT
					PNDIS_PACKET apCliPkt = NULL;
					UCHAR *pMacAddr = NULL;
#ifdef MAC_REPEATER_SUPPORT

					if (IS_REPT_LINK_UP(pMacEntry->pReptCli) && (pAd->ApCfg.bMACRepeaterEn)) {

						pAd->MatCfg.bMACRepeaterEn = pAd->ApCfg.bMACRepeaterEn;

						if (pAd->ApCfg.MACRepeaterOuiMode != CASUALLY_DEFINE_MAC_ADDR) {
							/* TODO: shiang-lock, fix ME! */
							apCliPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, pMacEntry->pReptCli->CliIdx, pMacEntry->EntryType);
							pMacAddr = pMacEntry->pReptCli->CurrentAddress;
						}
					} else
#endif /* MAC_REPEATER_SUPPORT */
					{
						/* For each tx packet, update our MAT convert engine databases.*/
#ifdef DOT11_EHT_BE
						MAC_TABLE_ENTRY *peer_entry = NULL;
#endif
						/* CFG_TODO */
#ifdef APCLI_AS_WDS_STA_SUPPORT
						if ((pMacEntry->func_tb_idx < MAX_MULTI_STA) &&
							pAd->StaCfg[pMacEntry->func_tb_idx].wdev.wds_enable == 0)
#endif /* APCLI_AS_WDS_STA_SUPPORT */
							apCliPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, pMacEntry->func_tb_idx, pMacEntry->EntryType);
#ifdef DOT11_EHT_BE
						/* replace with Mld address if mlo enabled */
						peer_entry = GetAssociatedAPByWdev(pAd, wdev);
						if (peer_entry && peer_entry->mlo.mlo_en && wdev->mld_dev)
							pMacAddr = wdev->mld_dev->mld_addr;
						else
#endif
						if (pMacEntry->func_tb_idx < MAX_MULTI_STA)
							pMacAddr = &pAd->StaCfg[pMacEntry->func_tb_idx].wdev.if_addr[0];
					}

					if (apCliPkt) {
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
						pPacket = apCliPkt;
						pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
						pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);
						pTxBlk->pPacket = apCliPkt;
					} else {
						if (OS_PKT_CLONED(pPacket)) {
							OS_PKT_COPY(pPacket, apCliPkt);

							if (apCliPkt == NULL) {
								MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
									"apCliPkt is NULL!!\n");
								return FALSE;
							}
							RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
							pPacket = apCliPkt;
							pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
							pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);
							pTxBlk->pPacket = apCliPkt;
						}
					}

					{
						PUCHAR pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);

						if (pMacAddr) {
#ifdef APCLI_AS_WDS_STA_SUPPORT
							if ((pMacEntry->func_tb_idx < MAX_MULTI_STA) &&
									pAd->StaCfg[pMacEntry->func_tb_idx].wdev.wds_enable == 0)

#endif /* APCLI_AS_WDS_STA_SUPPORT */
								NdisMoveMemory(pSrcBufVA + 6, pMacAddr, MAC_ADDR_LEN);
						}
					}

#endif /* MAT_SUPPORT */
					pTxBlk->pApCliEntry = GetStaCfgByWdev(pAd, pMacEntry->wdev);
					TX_BLK_SET_FLAG(pTxBlk, fTX_bApCliPacket);
				} else if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry))
					;
				else
					return FALSE;

				/* If both of peer and us support WMM, enable it.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM);
			}
		}

		if (pTxBlk->TxFrameType == TX_LEGACY_FRAME) {
			if (((RTMP_GET_PACKET_LOWRATE(pPacket))
#ifdef UAPSD_SUPPORT
				 && (!(pMacEntry && (pMacEntry->bAPSDFlagSPStart)))
#endif /* UAPSD_SUPPORT */
				) ||
				((pAd->OpMode == OPMODE_AP) && (pMacEntry->MaxHTPhyMode.field.MODE == MODE_CCK) && (pMacEntry->MaxHTPhyMode.field.MCS == RATE_1))
			   ) {
				/* Specific packet, i.e., bDHCPFrame, bEAPOLFrame, bWAIFrame, need force low rate. */
				tmp_entry = entry_get(pAd, MCAST_WCID_TO_REMOVE);
				pTxBlk->pTransmit = &tmp_entry->HTPhyMode;
				TX_BLK_SET_FLAG(pTxBlk, fTX_ForceRate);

				/* Modify the WMM bit for ICV issue. If we have a packet with EOSP field need to set as 1, how to handle it? */
				if (!pTxBlk->pMacEntry)
					MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
						"Err!! pTxBlk->pMacEntry is NULL!!\n");
				else if (IS_HT_STA(pTxBlk->pMacEntry) &&
						 (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET)))
					TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
			}

			if (!pMacEntry)
				MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
					"Err!! pMacEntry is NULL!!\n");

			if (RTMP_GET_PACKET_MOREDATA(pPacket))
				TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);

#ifdef UAPSD_SUPPORT

			if (RTMP_GET_PACKET_EOSP(pPacket))
				TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP);

#endif /* UAPSD_SUPPORT */
		} else if (pTxBlk->TxFrameType == TX_FRAG_FRAME)
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAllowFrag);

		if (!pMacEntry)
			MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
				"Err!! pMacEntry is NULL!!\n");
		else {
			pMacEntry->DebugTxCount++;
#ifdef MAC_REPEATER_SUPPORT
			if (IS_REPT_LINK_UP(pMacEntry->pReptCli))
				pMacEntry->pReptCli->ReptCliIdleCount = 0;

#endif
		}
	}

	pAd->LastTxRate = (USHORT)pTxBlk->pTransmit->word;
	ops->find_cipher_algorithm(pAd, wdev, pTxBlk);
	return TRUE;
}

BOOLEAN apcli_fill_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk)
{
	PNDIS_PACKET pPacket;
#if defined(MAT_SUPPORT) || defined(MAC_REPEATER_SUPPORT)
	UCHAR *pMacAddr = NULL;
#endif	/* MAT_SUPPORT || MAC_REPEATER_SUPPORT */
	PMAC_TABLE_ENTRY pMacEntry = NULL;
#ifdef MAT_SUPPORT
	PUCHAR pSrcBufVA = NULL;
	PNDIS_PACKET convertPkt = NULL;
	struct _QUEUE_HEADER TxPacketList = {NULL, NULL, 0}, *pTxPacketTmpList = &TxPacketList;
	struct _QUEUE_ENTRY *q_entry = pTxBlk->TxPacketList.Head;
	u32 i = 0;
#endif
	pPacket = pTxBlk->pPacket;
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
	pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);

	if (RTMP_GET_PACKET_MGMT_PKT(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_CT_WithTxD);

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);
	/* To make sure eapol frames go to setup link */
	if (RTMP_GET_PACKET_EAPOL(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_ForceLink);

	if (IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) {
		if ((pTxBlk->TxFrameType == TX_LEGACY_FRAME) ||
			(pTxBlk->TxFrameType == TX_AMSDU_FRAME) ||
			(pTxBlk->TxFrameType == TX_BMC_FRAME))
			TX_BLK_SET_FLAG(pTxBlk, fTX_HDR_TRANS);
	} else {
		struct wifi_dev_ops *ops = wdev->wdev_ops;

		/* only MAC TXD mode if not HW TX HDR_TRANS */
		TX_BLK_SET_FLAG(pTxBlk, fTX_CT_WithTxD);
		ops->find_cipher_algorithm(pAd, wdev, pTxBlk);
	}

	if IS_WCID_VALID(pAd, pTxBlk->Wcid)
		pMacEntry = entry_get(pAd, pTxBlk->Wcid);
	else
		goto err;
#ifdef A4_CONN
	if (IS_ENTRY_A4(pMacEntry)) {
		pTxBlk->pMacEntry = pMacEntry;
		pTxBlk->pApCliEntry = GetStaCfgByWdev(pAd, pMacEntry->wdev);
		TX_BLK_SET_FLAG(pTxBlk, fTX_bA4Frame);
	} else
#endif /* A4_CONN */

#ifdef APCLI_AS_WDS_STA_SUPPORT
	if (pAd->StaCfg[pMacEntry->func_tb_idx].wdev.wds_enable == 1) {
		pTxBlk->pMacEntry = pMacEntry;
		pTxBlk->pApCliEntry = GetStaCfgByWdev(pAd, pMacEntry->wdev);
	} else
#endif

	if ((IS_ENTRY_PEER_AP(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry)) &&
		((pTxBlk->TxFrameType != TX_BMC_FRAME) &&
		 (pTxBlk->TxFrameType != TX_MLME_DATAQ_FRAME) &&
		 (pTxBlk->TxFrameType != TX_MLME_MGMTQ_FRAME))) {

#ifdef MAT_SUPPORT
		/* Get TX pkt list from TxBlk and insert back after MAT */
		q_entry = pTxBlk->TxPacketList.Head;

		for (i = 0; i < pTxBlk->TxPacketList.Number && q_entry; i++) {
			pPacket = QUEUE_ENTRY_TO_PACKET(q_entry);
#ifdef MAC_REPEATER_SUPPORT
			if (IS_ENTRY_REPEATER(pMacEntry) &&
				IS_REPT_LINK_UP(pMacEntry->pReptCli) &&
				(pAd->ApCfg.bMACRepeaterEn)) {

				pAd->MatCfg.bMACRepeaterEn = pAd->ApCfg.bMACRepeaterEn;

				if (pAd->ApCfg.MACRepeaterOuiMode != CASUALLY_DEFINE_MAC_ADDR) {
					convertPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, pMacEntry->pReptCli->CliIdx, pMacEntry->EntryType);
					pMacAddr = pMacEntry->pReptCli->CurrentAddress;
				}
			} else
#endif /* MAC_REPEATER_SUPPORT */
			if (IS_ENTRY_PEER_AP(pMacEntry)) {
#ifdef DOT11_EHT_BE
				MAC_TABLE_ENTRY *peer_entry = NULL;
#endif
				if (pMacEntry->func_tb_idx >= pAd->ApCfg.ApCliNum)
					goto err;

				/* For each tx packet, update our MAT convert engine databases.*/
				convertPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, pMacEntry->func_tb_idx, pMacEntry->EntryType);

#ifdef DOT11_EHT_BE
				/* replace with Mld address if mlo enabled */
				peer_entry = GetAssociatedAPByWdev(pAd, wdev);
				if (!RTMP_GET_PACKET_EAPOL(pPacket) &&
					peer_entry && peer_entry->mlo.mlo_en && wdev->mld_dev)
					pMacAddr = wdev->mld_dev->mld_addr;
				else
#endif
					pMacAddr = &pAd->StaCfg[pMacEntry->func_tb_idx].wdev.if_addr[0];
			} else {
				goto err;
			}

			if (convertPkt == NULL && OS_PKT_CLONED(pPacket)) {
				OS_PKT_COPY(pPacket, convertPkt);

				if (convertPkt == NULL) {
					MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
						"convertPkt is NULL!!\n");
					goto err;
				}
			}

			if (convertPkt) {
				/* Get next packet for processing */
				q_entry = QUEUE_GET_NEXT_ENTRY(q_entry);

				/* Insert new packet to temp list */
				InsertTailQueue(pTxPacketTmpList, PACKET_TO_QUEUE_ENTRY(convertPkt));

				/* Free old packet */
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);

				pPacket = convertPkt;

				if (i == 0) {
					/* Assign first pkt's information to TxBlk */
					pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
					pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);
					pTxBlk->pPacket = convertPkt;
				}
			}

			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
				if ((pMacAddr != NULL)
				   ) {
					if (pMacAddr) {
						pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);
						NdisMoveMemory(pSrcBufVA + 6, pMacAddr, MAC_ADDR_LEN);
					}
				}
			}
		} /* End of for loop */

		if (pTxPacketTmpList->Number) {
		    /* refill Pkt tmp list to TxBlk's Pkt list */
			pTxBlk->TxPacketList.Head = pTxPacketTmpList->Head;
			pTxBlk->TxPacketList.Tail = pTxPacketTmpList->Tail;
			pTxBlk->TxPacketList.Number = pTxPacketTmpList->Number;
			pTxBlk->TxPacketList.state = pTxPacketTmpList->state;
		}

#endif /* MAT_SUPPORT */
		pTxBlk->pApCliEntry = GetStaCfgByWdev(pAd, pMacEntry->wdev);

#ifndef MWDS
		pTxBlk->pMacEntry = pMacEntry;
#endif
		TX_BLK_SET_FLAG(pTxBlk, fTX_bApCliPacket);
#ifdef MAC_REPEATER_SUPPORT
		if (IS_REPT_LINK_UP(pMacEntry->pReptCli))
			pMacEntry->pReptCli->ReptCliIdleCount = 0;

#endif
	}

	if (RTMP_TEST_FLAG(pAd->physical_dev, PH_DEV_CHECK_ETH_TYPE))
		pTxBlk->UserPriority = RTMP_GET_PACKET_UP(pPacket);
	pTxBlk->wmm_set = HcGetWmmIdx(pAd, wdev);
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
	return TRUE;

err:
	/* Error case, free temp list */
	while (pTxPacketTmpList->Head) {
		q_entry = RemoveHeadQueue(pTxPacketTmpList);
		RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
	}

	return FALSE;
}
/*
    ==========================================================================
    Description:
	APCLI Interface Up.
    ==========================================================================
 */
VOID ApCliIfUp(RTMP_ADAPTER *pAd)
{
	UCHAR ifIndex;
	STA_ADMIN_CONFIG *pApCliEntry;
#ifdef APCLI_CONNECTION_TRIAL
	PULONG pCurrState = NULL;
#endif /* APCLI_CONNECTION_TRIAL */
	struct DOT11_H *pDot11h = NULL;

	/* Reset is in progress, stop immediately */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_RADIO_OFF) ||
		(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)))
		return;

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++) {
		pApCliEntry = &pAd->StaCfg[ifIndex];

		if (pApCliEntry->wdev.radio_off_req)
			continue;

		/* sanity check whether the interface is initialized. */
		if (pApCliEntry->ApcliInfStat.ApCliInit != TRUE)
			continue;

		if (pApCliEntry->wdev.if_up_down_state == FALSE)
			continue;

#ifdef APCLI_CONNECTION_TRIAL
		pCurrState = &pAd->StaCfg[ifIndex].wdev.cntl_machine.CurrState;
#endif /* APCLI_CONNECTION_TRIAL */

		if (!pApCliEntry->wdev.DevInfo.WdevActive)
			continue;

		if (!HcIsRadioAcq(&pApCliEntry->wdev))
			continue;

		if (APCLI_IF_UP_CHECK(pAd, ifIndex)
			&& (pApCliEntry->ApcliInfStat.Enable == TRUE)
			&& (pApCliEntry->ApcliInfStat.Valid == FALSE)
#ifdef APCLI_CONNECTION_TRIAL
			&& (ifIndex != (pAd->ApCfg.ApCliNum - 1)) /* last IF is for apcli connection trial */
#endif /* APCLI_CONNECTION_TRIAL */
		   ) {
			pDot11h = pApCliEntry->wdev.pDot11_H;

			if (pDot11h == NULL)
				return;

			if (pDot11h->ChannelMode == CHAN_SWITCHING_MODE) {
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_INFO,
								"Switching mode: Continue\n");
				continue;
			}
			if (IS_DOT11_H_RADAR_STATE(pAd, CHAN_SILENCE_MODE, pApCliEntry->wdev.channel, pDot11h)) {
#ifdef DFS_SLAVE_SUPPORT
				if (SLAVE_MODE_EN(pAd) && slave_rdd_op(pAd, &pApCliEntry->wdev, flag_check)) {
					MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
							 "[SLAVE-DFS]radar detected prev\n");
					continue;
				}
#endif /* DFS_SLAVE_SUPPORT.*/
				if (pApCliEntry->ApcliInfStat.bPeerExist == TRUE) {
					/* Got peer's beacon; change to normal mode */
					pDot11h->RDCount = pDot11h->cac_time;
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_INFO,
							 "ApCliIfUp - PeerExist\n");
				} else
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_INFO,
							 "ApCliIfUp - Stop probing while Radar state is silent\n");

				continue;
			}

#ifdef WSC_INCLUDED
			if (pApCliEntry->wdev.WscControl.bWscTrigger
				&& ((pApCliEntry->wdev.WscControl.WscStatus == STATUS_WSC_SCAN_AP) ||
					(pApCliEntry->wdev.WscControl.WscStatus == STATUS_WSC_PBC_NO_AP)))
				continue;
#endif /* WSC_INCLUDED */
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_INFO,
				"ApCli interface[%d] startup.\n", ifIndex);
#ifdef WSC_INCLUDED
			if ((pApCliEntry->wdev.WscControl.bWscTrigger) &&
				(pApCliEntry->wdev.WscControl.WscStatus == STATUS_WSC_START_ASSOC)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_INFO,
					"Enqueue connect by BSSID for WPS\n");
				cntl_connect_request(&pApCliEntry->wdev, CNTL_CONNECT_BY_BSSID, MAC_ADDR_LEN,
								(UCHAR *)&pApCliEntry->wdev.WscControl.WscBssid[0]);
			} else
#endif /* WSC_INCLUDED */
			cntl_connect_request(&pApCliEntry->wdev, CNTL_CONNECT_BY_CFG, 0, NULL);
			/* MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ, 0, NULL, ifIndex); */
			/* Reset bPeerExist each time in case we could keep old status */
			pApCliEntry->ApcliInfStat.bPeerExist = FALSE;
		}

#ifdef APCLI_CONNECTION_TRIAL
		else if (
			APCLI_IF_UP_CHECK(pAd, ifIndex)
			&& (*pCurrState == APCLI_CTRL_DISCONNECTED)/* Apcli1 is not connected state. */
			&& (pApCliEntry->TrialCh != 0)
			/* && NdisCmpMemory(pApCliEntry->ApCliMlmeAux.Ssid, pApCliEntry->CfgSsid, pApCliEntry->SsidLen) != 0 */
			&& (pApCliEntry->CfgSsidLen != 0)
			&& (pApCliEntry->ApcliInfStat.Enable != 0)
			/* new ap ssid shall different from the origin one. */
		) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_INFO,
				"Enqueue APCLI_CTRL_TRIAL_CONNECT\n");
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_TRIAL_CONNECT, 0, NULL, ifIndex);
			/* Reset bPeerExist each time in case we could keep old status */
			pApCliEntry->ApcliInfStat.bPeerExist = FALSE;
		}

#endif /* APCLI_CONNECTION_TRIAL */
	}

	return;
}

/*
    ==========================================================================
    Description:
	Up one specific APCLI Interface.
    ==========================================================================
 */
VOID upSpecificApCliIf(RTMP_ADAPTER *pAd, UCHAR staidx)
{
	STA_ADMIN_CONFIG *pApCliEntry;
#ifdef APCLI_CONNECTION_TRIAL
	PULONG pCurrState = NULL;
#endif /* APCLI_CONNECTION_TRIAL */
	struct DOT11_H *pDot11h = NULL;
#ifdef DFS_SLAVE_SUPPORT
	ULONG bss_idx = 0;
	BSS_TABLE *ScanTab = NULL;
#endif

	/* Reset is in progress, stop immediately */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_RADIO_OFF) ||
		(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)))
		return;

	{
		pApCliEntry = &pAd->StaCfg[staidx];

		if (pApCliEntry->wdev.radio_off_req)
			return;

		/* sanity check whether the interface is initialized. */
		if (pApCliEntry->ApcliInfStat.ApCliInit != TRUE)
			return;

		if (pApCliEntry->wdev.if_up_down_state == FALSE)
			return;

#ifdef APCLI_CONNECTION_TRIAL
		pCurrState = &pAd->StaCfg[staidx].wdev.cntl_machine.CurrState;
#endif /* APCLI_CONNECTION_TRIAL */

		if (!pApCliEntry->wdev.DevInfo.WdevActive)
			return;

		if (!HcIsRadioAcq(&pApCliEntry->wdev))
			return;

#ifdef DFS_SLAVE_SUPPORT
		if (SLAVE_MODE_EN(pAd)) {
			ScanTab = get_scan_tab_by_wdev(pAd, &pApCliEntry->wdev);
			bss_idx = BssSsidTableSearchBySSID(ScanTab, (PCHAR)pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen);

			if (bss_idx < MAX_LEN_OF_BSS_TABLE) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
					"find SSID[%ld][%s] channel[%d-%d] in ScanTab.\n",
					bss_idx, pApCliEntry->CfgSsid,
					ScanTab->BssEntry[bss_idx].Channel,
					ScanTab->BssEntry[bss_idx].CentralChannel);
				pApCliEntry->ApcliInfStat.bPeerExist = TRUE;
			/* BssSearch Table has found the pEntry, send Prob Req. directly */
				if (pApCliEntry->wdev.channel != ScanTab->BssEntry[bss_idx].Channel) {
					pApCliEntry->MlmeAux.Channel = ScanTab->BssEntry[bss_idx].Channel;
					if (rtmp_set_channel(pAd, &pApCliEntry->wdev, pApCliEntry->MlmeAux.Channel) == FALSE) {
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
							"Channel switch Fail, Return!!!\n");
						return;
					}
					pApCliEntry->wdev.channel = pApCliEntry->MlmeAux.Channel;
				}
			}

			if (pApCliEntry->ApcliInfStat.bPeerExist == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
						"No PeerExit return!!!\n");
				return;
			}
		}
#endif

		if (APCLI_IF_UP_CHECK(pAd, staidx)
			&& (pApCliEntry->ApcliInfStat.Enable == TRUE)
			&& (pApCliEntry->ApcliInfStat.Valid == FALSE)
#ifdef APCLI_CONNECTION_TRIAL
			/* last IF is for apcli connection trial */
			&& (staidx != (pAd->ApCfg.ApCliNum - 1))
#endif /* APCLI_CONNECTION_TRIAL */
		   ) {
			pDot11h = pApCliEntry->wdev.pDot11_H;

			if (pDot11h == NULL)
				return;

			if (pDot11h->ChannelMode == CHAN_SWITCHING_MODE) {
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_INFO,
								"Switching mode: Continue\n");
				return;
			}

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
					"channel[%d] channelmode:%d\n", pApCliEntry->wdev.channel, pDot11h->ChannelMode);

			if (IS_DOT11_H_RADAR_STATE(pAd, CHAN_SILENCE_MODE, pApCliEntry->wdev.channel, pDot11h)) {
#ifdef DFS_SLAVE_SUPPORT
				if (SLAVE_MODE_EN(pAd) && slave_rdd_op(pAd, &pApCliEntry->wdev, flag_check)) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
							 "[SLAVE-DFS]radar detected prev\n");
					return;
				}
#endif /* DFS_SLAVE_SUPPORT.*/
				if (pApCliEntry->ApcliInfStat.bPeerExist == TRUE) {
					/* Got peer's beacon; change to normal mode */
					pDot11h->RDCount = pDot11h->cac_time;
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_INFO,
							 "ApCliIfUp - PeerExist\n");
				} else
					MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_INFO,
							 "ApCliIfUp - Stop probing while Radar state is silent\n");
#ifdef DFS_SLAVE_SUPPORT
				if (!SLAVE_MODE_EN(pAd))
#endif /* DFS_SLAVE_SUPPORT */
					return;
			}

#ifdef WSC_INCLUDED
			if (pApCliEntry->wdev.WscControl.bWscTrigger
				&& ((pApCliEntry->wdev.WscControl.WscStatus == STATUS_WSC_SCAN_AP) ||
					(pApCliEntry->wdev.WscControl.WscStatus == STATUS_WSC_PBC_NO_AP)))
				return;
#endif /* WSC_INCLUDED */
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_INFO,
				"ApCli interface[%d] startup.\n", staidx);
#ifdef WSC_INCLUDED
			if ((pApCliEntry->wdev.WscControl.bWscTrigger) &&
				(pApCliEntry->wdev.WscControl.WscStatus == STATUS_WSC_START_ASSOC)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_INTF,
					DBG_LVL_INFO, " Enqueue connect by BSSID for WPS\n");
				cntl_connect_request(&pApCliEntry->wdev, CNTL_CONNECT_BY_BSSID, MAC_ADDR_LEN,
								(UCHAR *)&pApCliEntry->wdev.WscControl.WscBssid[0]);
			} else
#endif /* WSC_INCLUDED */
				cntl_connect_request(&pApCliEntry->wdev, CNTL_CONNECT_BY_CFG, 0, NULL);
			pApCliEntry->ApcliInfStat.bPeerExist = FALSE;
		}

#ifdef APCLI_CONNECTION_TRIAL
		else if (
			APCLI_IF_UP_CHECK(pAd, staidx)
			&& (*pCurrState == APCLI_CTRL_DISCONNECTED)
			&& (pApCliEntry->TrialCh != 0)
			&& (pApCliEntry->CfgSsidLen != 0)
			&& (pApCliEntry->ApcliInfStat.Enable != 0)
			/* new ap ssid shall different from the origin one. */
		) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_INFO,
					"Enqueue APCLI_CTRL_TRIAL_CONNECT\n");
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_TRIAL_CONNECT, 0, NULL, staidx);
			/* Reset bPeerExist each time in case we could keep old status */
			pApCliEntry->ApcliInfStat.bPeerExist = FALSE;
		}

#endif /* APCLI_CONNECTION_TRIAL */
	}
}


/*
    ==========================================================================
    Description:
	APCLI Interface Down.
    ==========================================================================
 */
VOID ApCliIfDown(RTMP_ADAPTER *pAd)
{
	UCHAR ifIndex;
	PSTA_ADMIN_CONFIG pApCliEntry;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR idx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */

	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
		"caller:%pS", OS_TRACE);

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++) {
		pApCliEntry = &pAd->StaCfg[ifIndex];

		if (pApCliEntry->ApcliInfStat.Enable == TRUE)
			continue;

		if (pApCliEntry->ApcliInfStat.Valid == FALSE)
			continue;

		DoActionBeforeDownIntf(pAd, &pApCliEntry->wdev);

		if (pApCliEntry->PwrMgmt.bDoze) {
			RTMP_FORCE_WAKEUP(pAd, pApCliEntry);
			pApCliEntry->WindowsPowerMode = Ndis802_11PowerModeCAM;
		}

#ifdef APCLI_CFG80211_SUPPORT
		if (!STA_STATUS_TEST_FLAG(pApCliEntry, fSTA_STATUS_MEDIA_STATE_CONNECTED))
			LinkDown(pAd, 0, &pApCliEntry->wdev, NULL);
#endif
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_NOTICE, "ApCli interface[%d] start down.\n", ifIndex);
#ifdef MAC_REPEATER_SUPPORT
		repeater_disconnect_by_band(pAd, HcGetBandByWdev(&pApCliEntry->wdev));
#endif /* MAC_REPEATER_SUPPORT */
		RTMP_OS_INIT_COMPLETION(&pApCliEntry->linkdown_complete);
		pApCliEntry->ApcliInfStat.Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCLI_IF_DOWN;
		sta_deauth_act(&pApCliEntry->wdev);
		sta_wait_link_down(pApCliEntry);
		cntl_reset_all_fsm_in_ifdown(&pApCliEntry->wdev);

		DoActionAfterDownIntf(pAd, &pApCliEntry->wdev);
	}

#ifdef MAC_REPEATER_SUPPORT

	for (idx = 0; idx < MAX_IGNORE_AS_REPEATER_ENTRY_NUM; idx++) {
		pEntry = &pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntry[idx];

		if (pAd->ApCfg.ApCliInfRunned == 0)
			RepeaterRemoveIngoreEntry(pAd, idx, pEntry->MacAddr);
	}

#endif /* MAC_REPEATER_SUPPORT */
	return;
}

#ifdef DOT11_VHT_AC
UCHAR check_vht_op_bw(struct vht_opinfo *vht_op_info,
			VHT_CAP_IE *vht_cap_ie, ADD_HTINFO2 *add_ht_info);
#endif
#ifdef APCLI_AUTO_CONNECT_SUPPORT
#ifdef APCLI_AUTO_BW_TMP /* should be removed after apcli auto-bw is applied */
UCHAR ApCliAutoConnectBWAdjust(
	IN RTMP_ADAPTER	*pAd,
	IN struct wifi_dev	*wdev,
	IN BSS_ENTRY *bss_entry)
{
	BOOLEAN bAdjust = FALSE;
	BOOLEAN bAdjust_by_channel = FALSE;
	BOOLEAN bAdjust_by_ht = FALSE;
	BOOLEAN bAdjust_by_vht = FALSE;
	UCHAR	orig_op_ht_bw;
#ifdef DOT11_VHT_AC
	UCHAR	orig_op_vht_bw;
#endif
	UCHAR	orig_ext_cha;
	STA_ADMIN_CONFIG *pStaCfg = NULL;
	struct _RTMP_CHIP_CAP *cap = NULL;

	if (pAd == NULL || wdev == NULL || bss_entry == NULL) {
		ASSERT(pAd);
		ASSERT(wdev);
		ASSERT(bss_entry);
		return AUTO_BW_PARAM_ERROR;
	}

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	ASSERT(pStaCfg);
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_INFO,
			 "BW info of root AP (%s):\n", bss_entry->Ssid);
	orig_op_ht_bw = wlan_operate_get_ht_bw(wdev);
#ifdef DOT11_VHT_AC
	orig_op_vht_bw = wlan_operate_get_vht_bw(wdev);
#endif /*DOT11_VHT_AC*/
	orig_ext_cha = wlan_operate_get_ext_cha(wdev);

	if ((wdev->channel != bss_entry->Channel)) {
		bAdjust = TRUE;
		bAdjust_by_channel = TRUE;
	}

#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode) && HAS_HT_OP_EXIST(bss_entry->ie_exists)) {
		ADD_HTINFO *add_ht_info = &bss_entry->AddHtInfo.AddHtInfo;
		UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
		UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
		UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		UCHAR soft_ap_bw = wlan_operate_get_bw(&pAd->ApCfg.MBSSID[0].wdev);
#endif
		if (!bAdjust &&
			((ext_cha != add_ht_info->ExtChanOffset) ||
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		/* Soft AP BW : Sync Required */
			(soft_ap_bw != add_ht_info->RecomWidth) ||
#endif
			(op_ht_bw != add_ht_info->RecomWidth)))
				bAdjust = TRUE;

		if (bAdjust) {
		switch (add_ht_info->RecomWidth) { /* peer side vht bw */
			case BW_20:
				if (op_ht_bw == BW_40) {
					wlan_operate_set_ht_bw(wdev, add_ht_info->RecomWidth, EXTCHA_NONE);
					bAdjust_by_ht = TRUE;
#ifdef BW_VENDOR10_CUSTOM_FEATURE
					/* Sync new BW & Ext Channel for Soft AP */
					if (IS_SYNC_BW_POLICY_VALID(pAd, TRUE, HT_4020_DOWN_ENBL)) {
						MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_INFO,
							"Enter 4020 HT Sync\n");
						wdev_sync_ht_bw(pAd, wdev, add_ht_info);
					}
#endif
				}

				break;
			case BW_40:
#ifdef BW_VENDOR10_CUSTOM_FEATURE
				if (op_ht_bw == BW_20 || (soft_ap_bw == BW_20)) {
#else
				if (op_ht_bw == BW_20) {
#endif
#ifdef BT_APCLI_SUPPORT
					if (pAd->ApCfg.ApCliAutoBWBTSupport == TRUE) {
						/*set to config extension channel/bw to let ap use new configuration*/
						UCHAR mbss_idx = 0;
						/*Moving both AP and CLI to 40Mhz since RootAP is working in 40Mhz */
						for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
							struct wifi_dev *mbss_wdev;

							mbss_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;

							if (HcGetBandByWdev(mbss_wdev) ==
									HcGetBandByWdev(wdev)) {
								wlan_config_set_ht_bw(mbss_wdev,
										add_ht_info->RecomWidth);
								wlan_config_set_ext_cha(mbss_wdev,
										add_ht_info->ExtChanOffset);
							}
						}
						/*set Config BW of CLI to 40Mhz*/
						wlan_config_set_ht_bw(wdev, add_ht_info->RecomWidth);
						wlan_operate_set_ht_bw(wdev, add_ht_info->RecomWidth,
								add_ht_info->ExtChanOffset);
						wlan_config_set_ext_cha(wdev, add_ht_info->ExtChanOffset);
						bAdjust_by_ht = TRUE;
					}
#ifdef BW_VENDOR10_CUSTOM_FEATURE
					if (cfg_ht_bw == BW_40 && IS_SYNC_BW_POLICY_VALID(pAd, TRUE, HT_2040_UP_ENBL)) {
						MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_INFO,
							"Enter 2040 HT Sync\n");
						/*set Config BW of CLI to 40Mhz*/
						bAdjust_by_ht = TRUE;
					}
#endif
#endif
				} else {
					if (cfg_ht_bw == BW_40) {

						UCHAR mbss_idx = 0;
#ifdef BW_VENDOR10_CUSTOM_FEATURE
						if (IS_SYNC_BW_POLICY_VALID(pAd, TRUE, HT_2040_UP_ENBL) == FALSE)
							break;
#endif
						for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
							struct wifi_dev *mbss_wdev;
							mbss_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
							if (HcGetBandByWdev(mbss_wdev) == HcGetBandByWdev(wdev)) {
								wlan_config_set_ext_cha(mbss_wdev, add_ht_info->ExtChanOffset);
							}
						}
					wlan_config_set_ext_cha(wdev, add_ht_info->ExtChanOffset);
					bAdjust_by_ht = TRUE;

					}
				}
#ifdef BW_VENDOR10_CUSTOM_FEATURE
				if (bAdjust_by_ht && IS_SYNC_BW_POLICY_VALID(pAd, TRUE, HT_2040_UP_ENBL)) {
					/* Soft AP Op BW 20 M / Root AP Link Up when Soft AP is Down */
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_INFO,
						"Enter 2040 HT Sync\n");

					/*set Config BW of CLI to 40Mhz*/
					wlan_config_set_ht_bw(wdev, add_ht_info->RecomWidth);
					wlan_operate_set_ht_bw(wdev, add_ht_info->RecomWidth,
						add_ht_info->ExtChanOffset);
					wlan_config_set_ext_cha(wdev, add_ht_info->ExtChanOffset);

					wdev_sync_ht_bw(pAd, wdev, add_ht_info);
				}
#endif

				break;
			}
		}

	}

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode) && IS_CAP_BW160(cap) &&
		HAS_VHT_CAPS_EXIST(bss_entry->ie_exists) &&
		HAS_VHT_OP_EXIST(bss_entry->ie_exists)) {
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		BOOLEAN bDown80_2040 = FALSE, bDown160_80 = FALSE;
#endif
		BOOLEAN bResetVHTBw = FALSE, bDownBW = FALSE;
		UCHAR bw = VHT_BW_2040;
		struct vht_opinfo *vht_op = &bss_entry->vht_op_ie.vht_op_info;
		ADD_HTINFO2 *add_ht_info = &bss_entry->AddHtInfo.AddHtInfo2;
		UCHAR op_vht_bw = wlan_operate_get_vht_bw(wdev);
		UCHAR cfg_vht_bw = wlan_config_get_vht_bw(wdev);
		CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
		bw = check_vht_op_bw(vht_op, &bss_entry->vht_cap_ie, add_ht_info);

		if (!bAdjust &&
			(bw != op_vht_bw))
			bAdjust = TRUE;

		if (bAdjust) {
			switch (bw) {/* peer side vht bw */
			case VHT_BW_2040:
				if (cfg_vht_bw > VHT_BW_2040) {
					bResetVHTBw = TRUE;
#ifdef BW_VENDOR10_CUSTOM_FEATURE
				if (op_vht_bw == VHT_BW_80 && IS_SYNC_BW_POLICY_VALID(pAd, FALSE, VHT_80_2040_DOWN_CHK)) {
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_INFO,
						"Enter 802040 HT Sync\n");
					bDown80_2040 = TRUE;
				}
#endif
					bDownBW = TRUE;
					bAdjust_by_vht = TRUE;
				}

				break;

			case VHT_BW_80:
				if (cfg_vht_bw > VHT_BW_80) {
					bResetVHTBw = TRUE;
					bDownBW = TRUE;
					bAdjust_by_vht = TRUE;
#ifdef BW_VENDOR10_CUSTOM_FEATURE
				if (op_vht_bw == VHT_BW_160 && IS_SYNC_BW_POLICY_VALID(pAd, FALSE, VHT_160_80_DOWN_CHK))
					bDown160_80 = TRUE;
#endif
				}

				break;

			case VHT_BW_160:
				if (cfg_vht_bw == VHT_BW_160) {
					bAdjust_by_vht = TRUE;
					bResetVHTBw = 1;
				}

				break;

			case VHT_BW_8080:
				if (cfg_vht_bw == VHT_BW_8080) {
					wlan_operate_set_cen_ch_2(wdev, vht_op->ccfs_1);
					bResetVHTBw = 1;
					bAdjust_by_vht = TRUE;
				}

				break;
			}
		}

		if (bResetVHTBw) {
			INT Idx;
			BOOLEAN bMatch = FALSE;

			for (Idx = 0; Idx < pChCtrl->ChListNum; Idx++) {
				if (bss_entry->Channel == pChCtrl->ChList[Idx].Channel) {
					bMatch = TRUE;
					break;
				}
			}

			if (bMatch && (Idx < MAX_NUM_OF_CHANNELS)) {
				if (bDownBW) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_INFO,
							 " Follow BW info of root AP (%s) from vht_bw = %d to %d. (MAX=%d)\n",
							  bss_entry->Ssid,
							  op_vht_bw, bw,
							  cfg_vht_bw);
					wlan_operate_set_vht_bw(wdev, bw);
				} else if (!bDownBW && (pChCtrl->ChList[Idx].Flags & CHANNEL_80M_CAP))
					wlan_operate_set_vht_bw(wdev, cfg_vht_bw);

				wlan_operate_set_cen_ch_2(wdev, vht_op->ccfs_1);
#ifdef BW_VENDOR10_CUSTOM_FEATURE
				/* Sync new BW & Central Channel for Soft AP */
				if (bDown80_2040 || bDown160_80)
					wdev_sync_vht_bw(pAd, wdev,
						((bDownBW) ? (bw) : ((bw >= VHT_BW_160) ? (VHT_BW_160) : (cfg_vht_bw))), vht_op->ccfs_1);
#endif
			}
		}
	}

#endif /* DOT11_VHT_AC */
	bAdjust = FALSE;

	if (bAdjust_by_channel == TRUE)
		bAdjust = TRUE;

	if (bAdjust_by_ht == TRUE)
		bAdjust = TRUE;

	if (bAdjust_by_vht == TRUE)
		bAdjust = TRUE;

	if (bAdjust) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_INFO,
				"Adjust (%d %d %d)\n\r",
				 bAdjust_by_channel, bAdjust_by_ht, bAdjust_by_vht);
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_INFO,
				"HT BW:%d to %d. MAX(%d)\n\r",
				 orig_op_ht_bw, wlan_operate_get_ht_bw(wdev), wlan_config_get_ht_bw(wdev));
#ifdef DOT11_VHT_AC
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_INFO,
				"VHT BW:%d to %d. MAX(%d)\n\r",
				 orig_op_vht_bw, wlan_operate_get_vht_bw(wdev), wlan_config_get_vht_bw(wdev));
#endif /*DOT11_VHT_AC*/
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_INFO,
				"EXT CH:%d to %d\n\r",
				 orig_ext_cha, wlan_operate_get_ext_cha(wdev));
		return AUTO_BW_NEED_TO_ADJUST;
	}

	return AUTO_BW_NO_NEED_TO_ADJUST;
}
#endif /* APCLI_AUTO_BW_TMP */
/*
	===================================================

	Description:
		Find the AP that is configured in the ApcliTab, and switch to
		the channel of that AP

	Arguments:
		pAd: pointer to our adapter

	Return Value:
		TRUE: no error occured
		FALSE: otherwise

	Note:
	===================================================
*/
BOOLEAN ApCliAutoConnectExec(
	IN  PRTMP_ADAPTER   pAd,
	IN struct wifi_dev *wdev)
{
	UCHAR			ifIdx, CfgSsidLen;
	UINT entryIdx;
	RTMP_STRING *pCfgSsid;
	BSS_TABLE		*pScanTab, *pSsidBssTab;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	ASSERT(pStaCfg);
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
		"---> ()\n");

	if (wdev)
		ifIdx = wdev->func_idx;
	else
		return FALSE;

	if (ifIdx >= MAX_APCLI_NUM) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR, "Error  ifIdx=%d\n", ifIdx);
		return FALSE;
	}

	if (pStaCfg->ApcliInfStat.AutoConnectFlag != TRUE)
		return FALSE;

	CfgSsidLen = pStaCfg->CfgSsidLen;
	pCfgSsid = pStaCfg->CfgSsid;
	pScanTab = get_scan_tab_by_wdev(pAd, wdev);
	pSsidBssTab = &pStaCfg->MlmeAux.SsidBssTab;
	pSsidBssTab->BssNr = 0;

	/*
		Find out APs with the desired SSID.
	*/
	for (entryIdx = 0; entryIdx < pScanTab->BssNr; entryIdx++) {
		BSS_ENTRY *pBssEntry = &pScanTab->BssEntry[entryIdx];

		if (pBssEntry->Channel == 0)
			break;

		if (NdisEqualMemory(pCfgSsid, pBssEntry->Ssid, CfgSsidLen) &&
			pBssEntry->SsidLen &&
			(pBssEntry->SsidLen == CfgSsidLen) &&
			(pSsidBssTab->BssNr < MAX_LEN_OF_BSS_TABLE)) {
			if (((((wdev->SecConfig.AKMMap & pBssEntry->AKMMap) != 0) ||
				(IS_AKM_AUTOSWITCH(wdev->SecConfig.AKMMap) && IS_AKM_SHARED(pBssEntry->AKMMap)) ||
				(IS_AKM_WPA2PSK_ONLY(pBssEntry->AKMMap) && IS_AKM_WPA3PSK_ONLY(wdev->SecConfig.AKMMap)))
				 && ((wdev->SecConfig.PairwiseCipher & pBssEntry->PairwiseCipher) != 0))
#ifdef CONFIG_OWE_SUPPORT
					|| (!pBssEntry->hide_open_owe_bss
						&& (IS_AKM_OPEN_ONLY(pBssEntry->AKMMap) && IS_CIPHER_NONE(pBssEntry->PairwiseCipher))
						&& (IS_AKM_OWE(wdev->SecConfig.AKMMap)))
#endif
			   ) {
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
						 "Found desired ssid in Entry %2d:\n", entryIdx);
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
						 "I/F(apcli%d) ApCliAutoConnectExec:(Len=%d,Ssid=%s, Channel=%d, Rssi=%d)\n",
						  ifIdx, pBssEntry->SsidLen, pBssEntry->Ssid,
						  pBssEntry->Channel, pBssEntry->Rssi);
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
						 "I/F(apcli%d) ApCliAutoConnectExec::(AuthMode=%s, EncrypType=%s)\n", ifIdx,
						  GetAuthMode(pBssEntry->AuthMode),
						  GetEncryptType(pBssEntry->WepStatus));
				/* fix memory leak when trigger scan continuously */
				BssEntryCopy(pSsidBssTab,
					&pSsidBssTab->BssEntry[pSsidBssTab->BssNr++], pBssEntry);
			}
		}
	}
	if (pSsidBssTab->BssNr < MAX_LEN_OF_BSS_TABLE) {
		/* fix memory leak when trigger scan continuously */
		BssEntryReset(pSsidBssTab, &pSsidBssTab->BssEntry[pSsidBssTab->BssNr]);
	}
	/*
		Sort by Rssi in the increasing order, and connect to
		the last entry (strongest Rssi)
	*/
	BssTableSortByRssi(pSsidBssTab, TRUE);

	if ((pSsidBssTab->BssNr == 0)) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "No match entry.\n");
		pStaCfg->ApCliAutoConnectRunning = FALSE;
	} else if (pSsidBssTab->BssNr > 0 &&
			   pSsidBssTab->BssNr <= MAX_LEN_OF_BSS_TABLE) {
		/*
			Switch to the channel of the candidate AP
		*/
		BSS_ENTRY *pBssEntry = &pSsidBssTab->BssEntry[pSsidBssTab->BssNr - 1];
#ifdef APCLI_AUTO_BW_TMP /* should be removed after apcli auto-bw is applied */
		BOOLEAN bw_adj;

		bw_adj = ApCliAutoConnectBWAdjust(pAd, wdev, pBssEntry);

		if (bw_adj == AUTO_BW_NEED_TO_ADJUST || (!IS_INVALID_HT_SECURITY(pBssEntry->PairwiseCipher)))
#endif /* APCLI_AUTO_BW_TMP */
		{
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "Switch to channel :%d\n", pBssEntry->Channel);
			rtmp_set_channel(pAd, wdev, pBssEntry->Channel);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR, "Error! Out of table range: (BssNr=%d).\n", pSsidBssTab->BssNr);
		SetApCliEnableByWdev(pAd, wdev, TRUE);
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
			"<--- ()\n");
		return FALSE;
	}

	SetApCliEnableByWdev(pAd, wdev, TRUE);
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
		"<--- ()\n");
	return TRUE;
}
/*
	===================================================

	Description:
		If the previous selected entry connected failed, this function will
		choose next entry to connect. The previous entry will be deleted.

	Arguments:
		pAd: pointer to our adapter

	Note:
		Note that the table is sorted by Rssi in the "increasing" order, thus
		the last entry in table has stringest Rssi.
	===================================================
*/
VOID ApCliSwitchCandidateAP(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev)
{
	BSS_TABLE		*pSsidBssTab;
	PSTA_ADMIN_CONFIG	pApCliEntry;
	UINT			lastEntryIdx = 0;
	UCHAR			ifIdx = 0;
	SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

	if (ScanCtrl == NULL || ScanCtrl->PartialScan.bScanning == TRUE)
		return;

	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "---> ApCliSwitchCandidateAP()\n");

	if (wdev)
		ifIdx = wdev->func_idx;
	else
		return;

	if ((ifIdx >= MAX_MULTI_STA) || (ifIdx >= MAX_APCLI_NUM)) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, "Error  ifIdx=%d\n", ifIdx);
		return;
	}


	if (pAd->StaCfg[ifIdx].ApcliInfStat.AutoConnectFlag != TRUE)
		return;

	pApCliEntry = &pAd->StaCfg[ifIdx];

	pSsidBssTab = &pApCliEntry->MlmeAux.SsidBssTab;

	if (pSsidBssTab->BssNr == 0) {
		MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "No Bss\n");
		pApCliEntry->ApCliAutoConnectRunning = FALSE;
		return;
	}

	/*
		delete (zero) the previous connected-failled entry and always
		connect to the last entry in talbe until the talbe is empty.
	*/
	/* fix memory leak when trigger scan continuously */
	BssEntryReset(pSsidBssTab, &pSsidBssTab->BssEntry[--pSsidBssTab->BssNr]);
	lastEntryIdx = pSsidBssTab->BssNr - 1;

	if ((pSsidBssTab->BssNr > 0) && (pSsidBssTab->BssNr <= MAX_LEN_OF_BSS_TABLE)) {
		BSS_ENTRY *pBssEntry = &pSsidBssTab->BssEntry[pSsidBssTab->BssNr - 1];
#ifdef APCLI_AUTO_BW_TMP /* should be removed after apcli auto-bw is applied */
		BOOLEAN bw_adj;

		bw_adj = ApCliAutoConnectBWAdjust(pAd, wdev, pBssEntry);

		if (bw_adj)
#endif /* APCLI_AUTO_BW_TMP */
		{
			MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "Switch to channel :%d\n", pBssEntry->Channel);
			rtmp_set_channel(pAd, wdev, pBssEntry->Channel);
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "No candidate AP, the process is about to stop.\n");
		pApCliEntry->ApCliAutoConnectRunning = FALSE;
	}

	SetApCliEnableByWdev(pAd, wdev, TRUE);
	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "---> ApCliSwitchCandidateAP()\n");
}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

BOOLEAN isValidApCliIf(SHORT if_idx)
{
	return (((if_idx >= 0) && (if_idx < MAX_APCLI_NUM)) ? TRUE : FALSE);
}


/*! \brief init the management mac frame header
 *  \param p_hdr mac header
 *  \param subtype subtype of the frame
 *  \param p_ds destination address, don't care if it is a broadcast address
 *  \return none
 *  \pre the station has the following information in the pAd->UserCfg
 *   - bssid
 *   - station address
 *  \post
 *  \note this function initializes the following field
 */
VOID ApCliMgtMacHeaderInit(
	IN RTMP_ADAPTER *pAd,
	INOUT HEADER_802_11 *pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN UCHAR *pDA,
	IN UCHAR *pBssid,
	IN USHORT ifIndex)
{
	NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
	pHdr80211->FC.Type = FC_TYPE_MGMT;
	pHdr80211->FC.SubType = SubType;
	pHdr80211->FC.ToDs = ToDs;
	COPY_MAC_ADDR(pHdr80211->Addr1, pDA);
	COPY_MAC_ADDR(pHdr80211->Addr2, pAd->StaCfg[ifIndex].wdev.if_addr);
	COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
}


/*
    ==========================================================================
    Description:
	APCLI Interface Monitor.
    ==========================================================================
 */

#ifdef FOLLOW_HIDDEN_SSID_FEATURE
VOID ApCliCheckPeerExistence(RTMP_ADAPTER *pAd, CHAR *Ssid, UCHAR SsidLen, UCHAR *Bssid, UCHAR Channel)
#else
VOID ApCliCheckPeerExistence(RTMP_ADAPTER *pAd, CHAR *Ssid, UCHAR SsidLen, UCHAR Channel)
#endif
{
	UCHAR ifIndex;
	STA_ADMIN_CONFIG *pApCliEntry;
	struct wifi_dev *wdev = NULL;
#ifdef FOLLOW_HIDDEN_SSID_FEATURE
	UINT32 mbss_idx = 0;
	UCHAR ZeroSsid[MAX_LEN_OF_SSID] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};
#endif


	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++) {
		pApCliEntry = &pAd->StaCfg[ifIndex];
		wdev = &pApCliEntry->wdev;

		if (pApCliEntry->ApcliInfStat.bPeerExist == TRUE
#ifdef FOLLOW_HIDDEN_SSID_FEATURE
			&& !NdisEqualMemory(Bssid, pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN)
#endif
		)
			continue;
		else if (Channel == pApCliEntry->wdev.channel &&
				 ((SsidLen == pApCliEntry->CfgSsidLen && NdisEqualMemory(Ssid, pApCliEntry->CfgSsid, SsidLen)) ||
#ifdef FOLLOW_HIDDEN_SSID_FEATURE
				((SsidLen == 0 && NdisEqualMemory(Bssid, pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN)) ||
				((NdisEqualMemory(Ssid, ZeroSsid, SsidLen)) && NdisEqualMemory(Bssid, pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN)))
#else
				(SsidLen == 0 /* Hidden */)
#endif

#ifdef WSC_AP_SUPPORT
				  || ((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
						  (wdev->WscControl.bWscTrigger == TRUE) &&
						  NdisEqualMemory(Ssid, wdev->WscControl.WscSsid.Ssid, SsidLen))
#endif
				)){

				pApCliEntry->ApcliInfStat.bPeerExist = TRUE;
#ifdef FOLLOW_HIDDEN_SSID_FEATURE
				if (!INFRA_ON(pApCliEntry))
					continue;

				if ((SsidLen == 0 || NdisEqualMemory(Ssid, ZeroSsid, SsidLen)) && !pApCliEntry->MlmeAux.Hidden) {
						pApCliEntry->MlmeAux.Hidden = 1;
						MTWF_PRINT("Following Hidden SSID\n");
						for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum ; mbss_idx++) {
							if (pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode == pApCliEntry->wdev.PhyMode) {
									pAd->ApCfg.MBSSID[mbss_idx].bHideSsid = pApCliEntry->MlmeAux.Hidden;
									MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "-->Follow Root AP Hidden ssid\n");
									/*Feature:Update forwardhaul beaconing ssid to hidden if root ap hidden */
									UpdateBeaconHandler(pAd, &pAd->ApCfg.MBSSID[mbss_idx].wdev, BCN_REASON(BCN_UPDATE_IE_CHG));
								}
							}
					} else if (pApCliEntry->MlmeAux.Hidden && !(SsidLen == 0 || NdisEqualMemory(Ssid, ZeroSsid, SsidLen))) {
								pApCliEntry->MlmeAux.Hidden = 0;
								MTWF_PRINT("Following Broadcast SSID\n");
								for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum ; mbss_idx++) {
									if (pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode == pApCliEntry->wdev.PhyMode) {
											pAd->ApCfg.MBSSID[mbss_idx].bHideSsid = pApCliEntry->MlmeAux.Hidden;
											MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "-->Follow Root AP Broadcast ssid\n");
											UpdateBeaconHandler(pAd, &pAd->ApCfg.MBSSID[mbss_idx].wdev, BCN_REASON(BCN_UPDATE_IE_CHG));
									}
								}
					} else {
						MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG, "No action required SSID Len = %d,Hidden status = %d\n", SsidLen, pApCliEntry->MlmeAux.Hidden);
					}
#endif

		}
		else {
			/* No Root AP match the SSID */
		}
	}
}

struct wifi_dev_ops apcli_wdev_ops = {
	.tx_pkt_allowed = apcli_tx_pkt_allowed,
	.fp_tx_pkt_allowed = apcli_fp_tx_pkt_allowed,
	.send_data_pkt = ap_send_data_pkt,
	.fp_send_data_pkt = fp_send_data_pkt,
	.send_mlme_pkt = ap_send_mlme_pkt,
	.tx_pkt_handle = ap_tx_pkt_handle,
	.fill_non_offload_tx_blk = apcli_fill_non_offload_tx_blk,
	.fill_offload_tx_blk = apcli_fill_offload_tx_blk,
	.legacy_tx = ap_legacy_tx,
	.frag_tx = ap_frag_tx,
	.amsdu_tx = ap_amsdu_tx,
	.mlme_mgmtq_tx = ap_mlme_mgmtq_tx,
	.mlme_dataq_tx = ap_mlme_dataq_tx,
	.ieee_802_11_data_tx = ap_ieee_802_11_data_tx,
	.ieee_802_3_data_tx = ap_ieee_802_3_data_tx,
	.rx_pkt_allowed = sta_rx_pkt_allow,
	.rx_pkt_foward = sta_rx_fwd_hnd,
	.ieee_802_3_data_rx = ap_ieee_802_3_data_rx,
	.ieee_802_11_data_rx = ap_ieee_802_11_data_rx,
	.find_cipher_algorithm = ap_find_cipher_algorithm,
	/* snowpin for ap/sta ++ */
	.media_state_connected = NULL,/* media_state_connected, */
	.ioctl = rt28xx_ap_ioctl,
	.mac_entry_lookup = mac_entry_lookup,
	/* snowpin for ap/sta -- */

#ifdef VERIFICATION_MODE
	.verify_tx = verify_pkt_tx,
#endif
	.open = sta_inf_open,
	.close = sta_inf_close,
	.linkup = wifi_sys_linkup,
	.linkdown = wifi_sys_linkdown,
	.conn_act = wifi_sys_conn_act,
	.disconn_act = wifi_sys_disconn_act,
};

INT ApCliIfLookUp(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	SHORT if_idx;

	for (if_idx = 0; if_idx < MAX_APCLI_NUM; if_idx++) {
		if (MAC_ADDR_EQUAL(pAd->StaCfg[if_idx].wdev.if_addr, pAddr)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_INFO,
					"ApCliIfIndex=%d\n", if_idx);
			return if_idx;
		}
	}

	return -1;
}

/*
    ==========================================================================
    Description:
	APCLI Interface Monitor.
	EZ_SETUP please contruct a new function for ApCliIfMonitor
    ==========================================================================
 */
VOID ApCliIfMonitor(RTMP_ADAPTER *pAd)
{
	UCHAR index;
	STA_ADMIN_CONFIG *pApCliEntry;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef CONFIG_MAP_SUPPORT
	UCHAR IdBss = 0;
	BSS_STRUCT *pMbss = NULL;
	struct DOT11_H *pDot11h = NULL;
	struct wifi_dev *wdev = NULL;
	BOOLEAN ChChangeInProgress = FALSE;
#endif

	/* Reset is in progress, stop immediately */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
		!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return;

#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd)) {
		for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
			pMbss = &pAd->ApCfg.MBSSID[IdBss];
			if (pMbss == NULL)
				continue;
			wdev = &pMbss->wdev;
			if (wdev == NULL)
				continue;

			pDot11h = wdev->pDot11_H;
			if (pDot11h && pDot11h->ChChangeCSA) {
				ChChangeInProgress = TRUE;
				break;
			}
		}
	}
#endif

	for (index = 0; index < MAX_APCLI_NUM; index++) {
		UINT16 wcid;
		PMAC_TABLE_ENTRY pMacEntry;
		STA_TR_ENTRY *tr_entry;
		BOOLEAN bForceBrocken = FALSE;
		BOOLEAN bWpa_4way_too_log = FALSE;
		BOOLEAN bBeacon_miss = FALSE;

		pApCliEntry = &pAd->StaCfg[index];

		if (!pApCliEntry->wdev.DevInfo.WdevActive
			)
			continue;

		if (scan_in_run_state(pAd, &pApCliEntry->wdev) == TRUE) {
			/*Update rcv bcn time when scan ongoing
			 to avoid apcli hit bcn lost after scan end*/
			pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime = pAd->Mlme.Now32;
			continue;
		}

#ifdef APCLI_CONNECTION_TRIAL

		if (index == (pAd->ApCfg.ApCliNum - 1)) /* last IF is for apcli connection trial */
			continue;/* skip apcli1 monitor. FIXME:Carter shall find a better way. */

#endif /* APCLI_CONNECTION_TRIAL */

		/* sanity check whether the interface is initialized. */
		if (pApCliEntry->ApcliInfStat.ApCliInit != TRUE)
			continue;

#ifdef MAC_REPEATER_SUPPORT
		RepeaterLinkMonitor(pAd);
#endif /* MAC_REPEATER_SUPPORT */

		if (pApCliEntry->ApcliInfStat.Valid == TRUE) {
			BOOLEAN ApclibQosNull = FALSE;

			wcid = pAd->StaCfg[index].MacTabWCID;

			if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
				continue;

			pMacEntry = entry_get(pAd, wcid);
			tr_entry = tr_entry_get(pAd, wcid);

#ifdef PRE_CFG_SUPPORT
			/* [Guangbin] disable interface monitor in pre-cfg mode */
			if (pMacEntry && pMacEntry->fgPreCfgRunning)
				continue;
#endif /* PRE_CFG_SUPPORT */

			if ((IS_AKM_WPA_CAPABILITY(pMacEntry->SecConfig.AKMMap))
				&& (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)
				&& (RTMP_TIME_AFTER(pAd->Mlme.Now32, (pApCliEntry->ApcliInfStat.ApCliLinkUpTime + (30 * OS_HZ))))) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_ERROR,
					"STA WPA 4 way too log condition got hit.\n");
				bWpa_4way_too_log = TRUE;
				bForceBrocken = TRUE;
			}

			if (!pApCliEntry->PwrMgmt.bDoze) {
				if (RTMP_TIME_AFTER(pAd->Mlme.Now32, (pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime +
						 4 * OS_HZ))) {
#ifdef CONFIG_MAP_SUPPORT
					if (IS_MAP_ENABLE(pAd) && (ChChangeInProgress == TRUE) &&
						(RTMP_TIME_BEFORE(pAd->Mlme.Now32, (pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime +
						(8 * OS_HZ))))) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_ERROR,
							"ChChange due to CSA In Progress\n");
					} else {
#endif
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_ERROR,
							"STA Beacon loss condition got hit.\n");
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
						pMacEntry->bTxPktChk = FALSE;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
						bBeacon_miss = TRUE;
						bForceBrocken = TRUE;
#ifdef CONFIG_MAP_SUPPORT
					}
#endif
				}
			}

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
#ifdef BEACON_MISS_ON_PRIMARY_CHANNEL
				if ((cap->fgRateAdaptFWOffload == TRUE) &&
					(pMacEntry->TxStatRspCnt > 1) && (pMacEntry->TotalTxSuccessCnt)) {
				/*When Root AP changes the primary channel within the same group of bandwidth, APCLI not disconnects from Root AP.
				This happens as the NULL packet transmits in the configured bandwidth only, the transmitted NULL packet is succeeding
				which update TX Success count.
				Example, BW is configured for 80 MHz, Root AP switches primary channel from 36 to 40,
				NULL packet transmits will happen in 80 MHz only*/
					if ((!WMODE_CAP_2G(pApCliEntry->wdev.PhyMode)) && (pMacEntry->MaxHTPhyMode.field.BW > 0) &&
					(RTMP_TIME_AFTER(pAd->Mlme.Now32, (pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime_MlmeEnqueueForRecv) + (6 * OS_HZ)))) {

						bBeacon_miss = TRUE;
						bForceBrocken = TRUE;
					}
				}
#endif
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */


			if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
				ApclibQosNull = TRUE;
#if (defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)) && !defined(RT_CFG80211_SUPPORT)
			if (bWpa_4way_too_log == TRUE) {
				if (IS_AKM_SAE_SHA256(pApCliEntry->AKMMap)
						|| IS_AKM_OWE(pApCliEntry->AKMMap)
						|| IS_AKM_SAE_EXT(pApCliEntry->AKMMap)) {
					UCHAR pmkid[LEN_PMKID];
					UCHAR pmk[LEN_PMK];
					INT cached_idx;
					UCHAR if_index = pApCliEntry->wdev.func_idx;
					struct wifi_dev *wdev = &pApCliEntry->wdev;
					struct __SAE_CFG *sae_cfg = PD_GET_SAE_CFG_PTR(
									pAd->physical_dev);

					/* Connection taking too long update PMK cache  and delete sae instance*/
					if (
#ifdef DOT11_SAE_SUPPORT
						(IS_AKM_SAE_SHA256(pApCliEntry->AKMMap) &&
						 sae_get_pmk_cache(sae_cfg, pApCliEntry->wdev.if_addr, pApCliEntry->MlmeAux.Bssid, FALSE, pmkid, pmk))
						 || (IS_AKM_SAE_EXT(pApCliEntry->AKMMap) &&
						 sae_get_pmk_cache(sae_cfg, pApCliEntry->wdev.if_addr, pApCliEntry->MlmeAux.Bssid, FALSE, pmkid, pmk))
#endif

#ifdef CONFIG_OWE_SUPPORT
						|| IS_AKM_OWE(pApCliEntry->AKMMap)
#endif
					   ) {
							UINT32 sec_akm = 0;


							if (IS_AKM_SAE_SHA256(pApCliEntry->MlmeAux.AKMMap))
								SET_AKM_SAE_SHA256(sec_akm);
							else if (IS_AKM_OWE(pApCliEntry->MlmeAux.AKMMap))
								SET_AKM_OWE(sec_akm);
							else if (IS_AKM_SAE_EXT(pApCliEntry->MlmeAux.AKMMap))
								SET_AKM_SAE_EXT(sec_akm);

							cached_idx = sta_search_pmkid_cache(pAd, pApCliEntry->MlmeAux.Bssid, if_index, wdev,
							sec_akm, pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen);
						if (cached_idx != INVALID_PMKID_IDX) {
#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
							SAE_INSTANCE *pSaeIns = search_sae_instance(sae_cfg, pApCliEntry->wdev.if_addr, pApCliEntry->MlmeAux.Bssid);
							MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
								"Reconnection failed with pmkid ,delete cache entry and sae instance\n");

							if (pSaeIns != NULL)
								delete_sae_instance(pSaeIns);
#endif
								sta_delete_pmkid_cache(pAd, pApCliEntry->MlmeAux.Bssid, if_index, wdev,
								sec_akm, pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen);
						}
					}
				}
			}
#endif
			if (bForceBrocken == FALSE) {
				if (!pApCliEntry->PwrMgmt.bDoze) {
					USHORT PwrMgmt = PWR_ACTIVE;
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
					if (twtPlannerIsRunning(pAd, pApCliEntry))
						PwrMgmt = PWR_SAVE;
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
					ApCliRTMPSendNullFrame(pAd, pMacEntry->CurrTxRate, ApclibQosNull, pMacEntry, PwrMgmt);
			   }
			}
		} else
			continue;

		if (bForceBrocken == TRUE) {
			MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "ApCliIfMonitor: IF(apcli%d) - no Beancon is received from root-AP.\n", index);
			MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "ApCliIfMonitor: Reconnect the Root-Ap again.\n");

			if (bBeacon_miss) {
				ULONG Now32;

				MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "ApCliIfMonitor apcli%d time1: %lu\n", index, pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime_MlmeEnqueueForRecv);
				MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "ApCliIfMonitor apcli%d time2: %lu\n", index, pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime_MlmeEnqueueForRecv_2);
				MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "ApCliIfMonitor apcli%d time3: %lu\n", index, pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime);
				MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "ApCliIfMonitor apcli%d OS_HZ: %d\n", index, OS_HZ);
				NdisGetSystemUpTime(&Now32);
				MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "ApCliIfMonitor apcli%d time now: %lu\n", index, Now32);
			}
#ifdef MAC_REPEATER_SUPPORT
			if (pAd->ApCfg.bMACRepeaterEn) {
				STA_ADMIN_CONFIG *apcli_entry = pApCliEntry;
				repeater_disconnect_by_band(pAd, HcGetBandByWdev(&apcli_entry->wdev));
			}
#endif /* MAC_REPEATER_SUPPORT */
			/*
				Improve a case that beacon miss will be invoked constantly when tput. is buzy.
				And avoid a disconnection action may be enqueued repeatedly.
			*/
			pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime = (pAd->Mlme.Now32 + (300 * OS_HZ));

			if (bBeacon_miss)
				pApCliEntry->ApcliInfStat.Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_MNT_NO_BEACON;
			else
				pApCliEntry->ApcliInfStat.Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCLI_TRIGGER_TOO_LONG;

			cntl_disconnect_request(&pApCliEntry->wdev,
									CNTL_DISASSOC,
									pApCliEntry->Bssid,
									REASON_DISASSOC_STA_LEAVING);

		}
#ifdef DFS_SLAVE_SUPPORT
		else if (pApCliEntry->ApcliInfStat.Enable &&
				pApCliEntry->ApcliInfStat.Valid &&
				SLAVE_MODE_EN(pAd) && SLAVE_BEACON_STOPPED(pAd)) {
			/* if apcli connect event missed by WAPP, then send it again */
			slave_bh_event(pAd, &pApCliEntry->wdev, TRUE);
		}
#endif /* DFS_SLAVE_SUPPORT */
	}

	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_DEBUG, "ra offload=%d\n", cap->fgRateAdaptFWOffload);
}

#endif /*CONFIG_APSTA_MIXED_SUPPORT*/

#if defined(DOT11_SAE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
INT set_apcli_sae_group_proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR *pSaeCfgGroup = NULL;
	UCHAR group = 0;
	UINT32 staidx = 0;
	UINT32 sec_akm = 0;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			"pObj->ioctl_if is invalid value\n");
		return FALSE;
	}

	staidx = pObj->ioctl_if;
	pSaeCfgGroup = &pAd->StaCfg[staidx].sae_cfg_group;

	group = os_str_tol(arg, 0, 10);

	SET_AKM_SAE_SHA256(sec_akm);

	if ((group == 19) || (group == 20) || (group == 21)) {

		if (*pSaeCfgGroup != group) {
			/*clear SAE entries of pmk cache*/
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
				"Clear cache for sec 0x%x\n", sec_akm);
			sta_delete_pmkid_cache_by_akm(pAd, staidx, sec_akm);
		}


		*pSaeCfgGroup = (UCHAR) group;
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			"[SAE] Set group=%d\n",
					group);

	} else {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			"[SAE] group=%d not supported\n",
					group);
		return FALSE;
	}

	return TRUE;
}

INT Set_apcli_sae_pk_proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj;
	UCHAR *pSaepk = NULL;
	UCHAR saepk = 0;
	UINT32 staidx = 0;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			"pObj->ioctl_if is invalid value\n");
		return FALSE;
	}

	staidx = pObj->ioctl_if;
	pSaepk = &pAd->StaCfg[staidx].wdev.SecConfig.sae_cap.sae_pk_en;

	saepk = os_str_tol(arg, 0, 10);

	*pSaepk = (UCHAR) saepk;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "[SAE] saepk=%d\n",
		saepk);

	return TRUE;
}
INT Set_apcli_sae_pk_only_proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj;
	UCHAR *pSaepkonly = NULL;
	UCHAR saepkonly = 0;
	UINT32 staidx = 0;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			"pObj->ioctl_if is invalid value\n");
		return FALSE;
	}

	staidx = pObj->ioctl_if;
	pSaepkonly = &pAd->StaCfg[staidx].wdev.SecConfig.sae_cap.sae_pk_only_en;

	saepkonly = os_str_tol(arg, 0, 10);

	*pSaepkonly = (UCHAR) saepkonly;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "[SAE] saepk_only=%d\n",
		saepkonly);

	return TRUE;
}
#endif/*DOT11_SAE_SUPPORT*/

#ifdef CONFIG_OWE_SUPPORT
INT set_apcli_owe_group_proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR group = 0;
	UCHAR *pcurr_group = NULL;
	UINT32 staidx = 0;
	UINT32 sec_akm = 0;

	if (0 == strlen(arg))
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
			"pObj->ioctl_if is invalid value\n");
		return FALSE;
	}

	staidx = pObj->ioctl_if;
	pcurr_group = &pAd->StaCfg[staidx].curr_owe_group;

	SET_AKM_OWE(sec_akm);

	group = os_str_tol(arg, 0, 10);
	/*OWE-currently allowing configuration of groups 19(mandatory) and 20(optional) */
	if ((group == 19) || (group == 20)) {

		if (*pcurr_group != group) {
			/*clear owe entries of pmk cache*/
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_INFO,
				"Clear cache for sec 0x%x\n", sec_akm);
				sta_delete_pmkid_cache_by_akm(pAd, staidx, sec_akm);
		}
		*pcurr_group = (UCHAR) group;
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_INFO,
			"[OWE] Set group=%d\n",
					group);
	} else {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
			"[OWE] group=%d not supported\n",
					group);
				return FALSE;
	}
	return TRUE;
}
#endif/*CONFIG_OWE_SUPPORT*/

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
INT set_apcli_del_pmkid_list(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR action = 0;

	if (0 == strlen(arg))
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	action = os_str_tol(arg, 0, 10);

	/*Delete all pmkid list associated with this  ApCli Interface*/
	if (action == 1) {
		sta_delete_pmkid_cache_all(pAd, pObj->ioctl_if);
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_INFO, "Delete PMKID list (%d)\n",
					action);

	}
	return TRUE;
}

INT sta_add_pmkid_cache(
	IN  PRTMP_ADAPTER   pAd,
	IN UCHAR *paddr,
	IN UCHAR *pmkid,
	IN UCHAR *pmk,
	IN UINT8 pmk_len,
	IN UINT8 if_index,
	IN struct wifi_dev *wdev,
	IN UINT32 akm,
	IN UCHAR *ssid,
	IN UCHAR ssid_len
	)
{
	PSTA_ADMIN_CONFIG papcli_entry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY preptcli_entry = NULL;
#endif
	INT cached_idx;
	PBSSID_INFO psaved_pmk = NULL;
	PUINT psaved_pmk_num = NULL;
	UCHAR update_pmkid = FALSE;
	VOID *psaved_pmk_lock = NULL;

#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		preptcli_entry = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
		papcli_entry = &pAd->StaCfg[if_index];
		psaved_pmk = (PBSSID_INFO)&preptcli_entry->SavedPMK[0];
		psaved_pmk_num = &preptcli_entry->SavedPMKNum;
		psaved_pmk_lock = (VOID *)&preptcli_entry->SavedPMK_lock;
	} else
#endif
	{
		papcli_entry = &pAd->StaCfg[if_index];
		psaved_pmk = (PBSSID_INFO)&papcli_entry->SavedPMK[0];
		psaved_pmk_num = &papcli_entry->SavedPMKNum;
		psaved_pmk_lock = (VOID *)&papcli_entry->SavedPMK_lock;
	}
	cached_idx = sta_search_pmkid_cache(pAd, paddr, if_index, wdev, akm, ssid, ssid_len);

	if (psaved_pmk_lock)
		NdisAcquireSpinLock(psaved_pmk_lock);

	if (cached_idx != INVALID_PMKID_IDX) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_INFO,
					"PMKID found, %d\n", cached_idx);
#ifdef MAP_R3
		if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd))
			update_pmkid = TRUE;
#endif
	} else {
			/* Find free cache entry */
		for (cached_idx = 0; cached_idx < PMKID_NO; cached_idx++) {
			if (psaved_pmk[cached_idx].Valid == FALSE)
				break;
		}

		if (cached_idx < PMKID_NO) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_INFO,
						"Free Cache entry found,cached_idx %d\n", cached_idx);
			*psaved_pmk_num = *psaved_pmk_num + 1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_WARN,
						"cache full, overwrite cached_idx 0\n");
			cached_idx = 0;
		}
		update_pmkid = TRUE;
	}

	if (update_pmkid == TRUE) {
		psaved_pmk[cached_idx].Valid = TRUE;
		psaved_pmk[cached_idx].akm = akm;
		COPY_MAC_ADDR(&psaved_pmk[cached_idx].BSSID, paddr);

		NdisZeroMemory(&psaved_pmk[cached_idx].PMKID, LEN_PMKID);
		NdisZeroMemory(&psaved_pmk[cached_idx].ssid, MAX_LEN_OF_SSID);
		NdisZeroMemory(&psaved_pmk[cached_idx].PMK, LEN_MAX_PMK);

		NdisMoveMemory(&psaved_pmk[cached_idx].PMKID, pmkid, LEN_PMKID);
		if (pmk_len <= sizeof(psaved_pmk[cached_idx].PMK))
			NdisMoveMemory(&psaved_pmk[cached_idx].PMK, pmk, pmk_len);
		else
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_ERROR,
					"No enough buffer size for psaved_pmk[%d].PMK!\n", cached_idx);
		if (ssid_len <= sizeof(psaved_pmk[cached_idx].ssid))
			NdisMoveMemory(&psaved_pmk[cached_idx].ssid, ssid, ssid_len);
		else
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_ERROR,
					"No enough buffer size for psaved_pmk[%d].ssid\n", cached_idx);
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_INFO,
					"add "MACSTR" cache(%d) akm:0x%x,SSID:%s\n",
					 MAC2STR(paddr), cached_idx, psaved_pmk[cached_idx].akm, psaved_pmk[cached_idx].ssid);
#ifdef SUPP_SAE_SUPPORT
		psaved_pmk[cached_idx].pmk_len = pmk_len;
#endif
	}

	if (psaved_pmk_lock)
		NdisReleaseSpinLock(psaved_pmk_lock);

#ifdef SUPP_SAE_SUPPORT
	if (psaved_pmk[cached_idx].pmk_len <= sizeof(psaved_pmk[cached_idx].PMK))
		mtk_cfg80211_event_pmksa(pAd, psaved_pmk[cached_idx].PMK, psaved_pmk[cached_idx].pmk_len,
				psaved_pmk[cached_idx].PMKID, wdev->SecConfig.AKMMap, psaved_pmk[cached_idx].BSSID);
#endif

	return cached_idx;
}

INT sta_search_pmkid_cache(
	IN  PRTMP_ADAPTER   pAd,
	IN UCHAR *paddr,
	IN UCHAR if_index,
	IN struct wifi_dev *wdev,
	IN UINT32 akm,
	IN UCHAR *ssid,
	IN UCHAR ssid_len)
{
	INT i = 0;
	PBSSID_INFO psaved_pmk = NULL;
	PSTA_ADMIN_CONFIG papcli_entry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY preptcli_entry = NULL;
#endif
	VOID *psaved_pmk_lock = NULL;

#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		preptcli_entry = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
		psaved_pmk = (PBSSID_INFO)&preptcli_entry->SavedPMK[0];
		psaved_pmk_lock = (VOID *)&preptcli_entry->SavedPMK_lock;
	} else
#endif
	{
		papcli_entry = &pAd->StaCfg[if_index];
		psaved_pmk = (PBSSID_INFO)&papcli_entry->SavedPMK[0];
		psaved_pmk_lock = (VOID *)&papcli_entry->SavedPMK_lock;
	}
	if (psaved_pmk_lock)
		NdisAcquireSpinLock(psaved_pmk_lock);

	for (i = 0; i < PMKID_NO; i++) {
		if ((psaved_pmk[i].Valid == TRUE)
			&& MAC_ADDR_EQUAL(&psaved_pmk[i].BSSID, paddr)
			&& (psaved_pmk[i].akm == akm)
			&& (ssid && (ssid_len <= MAX_LEN_OF_SSID)
					&& NdisEqualMemory(&psaved_pmk[i].ssid[0], ssid, ssid_len))) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_NOTICE,
						""MACSTR" cache(%d),akm:0x%x,SSID:%s\n",
						 MAC2STR(paddr), i, akm, psaved_pmk[i].ssid);
			break;
		}
	}

	if (psaved_pmk_lock)
		NdisReleaseSpinLock(psaved_pmk_lock);

	if (i >= PMKID_NO) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_NOTICE,
					"not found\n");
		return INVALID_PMKID_IDX;
	}

	return i;
}

VOID sta_delete_pmkid_cache(
	IN  PRTMP_ADAPTER   pAd,
	IN UCHAR *paddr,
	IN UCHAR if_index,
	IN struct wifi_dev *wdev,
	IN UINT32 akm,
	IN UCHAR *ssid,
	IN UCHAR ssid_len)
{

	INT cached_idx;
	PBSSID_INFO psaved_pmk = NULL;
	PSTA_ADMIN_CONFIG papcli_entry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY preptcli_entry = NULL;
#endif
	VOID *psaved_pmk_lock = NULL;
	PUINT psaved_pmk_num = NULL;


#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		preptcli_entry = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
		psaved_pmk = (PBSSID_INFO)&preptcli_entry->SavedPMK[0];
		psaved_pmk_num = &preptcli_entry->SavedPMKNum;
		psaved_pmk_lock = (VOID *)&preptcli_entry->SavedPMK_lock;
	}  else
#endif
	{
		papcli_entry = &pAd->StaCfg[if_index];
		psaved_pmk = (PBSSID_INFO)&papcli_entry->SavedPMK[0];
		psaved_pmk_num = &papcli_entry->SavedPMKNum;
		psaved_pmk_lock = (VOID *)&papcli_entry->SavedPMK_lock;

	}
	cached_idx = sta_search_pmkid_cache(pAd, paddr, if_index, wdev, akm, ssid, ssid_len);

	if (cached_idx != INVALID_PMKID_IDX) {
		if (psaved_pmk_lock)
			NdisAcquireSpinLock(psaved_pmk_lock);

		if (psaved_pmk[cached_idx].Valid == TRUE) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_INFO,
						""MACSTR" cache(%d),akm:0x%x,SSID:%s\n",
						 MAC2STR(paddr), cached_idx, akm, psaved_pmk[cached_idx].ssid);
			psaved_pmk[cached_idx].Valid = FALSE;
			psaved_pmk[cached_idx].akm = 0;
			NdisZeroMemory(&psaved_pmk[cached_idx].PMKID, LEN_PMKID);
			NdisZeroMemory(&psaved_pmk[cached_idx].ssid, MAX_LEN_OF_SSID);
			NdisZeroMemory(&psaved_pmk[cached_idx].PMK, LEN_MAX_PMK);

			if (*psaved_pmk_num)
				*psaved_pmk_num = *psaved_pmk_num - 1;
		}
		if (psaved_pmk_lock)
			NdisReleaseSpinLock(psaved_pmk_lock);
	}
}

VOID sta_delete_pmkid_cache_all(
	IN  PRTMP_ADAPTER   pAd,
	IN UCHAR if_index)
{
#ifdef MAC_REPEATER_SUPPORT
	INT cli_idx = 0;
#endif	/* MAC_REPEATER_SUPPORT */
	INT cached_idx;
	PBSSID_INFO psaved_pmk = NULL;
	PSTA_ADMIN_CONFIG papcli_entry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY preptcli_entry = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
#if (defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)) && !defined(RT_CFG80211_SUPPORT)
	SAE_INSTANCE *pSaeIns = NULL;
	SAE_CFG *pSaeCfg = NULL;
	UINT32 i;
	UINT32 ins_cnt = 0;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);
#endif

	VOID *psaved_pmk_lock = NULL;
	PUINT psaved_pmk_num = NULL;
	papcli_entry = &pAd->StaCfg[if_index];
	psaved_pmk = (PBSSID_INFO)&papcli_entry->SavedPMK[0];
	psaved_pmk_num = &papcli_entry->SavedPMKNum;
	psaved_pmk_lock = (VOID *)&papcli_entry->SavedPMK_lock;

#if (defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)) && !defined(RT_CFG80211_SUPPORT)
	pSaeCfg = PD_GET_SAE_CFG_PTR(pAd->physical_dev);
	/*Delete all SAE instances for this ApCli Interface*/
	NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

	for (i = 0; i < wtbl_max_num; i++) {
		if (pSaeCfg->sae_ins[i].valid == FALSE)
			continue;

		if (RTMPEqualMemory(pSaeCfg->sae_ins[i].own_mac, papcli_entry->wdev.if_addr, MAC_ADDR_LEN)) {
			pSaeIns = &pSaeCfg->sae_ins[i];
			if (pSaeIns != NULL && (pSaeIns->valid == TRUE)) {
				NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
				delete_sae_instance(pSaeIns);
				NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);
			}
		}
		ins_cnt++;
		if (ins_cnt == pSaeCfg->total_ins)
			break;
	}

	NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
#endif
	/*Delete ApCli PMKID list*/
	for (cached_idx = 0; cached_idx < PMKID_NO; cached_idx++) {
		if (psaved_pmk_lock)
			NdisAcquireSpinLock(psaved_pmk_lock);

		if (psaved_pmk[cached_idx].Valid == TRUE) {
			psaved_pmk[cached_idx].Valid = FALSE;
			psaved_pmk[cached_idx].akm = 0;
			NdisZeroMemory(&psaved_pmk[cached_idx].PMKID, LEN_PMKID);
			NdisZeroMemory(&psaved_pmk[cached_idx].ssid, MAX_LEN_OF_SSID);
			NdisZeroMemory(&psaved_pmk[cached_idx].PMK, LEN_MAX_PMK);

			if (*psaved_pmk_num)
				*psaved_pmk_num = *psaved_pmk_num - 1;
		}
		if (psaved_pmk_lock)
			NdisReleaseSpinLock(psaved_pmk_lock);

	}
	/* Delete  PMKID list for MacRepeater linked with ApCli */

#ifdef MAC_REPEATER_SUPPORT
	if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
		for (cli_idx = 0; cli_idx < GET_MAX_REPEATER_ENTRY_NUM(cap); cli_idx++) {
			preptcli_entry = &pAd->ApCfg.pRepeaterCliPool[cli_idx];

			if (preptcli_entry && (preptcli_entry->CliValid == TRUE)) {
#ifdef DOT11_SAE_SUPPORT
				/* Delete all SAE instances for this Rept entry */
				NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);
				ins_cnt = 0;

				for (i = 0; i < wtbl_max_num; i++) {
					if (pSaeCfg->sae_ins[i].valid == FALSE)
						continue;

					if (RTMPEqualMemory(pSaeCfg->sae_ins[i].own_mac, preptcli_entry->CurrentAddress, MAC_ADDR_LEN)) {
						pSaeIns = &pSaeCfg->sae_ins[i];
						if (pSaeIns != NULL && (pSaeIns->valid == TRUE)) {
							NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
							delete_sae_instance(pSaeIns);
							NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

						}
					}

					ins_cnt++;

					if (ins_cnt == pSaeCfg->total_ins)
						break;
				}

				NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
#endif
				psaved_pmk = (PBSSID_INFO)&preptcli_entry->SavedPMK[0];
				psaved_pmk_num = &preptcli_entry->SavedPMKNum;
				psaved_pmk_lock = (VOID *)&preptcli_entry->SavedPMK_lock;


				for (cached_idx = 0; cached_idx < PMKID_NO; cached_idx++) {
					if (psaved_pmk_lock)
						NdisAcquireSpinLock(psaved_pmk_lock);

					if (psaved_pmk[cached_idx].Valid == TRUE) {
						psaved_pmk[cached_idx].Valid = FALSE;
						psaved_pmk[cached_idx].akm = 0;

						NdisZeroMemory(&psaved_pmk[cached_idx].PMKID, LEN_PMKID);
						NdisZeroMemory(&psaved_pmk[cached_idx].ssid, MAX_LEN_OF_SSID);
						NdisZeroMemory(&psaved_pmk[cached_idx].PMK, LEN_MAX_PMK);

						if (*psaved_pmk_num)
							*psaved_pmk_num = *psaved_pmk_num - 1;
					}

					if (psaved_pmk_lock)
						NdisReleaseSpinLock(psaved_pmk_lock);
				}
			}
		}
	}
#endif
}

VOID sta_delete_pmkid_cache_by_akm(
	IN  PRTMP_ADAPTER   pAd,
	IN UCHAR if_index,
	IN UINT32 akm)
{
#ifdef MAC_REPEATER_SUPPORT
	INT cli_idx = 0;
#endif	/* MAC_REPEATER_SUPPORT */
	INT cached_idx;
	PBSSID_INFO psaved_pmk = NULL;
	PSTA_ADMIN_CONFIG papcli_entry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY preptcli_entry = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
#if defined(DOT11_SAE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
	SAE_INSTANCE *pSaeIns = NULL;
	SAE_CFG *pSaeCfg = NULL;
	UINT32 i;
	UINT32 ins_cnt = 0;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);
#endif

	VOID *psaved_pmk_lock = NULL;
	PUINT psaved_pmk_num = NULL;
	papcli_entry = &pAd->StaCfg[if_index];
	psaved_pmk = (PBSSID_INFO)&papcli_entry->SavedPMK[0];
	psaved_pmk_num = &papcli_entry->SavedPMKNum;
	psaved_pmk_lock = (VOID *)&papcli_entry->SavedPMK_lock;

#if defined(DOT11_SAE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)

	pSaeCfg = PD_GET_SAE_CFG_PTR(pAd->physical_dev);
	if (IS_AKM_SAE(akm)) {
		/*Delete all SAE instances for this ApCli Interface*/
		NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

		for (i = 0; i < wtbl_max_num; i++) {
			if (pSaeCfg->sae_ins[i].valid == FALSE)
				continue;

			if (RTMPEqualMemory(pSaeCfg->sae_ins[i].own_mac, papcli_entry->wdev.if_addr, MAC_ADDR_LEN)) {
				pSaeIns = &pSaeCfg->sae_ins[i];
				if (pSaeIns != NULL && (pSaeIns->valid == TRUE)) {
					NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
					delete_sae_instance(pSaeIns);
					NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);
				}
			}
			ins_cnt++;
			if (ins_cnt == pSaeCfg->total_ins)
				break;
		}

		NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
	}
#endif
	/*Delete ApCli PMKID list*/
	for (cached_idx = 0; cached_idx < PMKID_NO; cached_idx++) {
		if (psaved_pmk_lock)
			NdisAcquireSpinLock(psaved_pmk_lock);

		if ((psaved_pmk[cached_idx].Valid == TRUE) &&
			(psaved_pmk[cached_idx].akm == akm)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_INFO,
						":"MACSTR" cache(%d),akm:0x%x,SSID:%s\n",
						 MAC2STR(psaved_pmk[cached_idx].BSSID),
						 cached_idx, akm, psaved_pmk[cached_idx].ssid);
			psaved_pmk[cached_idx].Valid = FALSE;
			psaved_pmk[cached_idx].akm = 0;
			NdisZeroMemory(&psaved_pmk[cached_idx].PMKID, LEN_PMKID);
			NdisZeroMemory(&psaved_pmk[cached_idx].ssid, MAX_LEN_OF_SSID);
			NdisZeroMemory(&psaved_pmk[cached_idx].PMK, LEN_MAX_PMK);

			if (*psaved_pmk_num)
				*psaved_pmk_num = *psaved_pmk_num - 1;
		}
		if (psaved_pmk_lock)
			NdisReleaseSpinLock(psaved_pmk_lock);

	}
	/* Delete  PMKID list for MacRepeater linked with ApCli */

#ifdef MAC_REPEATER_SUPPORT
	if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
		for (cli_idx = 0; cli_idx < GET_MAX_REPEATER_ENTRY_NUM(cap); cli_idx++) {
			preptcli_entry = &pAd->ApCfg.pRepeaterCliPool[cli_idx];

			if (preptcli_entry && (preptcli_entry->CliValid == TRUE)) {
#if defined(DOT11_SAE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
				if (IS_AKM_SAE(akm)) {
					/* Delete all SAE instances for this Rept entry */
					NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);
					ins_cnt = 0;

					for (i = 0; i < wtbl_max_num; i++) {
						if (pSaeCfg->sae_ins[i].valid == FALSE)
							continue;

						if (RTMPEqualMemory(pSaeCfg->sae_ins[i].own_mac, preptcli_entry->CurrentAddress, MAC_ADDR_LEN)) {
							pSaeIns = &pSaeCfg->sae_ins[i];
							if (pSaeIns != NULL && (pSaeIns->valid == TRUE)) {
								NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
								delete_sae_instance(pSaeIns);
								NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

							}
						}

						ins_cnt++;

						if (ins_cnt == pSaeCfg->total_ins)
							break;
					}

					NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
				}
#endif
				psaved_pmk = (PBSSID_INFO)&preptcli_entry->SavedPMK[0];
				psaved_pmk_num = &preptcli_entry->SavedPMKNum;
				psaved_pmk_lock = (VOID *)&preptcli_entry->SavedPMK_lock;


				for (cached_idx = 0; cached_idx < PMKID_NO; cached_idx++) {
					if (psaved_pmk_lock)
						NdisAcquireSpinLock(psaved_pmk_lock);

					if (psaved_pmk[cached_idx].Valid == TRUE &&
						(psaved_pmk[cached_idx].akm == akm)) {
						psaved_pmk[cached_idx].Valid = FALSE;
						psaved_pmk[cached_idx].akm = 0;

						NdisZeroMemory(&psaved_pmk[cached_idx].PMKID, LEN_PMKID);
						NdisZeroMemory(&psaved_pmk[cached_idx].ssid, MAX_LEN_OF_SSID);
						NdisZeroMemory(&psaved_pmk[cached_idx].PMK, LEN_MAX_PMK);

						if (*psaved_pmk_num)
							*psaved_pmk_num = *psaved_pmk_num - 1;
					}

					if (psaved_pmk_lock)
						NdisReleaseSpinLock(psaved_pmk_lock);
				}
			}
		}
	}
#endif
}


VOID sta_delete_psk_pmkid_cache_all(
	IN  PRTMP_ADAPTER   pAd,
	IN UCHAR if_index)
{
#ifdef MAC_REPEATER_SUPPORT
	INT cli_idx = 0;
#endif	/* MAC_REPEATER_SUPPORT */
	INT cached_idx;
	PBSSID_INFO psaved_pmk = NULL;
	PSTA_ADMIN_CONFIG papcli_entry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY preptcli_entry = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
#if defined(DOT11_SAE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
	SAE_INSTANCE *pSaeIns = NULL;
	SAE_CFG *pSaeCfg = NULL;
	UINT32 i;
	UINT32 ins_cnt = 0;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);
#endif

	VOID *psaved_pmk_lock = NULL;
	PUINT psaved_pmk_num = NULL;
	papcli_entry = &pAd->StaCfg[if_index];
	psaved_pmk = (PBSSID_INFO)&papcli_entry->SavedPMK[0];
	psaved_pmk_num = &papcli_entry->SavedPMKNum;
	psaved_pmk_lock = (VOID *)&papcli_entry->SavedPMK_lock;

#if defined(DOT11_SAE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
	pSaeCfg = PD_GET_SAE_CFG_PTR(pAd->physical_dev);
	/*Delete all SAE instances for this ApCli Interface*/
	NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

	for (i = 0; i < wtbl_max_num; i++) {
		if (pSaeCfg->sae_ins[i].valid == FALSE)
			continue;

		if (RTMPEqualMemory(pSaeCfg->sae_ins[i].own_mac, papcli_entry->wdev.if_addr, MAC_ADDR_LEN)) {
			pSaeIns = &pSaeCfg->sae_ins[i];
			if (pSaeIns != NULL && (pSaeIns->valid == TRUE)) {
				NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
				delete_sae_instance(pSaeIns);
				NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);
			}
		}
		ins_cnt++;
		if (ins_cnt == pSaeCfg->total_ins)
			break;
	}

	NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
#endif
	/*Delete ApCli PMKID list*/
	for (cached_idx = 0; cached_idx < PMKID_NO; cached_idx++) {
		if (psaved_pmk_lock)
			NdisAcquireSpinLock(psaved_pmk_lock);

		if ((psaved_pmk[cached_idx].Valid == TRUE) &&
			(IS_AKM_PSK(psaved_pmk[cached_idx].akm)
				&& !IS_AKM_OWE(psaved_pmk[cached_idx].akm))) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMK, DBG_LVL_INFO,
					":"MACSTR" cache(%d),akm:0x%x,SSID:%s\n",
					MAC2STR(psaved_pmk[cached_idx].BSSID),
					cached_idx, psaved_pmk[cached_idx].akm, psaved_pmk[cached_idx].ssid);
			psaved_pmk[cached_idx].Valid = FALSE;
			psaved_pmk[cached_idx].akm = 0;
			NdisZeroMemory(&psaved_pmk[cached_idx].PMKID, LEN_PMKID);
			NdisZeroMemory(&psaved_pmk[cached_idx].ssid, MAX_LEN_OF_SSID);
			NdisZeroMemory(&psaved_pmk[cached_idx].PMK, LEN_MAX_PMK);

			if (*psaved_pmk_num)
				*psaved_pmk_num = *psaved_pmk_num - 1;
		}
		if (psaved_pmk_lock)
			NdisReleaseSpinLock(psaved_pmk_lock);

	}
	/* Delete  PMKID list for MacRepeater linked with ApCli */

#ifdef MAC_REPEATER_SUPPORT
	if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
		for (cli_idx = 0; cli_idx < GET_MAX_REPEATER_ENTRY_NUM(cap); cli_idx++) {
			preptcli_entry = &pAd->ApCfg.pRepeaterCliPool[cli_idx];

			if (preptcli_entry && (preptcli_entry->CliValid == TRUE)) {
#if defined(DOT11_SAE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
				/* Delete all SAE instances for this Rept entry */
				NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);
				ins_cnt = 0;

				for (i = 0; i < wtbl_max_num; i++) {
					if (pSaeCfg->sae_ins[i].valid == FALSE)
						continue;

					if (RTMPEqualMemory(pSaeCfg->sae_ins[i].own_mac, preptcli_entry->CurrentAddress, MAC_ADDR_LEN)) {
						pSaeIns = &pSaeCfg->sae_ins[i];
						if (pSaeIns != NULL && (pSaeIns->valid == TRUE)) {
							NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
							delete_sae_instance(pSaeIns);
							NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

						}
					}

					ins_cnt++;

					if (ins_cnt == pSaeCfg->total_ins)
						break;
				}

				NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
#endif
				psaved_pmk = (PBSSID_INFO)&preptcli_entry->SavedPMK[0];
				psaved_pmk_num = &preptcli_entry->SavedPMKNum;
				psaved_pmk_lock = (VOID *)&preptcli_entry->SavedPMK_lock;


				for (cached_idx = 0; cached_idx < PMKID_NO; cached_idx++) {
					if (psaved_pmk_lock)
						NdisAcquireSpinLock(psaved_pmk_lock);

					if (psaved_pmk[cached_idx].Valid == TRUE &&
						(IS_AKM_PSK(psaved_pmk[cached_idx].akm)
							&& !IS_AKM_OWE(psaved_pmk[cached_idx].akm))) {
						psaved_pmk[cached_idx].Valid = FALSE;
						psaved_pmk[cached_idx].akm = 0;

						NdisZeroMemory(&psaved_pmk[cached_idx].PMKID, LEN_PMKID);
						NdisZeroMemory(&psaved_pmk[cached_idx].ssid, MAX_LEN_OF_SSID);
						NdisZeroMemory(&psaved_pmk[cached_idx].PMK, LEN_MAX_PMK);

						if (*psaved_pmk_num)
							*psaved_pmk_num = *psaved_pmk_num - 1;
					}

					if (psaved_pmk_lock)
						NdisReleaseSpinLock(psaved_pmk_lock);
				}
			}
		}
	}
#endif
}
#endif

#ifdef CONFIG_OWE_SUPPORT
VOID sta_reset_owe_parameters(
	IN  PRTMP_ADAPTER   pAd,
	IN UCHAR if_index)
{
	PSTA_ADMIN_CONFIG papcli_entry = NULL;
	papcli_entry = &pAd->StaCfg[if_index];

	/*OWE Trans reset the OWE trans bssid and ssid*/

	if (papcli_entry
		&& IS_AKM_OWE(papcli_entry->wdev.SecConfig.AKMMap)
		&& (papcli_entry->owe_trans_ssid_len > 0)) {
		NdisZeroMemory(papcli_entry->owe_trans_bssid, MAC_ADDR_LEN);
		NdisZeroMemory(papcli_entry->owe_trans_ssid, MAX_LEN_OF_SSID);
		papcli_entry->owe_trans_ssid_len = 0;

		NdisZeroMemory(papcli_entry->owe_trans_open_bssid, MAC_ADDR_LEN);
		NdisZeroMemory(papcli_entry->owe_trans_open_ssid, MAX_LEN_OF_SSID);
		papcli_entry->owe_trans_open_ssid_len = 0;
	}
}
#ifndef RT_CFG80211_SUPPORT
BOOLEAN sta_handle_owe_trans(
			IN  PRTMP_ADAPTER   pAd,
			IN struct wifi_dev *wdev,
			IN BSS_ENTRY *pInBss
			)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	if (!pStaCfg)
		return FALSE;
	if (IS_AKM_OWE(wdev->SecConfig.AKMMap) && pInBss->owe_trans_ie_len > 0) {
		UCHAR pair_ch = 0;
		UCHAR pair_bssid[MAC_ADDR_LEN] = {0};
		UCHAR pair_ssid[MAX_LEN_OF_SSID] = {0};
		UCHAR pair_band = 0;
		UCHAR pair_ssid_len = 0;
		extract_pair_owe_bss_info(pInBss->owe_trans_ie, pInBss->owe_trans_ie_len, pair_bssid, pair_ssid, &pair_ssid_len, &pair_ch);
		if (pInBss->RsnIE.IELen == 0) {
			UCHAR ch_band = wlan_config_get_ch_band(wdev);

			if (((ch_band == CMD_CH_BAND_24G) && (pair_ch <= 14))
				|| ((ch_band == CMD_CH_BAND_5G) && (pair_ch > 14))
				|| (ch_band == CMD_CH_BAND_6G)) {
				if (pair_ch != 0) {
					if (pair_ch != pStaCfg->wdev.channel) {
						wext_send_owe_trans_chan_event(pStaCfg->wdev.if_dev,
									OID_802_11_OWE_EVT_SAME_BAND_DIFF_CHANNEL,
									pair_bssid,
									pair_ssid,
									&pair_ssid_len,
									&pair_band,
									&pair_ch);
						rtmp_set_channel(pAd, wdev, pair_ch);
					}
				}
			} else {
				if (pair_ch != 0) {
					wext_send_owe_trans_chan_event(pStaCfg->wdev.if_dev,
									OID_802_11_OWE_EVT_DIFF_BAND,
									pair_bssid,
									pair_ssid,
									&pair_ssid_len,
									&pair_band,
									&pair_ch);
					rtmp_set_channel(pAd, wdev, pair_ch);
				}
			}
			NdisMoveMemory(&pStaCfg->owe_trans_bssid, pair_bssid, MAC_ADDR_LEN);
			if (pair_ssid_len <= sizeof(pStaCfg->owe_trans_ssid)) {
				NdisMoveMemory(&pStaCfg->owe_trans_ssid, pair_ssid, pair_ssid_len);
				pStaCfg->owe_trans_ssid_len = pair_ssid_len;
			} else {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
						"No enough buffer size for pStaCfg->owe_trans_ssid!\n");
			}
			NdisMoveMemory(&pStaCfg->owe_trans_open_bssid, pInBss->Bssid, MAC_ADDR_LEN);
			if (pInBss->SsidLen <= sizeof(pStaCfg->owe_trans_open_ssid)) {
				NdisMoveMemory(&pStaCfg->owe_trans_open_ssid, pInBss->Ssid, pInBss->SsidLen);
				pStaCfg->owe_trans_open_ssid_len = pInBss->SsidLen;
			} else {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
						"No enough buffer size for pStaCfg->owe_trans_open_ssid!\n");
			}
			CLEAR_SEC_AKM(pStaCfg->MlmeAux.AKMMap);
			CLEAR_CIPHER(pStaCfg->MlmeAux.PairwiseCipher);
			CLEAR_CIPHER(pStaCfg->MlmeAux.GroupCipher);
			return TRUE;
		} else {
			if (NdisEqualMemory(pStaCfg->owe_trans_open_bssid, pair_bssid, MAC_ADDR_LEN)) {
				if (NdisEqualMemory(pStaCfg->owe_trans_open_ssid, pair_ssid, pStaCfg->owe_trans_open_ssid_len)) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OWE,
						DBG_LVL_INFO, "Sanity Check Pass,BSS Parameters and Current Open parameters in Owe Trans IE match\n");
					return FALSE;
				} else {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OWE,
						DBG_LVL_ERROR, "Sanity Failed Stored Open BSS Parameters and Current Open parameters in Owe Trans IE don't match\n");
					return TRUE;
					}
			}
		}
	}
	return FALSE;
}
#endif /* RT_CFG80211_SUPPORT */
#endif
#ifdef APCLI_SUPPORT
INT Set_apcli_ocv_support_proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR apcli_ocv_support = 0;
	UINT32 staidx = 0;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
				"pObj->ioctl_if is invalid value\n");
		return FALSE;
	}

	staidx = pObj->ioctl_if;

	apcli_ocv_support = os_str_tol(arg, 0, 10);

	pAd->StaCfg[staidx].wdev.SecConfig.apcli_ocv_support = apcli_ocv_support;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_INFO,
		"[ApCli OCV] apcli_ocv_support:%d\n", apcli_ocv_support);
	return TRUE;
}
#ifdef APCLI_RANDOM_MAC_SUPPORT
BOOLEAN apcli_set_random_mac_addr(
	struct wifi_dev *wdev,
	UCHAR isJoin)
{
	RTMP_ADAPTER *pAd;
	UCHAR ifIndex = wdev->func_idx;
	INT apcli_wcid;
	UCHAR seq_num;

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	if (pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_AFC, DBG_LVL_ERROR,
			": pAd is NULL!\n");
		return FALSE;
	}
	apcli_wcid = 0;
	seq_num = pAd->StaCfg[ifIndex].apcli_seq_num;

	if (!isJoin) {
		if (wifi_sys_close(wdev) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"PE: wifi_sys_close fail!!!\n");
			return FALSE;
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
				"PE: updating current mac addr to random and Seq_num %d\n", seq_num);

		pAd->ApcliAddr[ifIndex][0] = RandomByte(pAd);
		pAd->ApcliAddr[ifIndex][1] = RandomByte(pAd);
		pAd->ApcliAddr[ifIndex][2] = RandomByte(pAd);
		pAd->ApcliAddr[ifIndex][3] = RandomByte(pAd);
		pAd->ApcliAddr[ifIndex][4] = RandomByte(pAd);
		pAd->ApcliAddr[ifIndex][5] = RandomByte(pAd);

		pAd->ApcliAddr[ifIndex][0] &= 0xfe;
		pAd->ApcliAddr[ifIndex][0] |= 0x02;
		if (wifi_sys_open(wdev) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"PE: wifi_sys_open fail!!!\n");
			return FALSE;
		}
		/* Set Sequence number , either random or reset SN*/
		UniCmdStaSNSet(pAd, apcli_wcid, seq_num);
	}
	return TRUE;
}
#endif

#endif
