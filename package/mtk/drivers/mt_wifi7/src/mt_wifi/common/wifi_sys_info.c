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
 ***************************************************************************

	Module Name:
	wifi_sys_info.c
*/
#include	"rt_config.h"
#ifdef VOW_SUPPORT
#include <ap_vow.h>
#endif /* VOW_SUPPORT */

/*Local function*/
static VOID get_network_type_str(UINT32 Type, CHAR *str)
{
	INT ret;

	if (Type & NETWORK_INFRA)
		ret = snprintf(str, 128, "%s", "NETWORK_INFRA");
	else if (Type & NETWORK_P2P)
		ret = snprintf(str, 128, "%s", "NETWORK_P2P");
	else if (Type & NETWORK_IBSS)
		ret = snprintf(str, 128, "%s", "NETWORK_IBSS");
	else if (Type & NETWORK_MESH)
		ret = snprintf(str, 128, "%s", "NETWORK_MESH");
	else if (Type & NETWORK_BOW)
		ret = snprintf(str, 128, "%s", "NETWORK_BOW");
	else if (Type & NETWORK_WDS)
		ret = snprintf(str, 128, "%s", "NETWORK_WDS");
	else
		ret = snprintf(str, 128, "%s", "UND");

	if (os_snprintf_error(128, ret)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_WARN,
			"snprintf error!\n");
		return;
	}
}

/*
*
*/
#ifdef CONFIG_AP_SUPPORT
#ifdef IGMP_SNOOP_SUPPORT
static VOID update_igmpinfo(struct wifi_dev *wdev, BOOLEAN bActive)
{
	struct _DEV_INFO_CTRL_T *devinfo = &wdev->DevInfo;
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	UINT Enable = FALSE;

#ifdef IGMP_TVM_SUPPORT
	if (bActive == TRUE) {
		wdev->pIgmpMcastTable = NULL;
		wdev->IgmpTableSize = 0;
	}
#endif /* IGMP_TVM_SUPPORT */

	if (wdev->wdev_type == WDEV_TYPE_AP) {

		if (bActive == TRUE && wdev->IgmpSnoopEnable == TRUE)
			Enable = TRUE;

#ifdef IGMP_TVM_SUPPORT
		wdev->u4AgeOutTime = IGMPMAC_TB_ENTRY_AGEOUT_TIME;
		if (IgmpSnEnableTVMode(ad,
			wdev,
			wdev->IsTVModeEnable,
			wdev->TVModeType))
			Enable = wdev->TVModeType;
#endif	/* IGMP_TVM_SUPPORT */
		CmdMcastCloneEnable(ad, Enable, devinfo->BandIdx, devinfo->OwnMacIdx);

#ifdef IGMP_TVM_SUPPORT
		if (IS_ASIC_CAP(ad, fASIC_CAP_MCU_OFFLOAD))
			MulticastFilterInitMcastTable(ad, wdev, bActive);
#endif /* IGMP_TVM_SUPPORT */

	}
}
#endif	/*IGMP_SNOOP_SUPPORT */
#endif	/*CONFIG_AP_SUPPORT*/

/*
*
*/
static VOID dump_devinfo(struct _WIFI_INFO_CLASS *class)
{
	struct _DEV_INFO_CTRL_T *devinfo = NULL;
	struct wifi_dev *wdev = NULL;

	DlListForEach(devinfo, &class->Head, DEV_INFO_CTRL_T, list) {
		wdev = (struct wifi_dev *)devinfo->priv;
		MTWF_PRINT("#####WdevIdx (%d)#####\n", wdev->wdev_idx);
		MTWF_PRINT("Active: %d\n", devinfo->WdevActive);
		MTWF_PRINT("BandIdx: %d\n", devinfo->BandIdx);
		MTWF_PRINT("EnableFeature: %d\n", devinfo->EnableFeature);
		MTWF_PRINT("OwnMacIdx: %d\n", devinfo->OwnMacIdx);
		MTWF_PRINT("OwnMacAddr: "MACSTR"\n", MAC2STR(devinfo->OwnMacAddr));
	}
}

/*
*
*/
static VOID dump_bssinfo(struct _WIFI_INFO_CLASS *class)
{
	BSS_INFO_ARGUMENT_T *bss = NULL;
	struct wifi_dev *wdev = NULL;
	CHAR str[128] = "";

	DlListForEach(bss, &class->Head, BSS_INFO_ARGUMENT_T, list) {
		wdev = (struct wifi_dev *)bss->priv;
		MTWF_PRINT("#####WdevIdx (%d)#####\n", wdev->wdev_idx);
		MTWF_PRINT("State: %d\n", bss->bss_state);
		MTWF_PRINT("Bssid: "MACSTR"\n", MAC2STR(bss->Bssid));
		MTWF_PRINT("CipherSuit: %d\n", bss->CipherSuit);
		get_network_type_str(bss->NetworkType, str);
		MTWF_PRINT("NetworkType: %s\n", str);
		MTWF_PRINT("OwnMacIdx: %d\n", bss->OwnMacIdx);
		MTWF_PRINT("BssInfoFeature: %llx\n", bss->u8BssInfoFeature);
		MTWF_PRINT("ConnectionType: %d\n", bss->u4ConnectionType);
		MTWF_PRINT("BcMcWlanIdx: %d\n", bss->bmc_wlan_idx);
		MTWF_PRINT("BssIndex: %d\n", bss->ucBssIndex);
		MTWF_PRINT("PeerWlanIdx: %d\n", bss->peer_wlan_idx);
		MTWF_PRINT("WmmIdx: %d\n", bss->WmmIdx);
		MTWF_PRINT("BcTransmit: (Mode/BW/MCS) %d/%d/%d\n", bss->BcTransmit.field.MODE,
			   bss->BcTransmit.field.BW, bss->BcTransmit.field.MCS);
		MTWF_PRINT("McTransmit: (Mode/BW/MCS) %d/%d/%d\n", bss->McTransmit.field.MODE,
			   bss->BcTransmit.field.BW, bss->BcTransmit.field.MCS);
	}
}

/*
*
*/
static VOID dump_starec(struct _WIFI_INFO_CLASS *class)
{
	struct _STA_REC_CTRL_T *starec = NULL;
	struct _STA_TR_ENTRY *tr_entry = NULL;

	DlListForEach(starec, &class->Head, STA_REC_CTRL_T, list) {
		tr_entry = (STA_TR_ENTRY *)starec->priv;
		MTWF_PRINT("#####MacEntry (%d)#####\n", tr_entry->wcid);
		MTWF_PRINT("PeerAddr: "MACSTR"\n", MAC2STR(tr_entry->Addr));
		MTWF_PRINT("WlanIdx: %d\n", starec->WlanIdx);
		MTWF_PRINT("BssIndex: %d\n", starec->BssIndex);
		MTWF_PRINT("ConnectionState: %d\n", starec->ConnectionState);
		MTWF_PRINT("ConnectionType: %d\n", starec->ConnectionType);
		MTWF_PRINT("EnableFeature: %llx\n", starec->EnableFeature);
	}
}

/*
*
*/
static VOID add_devinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct _WIFI_SYS_INFO *wsys = ad->WifiSysInfo;
	struct _DEV_INFO_CTRL_T *devinfo = &wdev->DevInfo, *tmp = NULL;

	OS_SEM_LOCK(&wsys->lock);
	DlListForEach(tmp, &wsys->DevInfo.Head, DEV_INFO_CTRL_T, list) {
		if (devinfo == tmp) {
			OS_SEM_UNLOCK(&wsys->lock);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"DevInfo %d already exist",
					devinfo->OwnMacIdx);
			return;
		}
	}
	DlListAddTail(&wsys->DevInfo.Head, &devinfo->list);
	devinfo->priv = (VOID *)wdev;
	wsys->DevInfo.Num++;
	OS_SEM_UNLOCK(&wsys->lock);
}

/*
*
*/
static VOID add_bssinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct _WIFI_SYS_INFO *wsys = ad->WifiSysInfo;
	struct _BSS_INFO_ARGUMENT_T *bss = &wdev->bss_info_argument, *tmp;

	OS_SEM_LOCK(&wsys->lock);
	DlListForEach(tmp, &wsys->BssInfo.Head, BSS_INFO_ARGUMENT_T, list) {
		if (bss == tmp) {
			OS_SEM_UNLOCK(&wsys->lock);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"BssInfo %d already exist",
					bss->ucBssIndex);
			return;
		}
	}
	DlListAddTail(&wsys->BssInfo.Head, &bss->list);
	bss->priv = (VOID *)wdev;
	wsys->BssInfo.Num++;
	OS_SEM_UNLOCK(&wsys->lock);
}

/*
*
*/
static VOID add_starec(struct _RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry)
{
	struct _WIFI_SYS_INFO *wsys = pAd->WifiSysInfo;
	struct _STA_REC_CTRL_T *strec = &tr_entry->StaRec, *tmp = NULL;

	OS_SEM_LOCK(&wsys->lock);
	DlListForEach(tmp, &wsys->StaRec.Head, STA_REC_CTRL_T, list) {
		if (tmp == strec) {
			OS_SEM_UNLOCK(&wsys->lock);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"STARec %d already exist\n",
					strec->WlanIdx);
			return;
		}
	}
	DlListAddTail(&wsys->StaRec.Head, &strec->list);
	strec->priv = (VOID *)tr_entry;
	wsys->StaRec.Num++;
	OS_SEM_UNLOCK(&wsys->lock);
}

/*
*
*/
struct _STA_REC_CTRL_T *get_starec_by_wcid(struct _RTMP_ADAPTER *ad, INT wcid)
{
	struct _WIFI_SYS_INFO *wsys = ad->WifiSysInfo;
	struct _STA_REC_CTRL_T *sta_rec = NULL, *tmp = NULL;

	OS_SEM_LOCK(&wsys->lock);
	DlListForEach(tmp, &wsys->StaRec.Head, STA_REC_CTRL_T, list) {
		if (tmp->WlanIdx == wcid) {
			sta_rec = tmp;
			break;
		}
	}
	OS_SEM_UNLOCK(&wsys->lock);
	return sta_rec;
}

/*
*
*/
static VOID del_devinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct _WIFI_SYS_INFO *wsys = ad->WifiSysInfo;
	struct _DEV_INFO_CTRL_T *devinfo = &wdev->DevInfo;

	OS_SEM_LOCK(&wsys->lock);
	DlListDel(&devinfo->list);
	wsys->DevInfo.Num--;
	OS_SEM_UNLOCK(&wsys->lock);
}

