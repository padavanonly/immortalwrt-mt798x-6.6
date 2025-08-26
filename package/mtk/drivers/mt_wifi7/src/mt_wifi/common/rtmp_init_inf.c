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
	rtmp_init_inf.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#include	"rt_config.h"
#ifdef DOT11R_FT_SUPPORT
#include	"ft/ft.h"
#endif /* DOT11R_FT_SUPPORT */
#ifdef CFG_SUPPORT_FALCON_SR
#include "sr_cmd.h"
#endif

#define PROBE2LOAD_L1PROFILE /* Capable to be turned off if not required */

#ifdef CONFIG_STA_SUPPORT
#ifdef PROFILE_STORE
NDIS_STATUS WriteDatThread(RTMP_ADAPTER *pAd);
#endif /* PROFILE_STORE */
#endif /* CONFIG_STA_SUPPORT */

INT rtmp_cfg_exit(RTMP_ADAPTER *pAd)
{
	UserCfgExit(pAd);
	return TRUE;
}


INT rtmp_cfg_init(RTMP_ADAPTER *pAd, RTMP_STRING *pHostName)
{
#ifndef CONFIG_PROFILE_OFF
	NDIS_STATUS status;
#endif /* !CONFIG_PROFILE_OFF */

/* For Vxworks we have already configured through IOCTL instead of profile, no need to init user cfg again */
	UserCfgInit(pAd);

#ifdef FTM_SUPPORT
	FtmMgmtInitByWdev(pAd);
#endif /* FTM_SUPPORT */
#ifdef MBO_SUPPORT
	MboInit(pAd);
#endif /* MBO_SUPPORT */
#ifdef OCE_SUPPORT
	OceInit(pAd);
#endif /* OCE_SUPPORT */
	CfgInitHook(pAd);
#ifdef DPP_SUPPORT
	pAd->dpp_rx_frm_counter = 0;
#endif /* DPP_SUPPORT */

	/*
		WiFi system operation mode setting base on following partitions:
		1. Parameters from config file
		2. Hardware cap from EEPROM
		3. Chip capabilities in code
	*/
	if (pAd->RfIcType == 0) {
		/* RfIcType not assigned, should not happened! */
		pAd->RfIcType = RFIC_UNKNOWN;
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			" Invalid RfIcType, reset it first\n");
	}

#ifndef CONFIG_PROFILE_OFF
	status = RTMPReadParametersHook(pAd);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"RTMPReadParametersHook failed, Status[=0x%08x]\n", status);
		return FALSE;
	}
#endif /* !CONFIG_PROFILE_OFF */

#ifdef OCE_SUPPORT
	OceTimerInit(pAd);
#endif /* OCE_SUPPORT */
	entrytb_aid_bitmap_init(&pAd->aid_info);
	entrytb_aid_bitmap_reserve(pAd, &pAd->aid_info);

	return TRUE;
}


INT rtmp_mgmt_init(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


static INT rtmp_sys_exit(RTMP_ADAPTER *pAd)
{
	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);
#ifdef DOT11_HE_AX
	bss_color_table_deinit(pAd);
#ifdef WIFI_TWT_SUPPORT
	twt_ctrl_exit(pAd);
#endif /* DOT11_HE_AX */
#endif /* WIFI_TWT_SUPPORT */
	rtmp_cfg_exit(pAd);
	HwCtrlExit(pAd);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);
	bcn_bpcc_TaskExit(pAd);
	RtmpMgmtTaskExit(pAd);
#ifdef RTMP_TIMER_TASK_SUPPORT
	if (IS_ASIC_CAP(pAd, fASIC_CAP_MGMT_TIMER_TASK))
		NdisFreeSpinLock(&pAd->TimerQLock);
#endif
	return TRUE;
}


static INT rtmp_sys_init(RTMP_ADAPTER *pAd, RTMP_STRING *pHostName)
{
	NDIS_STATUS status;

#ifdef DBG_STARVATION
	starv_log_init(&pAd->starv_log_ctrl);
#endif /*DBG_STARVATION*/

	status = RtmpMgmtTaskInit(pAd);

	if (status != NDIS_STATUS_SUCCESS)
		goto err0;

	status = bcn_bpcc_TaskInit(pAd);

	if (status != NDIS_STATUS_SUCCESS)
		goto err0;

	status = HwCtrlInit(pAd);

	if (status != NDIS_STATUS_SUCCESS)
		goto err1;

	/* Initialize pAd->StaCfg[], pAd->ApCfg, pAd->CommonCfg to manufacture default*/
	if (rtmp_cfg_init(pAd, pHostName) != TRUE) {
		MTWF_PRINT("%s: suggest to use l1profile. add l1profile.dat to /etc/wireless/.\n",
			__func__);
		goto err2;
	}

	status = MeasureReqTabInit(pAd);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"MeasureReqTabInit failed, Status[=0x%08x]\n", status);
		goto err2;
	}

	status = TpcReqTabInit(pAd);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"TpcReqTabInit failed, Status[=0x%08x]\n", status);
		goto err2;
	}

#ifdef DOT11_HE_AX
	bss_color_table_init(pAd);
#ifdef WIFI_TWT_SUPPORT
	twt_ctrl_init(pAd);
#endif /* DOT11_HE_AX */
#endif /* WIFI_TWT_SUPPORT */

	/* QM init */
	status = qm_init_perband(pAd);

	if (status)
		goto err2;

	/* TM init */
	status = tm_init(pAd);

	if (status)
		goto err2;

#ifdef ERR_RECOVERY
	pAd->ErrRecoveryCtl.ad = pAd;
#endif /* ERR_RECOVERY */

	return TRUE;
err2:
	rtmp_cfg_exit(pAd);
err1:
	HwCtrlExit(pAd);
err0:
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);
	bcn_bpcc_TaskExit(pAd);
	RtmpMgmtTaskExit(pAd);
	return FALSE;
}

/*
*
*/
static void mt_sys_ready(struct _RTMP_ADAPTER *ad)
{
	/* Now Enable RxTx*/
	RTMPEnableRxTx(ad);
	RTMP_SET_FLAG(ad, fRTMP_ADAPTER_START_UP);
	if (!RTMP_TEST_FLAG(ad->physical_dev, PH_DEV_SYSTEM_START_UP))
		RTMP_SET_FLAG(ad->physical_dev, PH_DEV_SYSTEM_START_UP);
}

