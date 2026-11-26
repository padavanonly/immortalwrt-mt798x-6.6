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
	cfg80211_inf.c

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
	YF Luo		06-28-2012		Init version
			12-26-2013		Integration of NXTC
*/
#include "rt_config.h"
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"

#if defined(IWCOMMAND_CFG80211_SUPPORT) || defined(HOSTAPD_MBSS_SUPPORT)
extern struct wifi_dev_ops ap_wdev_ops;

VOID RTMP_CFG80211_AddVifEntry(VOID *pAdSrc, PNET_DEV pNewNetDev, UINT32 DevType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PCFG80211_VIF_DEV pNewVifDev = NULL;

	os_alloc_mem(NULL, (UCHAR **)&pNewVifDev, sizeof(CFG80211_VIF_DEV));

	if (pNewVifDev) {
		os_zero_mem(pNewVifDev, sizeof(CFG80211_VIF_DEV));
		pNewVifDev->pNext = NULL;
		pNewVifDev->net_dev = pNewNetDev;
		pNewVifDev->devType = DevType;
		os_zero_mem(pNewVifDev->CUR_MAC, MAC_ADDR_LEN);
		NdisCopyMemory(pNewVifDev->CUR_MAC, pNewNetDev->dev_addr, MAC_ADDR_LEN);
		NdisCopyMemory(pNewVifDev->ucfgIfName, pNewNetDev->name, sizeof(pNewNetDev->name));

		insertTailList(&pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList, (RT_LIST_ENTRY *)pNewVifDev);
		MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_INFO,
				"Add CFG80211 VIF Device, Type: %d.\n", pNewVifDev->devType);
	} else
		MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
				"Error in alloc mem in New CFG80211 VIF Function.\n");
}

static
PCFG80211_VIF_DEV RTMP_CFG80211_FindVifEntry_ByName(VOID *pAdSrc, PNET_DEV pNewNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while (pDevEntry != NULL) {

		if (!NdisCmpMemory(pDevEntry->net_dev->name, pNewNetDev->name, sizeof(pDevEntry->net_dev->name)))
			return pDevEntry;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	return NULL;
}

VOID RTMP_CFG80211_RemoveVifEntry(VOID *pAdSrc, PNET_DEV pNewNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = (RT_LIST_ENTRY *)RTMP_CFG80211_FindVifEntry_ByName(pAd, pNewNetDev);

	if (pListEntry) {
		delEntryList(&pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList, pListEntry);
		os_free_mem(pListEntry);
		pListEntry = NULL;
	} else
		MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
			"Error\n");
}
#endif /* IWCOMMAND_CFG80211_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
extern INT apcli_tx_pkt_allowed(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pkt);

#if defined(IWCOMMAND_CFG80211_SUPPORT) || defined(HOSTAPD_MBSS_SUPPORT)