/*
*
*/
static VOID del_bssinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct _WIFI_SYS_INFO *wsys = ad->WifiSysInfo;
	struct _BSS_INFO_ARGUMENT_T *bss = &wdev->bss_info_argument, *tmp;

	OS_SEM_LOCK(&wsys->lock);
	DlListForEach(tmp, &wsys->BssInfo.Head, BSS_INFO_ARGUMENT_T, list) {
		if (tmp == bss) {
			DlListDel(&bss->list);
			wsys->BssInfo.Num--;
			break;
		}
	}
	OS_SEM_UNLOCK(&wsys->lock);
}

/*
*
*/
VOID del_starec(struct _RTMP_ADAPTER *ad, struct _STA_TR_ENTRY *tr_entry)
{
	struct _WIFI_SYS_INFO *wsys = ad->WifiSysInfo;
	struct _STA_REC_CTRL_T *starec = &tr_entry->StaRec, *tmp;

	if (starec ==  NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"starec is NULL\n");
		return;
	}

	if (wsys ==  NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"wsys is NULL\n");
		return;
	}

	OS_SEM_LOCK(&wsys->lock);
	DlListForEach(tmp, &wsys->StaRec.Head, STA_REC_CTRL_T, list) {
		if (tmp == starec) {
			DlListDel(&starec->list);
			wsys->StaRec.Num--;
			break;
		}
	}
	OS_SEM_UNLOCK(&wsys->lock);
}

/*
*
*/
static VOID fill_devinfo(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BOOLEAN act,
	struct _DEV_INFO_CTRL_T *devinfo)
{
	struct _DEV_INFO_CTRL_T *org = &wdev->DevInfo;

	os_move_mem(devinfo, org, sizeof(DEV_INFO_CTRL_T));
	devinfo->WdevActive = act;
	devinfo->OwnMacIdx = wdev->OmacIdx;
	os_move_mem(devinfo->OwnMacAddr, wdev->if_addr, MAC_ADDR_LEN);
	if (wdev->wdev_type != WDEV_TYPE_WDS)	/* WDS share DevInfo with normal AP */
		devinfo->EnableFeature = DEVINFO_ACTIVE_FEATURE;
	devinfo->BandIdx = HcGetBandByWdev(wdev);
	os_move_mem(&wdev->DevInfo, devinfo, sizeof(DEV_INFO_CTRL_T));

#ifdef CONFIG_AP_SUPPORT
#ifdef IGMP_SNOOP_SUPPORT
	update_igmpinfo(wdev, act);
#endif
#endif
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		"Active=%d,OwnMacIdx=%d,EnableFeature=%d,BandIdx=%d\n",
		devinfo->WdevActive, devinfo->OwnMacIdx, devinfo->EnableFeature, devinfo->BandIdx);
}

/*
*
*/
static inline VOID fill_bssinfo_active(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	struct _BSS_INFO_ARGUMENT_T *bssinfo)
{
	BssInfoArgumentLink(ad, wdev, bssinfo);
	bssinfo->bss_state = BSS_ACTIVE;
	WDEV_BSS_STATE(wdev) = BSS_ACTIVE;
}

/*
*
*/
static inline VOID fill_bssinfo_deactive(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	struct _BSS_INFO_ARGUMENT_T *bssinfo)
{
	WDEV_BSS_STATE(wdev) = BSS_INITED;
	os_move_mem(bssinfo, &wdev->bss_info_argument, sizeof(wdev->bss_info_argument));
}

/*
*
*/
static VOID fill_bssinfo(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	BOOLEAN act,
	struct _BSS_INFO_ARGUMENT_T *bssinfo)
{
	if (act)
		fill_bssinfo_active(ad, wdev, bssinfo);
	else
		fill_bssinfo_deactive(ad, wdev, bssinfo);
}

/*
*
*/
static VOID fill_starec(
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *entry,
	struct _STA_TR_ENTRY *tr_entry,
	struct _STA_REC_CTRL_T *starec)
{
	os_move_mem(starec, &tr_entry->StaRec, sizeof(tr_entry->StaRec));
#ifdef DOT11_HE_AX
	fill_starec_he(wdev, entry, starec);
#endif /*DOT11_HE_AX*/
}



/*
*
*/
static INT call_wsys_notifieriers(INT val, struct wifi_dev *wdev, void *v)
{
	struct _RTMP_ADAPTER *ad;
	struct _WIFI_SYS_INFO *wsys;
	struct wsys_notify_info info;
	INT ret;

	if (!wdev)
		return NOTIFY_STAT_FAILURE;
	ad = wdev->sys_handle;
	if (!ad)
		return NOTIFY_STAT_FAILURE;
	wsys = ad->WifiSysInfo;
	if (!wsys)
		return NOTIFY_STAT_FAILURE;
	/*fill wsys notify info*/
	info.wdev = wdev;
	info.v = v;
	/*traversal caller for wsys notify chain*/
	ret = mt_notify_call_chain(&wsys->wsys_notify_head, val, &info);
	return ret;
}

/*export function*/
/*
*
*/
INT register_wsys_notifier(struct _WIFI_SYS_INFO *wsys, struct notify_entry *ne)
{
	INT ret;

	ret = mt_notify_chain_register(&wsys->wsys_notify_head, ne);

	return ret;
}

/*
*
*/
INT unregister_wsys_notifier(struct _WIFI_SYS_INFO *wsys, struct notify_entry *ne)
{
	INT ret;

	ret = mt_notify_chain_unregister(&wsys->wsys_notify_head, ne);
	return ret;
}

/*
*
*/
VOID wifi_sys_reset(struct _WIFI_SYS_INFO *wsys)
{
	DlListInit(&wsys->DevInfo.Head);
	DlListInit(&wsys->StaRec.Head);
	DlListInit(&wsys->BssInfo.Head);

	wsys->DevInfo.Num = 0;
	wsys->StaRec.Num = 0;
	wsys->BssInfo.Num = 0;
}

/*
*
*/
VOID wifi_sys_init(struct _WIFI_SYS_INFO *wsys)
{
	struct notify_head *pnotify_head = &wsys->wsys_notify_head;

	OS_NdisAllocateSpinLock(&wsys->lock);
	wifi_sys_reset(wsys);
	OS_NdisAllocateSpinLock(&pnotify_head->lock);
	pnotify_head->head = NULL;
}


/*
*
*/
VOID wifi_sys_dump(struct _RTMP_ADAPTER *ad)
{
	struct _WIFI_SYS_INFO *wsys = ad->WifiSysInfo;

	MTWF_PRINT("===============================\n");
	MTWF_PRINT("Current DevInfo Num: %d\n", wsys->DevInfo.Num);
	MTWF_PRINT("===============================\n");
	dump_devinfo(&wsys->DevInfo);
	MTWF_PRINT("===============================\n");
	MTWF_PRINT("Current BssInfo Num: %d\n", wsys->BssInfo.Num);
	MTWF_PRINT("===============================\n");
	dump_bssinfo(&wsys->BssInfo);
	MTWF_PRINT("===============================\n");
	MTWF_PRINT("Current StaRec Num: %d\n", wsys->StaRec.Num);
	MTWF_PRINT("===============================\n");
	dump_starec(&wsys->StaRec);
}

/*
*
*/
#define INVALID_OMAC_VALUE 255
#define INVALID_WMM_VALUE 255
INT wifi_sys_update_devinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _DEV_INFO_CTRL_T *new)
{
	INT len = offsetof(DEV_INFO_CTRL_T, list);

	os_move_mem(&wdev->DevInfo, new, len);
	if (new->WdevActive) {
		add_devinfo(ad, wdev);
		/*notify other modules, hw resouce is acquired down*/
		call_wsys_notifieriers(WSYS_NOTIFY_OPEN, wdev, NULL);
	} else {
		del_devinfo(ad, wdev);
		/*release hw resource*/
		HcReleaseGroupKeyWcid(wdev->sys_handle, wdev, wdev->tr_tb_idx);
		HcReleaseRadioForWdev(wdev->sys_handle, wdev);
		wdev->OmacIdx = INVALID_OMAC_VALUE;
		wdev->WmmIdx = INVALID_WMM_VALUE;
	}
	return 0;
}

/*
*
*/
INT wifi_sys_update_bssinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _BSS_INFO_ARGUMENT_T *new)
{
	BSS_INFO_ARGUMENT_T *bss = &wdev->bss_info_argument;
	INT len = offsetof(BSS_INFO_ARGUMENT_T, list);

	if (new->bss_state >= BSS_ACTIVE) {
		os_move_mem(bss, new, len);
		add_bssinfo(ad, wdev);
		WDEV_BSS_STATE(wdev) = BSS_READY;
		/*notify other modules, bss related setting is done*/
		call_wsys_notifieriers(WSYS_NOTIFY_LINKUP, wdev, bss);
	} else {
		del_bssinfo(ad, wdev);
		BssInfoArgumentUnLink(ad, wdev);
	}

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
	/* Notify SR module */
	if (IS_MAP_ENABLE(ad) || IS_MAP_R3_ENABLE(ad))
		SrMeshSelfSrgBMChangeEvent(ad, wdev, TRUE);
#endif

	return 0;
}

/*
*
*/
INT wifi_sys_update_starec(struct _RTMP_ADAPTER *ad, struct _STA_REC_CTRL_T *new)
{
	struct _STA_TR_ENTRY *tr_entry = (struct _STA_TR_ENTRY *) new->priv;
	struct _STA_REC_CTRL_T *sta_rec = &tr_entry->StaRec;
	INT len = offsetof(STA_REC_CTRL_T, list);

	os_move_mem(sta_rec, new, len);
	if (new->ConnectionState == STATE_DISCONNECT) {
		/*remove starec*/
		del_starec(ad, tr_entry);
	} else {
		add_starec(ad, tr_entry);
		/*notify other modules, starec is prepeare done*/
		call_wsys_notifieriers(WSYS_NOTIFY_CONNT_ACT, tr_entry->wdev, tr_entry);
	}
	return 0;
}

