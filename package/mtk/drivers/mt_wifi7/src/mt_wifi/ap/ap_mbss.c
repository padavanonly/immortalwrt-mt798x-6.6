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

    Abstract:

    Support multi-BSS function.

    Note:
    1. Call RT28xx_MBSS_Init() in init function and
       call RT28xx_MBSS_Remove() in close function

    2. MAC of different BSS is initialized in APStartUp()

    3. BSS Index (0 ~ 15) of different rx packet is got in

    4. BSS Index (0 ~ 15) of different tx packet is assigned in

    5. BSS Index (0 ~ 15) of different BSS is got in tx_pkt_handle() by using

    6. BSS Index (0 ~ 15) of IOCTL command is put in pAd->OS_Cookie->ioctl_if

    7. Beacon of different BSS is enabled in APMakeAllBssBeacon() by writing 1
       to the register MAC_BSSID_DW1

    8. The number of MBSS can be 1, 2, 4, or 8

***************************************************************************/
#ifdef MBSS_SUPPORT


#include "rt_config.h"

extern struct wifi_dev_ops ap_wdev_ops;

/*
 * create and initialize virtual network interfaces
 */
VOID mbss_create_vif(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps, INT32 IdBss)
{
	PNET_DEV pDevNew;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	struct wifi_dev *wdev;
	UINT32 MC_RowID = 0, IoctlIF = 0;
	char *dev_name = NULL;
	INT32 Ret;
	UCHAR ifidx = IdBss;
	UCHAR final_name[32] = "";
	BOOLEAN autoSuffix = TRUE;
	int ret;


	dev_name = get_dev_name_prefix(pAd, INT_MBSSID);

	if (dev_name == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"get_dev_name_prefix error!\n");
		return;
	}

	ret = snprintf(final_name, sizeof(final_name), "%s", dev_name);
	if (os_snprintf_error(sizeof(final_name), ret)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		return;
	}

	pDevNew = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_MBSSID, ifidx,
					 sizeof(struct mt_dev_priv), final_name, autoSuffix);

	if (pDevNew == NULL) {
		pAd->ApCfg.BssidNum = IdBss; /* re-assign new MBSS number */
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"pDevNew == NULL!, Re-assign pAd->ApCfg.BssidNum(=%d)\n",
			pAd->ApCfg.BssidNum);
		return;
	}
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_INFO,
		"Register MBSSID IF %d (%s)\n", IdBss, RTMP_OS_NETDEV_GET_DEVNAME(pDevNew));

	if (!VALID_MBSS(pAd, IdBss))
		return;

	wdev = &pAd->ApCfg.MBSSID[IdBss].wdev;
	Ret = wdev_init(pAd, wdev, WDEV_TYPE_AP, pDevNew, IdBss,
					(VOID *)&pAd->ApCfg.MBSSID[IdBss], (void *)pAd);

	if (!Ret) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
				 "Assign wdev idx for %s failed, free net device!\n",
				 RTMP_OS_NETDEV_GET_DEVNAME(pDevNew));
		RtmpOSNetDevFree(pDevNew);
		return;
	}

	Ret = wdev_ops_register(wdev, WDEV_TYPE_AP, &ap_wdev_ops,
							cap->qos.wmm_detect_method);

	if (!Ret) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
				 "register wdev_ops %s failed, free net device!\n",
				  RTMP_OS_NETDEV_GET_DEVNAME(pDevNew));
		RtmpOSNetDevFree(pDevNew);
		return;
	}

	RTMP_OS_NETDEV_SET_PRIV(pDevNew, pAd);
	RTMP_OS_NETDEV_SET_WDEV(pDevNew, wdev);
	/* init operation functions and flags */
	NdisCopyMemory(&netDevHook, pNetDevOps, sizeof(netDevHook));
	netDevHook.priv_flags = INT_MBSSID;
	netDevHook.needProtcted = TRUE;
	netDevHook.wdev = wdev;
	/* Init MAC address of virtual network interface */
	NdisMoveMemory(&netDevHook.devAddr[0], &wdev->bssid[0], MAC_ADDR_LEN);