PWIRELESS_DEV RTMP_CFG80211_FindVifEntryWdev_ByType(VOID *pAdSrc, UINT32 devType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while (pDevEntry != NULL) {
		if (pDevEntry->devType == devType)
			return pDevEntry->net_dev->ieee80211_ptr;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	return NULL;
}

PWIRELESS_DEV RTMP_CFG80211_FindVifEntryWdev_ByName(VOID *pAdSrc, CHAR ucIfName[])
{
		PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;

		PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
		PCFG80211_VIF_DEV pDevEntry = NULL;
		RT_LIST_ENTRY *pListEntry = NULL;

		pListEntry = pCacheList->pHead;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

		while (pDevEntry != NULL) {
			if (!strcmp(pDevEntry->ucfgIfName, ucIfName))
				return pDevEntry->net_dev->ieee80211_ptr;

			pListEntry = pListEntry->pNext;
			pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
		}
		return NULL;
}

BOOLEAN CFG80211DRV_OpsVifAdd(VOID *pAdOrg, VOID *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_VIF_SET *pVifInfo;
	INT32 MaxNumBss = 0;
	ULONG flags = 0;
	UINT32 max_num_sta = pAd->MaxMSTANum;
	BOOLEAN ret = FALSE;

	pVifInfo = (CMD_RTPRIV_IOCTL_80211_VIF_SET *)pData;

	if (pVifInfo->vifType == RT_CMD_80211_IFTYPE_AP) {
		MaxNumBss = pAd->ApCfg.BssidNum;
		if (!VALID_MBSS(pAd, MaxNumBss))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		OS_SPIN_LOCK_IRQSAVE(&pAd->CfgIfUseCntLock, &flags);
		if (pAd->CfgAPIfUseCnt >= MaxNumBss) {
			OS_SPIN_UNLOCK_IRQRESTORE(&pAd->CfgIfUseCntLock, &flags);
			MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
					"Error useCut %d > supportNum %d\n", pAd->CfgAPIfUseCnt, MaxNumBss);
			return FALSE;
		}
		pAd->CfgAPIfUseCnt++;
		OS_SPIN_UNLOCK_IRQRESTORE(&pAd->CfgIfUseCntLock, &flags);
		ret = RTMP_CFG80211_VirtualIF_Init(pAd, pVifInfo->vifName, pVifInfo->vifType, pVifInfo->flags, &pVifInfo->pWdev);
		if (ret != TRUE) {
			OS_SPIN_LOCK_IRQSAVE(&pAd->CfgIfUseCntLock, &flags);
			pAd->CfgAPIfUseCnt--;
			OS_SPIN_UNLOCK_IRQRESTORE(&pAd->CfgIfUseCntLock, &flags);
		}
	} else if (pVifInfo->vifType == RT_CMD_80211_IFTYPE_STATION) {
		max_num_sta = min(pAd->ApCfg.ApCliNum, (UCHAR)MAX_APCLI_NUM);
		OS_SPIN_LOCK_IRQSAVE(&pAd->CfgIfUseCntLock, &flags);
		if (pAd->CfgSTAIfUseCnt >= max_num_sta) {
			OS_SPIN_UNLOCK_IRQRESTORE(&pAd->CfgIfUseCntLock, &flags);
			MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
					"Error useCut %d > supportNum %d\n", pAd->CfgSTAIfUseCnt, max_num_sta);
			return FALSE;
		}
		pAd->CfgSTAIfUseCnt++;
		OS_SPIN_UNLOCK_IRQRESTORE(&pAd->CfgIfUseCntLock, &flags);
		ret = RTMP_CFG80211_VirtualIF_Init(pAd, pVifInfo->vifName, pVifInfo->vifType, pVifInfo->flags, &pVifInfo->pWdev);
		if (ret != TRUE) {
			OS_SPIN_LOCK_IRQSAVE(&pAd->CfgIfUseCntLock, &flags);
			pAd->CfgSTAIfUseCnt--;
			OS_SPIN_UNLOCK_IRQRESTORE(&pAd->CfgIfUseCntLock, &flags);
		}
	} else if (pVifInfo->vifType == RT_CMD_80211_IFTYPE_AP_VLAN) {
		MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_NOTICE,
			"VifType : AP_VLAN\n");
		ret = RTMP_CFG80211_VirtualIF_Init(pAd, pVifInfo->vifName, pVifInfo->vifType, pVifInfo->flags, &pVifInfo->pWdev);
		if (ret == TRUE)
			MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_NOTICE,
				"VLAN: interface creation success\n");
	}
	return ret;
}

NET_DEV_STATS *RT28xx_get_ether_stats(PNET_DEV net_dev);