/*
* for peer update usage
*/
INT wifi_sys_update_starec_info(struct _RTMP_ADAPTER *ad, struct _STA_REC_CTRL_T *new)
{
	struct _STA_TR_ENTRY *tr_entry = (struct _STA_TR_ENTRY *) new->priv;
	struct _STA_REC_CTRL_T *sta_rec = &tr_entry->StaRec;
	INT len = offsetof(STA_REC_CTRL_T, list);

	os_move_mem(sta_rec, new, len);
	call_wsys_notifieriers(WSYS_NOTIFY_STA_UPDATE, tr_entry->wdev, tr_entry);

	return 0;
}


/*wcid: STA is peer root AP, AP is bmc wcid*/
UINT32 bssinfo_feature_decision(
	struct wifi_dev *wdev,
	UINT32 conn_type,
	UINT16 wcid,
	UINT64 *feature)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) wdev->sys_handle;
	/*basic features*/
	UINT64 features = (BSS_INFO_OWN_MAC_FEATURE
					   | BSS_INFO_BASIC_FEATURE
					   | BSS_INFO_RF_CH_FEATURE
					   | BSS_INFO_BROADCAST_INFO_FEATURE
					   | BSS_INFO_PROTECT_INFO_FEATURE
					   | BSS_INFO_MLD_FEATURE);

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	if (cap->fgRateAdaptFWOffload == TRUE)
		features |= BSS_INFO_RA_FEATURE;
#endif

	/*HW BSSID features*/
	if (wdev->OmacIdx > HW_BSSID_MAX)
		features |= BSS_INFO_EXT_BSS_FEATURE;
	else
		features |= BSS_INFO_SYNC_MODE_FEATURE;

	if (IS_ASIC_CAP(ad, fASIC_CAP_HW_TX_AMSDU))
		features |= BSS_INFO_HW_AMSDU_FEATURE;


#ifdef DOT11V_MBSSID_SUPPORT
		/* Remove BSS_INFO_11V_MBSSID_FEATURE for Legacy BSS
		* to prevent tsf adjust again with extra BSS_INFO_BASIC_FEATURE
		*/
		if (wdev->wdev_type == WDEV_TYPE_AP
			&& VALID_MBSS(ad, wdev->func_idx)) {
			BSS_STRUCT *mbss = &ad->ApCfg.MBSSID[wdev->func_idx];

			features |= BSS_INFO_11V_MBSSID_FEATURE;

			if (IS_BSSID_11V_ROLE_NONE(&mbss->mbss_11v)
					|| IS_BSSID_11V_CO_HOSTED(&mbss->mbss_11v)) {
				features &= ~BSS_INFO_11V_MBSSID_FEATURE;
				MTWF_DBG(NULL, DBG_CAT_AP, CATFW_11V_MBSS, DBG_LVL_NOTICE,
					"wdev(%d) is legacy bss, func_idx=%d, ROLE=%d\n",
					wdev->wdev_idx, wdev->func_idx, mbss->mbss_11v.mbss_11v_enable);
			} else {
				MTWF_DBG(NULL, DBG_CAT_AP, CATFW_11V_MBSS, DBG_LVL_NOTICE,
					"wdev(%d) is 11v bss, func_idx=%d, ROLE=%d\n",
					wdev->wdev_idx, wdev->func_idx, mbss->mbss_11v.mbss_11v_enable);
			}
		}
#endif /* DOT11V_MBSSID_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA && conn_type == CONNECTION_INFRA_STA)
		bssinfo_sta_feature_decision(wdev, wcid, &features);

#endif /*CONFIG_STA_SUPPORT*/

#ifdef DOT11_HE_AX
	bssinfo_he_feature_decision(wdev, &features);
#endif /*DOT11_HE_AX*/
	*feature = features;
	return TRUE;
}

#ifdef DOT11_HE_AX
UINT32 starec_muru_feature_decision(struct wifi_dev *wdev,
		struct _MAC_TABLE_ENTRY *entry, UINT64 *feature)
{
	UINT64 features = 0;

	if (wlan_config_get_mu_dl_ofdma(wdev) || wlan_config_get_mu_ul_ofdma(wdev)
		|| wlan_config_get_mu_dl_mimo(wdev) || wlan_config_get_mu_ul_mimo(wdev)) {

		if ((wdev->wdev_type == WDEV_TYPE_AP) || (wdev->wdev_type == WDEV_TYPE_STA))
			features |= STA_REC_MURU_FEATURE;
	}

	*feature |= features;

	return TRUE;
}
#endif /*DOT11_HE_AX*/

#ifdef MLR_SUPPORT
UINT32 starec_mlr_feature_decision(struct wifi_dev *wdev,
		struct _MAC_TABLE_ENTRY *entry, UINT64 *feature)
{
	UINT64 features = 0;

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		if (entry->MlrMode == MLR_MODE_MLR_V2)
			features |= STA_REC_MLR_INFO_FEATURE;
	}

	*feature |= features;

	return TRUE;
}
#endif /* MLR_SUPPORT */

VOID starec_feature_a4_info_decision(
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *entry,
	UINT64 *feature)
{
	struct _RTMP_ADAPTER *ad = NULL;
	UINT64 features = 0;

	if (wdev == NULL || !entry)
		return;

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (!ad)
		return;

	if (wdev->wdev_type == WDEV_TYPE_AP) {
#ifdef MWDS
		if (entry->bSupportMWDS && wdev->bSupportMWDS)
			features |= STA_REC_A4_INFO_FEATURE;
#endif
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(ad) &&
			((wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_BACKHAUL_BSS)) &&
			(entry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA))))
			features |= STA_REC_A4_INFO_FEATURE;
#endif
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
#ifdef MWDS
		PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(ad, wdev);

		if (pStaCfg->MlmeAux.bSupportMWDS && pStaCfg->wdev.bSupportMWDS)
			features |= STA_REC_A4_INFO_FEATURE;
#endif
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(ad) &&
			(entry->DevPeerRole & (BIT(MAP_ROLE_BACKHAUL_BSS))))
			features |= STA_REC_A4_INFO_FEATURE;
#endif
	}

	*feature |= features;
}
/*
*
*/
static VOID starec_security_set(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, struct _STA_REC_CTRL_T *sta_rec)
{
	ASIC_SEC_INFO *asic_sec_info = &sta_rec->asic_sec_info;
	/* Set key material to Asic */
	os_zero_mem(asic_sec_info, sizeof(ASIC_SEC_INFO));
	asic_sec_info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
	asic_sec_info->Direction = SEC_ASIC_KEY_BOTH;
	asic_sec_info->Wcid = entry->wcid;
	asic_sec_info->BssIndex = entry->func_tb_idx;
	asic_sec_info->KeyIdx = entry->SecConfig.PairwiseKeyId;
	asic_sec_info->Cipher = entry->SecConfig.PairwiseCipher;
	asic_sec_info->KeyIdx = entry->SecConfig.PairwiseKeyId;
	os_move_mem(&asic_sec_info->Key, &entry->SecConfig.WepKey[entry->SecConfig.PairwiseKeyId], sizeof(SEC_KEY_INFO));
	os_move_mem(&asic_sec_info->PeerAddr[0], entry->Addr, MAC_ADDR_LEN);
}

static UINT32 starec_feature_decision_FTM(
	struct wifi_dev *wdev,
	UINT32 conn_type,
	struct _MAC_TABLE_ENTRY *entry,
	UINT64 *feature)
{
	/*basic feature*/
	UINT64 features = (STA_REC_BASIC_STA_RECORD_FEATURE | STA_REC_RA_FEATURE
					   | STA_REC_TX_PROC_FEATURE | STA_REC_WTBL_FEATURE);

	if (wdev ==  NULL)
		return FALSE;

	if (conn_type & ~CONNECTION_INFRA_BC) {
		/*ht features */
#ifdef DOT11_N_SUPPORT
		starec_ht_feature_decision(wdev, entry, &features);
#ifdef DOT11_VHT_AC
		starec_vht_feature_decision(wdev, entry, &features);
#endif /*DOT11_VHT_AC*/
#endif /*DOT11_N_SUPPORT*/
	}

#ifdef CONFIG_AP_SUPPORT

	if (conn_type == CONNECTION_INFRA_STA)
		starec_ap_feature_decision(wdev, entry, &features);

#endif /*CONFIG_AP_SUPPORT*/

	/*return value, must use or operation*/
	*feature = features;
	return TRUE;
}

/*sta rec feature decision*/
static UINT32 starec_feature_decision(
	struct wifi_dev *wdev,
	UINT32 conn_type,
	struct _MAC_TABLE_ENTRY *entry,
	UINT64 *feature)
{
	/*basic feature*/
	UINT64 features = (STA_REC_BASIC_STA_RECORD_FEATURE
					   | STA_REC_TX_PROC_FEATURE);

	if (conn_type == CONNECTION_INFRA_BC)
		features |= STA_REC_BMC_WTBL_FEATURE;
	else
		features |= STA_REC_WTBL_FEATURE;

	if (wdev ==  NULL)
		return FALSE;

	if ((wdev->wdev_type == WDEV_TYPE_STA) && (conn_type == CONNECTION_INFRA_BC))
		features |= STA_REC_BASIC_STA_RECORD_BMC_FEATURE;

	if ((conn_type == CONNECTION_INFRA_BC) && WMODE_CAP_AX(wdev->PhyMode)) {
		/*for HE beacon set correct PE setting*/
#ifdef DOT11_HE_AX
#ifdef WIFI_UNIFIED_COMMAND
		features |= STA_REC_HE_BASIC_FEATURE;
#else
		features |= STA_REC_BASIC_HE_INFO_FEATURE;
#endif /* WIFI_UNIFIED_COMMAND */
#endif
	}
	if (conn_type & ~CONNECTION_INFRA_BC) {
		/*ht features */
#ifdef DOT11_N_SUPPORT
		starec_ht_feature_decision(wdev, entry, &features);
#ifdef DOT11_VHT_AC
		starec_vht_feature_decision(wdev, entry, &features);
#endif /*DOT11_VHT_AC*/
#ifdef TXBF_SUPPORT
		starec_txbf_feature_decision(wdev, entry, &features);
#endif /* TXBF_SUPPORT */
#ifdef DOT11_HE_AX
		starec_he_feature_decision(wdev, entry, &features);
		starec_muru_feature_decision(wdev, entry, &features);
#endif /*DOT11_HE_AX*/
#endif /*DOT11_N_SUPPORT*/
#ifdef DOT11_EHT_BE
		eht_starec_feature_decision(wdev, entry, &features);
#endif /* DOT11_EHT_BE */
		starec_feature_a4_info_decision(wdev, entry, &features);
#ifdef MLR_SUPPORT
		starec_mlr_feature_decision(wdev, entry, &features);
#endif /* MLR_SUPPORT */
	}

#ifdef CONFIG_AP_SUPPORT

	if (conn_type == CONNECTION_INFRA_STA)
		starec_ap_feature_decision(wdev, entry, &features);

#endif /*CONFIG_AP_SUPPORT*/

#ifdef AIR_MONITOR
	if (entry && IS_ENTRY_MONITOR(entry))
		features |= STA_REC_RCA1_FEATURE;
#endif /* AIR_MONITOR */

	/*return value, must use or operation*/
	*feature = features;
	return TRUE;
}