#ifdef RT_CFG80211_SUPPORT
	{
		struct wireless_dev *pWdev;
		CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
		UINT32 DevType = RT_CMD_80211_IFTYPE_AP;

		pWdev = &wdev->cfg80211_wdev;
		os_zero_mem(pWdev, sizeof(struct wireless_dev));
		if (pWdev && pDevNew) {
			pDevNew->ieee80211_ptr = pWdev;
			pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
#if (KERNEL_VERSION(4, 0, 0) <= LINUX_VERSION_CODE)
#ifdef WIFI_IAP_IW_SET_CHANNEL_FEATURE
			pWdev->wiphy->features |= NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE;
#endif/*WIFI_IAP_IW_SET_CHANNEL_FEATURE*/
#endif/*KERNEL_VERSION(4, 0, 0) */
			SET_NETDEV_DEV(pDevNew, wiphy_dev(pWdev->wiphy));
			pWdev->netdev = pDevNew;
			pWdev->iftype = DevType;
		}
	}
#endif /* RT_CFG80211_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_TURNKEY_ENABLE(pAd)) {
			if (wdev->wdev_type == WDEV_TYPE_AP)
				map_make_vend_ie(pAd, IdBss);
		}
#endif /* CONFIG_MAP_SUPPORT */

	/* register this device to OS */
	if (RtmpOSNetDevAttach(pAd->OpMode, pDevNew, &netDevHook) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
				 "create IF %d (%s) failed!!\n",
				 IdBss, RTMP_OS_NETDEV_GET_DEVNAME(pDevNew));
	}


}

/*
 * ========================================================================
 * Routine Description:
 *     Initialize Multi-BSS function.
 *
 * Arguments:
 *     pAd			points to our adapter
 *     pDevMain		points to the main BSS network interface
 *
 * Return Value:
 *     None
 *
 * Note:
 *   1. Only create and initialize virtual network interfaces.
 *   2. No main network interface here.
 *   3. If you down ra0 and modify the BssNum of RT2860AP.dat/RT2870AP.dat,
 *      it will not work! You must rmmod rt2860ap.ko and lsmod rt2860ap.ko again.
 * ========================================================================
 */
VOID MBSS_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps)
{
	INT32 IdBss, MaxNumBss;
	UCHAR tx_stream;
	UCHAR rx_stream;
	struct wifi_dev *dev1;
	struct wifi_dev *dev2 = &pAd->ApCfg.MBSSID[0].wdev;


	/* max bss number */
	MaxNumBss = pAd->ApCfg.BssidNum;
	if (!VALID_MBSS(pAd, MaxNumBss))
		MaxNumBss = MAX_MBSSID_NUM(pAd);

	/* sanity check to avoid redundant virtual interfaces are created */
	if (pAd->FlgMbssInit == TRUE)
		return;

	/* first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
	for (IdBss = FIRST_MBSSID; IdBss < MaxNumBss; IdBss++) {
		dev1 = &pAd->ApCfg.MBSSID[IdBss].wdev;
		dev1->if_dev = NULL;
		dev1->bcn_buf.BeaconPkt = NULL;

		// update mbss stream from main wdev
#ifdef DOT11_EHT_BE
		tx_stream = wlan_config_get_eht_tx_nss(dev2);
		wlan_config_set_eht_tx_nss(dev1,
			min(wlan_config_get_eht_tx_nss(dev1), tx_stream));
		rx_stream = wlan_config_get_eht_rx_nss(dev2);
		wlan_config_set_eht_rx_nss(dev1,
			min(wlan_config_get_eht_rx_nss(dev1), rx_stream));
#endif /*DOT11_EHT_BE*/
#ifdef DOT11_HE_AX
		tx_stream = wlan_config_get_he_tx_nss(dev2);
		wlan_config_set_he_tx_nss(dev1,
			min(wlan_config_get_he_tx_nss(dev1), tx_stream));
		rx_stream = wlan_config_get_he_rx_nss(dev2);
		wlan_config_set_he_rx_nss(dev1,
			min(wlan_config_get_he_rx_nss(dev1), rx_stream));
#endif

		/* create virtual network interface */
		mbss_create_vif(pAd, pNetDevOps, IdBss);
#ifdef DOT11_EHT_BE
		MBSS_Reconfig_Init(pAd, &pAd->ApCfg.MBSSID[IdBss]);
#endif
	}

	pAd->FlgMbssInit = TRUE;
}