BOOLEAN RTMP_CFG80211_VirtualIF_Init(
	IN VOID		*pAdSrc,
	IN CHAR * pDevName,
	IN UINT32                DevType,
	IN UINT32		flags,
	OUT VOID	**ppWdev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	RTMP_OS_NETDEV_OP_HOOK	netDevHook;
	PNET_DEV	new_dev_p;
	struct wifi_dev *wdev;
	UINT apidx = MAIN_MBSSID;
	CHAR preIfName[IFNAMSIZ];
	UINT devNameLen = strlen(pDevName);
	UINT preIfIndex = 1;
	INT32 IdBss, MaxNumBss;
	UINT32 sta_start_id = (MAIN_MSTA_ID + 1);
	UINT32 max_num_sta = pAd->MaxMSTANum;
	UINT32 inf_type = INT_MSTA;
	struct wireless_dev *pWdev;
	CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32 MC_RowID = 0, IoctlIF = 0;
	INT32 Ret;
	BOOLEAN retval = FALSE;


	memset(preIfName, 0, sizeof(preIfName));
	NdisCopyMemory(preIfName, pDevName, devNameLen);
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_INFO,
				"---> (%s, %s, %d)\n", pDevName, preIfName, preIfIndex);
	/* init operation functions and flags */
	switch (DevType) {

	case RT_CMD_80211_IFTYPE_AP:
			/* max bss number && avai apiadx */
			MaxNumBss = pAd->ApCfg.BssidNum;
			if (!VALID_MBSS(pAd, MaxNumBss))
				MaxNumBss = MAX_MBSSID_NUM(pAd);

			for (IdBss = FIRST_MBSSID; IdBss < MaxNumBss; IdBss++) {
				if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev == NULL) {
					preIfIndex = IdBss;
					break;
				}
			}
			pAd->ApCfg.MBSSID[preIfIndex].wdev.if_dev = NULL;
			pAd->ApCfg.MBSSID[preIfIndex].wdev.bcn_buf.BeaconPkt = NULL;
			new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_MBSSID, preIfIndex,
							sizeof(struct mt_dev_priv), preIfName, TRUE);
			if (new_dev_p == NULL) {
				/* allocation fail, exit */
				MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
							"Allocate network device fail (CFG80211)...\n");
				break;
			}


			pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_AP;
			apidx = preIfIndex;
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			if (!wdev) {
				MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
						 "%s wdev not exist, free net device!\n",
						 RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
				RtmpOSNetDevFree(new_dev_p);
				break;
			}
			Ret = wdev_init(pAd, wdev, WDEV_TYPE_AP, new_dev_p, apidx,
							(VOID *)&pAd->ApCfg.MBSSID[apidx], (void *)pAd);

			if (!Ret) {
				MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
						 "Assign wdev idx for %s failed, free net device!\n",
						 RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
				RtmpOSNetDevFree(new_dev_p);
				break;
			}

			Ret = wdev_ops_register(wdev, WDEV_TYPE_AP, &ap_wdev_ops,
									cap->qos.wmm_detect_method);

			if (!Ret) {
				MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
						 "register wdev_ops %s failed, free net device!\n",
						  RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
				RtmpOSNetDevFree(new_dev_p);
				break;
			}

			RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
			RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
			/* init operation functions and flags */
			NdisZeroMemory(&netDevHook, sizeof(netDevHook));
			netDevHook.open = mbss_virtual_if_open;	/* device opem hook point */
			netDevHook.stop = mbss_virtual_if_close;	/* device close hook point */
			netDevHook.xmit = rt28xx_send_packets;	/* hard transmit hook point */
			netDevHook.ioctl = rt28xx_ioctl;	/* ioctl hook point */
			netDevHook.priv_flags = INT_MBSSID;
#ifdef HOSTAPD_MBSS_SUPPORT
			netDevHook.set_mac_addr = rt28xx_change_mac;
#endif /*HOSTAPD_MBSS_SUPPORT*/
			netDevHook.needProtcted = TRUE;
			netDevHook.wdev = wdev;
			netDevHook.get_stats = RT28xx_get_ether_stats;
			/* Init MAC address of virtual network interface */