static UINT32 starec_security_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UCHAR *state)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _STA_TR_ENTRY *tr_entry;
	UCHAR port_sec = STATE_DISCONNECT;

	/*for bmc case*/
	if (!entry) {
		port_sec = STATE_PORT_SECURE;
		goto end;
	}

	/*for uc case*/
	tr_entry = tr_entry_get(ad, entry->wcid);

	switch (wdev->wdev_type) {
	case WDEV_TYPE_STA:
	case WDEV_TYPE_TDLS:
		if ((tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
			&& (IS_AKM_WPA_CAPABILITY_Entry(entry)
#ifdef WSC_INCLUDED
			|| ((wdev->WscControl.WscConfMode != WSC_DISABLE)
			&& (wdev->WscControl.bWscTrigger))
#endif
			))
			port_sec = STATE_CONNECTED;
		else if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
			port_sec = STATE_PORT_SECURE;

		break;

#ifdef CONFIG_AP_SUPPORT

	case WDEV_TYPE_AP: {
		if ((tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
			&& (IS_AKM_WPA_CAPABILITY_Entry(entry)
#ifdef DOT1X_SUPPORT
				|| IS_IEEE8021X(&entry->SecConfig)
#endif /* DOT1X_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
				|| wdev->IsCFG1xWdev
#endif /* RT_CFG80211_SUPPORT */
				|| entry->bWscCapable))
			port_sec = STATE_CONNECTED;
		else if ((tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
			|| (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)) { /* delay the port secured for open none seciruty */

#ifdef FAST_EAPOL_WAR
		/*
		*	set STATE_CONNECTED first in open security mode,
		*	after asso resp is sent out, then set STATE_PORT_SECURE.
		*/
			port_sec = STATE_CONNECTED;
#else /* FAST_EAPOL_WAR */
			port_sec = STATE_PORT_SECURE;
			CheckBMCPortSecured(ad, entry, TRUE);
#endif /* !FAST_EAPOL_WAR */
		}
	}
	break;
#endif /*CONFIG_AP_SUPPORT*/

	default:
		port_sec = STATE_PORT_SECURE;
		break;
	}

end:
	*state = port_sec;
	return TRUE;
}

/* add to handle wifi_sys operation race condition */
BOOLEAN wifi_sys_op_lock(struct wifi_dev *wdev)
{
	UINT32 ret = 0;
	BOOLEAN status;
	INT rt = 0;

	RTMP_SEM_EVENT_TIMEOUT(&wdev->wdev_op_lock, WIFI_LINK_MAX_TIME, ret);
	if (!ret) {
		rt = snprintf(wdev->dbg_wdev_op_lock_caller, sizeof(wdev->dbg_wdev_op_lock_caller), "%pS", OS_TRACE);
		if (os_snprintf_error(sizeof(wdev->dbg_wdev_op_lock_caller), rt)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_WARN,
					"final_name snprintf error!\n");
			return FALSE;
		}
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"%pS(%d): get wdev_op_lock.\n", OS_TRACE, wdev->wdev_idx);
		wdev->wdev_op_lock_flag = TRUE;
		status = TRUE;
	} else {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"%pS(%d): get wdev_op_lock fail!! The latest caller with the lock: %s\n",
			OS_TRACE, wdev->wdev_idx, wdev->dbg_wdev_op_lock_caller);
		/* force unlock */
		wdev->wdev_op_lock_flag = FALSE;
		NdisZeroMemory(wdev->dbg_wdev_op_lock_caller, sizeof(wdev->dbg_wdev_op_lock_caller));
		RTMP_SEM_EVENT_UP(&wdev->wdev_op_lock);
		status = FALSE;
	}

	return status;
}

/* add to handle wifi_sys operation race condition */
VOID wifi_sys_op_unlock(struct wifi_dev *wdev)
{
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		"%pS(%d): release wdev_op_lock.\n", OS_TRACE, wdev->wdev_idx);
	if (wdev->wdev_op_lock_flag) {
		wdev->wdev_op_lock_flag = FALSE;
		NdisZeroMemory(wdev->dbg_wdev_op_lock_caller, sizeof(wdev->dbg_wdev_op_lock_caller));
		RTMP_SEM_EVENT_UP(&wdev->wdev_op_lock);
	}
}

/*
*
*/
INT wifi_sys_open(struct wifi_dev *wdev)
{
	struct WIFI_SYS_CTRL *wsys;
	UINT32 ret;

	os_alloc_mem(NULL, (UCHAR **)&wsys, sizeof(struct WIFI_SYS_CTRL));

	if (!wsys) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"Allocate memory failed!");
		return FALSE;
	}

	os_zero_mem(wsys, sizeof(struct WIFI_SYS_CTRL));
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_WARN,
		"wdev idx = %d\n", wdev->wdev_idx);

	if (!wifi_sys_op_lock(wdev)) {
		os_free_mem(wsys);
		return FALSE;
	}

	if (!wdev->DevInfo.WdevActive && (wlan_operate_get_state(wdev) == WLAN_OPER_STATE_INVALID)) {
		wlan_operate_set_state(wdev, WLAN_OPER_STATE_VALID);
		/*acquire wdev related attribute*/
		if (wdev_attr_update(wdev->sys_handle, wdev) == FALSE) {
			wlan_operate_set_state(wdev, WLAN_OPER_STATE_INVALID);
			wifi_sys_op_unlock(wdev);
			os_free_mem(wsys);
			return FALSE;
		}
		/* init/re-init wdev fsm */
		wdev_fsm_init(wdev);
		fill_devinfo(wdev->sys_handle, wdev, TRUE, &wsys->DevInfoCtrl);
		wsys->wdev = wdev;
		/*update to hwctrl*/
		ret = HW_WIFISYS_OPEN(wdev->sys_handle, wsys);
		if (ret != NDIS_STATUS_SUCCESS && ret != NDIS_STATUS_TIMEOUT) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Enqueue failed!! wdev_idx=%d\n",
			wdev->wdev_idx);
			wifi_sys_op_unlock(wdev);
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"Not enqueue!! WdevActive=%d, wpf_op=%d\n",
			wdev->DevInfo.WdevActive, wlan_operate_get_state(wdev));
		wifi_sys_op_unlock(wdev);
	}

	if (wsys)
		os_free_mem(wsys);

	return TRUE;
}

/*
*
*/
INT wifi_sys_close(struct wifi_dev *wdev)
{
	struct WIFI_SYS_CTRL *wsys;
	UINT32 ret;

	os_alloc_mem(NULL, (UCHAR **)&wsys, sizeof(struct WIFI_SYS_CTRL));

	if (!wsys) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"Allocate memory failed!");
		return FALSE;
	}

	os_zero_mem(wsys, sizeof(struct WIFI_SYS_CTRL));
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_WARN,
		"wdev idx = %d\n", wdev->wdev_idx);

	if (!wifi_sys_op_lock(wdev)) {
		os_free_mem(wsys);
		return FALSE;
	}

	if (wlan_operate_get_state(wdev) == WLAN_OPER_STATE_VALID) {

		wlan_operate_set_state(wdev, WLAN_OPER_STATE_INVALID);

		/* Un-initialize the pDot11H of wdev */
		UpdateDot11hForWdev(wdev->sys_handle, wdev, FALSE);

		/* WDS share DevInfo with normal AP */
		if (wdev->wdev_type != WDEV_TYPE_WDS)
			fill_devinfo(wdev->sys_handle, wdev, FALSE, &wsys->DevInfoCtrl);

		wsys->wdev = wdev;
		/*notify other module will release hw resource*/
		call_wsys_notifieriers(WSYS_NOTIFY_CLOSE, wdev, NULL);
		/*update to hwctrl*/
		ret = HW_WIFISYS_CLOSE(wdev->sys_handle, wsys);
		if (ret != NDIS_STATUS_SUCCESS && ret != NDIS_STATUS_TIMEOUT) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Enqueue failed!! wdev_idx=%d\n",
			wdev->wdev_idx);
			wifi_sys_op_unlock(wdev);
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"Not enqueue!! WdevActive=%d, wpf_op=%d\n",
			wdev->DevInfo.WdevActive, wlan_operate_get_state(wdev));
		wifi_sys_op_unlock(wdev);
	}

	if (wsys)
		os_free_mem(wsys);

	return TRUE;
}