/*
========================================================================
Routine Description:
    Remove Multi-BSS network interface.

Arguments:
	pAd			points to our adapter

Return Value:
    None

Note:
    FIRST_MBSSID = 1
    Main BSS is not removed here.
========================================================================
*/
VOID MBSS_Remove(RTMP_ADAPTER *pAd)
{
	struct wifi_dev *wdev;
	UINT IdBss;
	BSS_STRUCT *pMbss;
	INT32 MaxNumBss;

	if (!pAd)
		return;

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_INFO, "(caller:%pS):\n", OS_TRACE);
	MaxNumBss = pAd->ApCfg.BssidNum;

	if (!VALID_MBSS(pAd, MaxNumBss))
		MaxNumBss = MAX_MBSSID_NUM(pAd);


	for (IdBss = FIRST_MBSSID; IdBss < MaxNumBss; IdBss++) {
		wdev = &pAd->ApCfg.MBSSID[IdBss].wdev;
		pMbss = &pAd->ApCfg.MBSSID[IdBss];

		if (pMbss)
			bcn_buf_deinit(pAd, wdev);

#ifdef DOT11_EHT_BE
		MBSS_Reconfig_Deinit(pAd, &pAd->ApCfg.MBSSID[IdBss]);
#endif

		if (wdev->if_dev) {
			RtmpOSNetDevProtect(1);
			RtmpOSNetDevDetach(wdev->if_dev);
			RtmpOSNetDevProtect(0);
			wdev_deinit(pAd, wdev);
#ifndef RT_CFG80211_SUPPORT
			RtmpOSNetDevFree(wdev->if_dev);
#endif /* !RT_CFG80211_SUPPORT */
			wdev->if_dev = NULL;
		}
	}
}
#ifdef ACTION_MONITOR_SUPPORT
VOID Action_Monitor_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps)
{
	PNET_DEV new_dev_p = NULL;
	INT idx = 0;
	INT ret;
	struct wifi_dev *wdev;
	UINT32 MC_RowID = 0, IoctlIF = 0;
	char *dev_name;

	if (pAd->bActionMonInit != FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_MISC, DBG_LVL_WARN, "Interface exists\n");
		return;
	}
	dev_name = get_dev_name_prefix(pAd, INT_ACT_MONITOR);
	new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_ACT_MONITOR, idx,
					sizeof(struct mt_dev_priv), dev_name, TRUE);
	if (!new_dev_p) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_MISC, DBG_LVL_ERROR,
				 "Create net_device for %s(%d) fail!\n", dev_name, idx);
		return;
	}
	wdev = &pAd->act_mon_wdev;
	wdev->sys_handle = (void *)pAd;
	wdev->if_dev = new_dev_p;
	RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
	RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
	COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);
	pNetDevOps->priv_flags = INT_ACT_MONITOR; /* we are virtual interface */
	pNetDevOps->needProtcted = TRUE;
	pNetDevOps->wdev = wdev;
	NdisMoveMemory(pNetDevOps->devAddr, &wdev->if_addr[0], MAC_ADDR_LEN);
	ret = RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, pNetDevOps);
	if (ret == 0) {
		pAd->bActionMonInit = TRUE;
		pAd->bActionMonOn = FALSE;
	} else
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_MISC, DBG_LVL_ERROR, "net dev attach fail\n");
}
VOID Action_Monitor_Remove(RTMP_ADAPTER *pAd)
{
	struct wifi_dev *wdev;

	wdev = &pAd->act_mon_wdev;
	if (wdev->if_dev) {
		RtmpOSNetDevProtect(1);
		RtmpOSNetDevDetach(wdev->if_dev);
		RtmpOSNetDevProtect(0);
		wdev_idx_unreg(pAd, wdev);
		RtmpOSNetDevFree(wdev->if_dev);
		wdev->if_dev = NULL;
		pAd->bActionMonInit = FALSE;
	}
}
BOOLEAN Action_Monitor_Open(RTMP_ADAPTER *pAd, PNET_DEV dev_p)
{
	if (pAd->act_mon_wdev.if_dev == dev_p)
		pAd->bActionMonOn = TRUE;
	RT_MOD_INC_USE_COUNT();
	RTMP_OS_NETDEV_START_QUEUE(dev_p);
	return TRUE;
}
BOOLEAN Action_Monitor_Close(RTMP_ADAPTER *pAd, PNET_DEV dev_p)
{
	if (pAd->act_mon_wdev.if_dev == dev_p)
		pAd->bActionMonOn = FALSE;
	RT_MOD_DEC_USE_COUNT();
	return TRUE;
}
#endif
/*
========================================================================
Routine Description:
    Get multiple bss idx.

Arguments:
	pAd				points to our adapter
	pDev			which WLAN network interface

Return Value:
    0: close successfully
    otherwise: close fail

Note:
========================================================================
*/
INT32 RT28xx_MBSS_IdxGet(RTMP_ADAPTER *pAd, PNET_DEV pDev)
{
	INT32 BssId = -1;
	INT32 IdBss;

	if (!pAd || !pDev)
		return -1;

	for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
		if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev == pDev) {
			BssId = IdBss;
			break;
		}
	}

	return BssId;
}