#ifdef CONFIG_WLAN_SERVICE
/* Allocate memory for wlan service */
int mt_service_open(struct _RTMP_ADAPTER *ad)
{
	struct service *serv = &ad->serv;
	struct service_test *serv_test = NULL;
	struct test_wlan_info *test_winfo = NULL;
	struct test_operation *test_op = NULL;

	if (ad->serv.serv_handle != NULL)
		return NDIS_STATUS_FAILURE;

	/* Allocate service struct memory */
	os_alloc_mem(ad, (UCHAR **)&serv_test, sizeof(struct service_test));

	if (!serv_test) {
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			" serv_test alloc mem fail!!!\n");
		return NDIS_STATUS_FAILURE;
	}
	os_alloc_mem(ad, (UCHAR **)&test_winfo, sizeof(struct test_wlan_info));

	if (!test_winfo) {
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			" test_winfo alloc mem fail!!!\n");
		os_free_mem(serv_test);
		return NDIS_STATUS_FAILURE;
	}

	os_alloc_mem(ad, (UCHAR **)&test_op, sizeof(struct test_operation));

	if (!test_op) {
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			" test_op alloc mem fail!!!\n");
		os_free_mem(serv_test);
		os_free_mem(test_winfo);
		return NDIS_STATUS_FAILURE;
	}


	/* Init service struct memory */
	os_zero_mem(serv_test, sizeof(struct service_test));
	os_zero_mem(test_winfo, sizeof(struct test_wlan_info));
	os_zero_mem(test_op, sizeof(struct test_operation));

	serv_test->test_winfo = test_winfo;
	serv_test->test_op = test_op;
	serv_test->engine_offload = FALSE;

	/* Assign service type for service.git */
	serv->serv_id = SERV_HANDLE_TEST;
	serv->serv_handle = (VOID *)serv_test;

	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
		" wlan service opens successfully!\n");

	return NDIS_STATUS_SUCCESS;
}

/* Init value for wlan service */
int mt_service_init(struct _RTMP_ADAPTER *ad)
{
	INT32 ret;
	struct service *serv = &ad->serv;
	struct service_test *serv_test;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	struct serv_chip_cap *chip_cap = NULL;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(ad->hdev_ctrl);

	serv_test = (struct service_test *)serv->serv_handle;
	chip_cap = &serv_test->test_winfo->chip_cap;

	serv_test->ctrl_band_idx = hc_get_hw_band_idx(ad);
	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx = %d!\n", serv_test->ctrl_band_idx);

	/* Fill service_wlan_info for service internal usage */
	serv_test->test_winfo->net_dev = ad->net_dev;
	serv_test->test_winfo->chip_id = ad->ChipID;
	serv_test->test_winfo->hdev_ctrl = ad->hdev_ctrl;
	serv_test->test_winfo->dbdc_mode = ad->CommonCfg.dbdc_mode;
	serv_test->test_winfo->pkt_tx_tkid_max = 1;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	serv_test->test_winfo->chip_cap.ra_offload = cap->fgRateAdaptFWOffload;
#endif
	chip_cap->swq_per_band = cap->multi_token_ques_per_band;
	chip_cap->tx_wi_size = cap->TXWISize;
	chip_cap->rx_wi_size = cap->RXWISize;
	chip_cap->tx_hw_hdr_len = cap->tx_hw_hdr_len;
	chip_cap->rx_hw_hdr_len = cap->rx_hw_hdr_len;
	chip_cap->num_of_tx_ring = hif->tx_res_num;
	chip_cap->num_of_rx_ring = hif->rx_res_num;
	chip_cap->tx_ring_size = cap->tx_ring_size;
	chip_cap->ht_ampdu_exp = cap->ppdu.ht_max_ampdu_len_exp;
	chip_cap->non_he_tx_ba_wsize = BA_WIN_SZ_64;
#if defined(DOT11_VHT_AC)
	chip_cap->max_mpdu_len = cap->ppdu.max_mpdu_len;
	chip_cap->vht_ampdu_exp = cap->ppdu.vht_max_ampdu_len_exp;
#endif	/* DOT11_VHT_AC */
#if defined(DOT11_HE_AX)
	chip_cap->he_ampdu_exp = cap->ppdu.he_max_ampdu_len_exp;
	chip_cap->he_tx_ba_wsize = BA_WIN_SZ_256;
#endif	/* DOT11_HE_AX */
	chip_cap->efuse_size = cap->EEPROM_DEFAULT_BIN_SIZE;
	chip_cap->ddie_lvts_num = cap->ddie_lvts_num;
	if (cap->phy_caps & fPHY_CAP_6G)
		chip_cap->support_6g = TRUE;
	if (IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_DUALPHY))
		chip_cap->is_dual_phy = TRUE;
	serv_test->test_winfo->use_efuse = PD_GET_USE_EFUSE(ad->physical_dev);
	serv_test->test_winfo->e2p_cur_mode = PD_GET_E2P_CUR_MODE(ad->physical_dev);
	serv_test->test_winfo->e2p_access_mode = PD_GET_E2P_ACCESS_MODE(ad->physical_dev);
	if (PD_GET_E2P_ACCESS_MODE(ad->physical_dev) == E2P_FLASH_MODE
		|| PD_GET_E2P_ACCESS_MODE(ad->physical_dev) == E2P_BIN_MODE)
		chip_cap->efuse_size = get_dev_eeprom_size(ad);

	os_move_mem(&serv_test->test_winfo->chip_cap.spe_map_list,
			&cap->spe_map_list, sizeof(cap->spe_map_list));
	os_move_mem(&serv_test->test_winfo->chip_cap.mcs_nss,
		MCS_NSS_CAP(ad), sizeof(struct serv_mcs_nss_caps));
	os_move_mem(&serv_test->test_winfo->chip_cap.qos,
		&cap->qos, sizeof(struct qos_caps));

	ret = mt_agent_init_service(serv);
	if (ret != SERV_STATUS_SUCCESS) {
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"wlan service inits failed!\n");
		mt_agent_exit_service(serv);

		return NDIS_STATUS_FAILURE;
	}

	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
		" wlan service inits successfully!\n");

	return NDIS_STATUS_SUCCESS;
}