/*
*
*/
INT wifi_sys_disconn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	struct WIFI_SYS_CTRL *wsys;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _STA_TR_ENTRY *tr_entry = tr_entry_get(ad, entry->tr_tb_idx);
	struct _STA_REC_CTRL_T *new_sta;
	UINT32 ret;

	os_alloc_mem(NULL, (UCHAR **)&wsys, sizeof(struct WIFI_SYS_CTRL));
	if (!wsys) {
		MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "Allocate memory failed!");
		return FALSE;
	}
	os_zero_mem(wsys, sizeof(struct WIFI_SYS_CTRL));
	new_sta = &wsys->StaRecCtrl;

	if (!wifi_sys_op_lock(wdev)) {
		os_free_mem(wsys);
		return FALSE;
	}

	if (entry->EntryState == ENTRY_STATE_SYNC) {
		entry->EntryState = ENTRY_STATE_NONE;
		entry->WtblSetFlag = FALSE;
		/* Deactive StaRec in FW */
		new_sta->BssIndex = wdev->bss_info_argument.ucBssIndex;
		new_sta->WlanIdx = entry->wcid;
		new_sta->ConnectionType = entry->ConnectionType;
		new_sta->ConnectionState = STATE_DISCONNECT;
#ifdef DOT11_EHT_BE
		if (entry->mlo.mlo_en)
			new_sta->EnableFeature = STA_REC_MLD_TEARDOWN_FEATURE;
		else
#endif /* DOT11_EHT_BE */
			new_sta->EnableFeature = STA_REC_BASIC_STA_RECORD_FEATURE;
		new_sta->priv = tr_entry;

		if (IS_ENTRY_CLIENT(entry)) {
			/* there is no need to set txop when sta connected */
			wsys->skip_set_txop = TRUE;
		} else if (IS_ENTRY_REPEATER(entry)) {
			/* skip disable txop for repeater case since apcli is connected */
			wsys->skip_set_txop = TRUE;
		}
#ifdef AIR_MONITOR
		else if (IS_ENTRY_MONITOR(entry)) {
			/* there is no need to set txop in monitor mode */
			wsys->skip_set_txop = TRUE;
		}
#endif /* AIR_MONITOR */
		wsys->wdev = wdev;
#ifdef SW_CONNECT_SUPPORT
		if (IS_SW_MAIN_STA(tr_entry) || IS_SW_STA(tr_entry)) {
			if (IS_SW_STA(tr_entry)) {
				new_sta->WlanIdx = tr_entry->HwWcid;
				entry->sta_rec_valid = FALSE;
			}
			if (atomic_read(&ad->dummy_obj.connect_cnt) > 0)
				atomic_dec(&ad->dummy_obj.connect_cnt);
		}

		if ((atomic_read(&ad->dummy_obj.connect_cnt) > 0) &&
			(IS_SW_MAIN_STA(tr_entry) || IS_SW_STA(tr_entry)))
			wifi_sys_op_unlock(wdev);
		else
#endif /* SW_CONNECT_SUPPORT */
		{
			/*notify other module will release starec*/
			call_wsys_notifieriers(WSYS_NOTIFY_DISCONNT_ACT, wdev, tr_entry);
			/*send event for release starec*/
			ret = HW_WIFISYS_PEER_LINKDOWN(wdev->sys_handle, wsys);
			if (ret != NDIS_STATUS_SUCCESS && ret != NDIS_STATUS_TIMEOUT) {
				MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"Enqueue failed!! wdev_idx=%d\n",
				wdev->wdev_idx);
				wifi_sys_op_unlock(wdev);
			}
		}
#ifdef CONFIG_AP_SUPPORT

		if (wdev->wdev_type == WDEV_TYPE_AP)
			CheckBMCPortSecured(ad, entry, FALSE);

#endif /* CONFIG_AP_SUPPORT */
	} else {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"%s: Not enqueue!! entry->EntryState=%d\n",
			__func__, entry->EntryState);
		wifi_sys_op_unlock(wdev);
	}

	if (wsys)
		os_free_mem(wsys);

	return TRUE;
}

/* Updating STA Connection Status */
VOID update_sta_conn_state(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _STA_TR_ENTRY *tr_entry = tr_entry_get(ad, entry->tr_tb_idx);
	UCHAR state = 0;

	starec_security_decision(wdev, entry, &state);
	tr_entry->StaRec.ConnectionState = state;
}


INT wifi_sys_conn_act_FTM(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct WIFI_SYS_CTRL *wsys;
	struct _STA_REC_CTRL_T *sta_rec;
	struct _STA_TR_ENTRY *tr_entry = tr_entry_get(ad, entry->tr_tb_idx);
	struct _BSS_INFO_ARGUMENT_T *bss = &wdev->bss_info_argument;
	PEER_LINKUP_HWCTRL *lu_ctrl = NULL;
	UINT64 features = 0;
	UCHAR state = 0;
	UINT32 ret;

	os_alloc_mem(NULL, (UCHAR **)&wsys, sizeof(struct WIFI_SYS_CTRL));
	if (!wsys) {
		MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"Allocate memory failed!");
		return FALSE;
	}

	if (!wifi_sys_op_lock(wdev)) {
		os_free_mem(wsys);
		return FALSE;
	}

	/*indicate mac entry under sync to hw*/
	entry->EntryState = ENTRY_STATE_SYNC;
	/*star to fill wifi sys control*/
	os_zero_mem(wsys, sizeof(struct WIFI_SYS_CTRL));
	/*generic connection action*/
	/*sta rec feature & security update*/
	starec_feature_decision_FTM(wdev, entry->ConnectionType, entry, &features);
	starec_security_decision(wdev, entry, &state);

	/*prepare basic sta rec*/
	sta_rec = &wsys->StaRecCtrl;
	fill_starec(wdev, entry, tr_entry, sta_rec);
	sta_rec->BssIndex = bss->ucBssIndex;
	sta_rec->WlanIdx = entry->wcid;
	sta_rec->ConnectionType = entry->ConnectionType;
	sta_rec->ConnectionState = state;
	sta_rec->EnableFeature =  features;
	sta_rec->IsNewSTARec = TRUE;
	sta_rec->priv = tr_entry;
	/*prepare starec */
#ifdef SW_CONNECT_SUPPORT
	/* skip set wep key in S/W Entry */
	if (IS_NORMAL_STA(tr_entry))
#endif /* SW_CONNECT_SUPPORT */
	{
		if (sta_rec->EnableFeature & STA_REC_INSTALL_KEY_FEATURE)
			starec_security_set(wdev, entry, sta_rec);
	}

	/*specific part*/
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		wsys->skip_set_txop = TRUE;/* there is no need to set txop when sta connected.*/
		os_alloc_mem(NULL, (UCHAR **)&lu_ctrl, sizeof(PEER_LINKUP_HWCTRL));
		if (lu_ctrl == NULL) {
			MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Allocate memory failed!");
			wifi_sys_op_unlock(wdev);
			if (wsys)
				os_free_mem(wsys);
			return FALSE;
		}
		os_zero_mem(lu_ctrl, sizeof(PEER_LINKUP_HWCTRL));
		wsys->priv = (VOID *)lu_ctrl;
	}

	wsys->wdev = wdev;
#ifdef SW_CONNECT_SUPPORT
	/* Use the same existed sta rec if connect_cnt > 0 */
	if ((atomic_read(&ad->dummy_obj.connect_cnt) > 0) &&
		(IS_SW_MAIN_STA(tr_entry) || IS_SW_STA(tr_entry))) {
		atomic_inc(&ad->dummy_obj.connect_cnt);
		entry->sta_rec_valid = TRUE;
		wifi_sys_op_unlock(wdev);
		if (lu_ctrl != NULL)
			os_free_mem(lu_ctrl);
	} else
#endif /* SW_CONNECT_SUPPORT */
	{
		ret = HW_WIFISYS_PEER_LINKUP(ad, wsys);
		if (ret != NDIS_STATUS_SUCCESS && ret != NDIS_STATUS_TIMEOUT) {
			MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Enqueue failed!! wdev_idx=%d\n",
			wdev->wdev_idx);
			wifi_sys_op_unlock(wdev);
		}
#ifdef SW_CONNECT_SUPPORT
		if (IS_SW_MAIN_STA(tr_entry))
			atomic_inc(&ad->dummy_obj.connect_cnt);
#endif /* SW_CONNECT_SUPPORT */
	}
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
		"===> wcid=%d, PortSecured=%d, AKMMap=%d\n",
		entry->wcid, sta_rec->ConnectionState, entry->SecConfig.AKMMap);

#ifdef CONFIG_STA_SUPPORT
	/* WiFi Certification config per peer */
#ifdef SW_CONNECT_SUPPORT
	if (IS_NORMAL_STA(tr_entry))
#endif /* SW_CONNECT_SUPPORT */
		sta_set_wireless_sta_configs(ad, entry);
#endif /* CONFIG_STA_SUPPORT */

	if (wsys)
		os_free_mem(wsys);

	return TRUE;
}

/*wifi system connection action*/
INT wifi_sys_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct WIFI_SYS_CTRL *wsys;
	struct _STA_REC_CTRL_T *sta_rec;
	struct _STA_TR_ENTRY *tr_entry = tr_entry_get(ad, entry->tr_tb_idx);
	struct _BSS_INFO_ARGUMENT_T *bss = &wdev->bss_info_argument;
	PEER_LINKUP_HWCTRL *lu_ctrl = NULL;
	UINT64 features = 0;
	UCHAR state = 0;
	UINT32 ret;

	os_alloc_mem(NULL, (UCHAR **)&wsys, sizeof(struct WIFI_SYS_CTRL));
	if (!wsys) {
		MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"Allocate memory failed!");
		return FALSE;
	}

	if (!wifi_sys_op_lock(wdev)) {
		os_free_mem(wsys);
		return FALSE;
	}

	/*indicate mac entry under sync to hw*/
	entry->EntryState = ENTRY_STATE_SYNC;
	/*star to fill wifi sys control*/
	os_zero_mem(wsys, sizeof(struct WIFI_SYS_CTRL));
	/*generic connection action*/
	/*sta rec feature & security update*/
	starec_feature_decision(wdev, entry->ConnectionType, entry, &features);
	starec_security_decision(wdev, entry, &state);

	/*prepare basic sta rec*/
	sta_rec = &wsys->StaRecCtrl;
	fill_starec(wdev, entry, tr_entry, sta_rec);
	sta_rec->BssIndex = bss->ucBssIndex;
	sta_rec->WlanIdx = entry->wcid;
	sta_rec->ConnectionType = entry->ConnectionType;
	sta_rec->ConnectionState = state;
	sta_rec->EnableFeature =  features;
	sta_rec->IsNewSTARec = TRUE;
	sta_rec->priv = tr_entry;
	/*prepare starec */
#ifdef SW_CONNECT_SUPPORT
	/* skip set wep key in S/W Entry */
	if (IS_NORMAL_STA(tr_entry))
#endif /* SW_CONNECT_SUPPORT */
	{
		if (sta_rec->EnableFeature & STA_REC_INSTALL_KEY_FEATURE)
			starec_security_set(wdev, entry, sta_rec);
	}

	/*specific part*/
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		wsys->skip_set_txop = TRUE;/* there is no need to set txop when sta connected.*/
		os_alloc_mem(NULL, (UCHAR **)&lu_ctrl, sizeof(PEER_LINKUP_HWCTRL));
		if (lu_ctrl == NULL) {
			MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Allocate memory failed!");
			wifi_sys_op_unlock(wdev);
			if (wsys)
				os_free_mem(wsys);
			return FALSE;
		}
		os_zero_mem(lu_ctrl, sizeof(PEER_LINKUP_HWCTRL));
		wsys->priv = (VOID *)lu_ctrl;
	}

	wsys->wdev = wdev;