#ifdef MT_MAC
INT32 ext_mbss_hw_cr_enable(PNET_DEV pDev)
{
	PRTMP_ADAPTER pAd;
	INT BssId;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	BssId = RT28xx_MBSS_IdxGet(pAd, pDev);
	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_INFO, "#####BssId = %d\n", BssId);

	if (BssId < 0)
		return -1;

	if (!IS_HIF_TYPE(pAd, HIF_MT))
		return 0;

	AsicSetExtMbssEnableCR(pAd, BssId, TRUE);/* enable rmac 0_1~0_15 bit */
	AsicSetMbssHwCRSetting(pAd, BssId, TRUE);/* enable lp timing setting for 0_1~0_15 */
	return 0;
}


INT ext_mbss_hw_cr_disable(PNET_DEV pDev)
{
	PRTMP_ADAPTER pAd;
	INT BssId;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	BssId = RT28xx_MBSS_IdxGet(pAd, pDev);

	if (BssId < 0)
		return -1;

	if (!IS_HIF_TYPE(pAd, HIF_MT))
		return 0;

	AsicSetMbssHwCRSetting(pAd, BssId, FALSE);
	AsicSetExtMbssEnableCR(pAd, BssId, FALSE);
	return 0;
}
#endif /* MT_MAC */

#ifdef DOT11_EHT_BE
static VOID MBSS_Reconfig_TO(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3)
{
	BSS_STRUCT *pMbss;
	RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;

	pMbss = (BSS_STRUCT *)FunctionContext;
	wdev = &pMbss->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	if (!pMbss->reconfig_state_init)
		return;
	OS_SEM_LOCK(&pMbss->reconfig_state_lock);
	switch (pMbss->reconfig_state) {
	case BSS_RECONFIG_IDLE_STAGE:
		OS_SEM_UNLOCK(&pMbss->reconfig_state_lock);
		MBSS_Reconfig_State_transition(pAd, pMbss, BSS_RECONFIG_COUNTDOWN_STAGE, 0);
		break;
	case BSS_RECONFIG_COUNTDOWN_STAGE:
		OS_SEM_UNLOCK(&pMbss->reconfig_state_lock);
		MBSS_Reconfig_State_transition(pAd, pMbss, BSS_RECONFIG_COUNTDOWN_STAGE_END, 0);
		break;
	case BSS_RECONFIG_COUNTDOWN_STAGE_END:
		OS_SEM_UNLOCK(&pMbss->reconfig_state_lock);
		MBSS_Reconfig_State_transition(pAd, pMbss, BSS_RECONFIG_BTM_DISASSOC_STAGE, 0);
		break;
	case BSS_RECONFIG_BTM_DISASSOC_STAGE:
		OS_SEM_UNLOCK(&pMbss->reconfig_state_lock);
		MBSS_Reconfig_State_transition(pAd, pMbss, BSS_RECONFIG_BTM_DISASSOC_STAGE_END, 0);
		break;
	case BSS_RECONFIG_BTM_DISASSOC_STAGE_END:
		OS_SEM_UNLOCK(&pMbss->reconfig_state_lock);
		MBSS_Reconfig_State_transition(pAd, pMbss, BSS_RECONFIG_BTM_TERMINATE_STAGE, 0);
		break;
	case BSS_RECONFIG_BTM_TERMINATE_STAGE:
		OS_SEM_UNLOCK(&pMbss->reconfig_state_lock);
		MBSS_Reconfig_State_transition(pAd, pMbss, BSS_RECONFIG_BTM_TERMINATE_STAGE_END, 0);
		break;
	case BSS_RECONFIG_BTM_TERMINATE_STAGE_END:
		OS_SEM_UNLOCK(&pMbss->reconfig_state_lock);
		MBSS_Reconfig_State_transition(pAd, pMbss, BSS_RECONFIG_IDLE_STAGE, 0);
		break;
	}
}