#ifdef HOSTAPD_MBSS_SUPPORT
			phy_sync_wdev(pAd, wdev);
			AsicSetWdevIfAddr(pAd, wdev, OPMODE_AP);
			NdisMoveMemory(&netDevHook.devAddr[0], &wdev->if_addr, MAC_ADDR_LEN);
#else
			NdisMoveMemory(&netDevHook.devAddr[0], &wdev->bssid[0], MAC_ADDR_LEN);
#endif /*HOSTAPD_MBSS_SUPPORT*/

#ifdef CONFIG_MAP_SUPPORT
			if (IS_MAP_TURNKEY_ENABLE(pAd)) {
				if (wdev && wdev->wdev_type == WDEV_TYPE_AP)
					map_make_vend_ie(pAd, apidx);
			}
#endif /* CONFIG_MAP_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
			pWdev = &wdev->cfg80211_wdev;
			os_zero_mem(pWdev, sizeof(struct wireless_dev));
			new_dev_p->ieee80211_ptr = pWdev;
			pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
			SET_NETDEV_DEV(new_dev_p, wiphy_dev(pWdev->wiphy));
			pWdev->netdev = new_dev_p;
			pWdev->iftype = DevType;
#endif /* RT_CFG80211_SUPPORT */
			/* register this device to OS */
			if (RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, &netDevHook) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
					"create IF %d (%s) failed!!\n",
					apidx, RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
				RtmpOSNetDevFree(new_dev_p);
				break;
			}
			(*ppWdev) = &wdev->cfg80211_wdev;
			RTMP_CFG80211_AddVifEntry(pAd, new_dev_p, DevType);
			retval = TRUE;
		break;
#ifdef CONFIG_VLAN_GTK_SUPPORT
	case RT_CMD_80211_IFTYPE_AP_VLAN:
		/* max bss number && avai apiadx */
		MaxNumBss = pAd->ApCfg.BssidNum;
		if (!VALID_MBSS(pAd, MaxNumBss))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		new_dev_p = alloc_netdev(sizeof(struct vlan_dev_priv), preIfName, 0, vlan_setup);
		if (!new_dev_p) {
			MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
				"Allocate network device fail (CFG80211)...\n");
			break;
		}

		pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_AP;
		wdev = CFG80211_GetWdevByVlandev(pAd, new_dev_p);
		if (!wdev) {
			free_netdev(new_dev_p);
			MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
				"%s(): can't find a matched wdev for AP_VLAN net_dev!\n", __func__);
			break;
		}

		if (wdev->vlan_cnt >= MAX_VLAN_NET_DEVICE) {
			free_netdev(new_dev_p);
			MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
				"%s(): can't create AP_VLAN net_dev, only support for %d vlan!\n",
				__func__, MAX_VLAN_NET_DEVICE);
			break;
		} else {
			struct wireless_dev *pWdev;
			CFG80211_CB *p80211CB;
			char *pch;
			long vlan_id;
			UINT16 vlan_bmc_idx;
			BOOLEAN isVlan;
			struct vlan_dev_priv *vlan;
			struct vlan_gtk_info *vg_info;

			isVlan = TRUE;
			p80211CB = pAd->pCfg80211_CB;
			pWdev = kzalloc(sizeof(*pWdev), GFP_KERNEL);
			if (pWdev == NULL)
				return FALSE;
			new_dev_p->ieee80211_ptr = pWdev;
			pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
			SET_NETDEV_DEV(new_dev_p, wiphy_dev(pWdev->wiphy));
			pWdev->netdev = new_dev_p;
			pWdev->iftype = RT_CMD_80211_IFTYPE_AP_VLAN;

			/* acquire bmc wcid for vlan_dev */
			vlan_bmc_idx = HcAcquireGroupKeyWcid(pAd, wdev, isVlan);
			MgmtTableSetMcastEntry(pAd, vlan_bmc_idx, wdev);

			/* init sta_rec and tr_entry for vlan bmc wtbl */
			wifi_vlan_starec_linkup(wdev, vlan_bmc_idx);

			/* parse vlan id from interface name */
			pch = strchr(new_dev_p->name, '.') + 1;
			if ((kstrtol(pch, 10, &vlan_id)) != 0) {
				MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
					"%s(): kstrtol failed!\n", __func__);
				return FALSE;
			}

			MTWF_PRINT("%s(): Create AP_VLAN net_dev name=%s vlan_id=%ld bmc_idx=%d\n",
				__func__, new_dev_p->name, vlan_id, vlan_bmc_idx);

			/* register vlan device */
			new_dev_p->mtu = wdev->if_dev->mtu;
			vlan = vlan_dev_priv(new_dev_p);
			vlan->vlan_proto = htons(ETH_P_8021Q);
			vlan->vlan_id = vlan_id;
			vlan->real_dev = wdev->if_dev;
			vlan->dent = NULL;
			vlan->flags = VLAN_FLAG_REORDER_HDR;
			if (register_vlan_dev(new_dev_p, NULL) < 0) {
				free_netdev(new_dev_p);
				MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
					"%s(): register_vlan_dev failed!\n", __func__);
				break;
			}

			/* add the vlan_gtk_info to list*/
			vg_info = kzalloc(sizeof(struct vlan_gtk_info), GFP_KERNEL);
			if (!vg_info) {
				unregister_vlan_dev(new_dev_p, NULL);
				free_netdev(new_dev_p);
				MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
					"%s(): Create vlan_gtk_info failed!\n", __func__);
				break;
			}
			vg_info->vlan_dev = new_dev_p;
			vg_info->vlan_id = vlan_id;
			vg_info->vlan_bmc_idx = vlan_bmc_idx;
			vg_info->vlan_tr_tb_idx = vlan_bmc_idx;
			list_add_tail(&vg_info->list, &wdev->vlan_gtk_list);