/* Free memory for wlan service */
int mt_service_close(struct _RTMP_ADAPTER *ad)
{
	INT32 ret;
	struct service *serv = &ad->serv;
	struct service_test *serv_test;

	serv_test = (struct service_test *)serv->serv_handle;

	ret = mt_agent_exit_service(serv);
	if (ret != SERV_STATUS_SUCCESS) {
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"wlan service exits failed!\n");

		return NDIS_STATUS_FAILURE;
	}

	os_free_mem(serv_test->test_op);
	os_free_mem(serv_test->test_winfo);
	os_free_mem(serv_test);

	serv->serv_id = 0;
	serv->serv_handle = NULL;

	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
		" wlan service closes successfully!\n");

	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_WLAN_SERVICE */

#ifdef INTERFACE_SPEED_DETECT
/* estimate the physical link speed with certain percentage decrease,
 * the default range is 20% for now and the value could be modified for
 * further customization
 */

#define ESTIMATE_INTFACE_SPD(_value_) ((_value_<<2)/5)
#endif
/*rename from rt28xx_init*/
int mt_wifi_init(VOID *pAdSrc, RTMP_STRING *pDefaultMac, RTMP_STRING *pHostName)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	NDIS_STATUS Status;
	/* UCHAR EDCCACtrl; */
	struct _RTMP_CHIP_CAP *cap = NULL;
	POS_COOKIE pObj = NULL;
	struct wifi_dev *wdev = NULL;
#ifdef INTERFACE_SPEED_DETECT
	UINT IfaceSpeed = 0;
#ifdef RTMP_PCI_SUPPORT
	UINT i;
	PCI_HIF_T *pci_hif = NULL;
#endif
#endif

	if (!pAd)
		return FALSE;
	pObj = (POS_COOKIE)pAd->OS_Cookie;
	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_INFO,
	"\x1b[1;33m Test - pObj->ioctl_if = %d, pObj->ioctl_if_type = %d \x1b[m\n",
	pObj->ioctl_if, pObj->ioctl_if_type);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[MAIN_MSTA_ID].wdev;
	}
#endif
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"wdev == NULL\n");
		return FALSE;
	}

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	asic_show_mac_info(pAd);

	/* reset Adapter flags */
	RTMP_CLEAR_FLAGS(pAd);

	/*for software system initialize*/
	if (rtmp_sys_init(pAd, pHostName) != TRUE)
		goto err2;

	Status = hw_start(pAd);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"hw_start failed!!, ret=%d, cap=%p\n", Status, cap);
		goto err2;
	}

	/* initialize MLME*/
	Status = MlmeInit(pAd);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"MlmeInit failed, Status[=0x%08x]\n", Status);
		goto err3;
	}

	tr_ctl_init(pAd);

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);
#ifdef LED_CONTROL_SUPPORT
	/* Send LED Setting to MCU */
	RTMPInitLEDMode(pAd);
#endif /* LED_CONTROL_SUPPORT */
#ifdef WIN_NDIS

	/* Patch cardbus controller if EEPROM said so. */
	if (pAd->bTest1 == FALSE)
		RTMPPatchCardBus(pAd);

#endif /* WIN_NDIS */
#ifdef IKANOS_VX_1X0
	VR_IKANOS_FP_Init(pAd->ApCfg.BssidNum, pAd->PermanentAddress);
#endif /* IKANOS_VX_1X0 */
	/* Microsoft HCT require driver send a disconnect event after driver initialization.*/
	/* STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED); */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_INFO,
		"NDIS_STATUS_MEDIA_DISCONNECT Event B!\n");
#ifdef TXBF_SUPPORT
	if (bf_init(pAd) != NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"beanform init fail!!\n");
#endif


#ifdef SMART_CARRIER_SENSE_SUPPORT
	SCS_init(pAd);
#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef CFG_SUPPORT_FALCON_SR
	SrInit(pAd);
#endif /* CFG_SUPPORT_FALCON_SR */
#ifdef RT_CFG80211_SUPPORT
#endif
	RTMPIoctlRvRDebug_Init(pAd);
#ifdef MAC_INIT_OFFLOAD
	AsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);
#endif /*MAC_INIT_OFFLOAD*/
#ifdef WIFI_MODULE_DVT
	mdvt_init(pAd);
#endif
#ifdef CONFIG_AP_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_AP(pAd->OpMode) {
		rtmp_ap_init(pAd);
	}
#endif
#ifdef CONFIG_STA_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_STA(pAd->OpMode) {
		rtmp_sta_init(pAd, &pAd->StaCfg[MAIN_MSTA_ID].wdev);
	}
#endif
#ifdef CONFIG_ATE
	rtmp_ate_init(pAd);
#endif /*CONFIG_ATE*/
#ifdef INTERFACE_SPEED_DETECT
#if defined(RTMP_PCI_SUPPORT)
	/* Set detected speed if device is PCIe */
	if (pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE) {
		pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
		for (i = 0; i < pci_hif->pci_hif_chip_num; i++) {
			if (IfaceSpeed < pci_hif->pci_hif_chip[i]->cfg.IfaceSpeed) {
				/* Return first speed back in Connac2 so far.*/
				IfaceSpeed = pci_hif->pci_hif_chip[i]->cfg.IfaceSpeed;
				break;
			}
		}
		if (IfaceSpeed != 0) {
			IfaceSpeed = ESTIMATE_INTFACE_SPD(IfaceSpeed);
			set_interface_speed(pAd, IfaceSpeed);
		}
	}
#elif defined(RTMP_RBUS_SUPPORT)
	/* Set a high value because AXI interface is fast */
	IfaceSpeed = UINT_MAX;
	if (pAd->infType == RTMP_DEV_INF_RBUS)
		set_interface_speed(pAd, IfaceSpeed);
#endif
#endif
	/*SW prepare done, enable system ready*/
	mt_sys_ready(pAd);
#ifdef DYNAMIC_VGA_SUPPORT

	if (pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable)
		dynamic_vga_enable(pAd);

#endif /* DYNAMIC_VGA_SUPPORT */

#ifdef UAPSD_SUPPORT
	UAPSD_Init(pAd);
#endif /* UAPSD_SUPPORT */
	/* assign function pointers*/
#ifdef MAT_SUPPORT
	/* init function pointers, used in OS_ABL */
	RTMP_MATOpsInit(pAd);
#endif /* MAT_SUPPORT */
#ifdef MT_WOW_SUPPORT
	ASIC_WOW_INIT(pAd);
#endif
#ifdef USB_IOT_WORKAROUND2
	pAd->bUSBIOTReady = TRUE;
#endif
#ifdef CONFIG_AP_SUPPORT
	AutoChSelInit(pAd);
#endif /* CONFIG_AP_SUPPORT */
#ifdef REDUCE_TCP_ACK_SUPPORT
	ReduceAckInit(pAd);
#endif
#ifdef CFG_SUPPORT_CSI
	csi_support_init(pAd);
#endif

	/* Trigger MIB counter update */
	pAd->OneSecMibBucket.Enabled = TRUE;
	pAd->MsMibBucket.Enabled = TRUE;
	MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_INFO,
		"<==== Status=%x\n", Status);
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
	/* TODO: consider DBDC case, move data to wdev*/
	pAd->bfdm.bfdm_bfee_enabled = TRUE; /* BFee HW is enabled by default */
	pAd->bfdm.bfdm_bitmap = 0; /* BFee adaption disabled by default. No BFDM_BFEE_ADAPTION_BITMAP */
#endif /* TXBF_SUPPORT */

	HeraInitStbcPriority(pAd);
#ifdef ACK_CTS_TIMEOUT_SUPPORT
	if (TRUE != set_datcfg_ack_cts_timeout(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"ERROR! DAT CONFIG ACK Fail!!\n");
	}
#endif/*ACK_CTS_TIMEOUT_SUPPORT*/

	if (pAd->CommonCfg.bUseVhtRateFor2g)
		MtCmdSetUseVhtRateFor2G(pAd);

	if (pAd->CommonCfg.vht_1024_qam)
		MtCmdSetVht1024QamSupport(pAd);

	return TRUE;
err3:
	MlmeHalt(pAd);
err2:
	rtmp_sys_exit(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INIT_FAIL);
	MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR, "!!! fail !!!\n");
	return FALSE;
}


VOID RTMPDrvOpen(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	UINT8 band_idx = 0;
	INT i;

#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[MAIN_MSTA_ID];
#endif
	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		FT_RIC_Init(pAd);
	}
#endif /* DOT11R_FT_SUPPORT */
#ifdef MT_MAC
#ifdef RT_CFG80211_SUPPORT
	CFG80211_InitTxSCallBack(pAd);
#endif /* RT_CFG80211_SUPPORT */
#endif /* MT_MAC */
#endif /* CONFIG_STA_SUPPORT */
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
	}

	/*
		To reduce connection time,
		do auto reconnect here instead of waiting STAMlmePeriodicExec to do auto reconnect.
	*/
	if (pAd->OpMode == OPMODE_STA)
		MlmeAutoReconnectLastSSID(pAd, &pStaCfg->wdev);

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11W_PMF_SUPPORT

	if (pAd->OpMode == OPMODE_STA) {
		PMF_CFG *pPmfCfg = &pStaCfg->wdev.SecConfig.PmfCfg;

		pPmfCfg->MFPC = FALSE;
		pPmfCfg->MFPR = FALSE;
		pPmfCfg->PMFSHA256 = FALSE;

		if ((IS_AKM_WPA2_Entry(&pStaCfg->wdev) ||
		     IS_AKM_WPA2PSK_Entry(&pStaCfg->wdev) ||
		     IS_AKM_OWE_Entry(&pStaCfg->wdev)) &&
		     IS_CIPHER_AES_Entry(&pStaCfg->wdev)) {
			pPmfCfg->PMFSHA256 = pPmfCfg->Desired_PMFSHA256;

			if (pPmfCfg->Desired_MFPC) {
				pPmfCfg->MFPC = TRUE;
				pPmfCfg->MFPR = pPmfCfg->Desired_MFPR;

				if (pPmfCfg->MFPR)
					pPmfCfg->PMFSHA256 = TRUE;
			}
		} else if (pPmfCfg->Desired_MFPC)
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_WARN,
				"[PMF] Security is not WPA2/WPA2PSK AES\n");

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO,
			"[PMF] MFPC=%d, MFPR=%d, SHA256=%d\n",
			pPmfCfg->MFPC, pPmfCfg->MFPR, pPmfCfg->PMFSHA256);
	}

#endif /* DOT11W_PMF_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef WSC_INCLUDED
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		PWSC_CTRL pWscControl = &pStaCfg->wdev.WscControl;

		WscGenerateUUID(pAd, &pWscControl->Wsc_Uuid_E[0], &pWscControl->Wsc_Uuid_Str[0], 0, FALSE, FALSE);
		WscInit(pAd, FALSE, BSS0);
#ifdef WSC_V2_SUPPORT
		WscInitRegistrarPair(pAd, pWscControl, BSS0);
#endif /* WSC_V2_SUPPORT */
	}
#endif /* CONFIG_STA_SUPPORT */
	/* WSC hardware push button function 0811 */
	WSC_HDR_BTN_Init(pAd);
#endif /* WSC_INCLUDED */

#ifdef MT_WOW_SUPPORT
	pAd->WOW_Cfg.bWoWRunning = FALSE;
#endif
#ifdef CONFIG_AP_SUPPORT
	vow_init(pAd);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CFG_RED_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
		red_init(pAd);
#endif /* CFG_RED_SUPPORT */

#ifdef DABS_QOS
	if (pAd->OpMode == OPMODE_AP)
		dabs_init(pAd);
#endif /* DABS_QOS */

	if (pAd->rts_retrylimit > 0) {
		band_idx = hc_get_hw_band_idx(pAd);
		asic_set_rts_retrylimit(pAd, band_idx, pAd->rts_retrylimit);
	}

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
		for (i = 0; i < 16; i++) {
			MtCmdCr4Set(pAd, WA_SET_OPTION_MPDU_RETRY_LIMIT, i, 0);
			if (pAd->retrylimit[i] > 0) {
				MtCmdCr4Set(pAd, WA_SET_OPTION_MPDU_RETRY_LIMIT, i, pAd->retrylimit[i]);
			}
		}
	}

	cp_support_is_enabled(pAd);
#ifdef GN_MIXMODE_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
		gn_mixmode_is_enable(pAd);
#endif /* GN_MIXMODE_SUPPORT */
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	/* DfsDedicatedScanStart(pAd); */
	/* DfsSetInitDediatedScanStart(pAd); */