BUILD_TIMER_FUNCTION(MBSS_Reconfig_TO);

VOID MBSS_Reconfig_Init(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	if (pMbss->reconfig_state_init)
		return;
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO,
		"\x1b[42mpMbss->wdev.wdev_idx=%d\x1b[m\n\r", pMbss->wdev.wdev_idx);

	NdisAllocateSpinLock(pAd, &pMbss->reconfig_state_lock);
	OS_SEM_LOCK(&pMbss->reconfig_state_lock);
	pMbss->reconfig_state = BSS_RECONFIG_IDLE_STAGE;
	RTMPInitTimer(pAd, &pMbss->btm_timer,
		GET_TIMER_FUNCTION(MBSS_Reconfig_TO), pMbss, FALSE);
	pMbss->reconfig_state_bitmap_bypass =
		(1 << BSS_RECONFIG_BTM_DISASSOC_STAGE) |
		(1 << BSS_RECONFIG_BTM_DISASSOC_STAGE_END) |
		(1 << BSS_RECONFIG_BTM_TERMINATE_STAGE) |
		(1 << BSS_RECONFIG_BTM_TERMINATE_STAGE_END);
	OS_SEM_UNLOCK(&pMbss->reconfig_state_lock);

	pMbss->reconfig_state_init = TRUE;
}

VOID MBSS_Reconfig_Deinit(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	BOOLEAN Cancelled;

	if (!pMbss->reconfig_state_init)
		return;
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO,
		"\x1b[42mpMbss->wdev.wdev_idx=%d\x1b[m\n\r", pMbss->wdev.wdev_idx);
	RTMPReleaseTimer(&pMbss->btm_timer, &Cancelled);
	NdisFreeSpinLock(&pMbss->reconfig_state_lock);

	pMbss->reconfig_state_init = FALSE;
}

BOOLEAN MBSS_Reconfig_Is_Idle(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	enum bss_reconfig_state orig_reconfig_state;

	OS_SEM_LOCK(&pMbss->reconfig_state_lock);
	orig_reconfig_state = pMbss->reconfig_state;
	OS_SEM_UNLOCK(&pMbss->reconfig_state_lock);

	return (orig_reconfig_state == BSS_RECONFIG_IDLE_STAGE) ? TRUE : FALSE;
}