#ifdef VLAN_SUPPORT
			/* prevent vlan_tag from being removed by ap_fp_tx_pkt_vlan_tag_handle() */
			wdev->bVLAN_Tag = TRUE;
#endif
			wdev->vlan_cnt++;
			RTMP_CFG80211_AddVifEntry(pAd, vg_info->vlan_dev, DevType);
			(*ppWdev) = &wdev->cfg80211_wdev;
			retval = TRUE;
		}
		break;
#endif

#ifdef CONFIG_STA_SUPPORT
	case RT_CMD_80211_IFTYPE_STATION:
#ifdef CONFIG_APSTA_MIXED_SUPPORT
		if (IF_COMBO_HAVE_AP_STA(pAd)) {
			sta_start_id = 0;
			max_num_sta = min(pAd->ApCfg.ApCliNum, (UCHAR)MAX_APCLI_NUM);
			inf_type = INT_APCLI;

			for (IdBss = 0; IdBss < max_num_sta; IdBss++) {
				if (pAd->StaCfg[IdBss].wdev.if_dev == NULL) {
					preIfIndex = IdBss;
					break;
				}
			}
			if (preIfIndex < MAX_MULTI_STA)
				pAd->StaCfg[preIfIndex].wdev.if_dev = NULL;
		}
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
		if (preIfIndex >= MAX_MULTI_STA) {
			MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
						"apcli preIfIndex out of range\n");
			break;
		}
		new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, inf_type,
					preIfIndex, sizeof(struct mt_dev_priv), preIfName, TRUE);
		if (new_dev_p == NULL) {
			/* allocation fail, exit */
			MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
						"Allocate network device fail (CFG80211)...\n");
			break;
		}

#ifdef DOT11_SAE_SUPPORT
		pAd->StaCfg->sae_cfg_group = SAE_DEFAULT_GROUP;
#endif
#ifdef CONFIG_OWE_SUPPORT
		pAd->StaCfg->curr_owe_group = ECDH_GROUP_256;