#ifdef SW_CONNECT_SUPPORT
	/* Use the same existed sta rec if connect_cnt > 0 */
	if ((atomic_read(&ad->dummy_obj.connect_cnt) > 0) &&
		(IS_SW_MAIN_STA(tr_entry) || IS_SW_STA(tr_entry))) {
		atomic_inc(&ad->dummy_obj.connect_cnt);
		entry->sta_rec_valid = TRUE;
		wifi_sys_op_unlock(wdev);
		if (lu_ctrl != NULL)
			os_free_mem(lu_ctrl);
	} else
#endif /* SW_CONNECT_SUPPORT */
	{
		ret = HW_WIFISYS_PEER_LINKUP(ad, wsys);
		if (ret != NDIS_STATUS_SUCCESS && ret != NDIS_STATUS_TIMEOUT) {
			MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Enqueue failed!! wdev_idx=%d\n",
			wdev->wdev_idx);
			if (lu_ctrl != NULL)
				os_free_mem(lu_ctrl);
			wifi_sys_op_unlock(wdev);
		}
#ifdef SW_CONNECT_SUPPORT
		if (IS_SW_MAIN_STA(tr_entry)) {
			entry->sta_rec_valid = TRUE;
			atomic_inc(&ad->dummy_obj.connect_cnt);
		}
#endif /* SW_CONNECT_SUPPORT */
	}
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
		"===> wcid=%d, PortSecured=%d, AKMMap=%d\n",
		entry->wcid, sta_rec->ConnectionState, entry->SecConfig.AKMMap);

#ifdef CONFIG_STA_SUPPORT
	/* WiFi Certification config per peer */
#ifdef SW_CONNECT_SUPPORT
	if (IS_NORMAL_STA(tr_entry))
#endif /* SW_CONNECT_SUPPORT */
		sta_set_wireless_sta_configs(ad, entry);
#endif /* CONFIG_STA_SUPPORT */

	if (wsys)
		os_free_mem(wsys);

	return TRUE;
}

/*
*
*/
INT wifi_sys_linkup(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	struct WIFI_SYS_CTRL *wsys;
	STA_REC_CTRL_T *sta_rec;
	struct _STA_TR_ENTRY *tr_entry;
	UCHAR state = 0;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _BSS_INFO_ARGUMENT_T *bss;
	UINT64 features = 0;
	UINT16 wcid;
	UCHAR *addr = NULL;
	UINT32 ret;

	MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_WARN,
		"wdev idx = %d\n", wdev->wdev_idx);

	os_alloc_mem(NULL, (UCHAR **)&wsys, sizeof(struct WIFI_SYS_CTRL));
	if (!wsys) {
		MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"Allocate memory failed!");
		return FALSE;
	}

	/*if interface down up should not run ap link up (for apstop/apstart check)*/
	if (!HcIsRadioAcq(wdev)) {
		os_free_mem(wsys);
		return TRUE;
	}
	os_zero_mem(wsys, sizeof(struct WIFI_SYS_CTRL));

	bss = &wsys->BssInfoCtrl;

	if (!wifi_sys_op_lock(wdev)) {
		os_free_mem(wsys);
		return FALSE;
	}

	if (WDEV_BSS_STATE(wdev) == BSS_INIT) {
#ifdef CONFIG_AP_SUPPORT
		ap_key_table_init(ad, wdev);
#endif
		/*prepare bssinfo*/
		if (wdev->wdev_type == WDEV_TYPE_STA && entry)
			COPY_MAC_ADDR(wdev->bssid, entry->Addr);
		fill_bssinfo(wdev->sys_handle, wdev, TRUE, bss);
		/* Need to update ucBssIndex of wdev here immediately because
		it could be used if invoke wifi_sys_conn_act subsequently. */
		wdev->bss_info_argument.ucBssIndex = bss->ucBssIndex;

		MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"wdev(type=%d,func_idx=%d,wdev_idx=%d),BssIndex(%d)\n",
			wdev->wdev_idx, wdev->wdev_type, wdev->func_idx,
			wdev->bss_info_argument.ucBssIndex);

#ifdef CONFIG_OWE_SUPPORT
		/* To allow OWE ApCli to connect to Open bss */
		if (((wdev->wdev_type == WDEV_TYPE_STA) && (IS_AKM_OWE(wdev->SecConfig.AKMMap)) && entry)) {
			if (IS_AKM_OPEN_ONLY(entry->SecConfig.AKMMap) && IS_CIPHER_NONE(entry->SecConfig.PairwiseCipher)) {
				bss->CipherSuit = SecHWCipherSuitMapping(entry->SecConfig.PairwiseCipher);
			}
		}
#endif
		wcid = (entry) ? entry->wcid : bss->bmc_wlan_idx;
		bssinfo_feature_decision(wdev, bss->u4ConnectionType, wcid, &features);
		bss->peer_wlan_idx = wcid;
		bss->u8BssInfoFeature = features;
		/*wds type should not this bmc starec*/
		if (wdev->wdev_type == WDEV_TYPE_WDS) {
			bss->bmc_wlan_idx = wcid;
		} else {
			/*prepare bmc starec*/
			starec_feature_decision(wdev, CONNECTION_INFRA_BC, NULL, &features);
			starec_security_decision(wdev, NULL, &state);

#ifdef MBSS_AS_WDS_AP_SUPPORT
			if (wdev->wds_enable) {
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"WDS Enable setting 4 address mode for %d entry\n",
					bss->bmc_wlan_idx);
				HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(ad, bss->bmc_wlan_idx, TRUE);
			}
#endif
			/*1. get tr entry here, since bss info is acquired above */
			tr_entry = tr_entry_get(ad, wdev->tr_tb_idx);
			sta_rec = &wsys->StaRecCtrl;
			/* BC sta record should always set STATE_PORT_SECURE*/
			sta_rec->BssIndex = bss->ucBssIndex;
			sta_rec->WlanIdx = bss->bmc_wlan_idx;
			sta_rec->WlanIdx2 = bss->bmc_wlan_idx2;
			sta_rec->ConnectionState = state;
			if (wdev->wdev_type == WDEV_TYPE_STA)
				sta_rec->ConnectionType = CONNECTION_INFRA_BC | STA_TYPE_AP;
			else
				sta_rec->ConnectionType = CONNECTION_INFRA_BC;
			sta_rec->EnableFeature = features;
			sta_rec->IsNewSTARec = TRUE;
			sta_rec->priv = tr_entry;
#ifdef CONFIG_AP_SUPPORT

			if (wdev->wdev_type == WDEV_TYPE_AP) {
#ifndef RT_CFG80211_SUPPORT
				ap_set_key_for_sta_rec(ad, wdev, sta_rec);
#endif
				/* register device to bss manager */
				bss_mngr_dev_reg(wdev);
#ifdef DOT11_EHT_BE
				/* sync ML Probe.rsp Per-STA Profile buffer */
				bss_mngr_mld_sync_ml_probe_rsp(wdev);

				wifi_sys_bss_query_mld(wdev, &bss->mld_info);
#ifdef RT_CFG80211_SUPPORT
				if (!(bss->mld_info.mld_group_idx == 0 ||
						bss->mld_info.mld_group_idx == MLD_GROUP_NONE)) {
					mtk_cfg80211_send_bss_ml_event(wdev, CFG80211_ML_EVENT_ADDLINK);
				}
#endif /*RT_CFG80211_SUPPORT*/
#endif /* DOT11_EHT_BE */
			}
#endif /*CONFIG_AP_SUPPORT*/

			/*BMC STAREC*/
			addr  = (entry) ? entry->Addr : wdev->bssid;
			os_move_mem(tr_entry->Addr, addr, MAC_ADDR_LEN);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"===> wcid=%d, PortSecured=%d\n",
				bss->bmc_wlan_idx, sta_rec->ConnectionState);
		}
		/*update to hw ctrl*/
		wsys->wdev = wdev;
		ret = HW_WIFISYS_LINKUP(ad, wsys);
		if (ret != NDIS_STATUS_SUCCESS && ret != NDIS_STATUS_TIMEOUT) {
			MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Enqueue failed!! wdev_idx=%d\n",
			wdev->wdev_idx);
			wifi_sys_op_unlock(wdev);
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"Not enqueue!! bss_state=%d\n", WDEV_BSS_STATE(wdev));
		wifi_sys_op_unlock(wdev);
	}

	if (wsys)
		os_free_mem(wsys);

	return TRUE;
}