VOID MBSS_Reconfig_State_transition(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, enum bss_reconfig_state next_state, UINT timeout)
{
	BOOLEAN Cancelled;
	enum bss_reconfig_state orig_reconfig_state;
	struct wifi_dev *wdev;

	/* sanity check to avoid redundant virtual interfaces are created */
	if (!pAd || !pMbss || !pMbss->reconfig_state_init) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_ERROR,
			"\x1b[41m[%s][%d]pAd or pMbss is null\x1b[m\n\r", __func__, __LINE__);
		return;
	}
	wdev = &pMbss->wdev;
	OS_SEM_LOCK(&pMbss->reconfig_state_lock);
	orig_reconfig_state = pMbss->reconfig_state;
	while (pMbss->reconfig_state_bitmap_bypass & (1 << next_state)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO,
			"\x1b[42m (%s)pMbss->reconfig_state=%d,bypass=0x%x, next_state=%d \x1b[m\n\r",
			RtmpOsGetNetDevName(wdev->if_dev), pMbss->reconfig_state,
			pMbss->reconfig_state_bitmap_bypass, next_state);
		pMbss->reconfig_state = next_state;
		next_state = (next_state+1) % (BSS_RECONFIG_BTM_TERMINATE_STAGE_END+1);
	}

	switch (pMbss->reconfig_state) {
	case BSS_RECONFIG_IDLE_STAGE:
		if ((next_state == BSS_RECONFIG_COUNTDOWN_STAGE) ||
			(next_state == BSS_RECONFIG_COUNTDOWN_STAGE_END))
			pMbss->reconfig_state = next_state;
		else
			goto fail_state;
		RTMPCancelTimer(&pMbss->btm_timer, &Cancelled);
		MBSS_Reconfig_SM(pAd, wdev, pMbss->reconfig_state);
		RTMPSetTimer(&pMbss->btm_timer, timeout+100);
		break;
	case BSS_RECONFIG_COUNTDOWN_STAGE:
		if (next_state == BSS_RECONFIG_COUNTDOWN_STAGE_END)
			pMbss->reconfig_state = next_state;
		else
			goto fail_state;
		RTMPCancelTimer(&pMbss->btm_timer, &Cancelled);
		MBSS_Reconfig_SM(pAd, wdev, pMbss->reconfig_state);
		RTMPSetTimer(&pMbss->btm_timer, timeout+100);
		break;
	case BSS_RECONFIG_COUNTDOWN_STAGE_END:
		if (next_state == BSS_RECONFIG_BTM_DISASSOC_STAGE)
			pMbss->reconfig_state = next_state;
		else
			goto fail_state;
		RTMPCancelTimer(&pMbss->btm_timer, &Cancelled);
		MBSS_Reconfig_SM(pAd, wdev, pMbss->reconfig_state);
		RTMPSetTimer(&pMbss->btm_timer, timeout+100);
		break;
	case BSS_RECONFIG_BTM_DISASSOC_STAGE:
		if (next_state == BSS_RECONFIG_BTM_DISASSOC_STAGE_END)
			pMbss->reconfig_state = next_state;
		else
			goto fail_state;
		RTMPCancelTimer(&pMbss->btm_timer, &Cancelled);
		MBSS_Reconfig_SM(pAd, wdev, pMbss->reconfig_state);
		RTMPSetTimer(&pMbss->btm_timer, timeout+100);
		break;
	case BSS_RECONFIG_BTM_DISASSOC_STAGE_END:
		if (next_state == BSS_RECONFIG_BTM_TERMINATE_STAGE)
			pMbss->reconfig_state = next_state;
		else
			goto fail_state;
		RTMPCancelTimer(&pMbss->btm_timer, &Cancelled);
		MBSS_Reconfig_SM(pAd, wdev, pMbss->reconfig_state);
		RTMPSetTimer(&pMbss->btm_timer, timeout+100);
		break;
	case BSS_RECONFIG_BTM_TERMINATE_STAGE:
		if (next_state == BSS_RECONFIG_BTM_TERMINATE_STAGE_END)
			pMbss->reconfig_state = next_state;
		else
			goto fail_state;
		RTMPCancelTimer(&pMbss->btm_timer, &Cancelled);
		MBSS_Reconfig_SM(pAd, wdev, pMbss->reconfig_state);
		RTMPSetTimer(&pMbss->btm_timer, timeout+100);
		break;
	case BSS_RECONFIG_BTM_TERMINATE_STAGE_END:
		if (next_state == BSS_RECONFIG_IDLE_STAGE)
			pMbss->reconfig_state = next_state;
		else
			goto fail_state;
		RTMPCancelTimer(&pMbss->btm_timer, &Cancelled);
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO,
			"\x1b[42mreturn to BSS_RECONFIG_IDLE_STAGE\x1b[m\n\r");
		break;
	}
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO,
		"\x1b[42m[Valid transition](%s)pMbss->reconfig_state = %d, next_state = %d, caller:%pS\x1b[m\n\r",
		RtmpOsGetNetDevName(wdev->if_dev), orig_reconfig_state, next_state, OS_TRACE);
	OS_SEM_UNLOCK(&pMbss->reconfig_state_lock);
#ifdef RT_CFG80211_SUPPORT
	mtk_cfg80211_event_reconf_sm(wdev->if_dev, next_state);
#endif
	return;
fail_state:
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_ERROR,
		"\x1b[41m[Invalid transition](%s)pMbss->reconfig_state = %d, next_state = %d\x1b[m\n\r",
		RtmpOsGetNetDevName(wdev->if_dev), orig_reconfig_state, next_state);
	OS_SEM_UNLOCK(&pMbss->reconfig_state_lock);
}

INT MBSS_Reconfig_flow_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BSS_STRUCT *pMbss;
	POS_COOKIE obj;
	UINT32 reconfig_flow;

	if (!pAd)
		goto err;

	obj = (POS_COOKIE) pAd->OS_Cookie;
	if ((obj->ioctl_if < 0) || (obj->ioctl_if >= MAX_BEACON_NUM))
		goto err;

	pMbss = &pAd->ApCfg.MBSSID[obj->ioctl_if];

	if (!pMbss || !pMbss->reconfig_state_init) {
		MTWF_PRINT("\x1b[41m pMbss is null\x1b[m\n\r");
		goto err;
	}
	reconfig_flow = os_str_tol(arg, 0, 10);
	MTWF_PRINT("\x1b[42m reconfig_flow = %d\x1b[m\n\r", reconfig_flow);

	if (reconfig_flow == 0)
		MBSS_Reconfig_State_transition(pAd, pMbss, BSS_RECONFIG_IDLE_STAGE, 0);
	else if (reconfig_flow == 1)
		MBSS_Reconfig_State_transition(pAd, pMbss, BSS_RECONFIG_COUNTDOWN_STAGE, 0);
	else if (reconfig_flow == 2)
		MBSS_Reconfig_State_transition(pAd, pMbss, BSS_RECONFIG_BTM_DISASSOC_STAGE, 0);
	else if (reconfig_flow == 3)
		MBSS_Reconfig_State_transition(pAd, pMbss, BSS_RECONFIG_BTM_TERMINATE_STAGE, 0);

	return TRUE;
err:
	return FALSE;
}

INT MBSS_Reconfig_flow_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BSS_STRUCT *pMbss;
	POS_COOKIE obj;

	if (!pAd)
		goto err;

	obj = (POS_COOKIE) pAd->OS_Cookie;
	if ((obj->ioctl_if < 0) || (obj->ioctl_if >= MAX_BEACON_NUM))
		goto err;

	pMbss = &pAd->ApCfg.MBSSID[obj->ioctl_if];

	if (!pMbss || !pMbss->reconfig_state_init) {
		MTWF_PRINT("\x1b[41m pMbss is null\x1b[m\n\r");
		goto err;
	}
	OS_SEM_LOCK(&pMbss->reconfig_state_lock);
	MTWF_PRINT("\x1b[42m pMbss->reconfig_state = %d\x1b[m\n\r", pMbss->reconfig_state);
	OS_SEM_UNLOCK(&pMbss->reconfig_state_lock);

	return TRUE;
err:
	return FALSE;
}

static VOID MBSS_Reconfig_Process_BTM_STA(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, enum bss_reconfig_state state)
{
	struct wifi_dev *wdev;

	/* sanity check to avoid redundant virtual interfaces are created */
	if (!pAd || !pMbss || !pMbss->reconfig_state_init) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_ERROR,
			"\x1b[41m[%s][%d]pAd or pMbss is null\x1b[m\n\r", __func__, __LINE__);
		return;
	}
	wdev = &pMbss->wdev;

	switch (state) {
	case BSS_RECONFIG_IDLE_STAGE:
		break;
	case BSS_RECONFIG_COUNTDOWN_STAGE:
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO,
			"\x1b[42m Send Reconf frame \x1b[m\n\r");
		break;
	case BSS_RECONFIG_BTM_DISASSOC_STAGE:
	/*MTI*/
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO,
			"\x1b[42m Send BTM req:disassoc (TBD)\x1b[m\n\r");
		break;
	case BSS_RECONFIG_BTM_TERMINATE_STAGE:
	/*MTI*/
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO,
			"\x1b[42m Send BTM req:terminate (TBD)\x1b[m\n\r");
		break;
	default:
		break;
	}
}

static VOID MBSS_Reconfig_Disconnect_STA(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, enum bss_reconfig_state state)
{
	UINT32 i;
	MAC_TABLE_ENTRY *pEntry;
	struct wifi_dev *wdev;

	/* sanity check to avoid redundant virtual interfaces are created */
	if (!pAd || !pMbss || !pMbss->reconfig_state_init) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_ERROR,
			"\x1b[41m[%s][%d]pAd or pMbss is null\x1b[m\n\r", __func__, __LINE__);
		return;
	}
	wdev = &pMbss->wdev;

	switch (state) {
	case BSS_RECONFIG_IDLE_STAGE:
		break;
	case BSS_RECONFIG_COUNTDOWN_STAGE_END:
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO,
			"\x1b[43m Remove Non-AP MLD STA\x1b[m\n\r");
		break;
	case BSS_RECONFIG_BTM_DISASSOC_STAGE_END:
	/*MTI*/
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO,
			"\x1b[43m Remove BTM STA (TBD)\x1b[m\n\r");
		break;
	case BSS_RECONFIG_BTM_TERMINATE_STAGE_END:
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO,
			"\x1b[43m Remove Legacy STA \x1b[m\n\r");
		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pEntry = entry_get(pAd, i);
			if (IS_ENTRY_MLO(pEntry))
				continue;
			if (pEntry && IS_ENTRY_CLIENT(pEntry) &&
				pEntry->wdev == wdev)
				APMlmeKickOutSta(pAd, pEntry->Addr, pEntry->wcid,
					REASON_DISASSOC_INACTIVE);
		}
		break;
	default:
		break;
	}
}

VOID MBSS_Reconfig_SM(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum bss_reconfig_state reconfig_state)
{
	UINT32 ret;
	struct MT_BSS_RECONF_SM rMtBssReconfSm;

	rMtBssReconfSm.wdev = wdev;
	rMtBssReconfSm.reconfig_state = (UINT32) reconfig_state;
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO,
		"\x1b[42mwdev(%d), Reconfg stat = %x\x1b[m\n\r",
		wdev->wdev_idx, reconfig_state);
	ret = RTEnqueueInternalCmd(pAd, CMDTHREAD_BSS_RECONF_SM,
		&rMtBssReconfSm, sizeof(struct MT_BSS_RECONF_SM));
}

NTSTATUS MBSS_Reconfig_SM_Handler(RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt)
{
	struct MT_BSS_RECONF_SM *prMtBssReconfSm = (struct MT_BSS_RECONF_SM *)CMDQelmt->buffer;
	struct wifi_dev *wdev = prMtBssReconfSm->wdev;
	UINT32 reconfig_state = prMtBssReconfSm->reconfig_state;

	switch (reconfig_state) {
	case BSS_RECONFIG_COUNTDOWN_STAGE:
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO,
			"\x1b[42mSend Reconf frame\x1b[m\n\r");
		break;
	case BSS_RECONFIG_COUNTDOWN_STAGE_END:
		/* done at eht_ap_mld_exec_link_reconfiguration */
		MBSS_Reconfig_Disconnect_STA(pAd, (BSS_STRUCT *) wdev->func_dev, BSS_RECONFIG_COUNTDOWN_STAGE_END);
		break;
	case BSS_RECONFIG_BTM_DISASSOC_STAGE:
		MBSS_Reconfig_Process_BTM_STA(pAd, (BSS_STRUCT *) wdev->func_dev, BSS_RECONFIG_BTM_DISASSOC_STAGE);
		break;
	case BSS_RECONFIG_BTM_DISASSOC_STAGE_END:
		MBSS_Reconfig_Disconnect_STA(pAd, (BSS_STRUCT *) wdev->func_dev, BSS_RECONFIG_BTM_DISASSOC_STAGE_END);
		break;
	case BSS_RECONFIG_BTM_TERMINATE_STAGE:
		MBSS_Reconfig_Process_BTM_STA(pAd, (BSS_STRUCT *) wdev->func_dev, BSS_RECONFIG_BTM_TERMINATE_STAGE);
		break;
	case BSS_RECONFIG_BTM_TERMINATE_STAGE_END:
		MBSS_Reconfig_Disconnect_STA(pAd, (BSS_STRUCT *) wdev->func_dev, BSS_RECONFIG_BTM_TERMINATE_STAGE_END);
		break;
	default:
		break;
	}
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO,
		"\x1b[43m(%s)reconfig_state=%d\x1b[m\n\r", RtmpOsGetNetDevName(wdev->if_dev), reconfig_state);

	return NDIS_STATUS_SUCCESS;
}
#endif

#endif /* MBSS_SUPPORT */