#endif
		wdev = &pAd->StaCfg[preIfIndex].wdev;
		Ret = wdev_init(pAd, wdev, WDEV_TYPE_STA, new_dev_p, preIfIndex,
						(VOID *)&pAd->StaCfg[preIfIndex], (VOID *)pAd);

		if (!Ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
					"Assign wdev idx for %s failed, free net device!\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
			RtmpOSNetDevFree(new_dev_p);
			break;
		}

		Ret = wdev_ops_register(wdev, WDEV_TYPE_STA, &apcli_wdev_ops,
								cap->qos.wmm_detect_method);

		if (!Ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
					 "register wdev_ops %s failed, free net device!\n"
						, RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
			RtmpOSNetDevFree(new_dev_p);
			break;
		}
		RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
		RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
		/* init operation functions and flags */

		NdisZeroMemory(&netDevHook, sizeof(netDevHook));
		netDevHook.open = msta_virtual_if_open;  /* device opem hook point */
		netDevHook.stop = msta_virtual_if_close; /* device close hook point */
		netDevHook.xmit = rt28xx_send_packets;	/* hard transmit hook point */
		netDevHook.ioctl = rt28xx_ioctl;	/* ioctl hook point */
		netDevHook.priv_flags = INT_APCLI;
		netDevHook.needProtcted = TRUE;
		netDevHook.wdev = wdev;
		netDevHook.get_stats = RT28xx_get_ether_stats;
		/* Init MAC address of virtual network interface */
		COPY_MAC_ADDR(&pAd->StaCfg[preIfIndex].wdev.if_addr, pAd->CurrentAddress);
		AsicSetWdevIfAddr(pAd, wdev, OPMODE_STA);
		NdisMoveMemory(&netDevHook.devAddr[0], &wdev->if_dev, MAC_ADDR_LEN);
#ifdef CONFIG_APSTA_MIXED_SUPPORT
		if ((IF_COMBO_HAVE_AP_STA(pAd))) {
			phy_sync_wdev(pAd, wdev);
			/*update rate info*/
			SetCommonHtVht(pAd, wdev);
			RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);
			AsicSetWdevIfAddr(pAd, wdev, OPMODE_STA);
		}
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
		NdisMoveMemory(&netDevHook.devAddr[0], pAd->StaCfg[preIfIndex].wdev.if_addr, MAC_ADDR_LEN);

		if (IF_COMBO_HAVE_AP_STA(pAd)) {
			struct wireless_dev *pWdev;
			CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
			UINT32 DevType = RT_CMD_80211_IFTYPE_STATION;

			pWdev = &wdev->cfg80211_wdev;
			os_zero_mem(pWdev, sizeof(struct wireless_dev));
			new_dev_p->ieee80211_ptr = pWdev;
			pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
			SET_NETDEV_DEV(new_dev_p, wiphy_dev(pWdev->wiphy));
			pWdev->netdev = new_dev_p;
			pWdev->iftype = DevType;

			if (flags & WIPHY_FLAG_4ADDR_STATION)
				pWdev->use_4addr = true;
		}
		/* register this device to OS */
		if (RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, &netDevHook) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
					"create IF %d (%s) failed!!\n",
					apidx, RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
		}

		RTMP_CFG80211_AddVifEntry(pAd, new_dev_p, DevType);
		(*ppWdev) = &wdev->cfg80211_wdev;
		pAd->StaCfg->ApcliInfStat.ApCliInit = TRUE;
		retval = TRUE;
#ifdef MAC_REPEATER_SUPPORT
		CliLinkMapInit(pAd);
#endif
		break;
#endif /* CONFIG_STA_SUPPORT */

	default:
		MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
			"Unknown CFG80211 I/F Type (%d)\n", DevType);
	}

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_INFO, "<--- %d\n", retval);
	return retval;
}