INT wifi_sys_linkdown(struct wifi_dev *wdev)
{
	struct WIFI_SYS_CTRL *wsys;
	struct _STA_REC_CTRL_T *sta_rec;
	struct _STA_TR_ENTRY *tr_entry = NULL;
	struct _BSS_INFO_ARGUMENT_T *bss;
	UINT64 features = 0;
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	UINT32 ret;

	MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "wdev idx = %d\n", wdev->wdev_idx);
	os_alloc_mem(NULL, (UCHAR **)&wsys, sizeof(struct WIFI_SYS_CTRL));
	if (!wsys) {
		MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "Allocate memory failed!");
		return FALSE;
	}
	os_zero_mem(wsys, sizeof(struct WIFI_SYS_CTRL));
	MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_WARN, "wdev idx = %d\n", wdev->wdev_idx);

	bss = &wsys->BssInfoCtrl;

	if (!wifi_sys_op_lock(wdev)) {
		os_free_mem(wsys);
		return FALSE;
	}

	if (WDEV_BSS_STATE(wdev) >= BSS_INITED) {
		fill_bssinfo(ad, wdev, FALSE, bss);

		MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"wdev(type=%d,func_idx=%d,wdev_idx=%d),BssIndex(%d)\n",
			wdev->wdev_type, wdev->func_idx, wdev->wdev_idx,
			wdev->bss_info_argument.ucBssIndex);

		bssinfo_feature_decision(wdev, bss->u4ConnectionType, bss->bmc_wlan_idx, &features);
		/* from fw sw request, do not include BSS_INFO_MLD when apcli link down*/
		if (wdev->wdev_type == WDEV_TYPE_STA)
			features &= ~BSS_INFO_MLD_FEATURE;
		bss->u8BssInfoFeature = features;
		/* refill BandIdx that be cleared in fill_bssinfo */
		bss->ucBandIdx = HcGetBandByWdev(wdev);
		/*wds should not bmc starec*/
		if (wdev->wdev_type != WDEV_TYPE_WDS) {
			/*update sta rec.*/
			/*1. get tr entry here, since bss info is acquired above */
			sta_rec = &wsys->StaRecCtrl;
			tr_entry = tr_entry_get(ad, bss->bmc_wlan_idx);
			if (tr_entry->StaRec.ConnectionState) {
				sta_rec->ConnectionState = STATE_DISCONNECT;
				sta_rec->EnableFeature = STA_REC_BASIC_STA_RECORD_FEATURE;
				sta_rec->BssIndex = bss->ucBssIndex;
				sta_rec->ConnectionType = CONNECTION_INFRA_BC;
				sta_rec->WlanIdx = bss->bmc_wlan_idx;
				sta_rec->priv = tr_entry;
			}

			if (wdev->wdev_type == WDEV_TYPE_STA)
				ad->fgApCliBfStaRecRegister = FALSE;
		}

		/* for bmc starec */
		if (tr_entry
			&& tr_entry->StaRec.ConnectionState && (wdev->wdev_type == WDEV_TYPE_STA)) {
			sta_rec->ConnectionType |=  STA_TYPE_AP;
			sta_rec->EnableFeature |= STA_REC_BASIC_STA_RECORD_BMC_FEATURE;
			sta_rec->WlanIdx2 = bss->bmc_wlan_idx2;
		}

		if (wdev->wdev_type == WDEV_TYPE_AP) {
			/* de-register device to bss manager */
			bss_mngr_dev_dereg(wdev);
#ifdef DOT11_EHT_BE
			/* sync ML Probe.rsp Per-STA Profile buffer */
			bss_mngr_mld_sync_ml_probe_rsp(wdev);
#ifdef RT_CFG80211_SUPPORT
			mtk_cfg80211_send_bss_ml_event(wdev, CFG80211_ML_EVENT_DELLINK);
#endif /*RT_CFG80211_SUPPORT*/

#endif
		}

		/*update to hwctrl for hw seting*/
		wsys->wdev = wdev;
		/*notify other module will leave bss*/
		call_wsys_notifieriers(WSYS_NOTIFY_LINKDOWN, wdev, NULL);
		ret = HW_WIFISYS_LINKDOWN(ad, wsys);
		if (ret != NDIS_STATUS_SUCCESS && ret != NDIS_STATUS_TIMEOUT) {
			MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Enqueue failed!! wdev_idx=%d\n",
			wdev->wdev_idx);
			wifi_sys_op_unlock(wdev);
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"Not enqueue!! bss_state=%d\n",
			WDEV_BSS_STATE(wdev));
		wifi_sys_op_unlock(wdev);
	}

	if (wsys)
		os_free_mem(wsys);

	return TRUE;
}

VOID wifi_mlme_ops_register(struct wifi_dev *wdev)
{
	switch (wdev->wdev_type) {
#ifdef CONFIG_AP_SUPPORT
	case WDEV_TYPE_AP:
		ap_fsm_ops_hook(wdev);
		break;
#endif /*CONFIG_AP_SUPPORT*/

#ifdef CONFIG_STA_SUPPORT
	case WDEV_TYPE_ADHOC:
	case WDEV_TYPE_STA:
		sta_fsm_ops_hook(wdev);
		break;
#ifdef MAC_REPEATER_SUPPORT
case WDEV_TYPE_REPEATER:
		sta_fsm_ops_hook(wdev);
		break;
#endif /* MAC_REPEATER_SUPPORT */
#endif /*CONFIG_STA_SUPPORT*/
	}

}

/*
*
*/
VOID WifiSysUpdatePortSecur(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, ASIC_SEC_INFO *asic_sec_info)
{
	struct WIFI_SYS_CTRL wsys;
	struct wifi_dev *wdev = pEntry->wdev;
	struct _STA_REC_CTRL_T *sta_ctrl = &wsys.StaRecCtrl;
	struct _STA_TR_ENTRY *tr_entry = tr_entry_get(pAd, pEntry->tr_tb_idx);
	struct _STA_REC_CTRL_T *org = &tr_entry->StaRec;

	if (org->ConnectionState) {
		os_zero_mem(&wsys, sizeof(wsys));
		sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
		sta_ctrl->ConnectionState = STATE_PORT_SECURE;
		sta_ctrl->ConnectionType = pEntry->ConnectionType;
		sta_ctrl->EnableFeature = STA_REC_BASIC_STA_RECORD_FEATURE;
		sta_ctrl->WlanIdx = pEntry->wcid;
		sta_ctrl->IsNewSTARec = FALSE;
		sta_ctrl->priv = tr_entry;
		if (asic_sec_info) {
			sta_ctrl->EnableFeature |= STA_REC_INSTALL_KEY_FEATURE;
			NdisMoveMemory(&sta_ctrl->asic_sec_info, asic_sec_info, sizeof(ASIC_SEC_INFO));
		}
#ifdef SW_CONNECT_SUPPORT
		if (IS_SW_MAIN_STA(tr_entry) || IS_SW_STA(tr_entry)) {
			if (asic_sec_info)
				sta_ctrl->EnableFeature &= ~STA_REC_INSTALL_KEY_FEATURE;

			/*Fill the real H/W wcid for WM */
			if (IS_SW_STA(tr_entry))
				sta_ctrl->WlanIdx = tr_entry->HwWcid;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"entry->wcid=%u, entry->hw_wcid=%u, entry->Addr="MACSTR"\n",
					 pEntry->wcid, tr_entry->HwWcid, MAC2STR(pEntry->Addr));
		}
#endif /* SW_CONNECT_SUPPORT */

		wsys.wdev = wdev;
		HW_WIFISYS_PEER_UPDATE(pAd, &wsys);
#ifdef CONFIG_AP_SUPPORT
		CheckBMCPortSecured(pAd, pEntry, TRUE);
#endif /* CONFIG_AP_SUPPORT */
#ifdef APCLI_AS_WDS_STA_SUPPORT
	if (IS_ENTRY_PEER_AP(pEntry)) {
		pEntry->bEnable4Addr = TRUE;
			if (wdev->wds_enable) {
#ifdef WIFI_UNIFIED_COMMAND
				if (wifi_sys_header_trans_update(pAd, pEntry) != NDIS_STATUS_SUCCESS)
					MTWF_PRINT("WDS AP Address Mode failed to set for wcid %d\n", pEntry->wcid);
#else

				HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pEntry->wcid, TRUE);
#endif
			}
	}
#endif /* APCLI_AS_WDS_STA_SUPPORT */

#ifdef MBSS_AS_WDS_AP_SUPPORT
	if (IS_ENTRY_CLIENT(pEntry)) {
		pEntry->bEnable4Addr = TRUE;
		if (wdev->wds_enable) {
#ifdef WIFI_UNIFIED_COMMAND
			if (wifi_sys_header_trans_update(pAd, pEntry) != NDIS_STATUS_SUCCESS)
				MTWF_PRINT("WDS AP Address Mode failed to set for wcid %d\n", pEntry->wcid);
#else


			HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pEntry->wcid, TRUE);
#endif
		}

		else if (MAC_ADDR_EQUAL(pAd->ApCfg.wds_mac, pEntry->Addr)) {
#ifdef WIFI_UNIFIED_COMMAND
			if (wifi_sys_header_trans_update(pAd, pEntry) != NDIS_STATUS_SUCCESS)
				MTWF_PRINT("WDS AP Address Mode failed to set for wcid %d\n", pEntry->wcid);
#else

			HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pEntry->wcid, TRUE);
#endif
			}

		}
#endif
#if defined(HOSTAPD_MAP_SUPPORT) || defined(MTK_HOSTAPD_SUPPORT)
		if (IS_ENTRY_CLIENT(pEntry)) {
			BOOLEAN map_a4_peer_en = FALSE;
#if defined(MWDS) || defined(CONFIG_MAP_SUPPORT) || defined(WAPP_SUPPORT)
#ifdef MWDS
			MWDSAPPeerEnable(pAd, pEntry);
#endif /* MWDS */
#if defined(CONFIG_MAP_SUPPORT)
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "MAP_ENABLE\n");
#if defined(A4_CONN)
			if (IS_MAP_ENABLE(pAd)) {
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
				if (pEntry->mlo.mlo_en)
					map_a4_mlo_peer_enable(pAd, NULL, pEntry, TRUE);
				else
#endif
					map_a4_peer_en = map_a4_peer_enable(pAd, pEntry, TRUE);
			}
#endif /* A4_CONN */
			/*comment:map_send_bh_sta_wps_done_event(pAd, pEntry, TRUE);*/
#endif /* CONFIG_MAP_SUPPORT */
#ifdef WAPP_SUPPORT
			if (!pEntry->is_cli_join_sent) {
				wapp_send_cli_join_event(pAd, pEntry);
				pEntry->is_cli_join_sent = 1;
#if (defined(MAP_R6) && defined(DOT11_EHT_BE))
			if (IS_ENTRY_CLIENT(pEntry) && wdev) {
				if (pEntry->mlo.mlo_en && pEntry->mlo.is_setup_link_entry) {
					if ((IS_MAP_ENABLE(pAd)) && (IS_MAP_CERT_ENABLE(pAd))) {
						/* call function with event WAPP_STA_INFO */
						wext_send_sta_info(pAd, wdev, pEntry);
					}
				}
			}
#endif
		}
#endif /* WAPP_SUPPORT */
#endif /* defined(MWDS) || defined(CONFIG_MAP_SUPPORT) || defined(WAPP_SUPPORT) */
		}
#endif /* HOSTAPD_MAP_SUPPORT */

	}
}

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
/*
*
*/
VOID WifiSysRaInit(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	struct WIFI_SYS_CTRL wsys;
	struct _STA_REC_CTRL_T *sta_rec = &wsys.StaRecCtrl;
	struct _STA_TR_ENTRY *tr_entry = tr_entry_get(pAd, pEntry->tr_tb_idx);

	os_zero_mem(&wsys, sizeof(wsys));
	sta_rec->BssIndex = pEntry->wdev->bss_info_argument.ucBssIndex;
	sta_rec->WlanIdx = pEntry->wcid;
	sta_rec->ConnectionType = pEntry->ConnectionType;
	sta_rec->ConnectionState = STATE_CONNECTED;
	sta_rec->EnableFeature = STA_REC_RA_FEATURE;
	sta_rec->priv = tr_entry;
	wsys.wdev = pEntry->wdev;

#ifdef SW_CONNECT_SUPPORT
	if (IS_SW_STA(tr_entry)) {
		/*Fill the real H/W wcid for WM */
		sta_rec->WlanIdx = tr_entry->HwWcid;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"entry->wcid=%u, entry->hw_wcid=%u, entry->Addr="MACSTR"\n",
				 pEntry->wcid, tr_entry->HwWcid, MAC2STR(pEntry->Addr));
	}
#endif /* SW_CONNECT_SUPPORT */

	HW_WIFISYS_PEER_UPDATE(pAd, &wsys);
}


/*
*
*/
VOID WifiSysUpdateRa(RTMP_ADAPTER *pAd,
					 MAC_TABLE_ENTRY *pEntry,
					 P_CMD_STAREC_AUTO_RATE_UPDATE_T prParam
					)
{
	struct WIFI_SYS_CTRL wsys;
	struct _STA_REC_CTRL_T *sta_rec = &wsys.StaRecCtrl;
	struct _STA_TR_ENTRY *tr_entry = tr_entry_get(pAd, pEntry->tr_tb_idx);
	CMD_STAREC_AUTO_RATE_UPDATE_T *ra_parm = NULL;

	os_zero_mem(&wsys, sizeof(wsys));
	sta_rec->BssIndex = pEntry->wdev->bss_info_argument.ucBssIndex;
	sta_rec->WlanIdx = pEntry->wcid;
	sta_rec->ConnectionType = pEntry->ConnectionType;
	sta_rec->ConnectionState = STATE_CONNECTED;
	sta_rec->EnableFeature = STA_REC_RA_UPDATE_FEATURE;
	sta_rec->priv = tr_entry;
	os_alloc_mem(NULL, (UCHAR **)&ra_parm, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
	if (!ra_parm) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				 "mem alloc ra_parm failed\n");
		return;
	}
	os_move_mem(ra_parm, prParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
	wsys.priv = (VOID *)ra_parm;
	wsys.wdev = pEntry->wdev;

#ifdef SW_CONNECT_SUPPORT
	if (IS_SW_STA(tr_entry)) {
		/*Fill the real H/W wcid for WM */
		sta_rec->WlanIdx = tr_entry->HwWcid;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"entry->wcid=%u, entry->hw_wcid=%u, entry->Addr="MACSTR"\n",
				 pEntry->wcid, tr_entry->HwWcid, MAC2STR(pEntry->Addr));
	}
#endif /* SW_CONNECT_SUPPORT */

	HW_WIFISYS_RA_UPDATE(pAd, &wsys);
}
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/

VOID wifi_sys_update_wds(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	struct WIFI_SYS_CTRL wsys;
	struct wifi_dev *wdev = pEntry->wdev;
	struct _STA_REC_CTRL_T *sta_ctrl = &wsys.StaRecCtrl;
	struct _STA_TR_ENTRY *tr_entry = tr_entry_get(pAd, pEntry->tr_tb_idx);
	struct _STA_REC_CTRL_T *org = &tr_entry->StaRec;
	UINT64 features = 0;

	if (org->ConnectionState) {
		os_zero_mem(&wsys, sizeof(wsys));
		starec_feature_decision(wdev, pEntry->ConnectionType, pEntry, &features);
		fill_starec(wdev, pEntry, tr_entry, sta_ctrl);
		sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
		sta_ctrl->ConnectionState = org->ConnectionState;
		sta_ctrl->ConnectionType = pEntry->ConnectionType;
		sta_ctrl->WlanIdx = pEntry->wcid;
		sta_ctrl->EnableFeature =  features;
		sta_ctrl->IsNewSTARec = FALSE;
		sta_ctrl->priv = tr_entry;
		wsys.wdev = wdev;
		HW_WIFISYS_PEER_UPDATE(pAd, &wsys);
	}
}

UINT32 wifi_sys_header_trans_update(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	struct WIFI_SYS_CTRL wsys;
	struct wifi_dev *wdev = pEntry->wdev;
	struct _STA_REC_CTRL_T *sta_ctrl = &wsys.StaRecCtrl;
	struct _STA_TR_ENTRY *tr_entry = tr_entry_get(pAd, pEntry->tr_tb_idx);
	struct _STA_REC_CTRL_T *org = &tr_entry->StaRec;

	if (org->ConnectionState) {
		os_zero_mem(&wsys, sizeof(wsys));
		fill_starec(wdev, pEntry, tr_entry, sta_ctrl);
		sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
		sta_ctrl->ConnectionState = org->ConnectionState;
		sta_ctrl->ConnectionType = pEntry->ConnectionType;
		sta_ctrl->WlanIdx = pEntry->wcid;
		sta_ctrl->EnableFeature = STA_REC_HDR_TRANS_FEATURE;
		sta_ctrl->IsNewSTARec = FALSE;
		sta_ctrl->priv = tr_entry;
		wsys.wdev = wdev;
		return HW_WIFISYS_PEER_UPDATE(pAd, &wsys);
	}

	return NDIS_STATUS_FAILURE;
}

#ifdef DOT11_EHT_BE
INT wifi_sys_bss_query_mld(struct wifi_dev *wdev, struct bss_mld_info *mld_info)
{
	struct _RTMP_ADAPTER *ad;
	struct query_mld_ap_basic bss_mld_info_basic = {0};
	struct _BSS_STRUCT *mbss = NULL;
	uint8_t mld_group_idx = 0;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
			"wdev=NULL,fail");
		return NDIS_STATUS_FAILURE;
	}

	ad = (PRTMP_ADAPTER)wdev->sys_handle;

	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR, "ad=NULL,fail");
		return NDIS_STATUS_FAILURE;
	}
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		mbss = &ad->ApCfg.MBSSID[wdev->func_idx];
		mld_group_idx = mbss->mld_grp_idx;
		if (!WMODE_CAP_BE(wdev->PhyMode) || (mld_group_idx == 0)) {
			/* legacy case */
			mld_info->mld_group_idx = MLD_GROUP_NONE;
			NdisCopyMemory(mld_info->mld_addr, ZERO_MAC_ADDR, MAC_ADDR_LEN);
		} else {
			/* mld case */
			bss_mngr_query_mld_ap_basic(wdev, &bss_mld_info_basic);
			mld_info->mld_group_idx = mld_group_idx;
			mld_info->link_id = bss_mld_info_basic.link_id;
			NdisCopyMemory(mld_info->mld_addr, bss_mld_info_basic.addr, MAC_ADDR_LEN);
		}
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_NOTICE,
			"(in):lk_cnt=%d,lk_id=%d,mld_gp_idx=%d,mld_addr=%pM\n",
			bss_mld_info_basic.link_cnt,
			bss_mld_info_basic.link_id,
			mld_group_idx,
			bss_mld_info_basic.addr);
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
		if (wdev->mld_dev && wdev->mld_dev->peer_mld.valid && wdev->mld_dev->master_link) {
			mld_info->mld_group_idx = wdev->mld_dev->mld_grp_idx;
			NdisCopyMemory(mld_info->mld_addr, wdev->mld_dev->master_link->wdev->if_addr, MAC_ADDR_LEN);
		} else {
			mld_info->mld_group_idx = MLD_GROUP_NONE;
			NdisCopyMemory(mld_info->mld_addr, ZERO_MAC_ADDR, MAC_ADDR_LEN);
		}
	}
	wlan_operate_set_mld_addr(wdev, mld_info->mld_addr);
	os_move_mem(&wdev->bss_info_argument.mld_info, mld_info, sizeof(struct bss_mld_info));
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_NOTICE,
		"(out):mld_gp_idx=%d,mld_addr=%pM\n",
		mld_info->mld_group_idx,
		mld_info->mld_addr);
	return NDIS_STATUS_SUCCESS;
}
#ifdef RT_CFG80211_SUPPORT
VOID mtk_fill_bss_ml_info_event(struct wifi_dev *wdev, struct bss_mld_info *mld_info, enum CFG80211_ML_EVENT event_id)
{
	struct query_mld_ap_basic bss_mld_info_basic = {0};

	bss_mngr_query_mld_ap_basic(wdev, &bss_mld_info_basic);
	wdev->ml_event.event_id = event_id;
	wdev->ml_event.link_id =  bss_mld_info_basic.link_id;
	wdev->ml_event.mld_grp_idx = mld_info->mld_group_idx;
	NdisCopyMemory(&wdev->ml_event.addr[0], mld_info->mld_addr, MAC_ADDR_LEN);
}
#endif
#endif /* DOT11_EHT_BE */

#ifdef CONFIG_VLAN_GTK_SUPPORT
INT wifi_vlan_starec_linkup(struct wifi_dev *wdev, int bmc_idx)
{
	STA_REC_CTRL_T *sta_rec;
	struct _STA_TR_ENTRY *tr_entry;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	UINT32 features = 0;

	/*get tr entry here*/
	tr_entry = tr_entry_get(ad, bmc_idx);
	os_move_mem(tr_entry->Addr, wdev->bssid, MAC_ADDR_LEN);
	sta_rec = &tr_entry->StaRec;
	sta_rec->BssIndex = wdev->bss_info_argument.ucBssIndex;
	sta_rec->WlanIdx = bmc_idx;
	/* BC sta record should always set STATE_PORT_SECURE */
	sta_rec->ConnectionState = STATE_PORT_SECURE;
	sta_rec->ConnectionType = CONNECTION_INFRA_BC;
	features = (BSS_INFO_OWN_MAC_FEATURE
			| BSS_INFO_BASIC_FEATURE
			| BSS_INFO_BROADCAST_INFO_FEATURE
			| STA_REC_TX_PROC_FEATURE
			| STA_REC_WTBL_FEATURE);
	sta_rec->EnableFeature = features;
	sta_rec->IsNewSTARec = TRUE;
	sta_rec->priv = tr_entry;

	/*update starec to tr_entry*/
	ap_set_key_for_sta_rec(ad, wdev, sta_rec);
	/*ap_set_key_for_sta_rec will reset tr_entry->PortSecured*/
	tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
	AsicStaRecUpdate(ad, sta_rec);
	wifi_sys_update_starec_info(ad, sta_rec);

	return TRUE;
}
#endif