#endif

#ifdef BAND_STEERING
#ifdef CONFIG_AP_SUPPORT
	if (pAd->ApCfg.BandSteering)
		/* Inform daemon interface ready */
		BndStrg_SetInfFlags(
			pAd, &pAd->ApCfg.MBSSID[BSS0].wdev, &pAd->ApCfg.BndStrgTable, TRUE);
#endif /* CONFIG_AP_SUPPORT */
#endif /* BAND_STEERING */
#ifdef WIFI_DIAG
	diag_proc_init(pAd);/* init*/
#endif
}

VOID RTMPDrvClose(VOID *pAdSrc, VOID *net_dev)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	struct MCU_CTRL *prCtl = NULL;
#ifdef CONFIG_AP_SUPPORT
	INT j;
#endif  /* CONFIG_AP_SUPPORT */
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	struct customer_vendor_ie *ap_vendor_ie;
	struct customer_vendor_ie *apcli_vendor_ie;
	CUSTOMER_PROBE_RSP_VENDOR_IE *ap_probe_rsp_vendor_ie = NULL, *ap_probe_rsp_vendor_ie_temp = NULL;
	BSS_STRUCT *mbss = NULL;
	PDL_LIST ap_probe_rsp_vendor_ie_list = NULL;
	UINT32 ie_count;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
#endif /* CONFIG_STA_SUPPORT */
	prCtl = PD_GET_MCU_CTRL_PTR(pAd->physical_dev);
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE);
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		UINT32 i = 0;

		/* If dirver doesn't wake up firmware here,*/
		/* NICLoadFirmware will hang forever when interface is up again.*/
		for (i = 0; i < MAX_MULTI_STA; i++) {
			pStaCfg = &pAd->StaCfg[i];

			if (INFRA_ON(pStaCfg) && pStaCfg->PwrMgmt.bDoze)
				RTMP_FORCE_WAKEUP(pAd, pStaCfg);
		}

#ifdef RTMP_MAC_PCI
		{
			PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

			pci_hif->bPCIclkOff = FALSE;
		}
#endif /* RTMP_MAC_PCI */
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef MT_MAC
	/* clear extend bss mac address */
	NdisZeroMemory(pAd->ExtendMBssAddr, sizeof(pAd->ExtendMBssAddr));
	NdisZeroMemory(pAd->ApcliAddr, sizeof(pAd->ApcliAddr));

	if (IS_HIF_TYPE(pAd, HIF_MT))
#endif /* MT_MAC */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

#ifdef CONFIG_AP_SUPPORT
	for (j = 0; j < pAd->ApCfg.BssidNum; j++) {
#ifdef DOT11V_MBSSID_SUPPORT
		mbss_11v_exit(pAd, &pAd->ApCfg.MBSSID[j]);
#endif /* DOT11V_MBSSID_SUPPORT */
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
		ap_vendor_ie = &pAd->ApCfg.MBSSID[j].ap_vendor_ie;
		if (ap_vendor_ie->pointer != NULL)
			os_free_mem(ap_vendor_ie->pointer);
		ap_vendor_ie->pointer = NULL;
		ap_vendor_ie->length = 0;

		mbss = &pAd->ApCfg.MBSSID[j];
		RTMP_SPIN_LOCK(&mbss->probe_rsp_vendor_ie_lock);
		ap_probe_rsp_vendor_ie_list = &mbss->ap_probe_rsp_vendor_ie_list;
		ie_count = DlListLen(ap_probe_rsp_vendor_ie_list);
		if (ie_count) {
			DlListForEachSafe(ap_probe_rsp_vendor_ie, ap_probe_rsp_vendor_ie_temp, ap_probe_rsp_vendor_ie_list,
				CUSTOMER_PROBE_RSP_VENDOR_IE, List) {
				if (ap_probe_rsp_vendor_ie) {
					DlListDel(&ap_probe_rsp_vendor_ie->List);
					MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO,
						"remove MAC["MACSTR"]\n",
						MAC2STR(ap_probe_rsp_vendor_ie->stamac));
					if (ap_probe_rsp_vendor_ie->pointer){
						os_free_mem(ap_probe_rsp_vendor_ie->pointer);
					}
					os_free_mem(ap_probe_rsp_vendor_ie);
				}
			}
		}
		RTMP_SPIN_UNLOCK(&mbss->probe_rsp_vendor_ie_lock);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	pAd->ApCfg.ap_probe_rsp_vendor_ie_count = 0;
#ifdef APCLI_SUPPORT
	for (j = 0; j < MAX_APCLI_NUM; j++) {
		apcli_vendor_ie = &pAd->StaCfg[j].apcli_vendor_ie;
		if (apcli_vendor_ie->pointer != NULL)
			os_free_mem(apcli_vendor_ie->pointer);
		apcli_vendor_ie->pointer = NULL;
		apcli_vendor_ie->length = 0;
	}
#endif /* APCLI_SUPPORT */
	UINT32 i = 0;

	for (i = 0; i < pAd->ScanCtrl.ScanTab.BssNr; i++) {
		if (pAd->ScanCtrl.ScanTab.BssEntry[i].CustomerBssEntry.vendor_ie.pointer != NULL) {
			os_free_mem(pAd->ScanCtrl.ScanTab.BssEntry[i].CustomerBssEntry.vendor_ie.pointer);
			pAd->ScanCtrl.ScanTab.BssEntry[i].CustomerBssEntry.vendor_ie.pointer = NULL;
		}
		pAd->ScanCtrl.ScanTab.BssEntry[i].CustomerBssEntry.vendor_ie.length = 0;
	}
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

#ifdef EXT_BUILD_CHANNEL_LIST

	if (pAd->CommonCfg.pChDesp != NULL)
		os_free_mem(pAd->CommonCfg.pChDesp);

	pAd->CommonCfg.pChDesp = NULL;
	pAd->CommonCfg.DfsType = MAX_RD_REGION;
	pAd->CommonCfg.bCountryFlag = 0;
#endif /* EXT_BUILD_CHANNEL_LIST */
	pAd->CommonCfg.bCountryFlag = FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		BOOLEAN Cancelled = FALSE;
#ifdef DOT11N_DRAFT3

		if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_TIMER_FIRED) {
			RTMPCancelTimer(&pAd->CommonCfg.Bss2040CoexistTimer, &Cancelled);
			pAd->CommonCfg.Bss2040CoexistFlag  = 0;
		}

#endif /* DOT11N_DRAFT3 */
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#if (defined(MT_WOW_SUPPORT) && defined(WOW_IFDOWN_SUPPORT))

		if (!((pAd->WOW_Cfg.bEnable == TRUE) && INFRA_ON(&pAd->StaCfg[0])))
#endif
		{
			MacTableReset(pAd);
		}
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef DOT11V_MBSSID_SUPPORT
		pAd->ApCfg.dot11v_mbssid_bitmap = 0;
#endif
#ifdef CLIENT_WDS
		CliWds_ProxyTabDestory(pAd);
#endif /* CLIENT_WDS */
		/* Shutdown Access Point function, release all related resources */
		APShutdown(pAd);
		/*#ifdef AUTO_CH_SELECT_ENHANCE*/
		/* Free BssTab & ChannelInfo tabbles.*/
		/*		AutoChBssTableDestroy(pAd); */
		/*		ChannelInfoDestroy(pAd); */
		/*#endif  AUTO_CH_SELECT_ENHANCE */
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_ATE
	ATEExit(pAd);
#endif /*CONFIG_ATE*/
	/* Stop Mlme state machine*/
	MlmeHalt(pAd);
	/* Close net tasklets*/
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
		UINT32 i = 0;

		for (i = 0; i < MAX_MULTI_STA; i++) {
			pStaCfg = &pAd->StaCfg[i];

			if ((pAd->WOW_Cfg.bEnable) &&
				(pAd->WOW_Cfg.bWowIfDownSupport) &&
				INFRA_ON(pStaCfg)) {
				break;
			}
		}

#endif /* WOW */
	}
#endif /* CONFIG_STA_SUPPORT */
	TpcReqTabExit(pAd);
#ifdef DOT11_HE_AX
	bss_color_table_deinit(pAd);
#ifdef WIFI_TWT_SUPPORT
	twt_ctrl_exit(pAd);
#endif /* DOT11_HE_AX */
#endif /* WIFI_TWT_SUPPORT */
#ifdef LED_CONTROL_SUPPORT
	RTMPExitLEDMode(pAd);
#endif /* LED_CONTROL_SUPPORT */
	bcn_bpcc_TaskExit(pAd);
	/* Close kernel threads*/
	RtmpMgmtTaskExit(pAd);
#ifdef RTMP_MAC_PCI
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE)) {
			DISABLE_TX_RX(pAd, RTMP_HALT);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
		}

		/* Receive packets to clear DMA index after disable interrupt. */
		/*RTMPHandleRxDoneInterrupt(pAd);*/
		/* put to radio off to save power when driver unload.  After radiooff, can't write /read register.  So need to finish all */
		/* register access before Radio off.*/
#endif /* RTMP_MAC_PCI */

	/* Free IRQ*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);
	}

#ifdef SINGLE_SKU_V2
	RTMPResetSkuParam(pAd);
	RTMPResetBackOffParam(pAd);
#endif

	/* tm exit */
	tm_exit(pAd);

	/* qm exit */
	qm_exit_perband(pAd);

	tr_ctl_exit(pAd);

	/*remove hw related system info*/
	hw_stop(pAd);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
#ifdef WLAN_SKB_RECYCLE
	skb_queue_purge(&pAd->rx0_recycle);
#endif /* WLAN_SKB_RECYCLE */
	UserCfgExit(pAd); /* must after ba_reordering_resource_release */

#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		FT_RIC_Release(pAd);
	}
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef BACKGROUND_SCAN_SUPPORT
	BackgroundScanDeInit(pAd);
#endif /* BACKGROUND_SCAN_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	AutoChSelRelease(pAd);
#endif/* CONFIG_AP_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
	RepeaterCliReset(pAd);
#endif /* MAC_REPEATER_SUPPORT */
	/* release all timers */
	RtmpusecDelay(2000);
	MeasureReqTabExit(pAd);
	/* WCNCR00034259: moved from RTMP{Reset, free}TxRxRingMemory() */
	NdisFreeSpinLock(&pAd->CmdQLock);
	NdisFreeSpinLock(&pAd->BPCC_CmdQLock);
#ifdef RTMP_TIMER_TASK_SUPPORT
	if (IS_ASIC_CAP(pAd, fASIC_CAP_MGMT_TIMER_TASK))
		NdisFreeSpinLock(&pAd->TimerQLock);
#endif /* RTMP_TIMER_TASK_SUPPORT */
	/* Close Hw ctrl*/
	HwCtrlExit(pAd);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	/*move after HwCtrlExit , make sure it can flush all queued command*/
#ifdef REDUCE_TCP_ACK_SUPPORT
	ReduceAckExit(pAd);
#endif
#ifdef WIFI_MODULE_DVT
	mdvt_exit(pAd);
#endif

#ifdef PRE_CAL_TRX_SET1_SUPPORT

	if (PD_GET_E2P_ACCESS_MODE(pAd->physical_dev) == E2P_BIN_MODE) {
		if (pAd->CalDCOCImage != NULL)
			os_free_mem(pAd->CalDCOCImage);

		if (pAd->CalDPDAPart1Image != NULL)
			os_free_mem(pAd->CalDPDAPart1Image);

		if (pAd->CalDPDAPart2Image != NULL)
			os_free_mem(pAd->CalDPDAPart2Image);
	}

#endif /* PRE_CAL_TRX_SET1_SUPPORT */
#ifdef PRE_CAL_TRX_SET2_SUPPORT

	if (PD_GET_E2P_ACCESS_MODE(pAd->physical_dev) != E2P_FLASH_MODE
		&& pAd->PreCalStoreBuffer != NULL) {
		os_free_mem(pAd->PreCalStoreBuffer);
		pAd->PreCalStoreBuffer = NULL;
	}

	if (pAd->PreCalReStoreBuffer != NULL) {
		os_free_mem(pAd->PreCalReStoreBuffer);
		pAd->PreCalReStoreBuffer = NULL;
	}

#endif/* PRE_CAL_TRX_SET2_SUPPORT */

	if (hwifi_get_inf_num(pAd) == 0) {
		struct physical_device *ph_dev = pAd->physical_dev;

		if (ph_dev->EEPROMImage)
			os_free_mem(ph_dev->EEPROMImage);
		ph_dev->EEPROMImage = NULL;
#ifndef RT_CFG80211_SUPPORT
#ifdef DOT11_SAE_SUPPORT
		big_integer_lock_deinit();
#endif /* DOT11_SAE_SUPPORT */
#endif /* !RT_CFG80211_SUPPORT */
		physical_device_all_timer_list_release(ph_dev);
	}

	/* free memory for rxv entry raw data */
	if (pAd->rxv_raw_data.rxv_pkt) {
		/* free memory for rxv pkt */
		os_zero_mem(pAd->rxv_raw_data.rxv_pkt, pAd->rxv_raw_data.rxv_byte_cnt);
		os_free_mem(pAd->rxv_raw_data.rxv_pkt);
		pAd->rxv_raw_data.rxv_pkt = NULL;
		pAd->rxv_raw_data.rxv_byte_cnt = 0;
	}

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
	if (pAd->pIQ_Array != NULL)
		os_free_mem(pAd->pIQ_Array);

	if (pAd->pL32Bit != NULL)
		os_free_mem(pAd->pL32Bit);

	if (pAd->pM32Bit != NULL)
		os_free_mem(pAd->pM32Bit);

	if (pAd->pH32Bit != NULL)
		os_free_mem(pAd->pH32Bit);
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */
	/*multi profile release*/
#ifdef OCE_SUPPORT
	OceRelease(pAd);
#endif /* OCE_SUPPORT */
#if defined(OFFCHANNEL_SCAN_FEATURE) || defined(NF_SUPPORT)
	/*pAd->Avg_NF = pAd->Avg_NFx16[DBDC_BAND0] = 0;
	if (pAd->CommonCfg.dbdc_mode)
		pAd->Avg_NF = pAd->Avg_NFx16[DBDC_BAND1] = 0;
	*/
#endif
#ifdef DABS_QOS
	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	memset(&qos_param_table[0], 0, sizeof(struct qos_param_rec)*MAX_QOS_PARAM_TBL);
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
	NdisFreeSpinLock(&qos_param_table_lock);
#endif
#ifdef MT_DFS_SUPPORT
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Nop_List_Backup call!!\n");
	Nop_List_Backup(pAd);
#endif /* MT_DFS_SUPPORT */
#ifdef WIFI_DIAG
	diag_proc_exit(pAd);
#endif
#ifdef WF_RESET_SUPPORT
	ser_mngr_free_event_cb();
#endif
}

PNET_DEV RtmpPhyNetDevMainCreate(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	PNET_DEV pDevNew;
	UINT32 MC_RowID = 0, IoctlIF = 0;
	char *dev_name;

	dev_name = get_dev_name_prefix(pAd, INT_MAIN);
	if (!dev_name)
		return NULL;

	pDevNew = RtmpOSNetDevCreate((INT32)MC_RowID, (UINT32 *)&IoctlIF,
					 INT_MAIN, 0, sizeof(struct mt_dev_priv), dev_name, FALSE);
#ifdef RT_CFG80211_SUPPORT
#if defined(IWCOMMAND_CFG80211_SUPPORT) || defined(HOSTAPD_MBSS_SUPPORT)
	if (pDevNew) {
		ULONG flags = 0;

		OS_SPIN_LOCK_IRQSAVE(&pAd->CfgIfUseCntLock, &flags);
		pAd->CfgAPIfUseCnt++;
		OS_SPIN_UNLOCK_IRQRESTORE(&pAd->CfgIfUseCntLock, &flags);
	}
#endif /* defined(IWCOMMAND_CFG80211_SUPPORT) || defined(HOSTAPD_MBSS_SUPPORT) */
#endif /* RT_CFG80211_SUPPORT */
	return pDevNew;
}


#ifdef CONFIG_STA_SUPPORT
#ifdef PROFILE_STORE
static void WriteConfToDatFile(RTMP_ADAPTER *pAd)
{
	char	*cfgData = 0, *offset = 0;
	RTMP_STRING *fileName = NULL, *pTempStr = NULL;
	RTMP_OS_FD file_r, file_w;
	RTMP_OS_FS_INFO osFSInfo;
	LONG rv, fileLen = 0;
	int ret;

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "----->\n");
		fileName = STA_PROFILE_PATH;

	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	file_r = RtmpOSFileOpen(fileName, O_RDONLY, 0);

	if (IS_FILE_OPEN_ERR(file_r)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"-->1) Error opening file %s\n", fileName);
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
		return;
	} else {
		char tempStr[64] = {0};

		while ((rv = RtmpOSFileRead(file_r, tempStr, 64)) > 0)
			fileLen += rv;

		os_alloc_mem(NULL, (UCHAR **)&cfgData, fileLen);

		if (cfgData == NULL) {
			RtmpOSFileClose(file_r);
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"CfgData mem alloc fail. (fileLen = %ld)\n", fileLen);
			goto out;
		}

		NdisZeroMemory(cfgData, fileLen);
		RtmpOSFileSeek(file_r, 0);
		rv = RtmpOSFileRead(file_r, (RTMP_STRING *)cfgData, fileLen);
		RtmpOSFileClose(file_r);

		if (rv != fileLen) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"CfgData mem alloc fail, fileLen = %ld\n", fileLen);
			goto ReadErr;
		}
	}

	file_w = RtmpOSFileOpen(fileName, O_WRONLY | O_TRUNC, 0);

	if (IS_FILE_OPEN_ERR(file_w))
		goto WriteFileOpenErr;
	else {
		offset = (PCHAR) rtstrstr((RTMP_STRING *) cfgData, "Default\n");
		offset += strlen("Default\n");
		RtmpOSFileWrite(file_w, (RTMP_STRING *)cfgData, (int)(offset - cfgData));
		os_alloc_mem(NULL, (UCHAR **)&pTempStr, 512);

		if (!pTempStr) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
				"pTempStr mem alloc fail. (512)\n");
			RtmpOSFileClose(file_w);
			goto WriteErr;
		}

		for (;;) {
			int i = 0;
			RTMP_STRING *ptr;

			NdisZeroMemory(pTempStr, 512);
			ptr = (RTMP_STRING *) offset;

			while (*ptr && *ptr != '\n')
				pTempStr[i++] = *ptr++;

			pTempStr[i] = 0x00;

			if ((size_t)(offset - cfgData) < fileLen) {
				offset += strlen(pTempStr) + 1;

				if (strncmp(pTempStr, "SSID=", strlen("SSID=")) == 0) {
					NdisZeroMemory(pTempStr, 512);
					NdisMoveMemory(pTempStr, "SSID=", strlen("SSID="));
					NdisMoveMemory(pTempStr + 5, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen);
				} else if (strncmp(pTempStr, "AuthMode=", strlen("AuthMode=")) == 0) {
					struct _SECURITY_CONFIG *pSecConfig = &pStaCfg->wdev.SecConfig;

					NdisZeroMemory(pTempStr, 512);

					if (IS_AKM_OPEN(pSecConfig->AKMMap))
						ret = snprintf(pTempStr, 512,
							"%s", "AuthMode=OPEN");
					else if (IS_AKM_SHARED(pSecConfig->AKMMap))
						ret = snprintf(pTempStr, 512,
							"%s", "AuthMode=SHARED");
					else if (IS_AKM_AUTOSWITCH(pSecConfig->AKMMap))
						ret = snprintf(pTempStr, 512,
							"%s", "AuthMode=WEPAUTO");
					else if (IS_AKM_WPA1PSK(pSecConfig->AKMMap))
						ret = snprintf(pTempStr, 512,
							"%s", "AuthMode=WPAPSK");
					else if (IS_AKM_WPA2PSK(pSecConfig->AKMMap))
						ret = snprintf(pTempStr, 512,
							"%s", "AuthMode=WPA2PSK");
					else if (IS_AKM_WPA1(pSecConfig->AKMMap))
						ret = snprintf(pTempStr, 512,
							"%s", "AuthMode=WPA");
					else if (IS_AKM_WPA2(pSecConfig->AKMMap))
						ret = snprintf(pTempStr, 512,
							"%s", "AuthMode=WPA2");
					else if (IS_AKM_WPANONE(pSecConfig->AKMMap))
						ret = snprintf(pTempStr, 512,
							"%s", "AuthMode=WPANONE");

					if (os_snprintf_error(512, ret)) {
						MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"snprintf check error!\n");
						goto WriteErr;
					}
				} else if (strncmp(pTempStr, "EncrypType=", strlen("EncrypType=")) == 0) {
					struct _SECURITY_CONFIG *pSecConfig = &pAd->StaCfg[0].wdev.SecConfig;

					NdisZeroMemory(pTempStr, 512);

					if (IS_CIPHER_NONE(pSecConfig->PairwiseCipher))
						ret = snprintf(pTempStr, 512,
							"%s", "EncrypType=NONE");
					else if (IS_CIPHER_WEP(pSecConfig->PairwiseCipher))
						ret = snprintf(pTempStr, 512,
							"%s", "EncrypType=WEP");
					else if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher))
						ret = snprintf(pTempStr, 512,
							"%s", "EncrypType=TKIP");
					else if (IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher))
						ret = snprintf(pTempStr, 512,
							"%s", "EncrypType=AES");
					else if (IS_CIPHER_CCMP256(pSecConfig->PairwiseCipher))
						ret = snprintf(pTempStr, 512,
							"%s", "EncrypType=CCMP256");
					else if (IS_CIPHER_GCMP128(pSecConfig->PairwiseCipher))
						ret = snprintf(pTempStr, 512,
							"%s", "EncrypType=GCMP128");
					else if (IS_CIPHER_GCMP256(pSecConfig->PairwiseCipher))
						ret = snprintf(pTempStr, 512,
							"%s", "EncrypType=GCMP256");

					if (os_snprintf_error(512, ret)) {
						MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
							"snprintf check error!\n");
						goto WriteErr;
					}
				}

				RtmpOSFileWrite(file_w, pTempStr, strlen(pTempStr));
				RtmpOSFileWrite(file_w, "\n", 1);
			} else
				break;
		}

		RtmpOSFileClose(file_w);
	}

WriteErr:

	if (pTempStr)
		os_free_mem(pTempStr);

ReadErr:
WriteFileOpenErr:

	if (cfgData)
		os_free_mem(cfgData);

out:
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "<-----\n");
	return;
}


INT write_dat_file_thread(
	IN ULONG Context)
{
	RTMP_OS_TASK *pTask;
	RTMP_ADAPTER *pAd;
	/* int	Status = 0; */
	pTask = (RTMP_OS_TASK *)Context;

	if (pTask == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"pTask is NULL\n");
		return 0;
	}

	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

	if (pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"pAd is NULL\n");
		return 0;
	}

	RtmpOSTaskCustomize(pTask);
	/* Update ssid, auth mode and encr type to DAT file */
	WriteConfToDatFile(pAd);
	RtmpOSTaskNotifyToExit(pTask);
	return 0;
}

NDIS_STATUS WriteDatThread(
	IN  RTMP_ADAPTER *pAd)
{
	NDIS_STATUS status = NDIS_STATUS_FAILURE;
	RTMP_OS_TASK *pTask;

	if (pAd->bWriteDat == FALSE)
		return 0;

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "-->\n");
	pTask = &pAd->WriteDatTask;
	RTMP_OS_TASK_INIT(pTask, "RtmpWriteDatTask", pAd);
	status = RtmpOSTaskAttach(pTask, write_dat_file_thread, (ULONG)&pAd->WriteDatTask);
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
		"<--, status=%d!\n", status);
	return status;
}
#endif /* PROFILE_STORE */
#endif /* CONFIG_STA_SUPPORT */

#ifdef ERR_RECOVERY
INT	Set_ErrDetectOn_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	UINT32 Enable;

	Enable = os_str_tol(arg, 0, 10);
	CmdExtGeneralTestOn(pAd, (Enable == 0) ? (FALSE) : (TRUE));
	return TRUE;
}

INT	Set_ErrDetectMode_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	UINT8 mode = 0;
	UINT8 sub_mode = 0;
	PCHAR seg_str;

	seg_str = strsep((char **)&arg, "_");

	if (seg_str != NULL)
		mode = (BOOLEAN) os_str_tol(seg_str, 0, 10);

	seg_str = strsep((char **)&arg, "_");

	if (seg_str != NULL)
		sub_mode = (BOOLEAN) os_str_tol(seg_str, 0, 10);

	CmdExtGeneralTestMode(pAd, mode, sub_mode);
	return TRUE;
}
#endif /* ERR_RECOVERY */