VOID RTMP_CFG80211_VirtualIF_Remove(
	IN  VOID				 *pAdSrc,
	IN	PNET_DEV			  dev_p,
	IN  UINT32                DevType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	struct wifi_dev *wdev;
	INT32 apidx = 0, staidx = 0;
	ULONG flags = 0;

	if (DevType == RT_CMD_80211_IFTYPE_AP) {
		apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, dev_p);
		if (apidx == WDEV_NOT_FOUND) {
			MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
				"failed - [ERROR]can't find wdev in driver MBSS.\n");
			return;
		}
		if (dev_p) {
			RTMP_CFG80211_RemoveVifEntry(pAd, dev_p);
			RTMP_OS_NETDEV_STOP_QUEUE(dev_p);
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			bcn_buf_deinit(pAd, &pAd->ApCfg.MBSSID[apidx].wdev);
#if	defined(RT_CFG80211_SUPPORT) && defined(BACKPORT_NOSTDINC)
			cfg80211_unregister_wdev(dev_p->ieee80211_ptr);
#else
			RtmpOSNetDevDetach(dev_p);
#endif
			wdev_deinit(pAd, wdev);
			wdev->if_dev = NULL;

		}
		OS_SPIN_LOCK_IRQSAVE(&pAd->CfgIfUseCntLock, &flags);
		pAd->CfgAPIfUseCnt--;
		OS_SPIN_UNLOCK_IRQRESTORE(&pAd->CfgIfUseCntLock, &flags);
	} else if (DevType == RT_CMD_80211_IFTYPE_STATION) {
		staidx = CFG80211_FindStaIdxByNetDevice(pAd, dev_p);
		if (staidx == WDEV_NOT_FOUND) {
			MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
				"failed - [ERROR]can't find wdev in driver STA. \n");
			return;
		}

		if (dev_p) {
			RTMP_CFG80211_RemoveVifEntry(pAd, dev_p);
			RTMP_OS_NETDEV_STOP_QUEUE(dev_p);

			wdev = &pAd->StaCfg[staidx].wdev;
			RtmpOSNetDevDetach(dev_p);
			wdev_deinit(pAd, wdev);
			wdev->if_dev = NULL;

		}
		OS_SPIN_LOCK_IRQSAVE(&pAd->CfgIfUseCntLock, &flags);
		pAd->CfgSTAIfUseCnt--;
		OS_SPIN_UNLOCK_IRQRESTORE(&pAd->CfgIfUseCntLock, &flags);
	}
#ifdef CONFIG_VLAN_GTK_SUPPORT
	else if (DevType == RT_CMD_80211_IFTYPE_AP_VLAN) {
		if (dev_p) {
			struct vlan_gtk_info *vg_info;

			wdev = CFG80211_GetWdevByVlandev(pAd, dev_p);
			if (wdev == NULL) {
				MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
				"failed - [ERROR]wdev is NULL, returning\n");
				return;
			}
			vg_info = CFG80211_GetVlanInfoByVlandev(wdev, dev_p);
			if (vg_info) {
				MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
					"%s(): Remove AP_VLAN net_dev name=%s vlan_id=%d bmc_idx=%d\n",
					 __func__, dev_p->name, vg_info->vlan_id, vg_info->vlan_bmc_idx);
				HcReleaseGroupKeyWcid(pAd, wdev, vg_info->vlan_bmc_idx);
				list_del(&vg_info->list);
				kfree(vg_info);
			}
			RTMP_CFG80211_RemoveVifEntry(pAd, dev_p);
			RTMP_OS_NETDEV_STOP_QUEUE(dev_p);
			unregister_vlan_dev(dev_p, NULL);
			if (dev_p->ieee80211_ptr) {
				kfree(dev_p->ieee80211_ptr);
				dev_p->ieee80211_ptr = NULL;
			}
			wdev->vlan_cnt--;
		}
	}
#endif


}
#endif /* IWCOMMAND_CFG80211_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */

