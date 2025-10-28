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
	rtmp_init.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#include	"rt_config.h"
#ifdef ZERO_PKT_LOSS_SUPPORT
#include	"hdev/hdev_basic.h"
#endif

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
#include "phy/rlm_cal_cache.h"
#endif /* RLM_CAL_CACHE_SUPPORT */

UCHAR NUM_BIT8[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
#ifdef DBG
char *CipherName[] = {"none", "wep64", "wep128", "TKIP", "AES", "CKIP64", "CKIP128", "CKIP152", "SMS4", "WEP152"};
#endif


NDIS_STATUS RTMPAllocGlobalUtility(VOID)
{
	/* init UTIL module */
	RtmpUtilInit();
	return NDIS_STATUS_SUCCESS;
}


/*
	========================================================================

	Routine Description:
		Allocate RTMP_ADAPTER data block and do some initialization

	Arguments:
		Adapter		Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS RTMPAllocAdapterBlock(VOID *handle, VOID **ppAdapter, INT type)
{
	RTMP_ADAPTER *pAd = NULL;
	NDIS_STATUS	 Status;
	INT index;

	MTWF_PRINT("--> RTMPAllocAdapterBlock\n");
	RTMPAllocGlobalUtility();
	*ppAdapter = NULL;

	do {
		/* Allocate RTMP_ADAPTER memory block*/
		Status = AdapterBlockAllocateMemory(handle, (PVOID *)&pAd, sizeof(RTMP_ADAPTER));

		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_PRINT("Failed to allocate memory - ADAPTER\n");
			break;
		} else {
			/* init resource list (must be after pAd allocation) */
			initList(&pAd->RscTaskMemList);
			initList(&pAd->RscLockMemList);
			initList(&pAd->RscTaskletMemList);
			initList(&pAd->RscSemMemList);
			initList(&pAd->RscAtomicMemList);
			pAd->OS_Cookie = handle;
			((POS_COOKIE)(handle))->pAd_va = (LONG)pAd;
		}

		MTWF_PRINT("\n\n=== pAd = %p, size = %zu ===\n\n", pAd,	sizeof(RTMP_ADAPTER));

		if (RtmpOsStatsAlloc(&pAd->stats, &pAd->iw_stats) == FALSE) {
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		/* Init spin locks*/
		NdisAllocateSpinLock(pAd, &pAd->WdevListLock);
		NdisAllocateSpinLock(pAd, &pAd->BssInfoIdxBitMapLock);
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
		NdisAllocateSpinLock(pAd, &pAd->ApCfg.ReptCliEntryLock);
		NdisAllocateSpinLock(pAd, &pAd->ApCfg.CliLinkMapLock);
		NdisAllocateSpinLock(pAd, &pAd->ApCfg.btwt_ie_lock);
#endif
#endif
#endif
#ifdef GREENAP_SUPPORT
		NdisAllocateSpinLock(pAd, &pAd->ApCfg.greenap.lock);
#endif /* GREENAP_SUPPORT */
		/*Allocate interface lock*/
		NdisAllocateSpinLock(pAd, &pAd->VirtualIfLock);
#if defined(IWCOMMAND_CFG80211_SUPPORT) || defined(HOSTAPD_MBSS_SUPPORT)
		NdisAllocateSpinLock(pAd, &pAd->CfgIfUseCntLock);
#endif /* IWCOMMAND_CFG80211_SUPPORT || HOSTAPD_MBSS_SUPPORT */
#if defined(MTK_HOSTAPD_SUPPORT) && defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
		NdisAllocateSpinLock(pAd, &pAd->gas_frame_list_lock);
#endif /* MTK_HOSTAPD_SUPPORT && CONFIG_MAP_SUPPORT && MAP_R3 */
#ifdef RLM_CAL_CACHE_SUPPORT
		rlmCalCacheInit(pAd, &pAd->rlmCalCache);
#endif /* RLM_CAL_CACHE_SUPPORT */
		/*Mlme Queue Entry Buffer alloc*/
		Status = MlmeQueueInit(pAd);
		*ppAdapter = (VOID *)pAd;
	} while (FALSE);

	if (Status != NDIS_STATUS_SUCCESS) {
		if (pAd) {
			if (pAd->stats) {
				os_free_mem(pAd->stats);
				pAd->stats = NULL;
			}

			if (pAd->iw_stats) {
				os_free_mem(pAd->iw_stats);
				pAd->iw_stats = NULL;
			}

			RtmpOsVfree(pAd);
		}

		return Status;
	}

	/* Init ProbeRespIE Table */
	for (index = 0; index < MAX_LEN_OF_BSS_TABLE; index++) {
		if (os_alloc_mem(pAd, &pAd->ProbeRespIE[index].pIe, MAX_VIE_LEN) == NDIS_STATUS_SUCCESS)
			RTMPZeroMemory(pAd->ProbeRespIE[index].pIe, MAX_VIE_LEN);
		else
			pAd->ProbeRespIE[index].pIe = NULL;
	}
	/*allocate hdev_ctrl struct for prepare chip_cap & chip_ops */
	hdev_ctrl_init(pAd, type);
	/*allocate wpf related memory*/
	wpf_config_init(pAd);
	/* init fwlog file status */
	pAd->srcf.Status = 1;
#ifdef WIFI_DIAG
	diag_ctrl_alloc(pAd);
#endif
	MTWF_PRINT("<-- RTMPAllocAdapterBlock, Status=%x\n", Status);
	return Status;
}

/*
	========================================================================

	Routine Description:
		Set default value from EEPROM

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID NICInitAsicFromEEPROM(RTMP_ADAPTER *pAd)
{
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;

	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_EEPROM, DBG_LVL_INFO, "-->\n");
	NicConfig2.word = pAd->NicConfig2.word;
	RTMP_CHIP_ASIC_INIT_TEMPERATURE_COMPENSATION(pAd);
#ifndef MAC_INIT_OFFLOAD
	AsicSetRxStream(pAd, pAd->Antenna.field.RxPath, 0);
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		UCHAR bandIdx = HcGetBandByWdev(&pAd->StaCfg[MAIN_MSTA_ID].wdev);

		AsicSetTxStream(pAd, pAd->Antenna.field.TxPath, OPMODE_STA, FALSE, bandIdx);
	}
#endif /* CONFIG_STA_SUPPORT */
	RTMP_EEPROM_ASIC_INIT(pAd);
	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_EEPROM, DBG_LVL_INFO,
		"TxPath = %d, RxPath = %d, RFIC=%d\n",
		pAd->Antenna.field.TxPath, pAd->Antenna.field.RxPath, pAd->RfIcType);
	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_EEPROM, DBG_LVL_INFO, "<--\n");
}

INT32 WfMcuSysInit(RTMP_ADAPTER *pAd)
{
	MCU_CTRL_INIT(pAd);
	chip_fw_init(pAd);

	return NDIS_STATUS_SUCCESS;
}

INT32 WfMcuSysExit(RTMP_ADAPTER *pAd)
{
	MCUSysExit(pAd);
	return 0;
}


extern RTMP_STRING *mac;

INT32 WfEPROMSysInit(RTMP_ADAPTER *pAd, BOOLEAN is_cal, UCHAR dbdc_mode)
{
	UCHAR RfIC;

	/* hook e2p operation */
	RtmpChipOpsEepromHook(pAd, E2P_NONE);
	/* We should read EEPROM for all cases */
	NICReadEEPROMParameters(pAd, (RTMP_STRING *)mac, is_cal);
	hc_radio_init(pAd, pAd->RfIcType, dbdc_mode);
	/* +++Add by shiang for debug */
	RfIC = HcGetRadioRfIC(pAd);
	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_EEPROM, DBG_LVL_INFO,
			 "%s():PhyCtrl=>RfIcType/rf_band_cap = 0x%x/0x%x\n",
			  __func__, pAd->RfIcType, RfIC);
	RTMP_NET_DEV_NICKNAME_INIT(pAd);
	return NDIS_STATUS_SUCCESS;
}


INT32 WfEPROMSysExit(RTMP_ADAPTER *pAd)
{
	hc_radio_exit(pAd, pAd->CommonCfg.dbdc_mode);
	return NDIS_STATUS_SUCCESS;
}

/*
	========================================================================

	Routine Description:
		Initialize NIC hardware

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS	NICInitializeAdapter(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;

	ret = WfMacInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS) {
		ret = NDIS_STATUS_FAILURE;
		goto err;
	}

	MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_INFO, "MAC Init Done!\n");
	ret = WfPhyInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS) {
		ret = NDIS_STATUS_FAILURE;
		goto err;
	}
err:
	return ret;
}

/*
	========================================================================

	Routine Description:
		Compare two memory block

	Arguments:
		pSrc1		Pointer to first memory address
		pSrc2		Pointer to second memory address

	Return Value:
		0:			memory is equal
		1:			pSrc1 memory is larger
		2:			pSrc2 memory is larger

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
ULONG RTMPCompareMemory(VOID *pSrc1, VOID *pSrc2, ULONG Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	ULONG	Index = 0;

	pMem1 = (PUCHAR) pSrc1;
	pMem2 = (PUCHAR) pSrc2;

	for (Index = 0; Index < Length; Index++) {
		if (pMem1[Index] > pMem2[Index])
			return 1;
		else if (pMem1[Index] < pMem2[Index])
			return 2;
	}

	/* Equal*/
	return 0;
}


/*
	========================================================================

	Routine Description:
		Zero out memory block

	Arguments:
		pSrc1		Pointer to memory address
		Length		Size

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPZeroMemory(VOID *pSrc, ULONG Length)
{
	PUCHAR	pMem;
	ULONG	Index = 0;

	pMem = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++)
		pMem[Index] = 0x00;
}


/*
	========================================================================

	Routine Description:
		Copy data from memory block 1 to memory block 2

	Arguments:
		pDest		Pointer to destination memory address
		pSrc		Pointer to source memory address
		Length		Copy size

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPMoveMemory(VOID *pDest, VOID *pSrc, ULONG Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	UINT	Index;

	ASSERT((Length == 0) || (pDest && pSrc));
	pMem1 = (PUCHAR) pDest;
	pMem2 = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++) {
		if (pMem1 && pMem2)
			pMem1[Index] = pMem2[Index];
	}
}


VOID UserCfgExit(RTMP_ADAPTER *pAd)
{
#ifdef RATE_PRIOR_SUPPORT
	INT idx;
	PMAC_TABLE_ENTRY pEntry = NULL;
	PBLACK_STA pBlackSta = NULL, tmp;
	UINT16 wtbl_max_num;
#endif /*RATE_PRIOR_SUPPORT*/
#ifdef RT_CFG80211_SUPPORT
	/* Reset the CFG80211 Internal Flag */
	RTMP_DRIVER_80211_RESET(pAd);
#endif /* RT_CFG80211_SUPPORT */
	entrytb_aid_bitmap_free(&pAd->aid_info);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef BAND_STEERING
	if (pAd->ApCfg.BandSteering) {
		BndStrg_Release(pAd);
	}
#endif /* BAND_STEERING */
#ifdef ZERO_PKT_LOSS_SUPPORT
	{
		BOOLEAN Cancelled;

		//cancel and release CSA0Event timer
		RTMPCancelTimer(&pAd->Dot11_H.CSALastBcnTxEventTimer, &Cancelled);
		RTMPReleaseTimer(&pAd->Dot11_H.CSALastBcnTxEventTimer, &Cancelled);

		//cancel and release NULL tx timer
		RTMPCancelTimer(&pAd->Dot11_H.ChnlSwitchStaNullDataTxTimer, &Cancelled);
		RTMPReleaseTimer(&pAd->Dot11_H.ChnlSwitchStaNullDataTxTimer, &Cancelled);

		//cancel and release last bcn timer
		RTMPCancelTimer(&pAd->Dot11_H.ChnlSwitchStaNullAckWaitTimer, &Cancelled);
		RTMPReleaseTimer(&pAd->Dot11_H.ChnlSwitchStaNullAckWaitTimer, &Cancelled);

	}
#endif /* ZERO_PKT_LOSS_SUPPORT */
	{
		BOOLEAN Canceled;

		RTMPCancelTimer(&pAd->Dot11_H.CSAEventTimer, &Canceled);
		RTMPReleaseTimer(&pAd->Dot11_H.CSAEventTimer, &Canceled);
	}
#ifdef LED_CONTROL_SUPPORT
	{
		BOOLEAN Canceled;

		RTMPCancelTimer(&pAd->LedCntl.LEDControlTimer, &Canceled);
		RTMPReleaseTimer(&pAd->LedCntl.LEDControlTimer, &Canceled);
	}
#endif /* LED_CONTROL_SUPPORT */

#ifdef RADIUS_MAC_ACL_SUPPORT
		{
			PLIST_HEADER pListHeader = NULL;
			RT_LIST_ENTRY *pListEntry = NULL;
			UCHAR apidx = 0;

			for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
				pListHeader = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList;

				if (pListHeader->size == 0)
					continue;

				pListEntry = pListHeader->pHead;

				while (pListEntry != NULL) {
					removeHeadList(pListHeader);
					os_free_mem(pListEntry);
					pListEntry = pListHeader->pHead;
				}

				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"Clean [%d] Radius ACL Cache.\n", apidx);
			}
		}
#endif /* RADIUS_MAC_ACL_SUPPORT */
	}

	pAd->ApCfg.set_ch_async_flag = FALSE;
	pAd->ApCfg.iwpriv_event_flag = FALSE;
	RTMP_OS_COMPLETE_ALL(&pAd->ApCfg.set_ch_aync_done);
#endif /* CONFIG_AP_SUPPORT */
	ChannelOpCtrlDeinit(pAd);

	ChannelPriorityOpCtrlDeinit(pAd);

#ifdef DATA_TXPWR_CTRL
	pAd->ApCfg.data_pwr_cmd_flag = FALSE;
	RTMP_OS_COMPLETE_ALL(&pAd->ApCfg.get_tx_pwr_aync_done);
#endif


#ifdef CONFIG_STA_SUPPORT

	do {
		INT i;
		PSTA_ADMIN_CONFIG	pStaCfg = NULL;

		for (i = 0; i < MAX_MULTI_STA; i++) {
			pStaCfg = &pAd->StaCfg[i];

			if (pStaCfg->wdev.pEapolPktFromAP) {
				os_free_mem(pStaCfg->wdev.pEapolPktFromAP);
				pStaCfg->wdev.pEapolPktFromAP = NULL;
			}

#ifdef	WSC_STA_SUPPORT
			{
				PWSC_CTRL		pWscControl = NULL;

				pWscControl = &pStaCfg->wdev.WscControl;

				if (pWscControl->pWscRxBuf)
					os_free_mem(pWscControl->pWscRxBuf);

				pWscControl->pWscRxBuf = NULL;

				if (pWscControl->pWscTxBuf)
					os_free_mem(pWscControl->pWscTxBuf);

				pWscControl->pWscTxBuf = NULL;
			}
#endif /*WSC_STA_SUPPORT*/
		}
	} while (0);

#endif /* CONFIG_STA_SUPPORT */
	wdev_config_init(pAd);

#ifdef LINUX
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
#ifndef RT_CFG80211_SUPPORT
#ifdef MULTI_INF_SUPPORT
    /* The group info shared by all interface so make sure all interface down
     * before calling group_info_bi_deinit
     */
	if (multi_inf_active_cnt() == 0)
#endif
		group_info_bi_deinit();
#endif /* RT_CFG80211_SUPPORT */
#endif
#endif /* LINUX */

#if defined(DOT11_SAE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
	sae_pwd_id_deinit(pAd);
#endif
#ifdef RATE_PRIOR_SUPPORT
	/*clear the list*/
	OS_SEM_LOCK(&pAd->LowRateCtrl.BlackListLock);
		DlListForEach(pBlackSta, &pAd->LowRateCtrl.BlackList, BLACK_STA, List) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"Remove from blklist, "MACSTR"\n", MAC2STR(pBlackSta->Addr));
			tmp = pBlackSta;
			pBlackSta = DlListEntry(pBlackSta->List.Prev, BLACK_STA, List);
			DlListDel(&(tmp->List));
			os_free_mem(tmp);
	}
	OS_SEM_UNLOCK(&pAd->LowRateCtrl.BlackListLock);
	NdisFreeSpinLock(&pAd->LowRateCtrl.BlackListLock);
#endif/*RATE_PRIOR_SUPPORT*/
	wpf_exit(pAd);
	scan_release_mem(pAd);
#ifdef WIFI_MD_COEX_SUPPORT
	LteSafeChannelDeinit(pAd);
#endif
#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_BL_SUPPORT)
	{
		PLIST_HEADER pListHeader = NULL;
		RT_LIST_ENTRY *pListEntry = NULL;
		UCHAR apidx = 0;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			pListHeader = &pAd->ApCfg.MBSSID[apidx].BlackList;

			if (pListHeader->size == 0)
				continue;

			pListEntry = pListHeader->pHead;

			while (pListEntry != NULL) {
				removeHeadList(pListHeader);
				os_free_mem(pListEntry);
				pListEntry = pListHeader->pHead;
			}
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"Clean [%d] BS Blacklist\n", apidx);
		}
	}
#endif
#ifdef DOT11_EHT_BE
#ifdef CFG_SUPPORT_FALCON_PP
	pp_black_list_reset(pAd);
#endif
#endif
}


/*
	========================================================================

	Routine Description:
		Initialize port configuration structure

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID UserCfgInit(RTMP_ADAPTER *pAd)
{
	UINT i;
	UINT key_index, bss_index;
#ifdef ANTENNA_DIVERSITY_SUPPORT
	UINT band_idx;
#endif /* ANTENNA_DIVERSITY_SUPPORT */
#ifdef SINGLE_SKU_V2
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
#endif
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef FW_LOG_DUMP
	PFW_LOG_CTRL FwLogCtrl = &(pAd->physical_dev->fw_log_ctrl);
#endif
#ifdef ZERO_PKT_LOSS_SUPPORT
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
#endif
	int ret_val;

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "-->\n");
	/*init wifi profile*/
	wpf_init(pAd);
	/* part I. intialize common configuration */
	pAd->CommonCfg.BasicRateBitmap = 0xF;
	pAd->CommonCfg.BasicRateBitmapOld = 0xF;
#ifdef MSCS_PROPRIETARY
	pAd->dabs_drop_threashold = 1;/*1%*/
#endif/*MSCS_PROPRIETARY*/

#ifdef SINGLE_SKU_V2
	if (arch_ops && arch_ops->arch_txpower_sku_cfg_para)
		arch_ops->arch_txpower_sku_cfg_para(pAd);
#endif /* SINGLE_SKU_V2 */
	pAd->CommonCfg.SKU_DUP_Patch_enable = TRUE;
#ifdef TX_POWER_CONTROL_SUPPORT
	os_zero_mem(&pAd->CommonCfg.PowerBoostParamV0,
		sizeof(POWER_BOOST_PARA_V0));
	os_zero_mem(&pAd->CommonCfg.PowerBoostParamV1,
		sizeof(POWER_BOOST_PARA_V1));
#endif /* TX_POWER_CONTROL_SUPPORT */
#ifdef TX_POWER_CONTROL_SUPPORT_V2
	os_zero_mem(pAd->CommonCfg.PowerBoostParamV2,
		sizeof(struct POWER_BOOST_PARA_V2) * POWER_UP_CATE_V2_NUM);

	pAd->CommonCfg.PowerBoostParamV2[POWER_UP_CATE_V2_CCK].catSize = 4;
	pAd->CommonCfg.PowerBoostParamV2[POWER_UP_CATE_V2_OFDM].catSize = 8;
	pAd->CommonCfg.PowerBoostParamV2[POWER_UP_CATE_V2_HT20].catSize = 8;
	pAd->CommonCfg.PowerBoostParamV2[POWER_UP_CATE_V2_HT40].catSize = 9;

	for (i = POWER_UP_CATE_V2_VHT20; i <= POWER_UP_CATE_V2_HE996X2; i++)
		pAd->CommonCfg.PowerBoostParamV2[i].catSize = 12;

	for (i = POWER_UP_CATE_V2_EHT26; i <= POWER_UP_CATE_V2_EHT996X3_484; i++)
		pAd->CommonCfg.PowerBoostParamV2[i].catSize = 16;
#endif /* TX_POWER_CONTROL_SUPPORT_V2 */

	os_zero_mem(&pAd->rxv_dump_ctrl, sizeof(RXV_DUMP_CTRL));
	pAd->rxv_dump_ctrl.rxv_dump_entry_list = NULL;

#ifdef LINK_TEST_SUPPORT
	/* state machine state flag */
	pAd->ucLinkBwState = TX_UNDEFINED_BW_STATE;
	pAd->ucRxStreamState = RX_UNDEFINED_RXSTREAM_STATE;
	pAd->ucRxStreamStatePrev = RX_UNDEFINED_RXSTREAM_STATE;
	pAd->ucRxFilterstate = TX_UNDEFINED_RXFILTER_STATE;
	pAd->ucTxCsdState = TX_UNDEFINED_CSD_STATE;
	pAd->ucTxPwrBoostState = TX_UNDEFINED_POWER_STATE;
	pAd->ucLinkRcpiState = RX_UNDEFINED_RCPI_STATE;
	pAd->ucLinkSpeState = TX_UNDEFINED_SPEIDX_STATE;
	pAd->ucLinkSpeStatePrev = TX_UNDEFINED_SPEIDX_STATE;
	pAd->ucLinkBwStatePrev = TX_UNDEFINED_BW_STATE;
	pAd->ucTxCsdStatePrev = TX_UNDEFINED_CSD_STATE;

	/* BW Control Paramter */
	pAd->fgBwInfoUpdate					   =   FALSE;

	/* Rx Control Parameter */
	pAd->ucRxTestTimeoutCount			   =	   0;
	pAd->c8TempRxCount					   =	   0;
	pAd->ucRssiTh						   =	  10;
	pAd->ucRssiSigniTh					   =	  15;
	pAd->c8RxCountTh					   =	   5;
	pAd->ucTimeOutTh					   =	 200;  /* 20s */
	pAd->ucPerTh						   =	  50;
	pAd->cNrRssiTh						   =	 -40;
	pAd->cChgTestPathTh					   =	 -30;
	pAd->ucRxSenCountTh					   =	   3;
	pAd->cWBRssiTh						   =	 -70;
	pAd->cIBRssiTh						   =	 -80;
	pAd->u1RxStreamSwitchReason			   =	   0;
	pAd->u1RxSenCount					   =	   0;
	pAd->u1SpeRssiIdxPrev				   =	   0;
	pAd->ucRxSenCount					   =	   0;

	/* ACR Control Parameter */
	pAd->ucACRConfidenceCntTh			   =	  10;
	pAd->ucMaxInConfidenceCntTh			   =	  10;
	pAd->cMaxInRssiTh					   =	 -40;
	pAd->ucRxFilterConfidenceCnt		   =	   0;

	/* Tx Control Parameter */
	pAd->ucCmwCheckCount				   =	   0;
	pAd->ucCmwCheckCountTh				   =	  20;  /* 2s */
	pAd->fgCmwInstrumBack4T				   =   FALSE;
	pAd->ucRssiBalanceCount				   =	   0;
	pAd->ucRssiIBalanceCountTh			   =	 100;  /* 10s */
	pAd->fgRssiBack4T					   =   FALSE;
	pAd->ucCableRssiTh					   =	  25;
	pAd->fgCmwLinkDone					   =   FALSE;
	pAd->fgApclientLinkUp				   =   FALSE;
	pAd->ucLinkCount					   =	   0;
	pAd->ucLinkCountTh					   =	  30;
	pAd->fgLinkRSSICheck				   =   FALSE;
	pAd->ucCmwChannelBand				   =   CHANNEL_BAND_2G;

	/* channel band Control Paramter */
	pAd->fgChannelBandInfoUpdate		   =   FALSE;

	/* Tx Power Control Paramter */
	os_zero_mem(pAd->ucTxPwrUpTbl, sizeof(UINT8)*CMW_POWER_UP_RATE_NUM*4);

	/* manual command control function enable/disable flag */
	pAd->fgTxSpeEn						   = TRUE;
	pAd->fgRxRcpiEn						   = TRUE;
	pAd->fgTxSpurEn						   = TRUE;
	pAd->fgRxSensitEn					   = TRUE;
	pAd->fgACREn						   = TRUE;
#endif /* LINK_TEST_SUPPORT */

#ifdef	ETSI_RX_BLOCKER_SUPPORT
	pAd->c1RWbRssiHTh	= -70;
	pAd->c1RWbRssiLTh	= -70;
	pAd->c1RIbRssiLTh	= -80;
	pAd->c1WBRssiTh4R	= -75;

	pAd->fgFixWbIBRssiEn = FALSE;
	pAd->c1WbRssiWF0 = 0xFF;
	pAd->c1WbRssiWF1 = 0xFF;
	pAd->c1WbRssiWF2 = 0xFF;
	pAd->c1WbRssiWF3 = 0xFF;
	pAd->c1IbRssiWF0 = 0xFF;
	pAd->c1IbRssiWF1 = 0xFF;
	pAd->c1IbRssiWF2 = 0xFF;
	pAd->c1IbRssiWF3 = 0xFF;

	pAd->u1RxBlockerState = ETSI_RXBLOCKER4R;
	pAd->u1To1RCheckCnt  = 10;
	pAd->u2To1RvaildCntTH = 100;
	pAd->u2To4RvaildCntTH = 3;
	pAd->u1ValidCnt		 = 0;
	pAd->u14RValidCnt	= 0;

	pAd->u1CheckTime	 = 1;
	pAd->u1TimeCnt		 = 0;

	pAd->i1MaxWRssiIdxPrev  = 0xFF;
	pAd->fgAdaptRxBlock  = 0;
#endif /* end ETSI_RX_BLOCKER_SUPPORT */


	/* disable QA Effuse Write back status by default */
	pAd->fgQAEffuseWriteBack = FALSE;
	/* Disable EPA flag */
	pAd->fgEPA = FALSE;
	/* Apply Cal-Free Effuse value by default */
	pAd->fgCalFreeApply = TRUE;
	pAd->RFlockTempIdx  =    0;
	pAd->CalFreeTempIdx =    0;

	for (key_index = 0; key_index < SHARE_KEY_NUM; key_index++) {
		for (bss_index = 0; bss_index < MAX_MBSSID_NUM(pAd) + MAX_P2P_NUM; bss_index++) {
			pAd->SharedKey[bss_index][key_index].KeyLen = 0;
			pAd->SharedKey[bss_index][key_index].CipherAlg = CIPHER_NONE;
		}
	}

	pAd->Antenna.word = 0;
#ifdef LED_CONTROL_SUPPORT
	pAd->LedCntl.LedIndicatorStrength = 0;
	RTMPInitTimer(pAd, &pAd->LedCntl.LEDControlTimer, GET_TIMER_FUNCTION(LEDControlTimer), pAd, FALSE);
#endif /* LED_CONTROL_SUPPORT */

	RTMPInitTimer(pAd, &pAd->Dot11_H.CSAEventTimer, GET_TIMER_FUNCTION(CSAEventTimeout), pAd, FALSE);
	RTMPInitTimer(pAd, &pAd->Dot11_H.PPCSAEventTimer, GET_TIMER_FUNCTION(PPCSAEventTimeout), pAd, FALSE);

	pAd->force_one_tx_stream = FALSE;
	pAd->RfIcType = RFIC_UNKNOWN;
	/* Init timer for reset complete event*/
	pAd->bForcePrintTX = FALSE;
	pAd->bForcePrintRX = FALSE;
	pAd->bStaFifoTest = FALSE;
	pAd->bProtectionTest = FALSE;
	pAd->bHCCATest = FALSE;
	pAd->bGenOneHCCA = FALSE;
	pAd->CommonCfg.Dsifs = 10;      /* in units of usec */
	pAd->CommonCfg.TxPower = 100; /* mW*/
	pAd->CommonCfg.ucTxPowerPercentage = 100; /* AUTO*/
	pAd->CommonCfg.ucTxPowerDefault = 100; /* AUTO*/
	pAd->CommonCfg.TxPreamble = Rt802_11PreambleAuto; /* use Long preamble on TX by defaut*/
	pAd->CommonCfg.bUseZeroToDisableFragment = FALSE;
	pAd->bDisableRtsProtect = FALSE;
	pAd->CommonCfg.UseBGProtection = 0;    /* 0: AUTO*/
	pAd->CommonCfg.bEnableTxBurst = TRUE; /* 0;	*/
#ifdef PEAK_ENHANCE
	peak_adjust_init_mac_ad(pAd);
#endif /* PEAK_ENHANCE */
	pAd->CommonCfg.SavedPhyMode = 0xff;
	pAd->CommonCfg.BandState = UNKNOWN_BAND;
	pAd->wmm_cw_min = 4;
#ifdef VLAN_SUPPORT
	pAd->CommonCfg.bEnableVlan = TRUE;	/* default enble vlan function */
#endif /*VLAN_SUPPORT*/

	switch (pAd->OpMode) {
	case OPMODE_AP:
		pAd->wmm_cw_max = 6;
		break;

	case OPMODE_STA:
		pAd->wmm_cw_max = 10;
		break;
	}


#ifdef CONFIG_AP_SUPPORT
#ifdef ZERO_PKT_LOSS_SUPPORT
	pAd->Zero_Loss_Enable = 0;
	pAd->Csa_Action_Frame_Enable = 0;
	pAd->ZeroLossStaCount = 0;
	pAd->ucSTATimeout = 500;
	pAd->ZeroLossStaPsQLimit = 600;

	RTMPInitTimer(pAd, &pAd->Dot11_H.CSALastBcnTxEventTimer, GET_TIMER_FUNCTION(CSALastBcnTxEventTimeout), ctrl, FALSE);
	RTMPInitTimer(pAd, &pAd->Dot11_H.ChnlSwitchStaNullDataTxTimer, GET_TIMER_FUNCTION(ChnlSwitchStaNullDataTxTimeout), ctrl, FALSE);
	RTMPInitTimer(pAd, &pAd->Dot11_H.ChnlSwitchStaNullAckWaitTimer, GET_TIMER_FUNCTION(ChnlSwitchStaNullAckWaitTimeout), ctrl, FALSE);
	pAd->Dot11_H.ChannelSwitchTriggerCSACount = 1;
#endif /*ZERO_PKT_LOSS_SUPPORT*/
#ifdef AP_SCAN_SUPPORT
	acs_config_init(pAd);
#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT

	/* If profile parameter is set, ACS parameters of HW bands need to be reset*/
	hc_init_ACSChCtrl(pAd);
#endif/* CONFIG_AP_SUPPORT */

	pAd->CommonCfg.bNeedSendTriggerFrame = FALSE;
	pAd->CommonCfg.TriggerTimerCount = 0;
	pAd->CommonCfg.bAPSDForcePowerSave = FALSE;
	/*pAd->CommonCfg.bCountryFlag = FALSE;*/
	/*pAd->CommonCfg.TxStream = 0;*/
	/*pAd->CommonCfg.RxStream = 0;*/
	pAd->CommonCfg.bTpePSDsingleBW20Flag = TRUE;
#ifdef DOT11_N_SUPPORT
	pAd->bBroadComHT = FALSE;
	pAd->CommonCfg.bRdg = FALSE;
#ifdef DOT11N_DRAFT3
	pAd->CommonCfg.Dot11OBssScanPassiveDwell = dot11OBSSScanPassiveDwell;	/* Unit : TU. 5~1000*/
	pAd->CommonCfg.Dot11OBssScanActiveDwell = dot11OBSSScanActiveDwell;	/* Unit : TU. 10~1000*/
	pAd->CommonCfg.Dot11BssWidthTriggerScanInt = dot11BSSWidthTriggerScanInterval;	/* Unit : Second	*/
	pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = dot11OBSSScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000*/
	pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = dot11OBSSScanActiveTotalPerChannel;	/* Unit : TU. 20~10000*/
	pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = dot11BSSWidthChannelTransactionDelayFactor;
	pAd->CommonCfg.Dot11OBssScanActivityThre = dot11BSSScanActivityThreshold;	/* Unit : percentage*/
	pAd->CommonCfg.Dot11BssWidthChanTranDelay = (ULONG)(pAd->CommonCfg.Dot11BssWidthTriggerScanInt *
			pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
	pAd->CommonCfg.bBssCoexEnable =
		TRUE; /* by default, we enable this feature, you can disable it via the profile or ioctl command*/
	pAd->CommonCfg.BssCoexApCntThr = 0;
	pAd->CommonCfg.Bss2040NeedFallBack = 0;
	NdisZeroMemory(&pAd->CommonCfg.BssCoexScanLastResult, sizeof(BSS_COEX_SCAN_LAST_RESULT));
#endif  /* DOT11N_DRAFT3 */
	pAd->CommonCfg.bRcvBSSWidthTriggerEvents = FALSE;
	pAd->CommonCfg.bExtChannelSwitchAnnouncement = 1;
	pAd->CommonCfg.bMIMOPSEnable = TRUE;
	pAd->CommonCfg.bDisableReordering = FALSE;

	if (pAd->MACVersion == 0x28720200)
		pAd->CommonCfg.TxBASize = 13; /*by Jerry recommend*/
	else
		pAd->CommonCfg.TxBASize = 7;

	pAd->CommonCfg.REGBACapability.word = pAd->CommonCfg.BACapability.word;
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_EHT_BE
	pAd->CommonCfg.bMaxChannelSwitchTime = 1;
#endif
	pAd->CommonCfg.TxRate = RATE_6;
	pAd->CommonCfg.BeaconPeriod = 100;   /* in mSec*/
#ifdef TXBF_SUPPORT
	pAd->CommonCfg.ETxBfNoncompress = 0;
	pAd->CommonCfg.ETxBfIncapable = 0;
	pAd->CommonCfg.MaxBfeeSS = 255;
	pAd->CommonCfg.BfNDPARate = 255;
#endif /* TXBF_SUPPORT */
#if defined(RATE_ADAPT_AGBS_SUPPORT)
	pAd->CommonCfg.lowTrafficThrd = 2;
	pAd->CommonCfg.TrainUpRule = 2; /* 1; */
	pAd->CommonCfg.TrainUpRuleRSSI = -70; /* 0; */
	pAd->CommonCfg.TrainUpLowThrd = 90;
	pAd->CommonCfg.TrainUpHighThrd = 110;
#endif /* defined(RATE_ADAPT_AGBS_SUPPORT) */
	/* WFA policy - disallow TH rate in WEP or TKIP cipher */
	pAd->CommonCfg.HT_DisallowTKIP = TRUE;
	/* Frequency for rate adaptation */
	pAd->ra_interval = DEF_RA_TIME_INTRVAL;
	pAd->ra_fast_interval = DEF_QUICK_RA_TIME_INTERVAL;
	/* global variables mXXXX used in MAC protocol state machines*/
	OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADHOC_ON);
	/* PHY specification*/
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);  /* CCK use LONG preamble*/
#ifdef CONFIG_AP_SUPPORT
	/* Default Config change flag*/
#ifdef GREENAP_SUPPORT
	greenap_init(pAd);
#endif /* GREENAP_SUPPORT */
#endif
	/*
		part III. AP configurations
	*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* Set MBSS Default Configurations*/
		UCHAR j;

		pAd->ApCfg.BandMaxStaNum = 0;
		NdisAcquireSpinLock(pAd->MacTabLock);
		pAd->ApCfg.perBandStaCount = 0;
		NdisReleaseSpinLock(pAd->MacTabLock);

		pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);
		pAd->ApCfg.reject_mgmt_rx = MGMT_RX_ACCEPT_ALL;
#ifdef CONFIG_MAP_SUPPORT
		pAd->ApCfg.Disallow_ProbeEvent = FALSE;
#ifdef MAP_R4
		pAd->ReconfigTrigger = FALSE;
#endif /* MAP_R4 */
#endif /* CONFIG_MAP_SUPPORT */

#if defined(MTK_HOSTAPD_SUPPORT) && defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
		OS_SEM_LOCK(&pAd->gas_frame_list_lock);
		pAd->is_dpp_gas_list_init = TRUE;
		DlListInit(&pAd->dpp_gas_event_list);
		MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_WARN,
				"[DPP]: Gas DL List Init!\n");
		OS_SEM_UNLOCK(&pAd->gas_frame_list_lock);
#endif /* MTK_HOSTAPD_SUPPORT && CONFIG_MAP_SUPPORT && MAP_R3 */

		for (j = BSS0; j < pAd->ApCfg.BssidNum; j++) {
			BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[j];
			struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[j].wdev;

			wdev_ap_default_init(pAd, wdev);
#ifdef CONFIG_MAP_SUPPORT
#ifdef MAP_BL_SUPPORT
			initList(&mbss->BlackList);
			NdisAllocateSpinLock(pAd, &mbss->BlackListLock);
#endif /*  MAP_BL_SUPPORT */
#endif /* CONFIG_MAP_SUPPORT */

#ifdef DOT11_EHT_BE
			/* default to 0xff */
			mbss->tid_mapping = 0xff;
#endif
#if defined(DOT1X_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
			/* PMK cache setting*/

#ifdef R1KH_HARD_RETRY /* yiwei no give up! */
			/* profile already set to 120to prevent PMK is delete on DUT */
			mbss->PMKCachePeriod = (120 * 60 * OS_HZ); /* unit : tick(default: 120 minute)*/
#else /* R1KH_HARD_RETRY */
			mbss->PMKCachePeriod = (10 * 60 * OS_HZ); /* unit : tick(default: 10 minute)*/
#endif /* !R1KH_HARD_RETRY */
#endif /* DOT1X_SUPPORT */
		/* VLAN related */
#ifdef VLAN_SUPPORT
			mbss->wdev.bVLAN_Tag = 0;
			mbss->wdev.VLAN_VID = 0;
			mbss->wdev.VLAN_Priority = 0;
			mbss->wdev.VLAN_Policy[TX_VLAN] = VLAN_TX_ALLOW;
			mbss->wdev.VLAN_Policy[RX_VLAN] = 0;
#endif
			/* Default MCS as AUTO*/
			wdev->bAutoTxRateSwitch = TRUE;
			wdev->IsAXOnly = 0;
			wdev->IsBEOnly = 0;
			wdev->IsNoAGMode = 0;
			wdev->IsNoNMode = 0;
			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
			wdev->bcn_buf.stop_tx = FALSE;

			/* Default is zero. It means no limit.*/
			mbss->StaCount = 0;



#ifdef CUSTOMER_VENDOR_IE_SUPPORT
			pAd->ApCfg.MBSSID[j].ap_vendor_ie.length = 0;
			pAd->ApCfg.MBSSID[j].ap_vendor_ie.pointer = NULL;
			NdisAllocateSpinLock(pAd, &pAd->ApCfg.MBSSID[j].ap_vendor_ie.vendor_ie_lock);
			NdisAllocateSpinLock(pAd, &pAd->ApCfg.MBSSID[j].probe_rsp_vendor_ie_lock);
			DlListInit(&mbss->ap_probe_rsp_vendor_ie_list);
			pAd->ApCfg.ap_probe_rsp_vendor_ie_count = 0;
			pAd->ApCfg.ap_probe_rsp_vendor_ie_max_count = AP_PROBE_RSP_VIE_MAX_CNT;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
#if defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)
			mbss->IGMPPeriodicQuerySent = FALSE;
			mbss->MLDPeriodicQuerySent = FALSE;
			mbss->IgmpQueryHoldTick = 0;
			mbss->IgmpQueryHoldTickChanged = FALSE;
			mbss->MldQueryHoldTick = 0;
			mbss->MldQueryHoldTickChanged = FALSE;
			mbss->MldQryChkSum = 0x0;
			NdisZeroMemory(&mbss->ipv6LinkLocalSrcAddr[0], 16);
#endif

#ifdef MBSS_DTIM_SUPPORT
			mbss->DtimCount = 0;
			mbss->DtimPeriod = DEFAULT_DTIM_PERIOD;
#endif
			/* Init no forwarding */
			mbss->IsolateInterStaTraffic = 0;

#ifdef DOT11_EHT_BE
			/* default to 0xff */
			mbss->tid_mapping = 0xff;
#endif
			/* Init BSS Max Idle */
			mbss->max_idle_ie_en = TRUE;
			mbss->max_idle_option = 0;
			mbss->max_idle_period = MAC_TABLE_AGEOUT_TIME;
#ifdef DOT11V_MBSSID_SUPPORT
			mbss_11v_init(pAd, mbss);
#endif /* DOT11V_MBSSID_SUPPORT */
		}

#ifdef DOT11V_MBSSID_SUPPORT
		/* set for 11v NT-BSS can carry */
		pAd->b11vExtraIe = FALSE;
#endif /* DOT11V_MBSSID_SUPPORT */

#ifdef DOT1X_SUPPORT
#endif /* DOT1X_SUPPORT */
		pAd->ApCfg.DtimCount  = 0;
		pAd->ApCfg.DtimPeriod = DEFAULT_DTIM_PERIOD;
		pAd->ApCfg.ErpIeContent = 0;
		pAd->ApCfg.BANClass3Data = FALSE;
#ifdef IDS_SUPPORT
		/* Default disable IDS threshold and reset all IDS counters*/
		pAd->ApCfg.IdsEnable = FALSE;
		pAd->ApCfg.AuthFloodThreshold = 0;
		pAd->ApCfg.AssocReqFloodThreshold = 0;
		pAd->ApCfg.ReassocReqFloodThreshold = 0;
		pAd->ApCfg.ProbeReqFloodThreshold = 0;
		pAd->ApCfg.DisassocFloodThreshold = 0;
		pAd->ApCfg.DeauthFloodThreshold = 0;
		pAd->ApCfg.EapReqFloodThreshold = 0;
		RTMPClearAllIdsCounter(pAd);
#endif /* IDS_SUPPORT */
#ifdef WDS_SUPPORT
		APWdsInitialize(pAd);
#endif /* WDS_SUPPORT*/
#ifdef WSC_INCLUDED
		pAd->WriteWscCfgToDatFile = 0xFF;
		pAd->WriteWscCfgToAr9DatFile = FALSE;
#ifdef CONFIG_AP_SUPPORT
#if defined(RTMP_PCI_SUPPORT) && defined(RTMP_RBUS_SUPPORT)
		pAd->bWscDriverAutoUpdateCfg = (IS_RBUS_INF(pAd)) ? FALSE : TRUE;
#else
		pAd->bWscDriverAutoUpdateCfg = TRUE;
#endif /* defined(RTMP_PCI_SUPPORT) && defined (RTMP_RBUS_SUPPORT) */
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef APCLI_SUPPORT
		pAd->ApCfg.FlgApCliIsUapsdInfoUpdated = FALSE;
		pAd->ApCfg.ApCliNum = MAX_APCLI_NUM;
#ifdef BT_APCLI_SUPPORT
		pAd->ApCfg.ApCliAutoBWBTSupport = FALSE;
#endif
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		pAd->ApCfg.ApCliAutoBWRules.majorPolicy.ApCliBWSyncBandSupport = 0;
		pAd->ApCfg.ApCliAutoBWRules.majorPolicy.ApCliBWSyncDeauthSupport = FALSE;

		pAd->ApCfg.ApCliAutoBWRules.minorPolicy.ApCliBWSyncHTSupport = 0;
		pAd->ApCfg.ApCliAutoBWRules.minorPolicy.ApCliBWSyncVHTSupport = 0;
#endif
		for (j = 0; j < MAX_APCLI_NUM; j++) {
			STA_ADMIN_CONFIG *apcli_entry = &pAd->StaCfg[j];
			struct wifi_dev *wdev = &apcli_entry->wdev;

			wdev_sta_default_init(pAd, wdev);
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			apcli_entry->ApcliInfStat.AutoConnectFlag = FALSE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
			apcli_entry->bBlockAssoc = FALSE;
#if defined(APCLI_CFG80211_SUPPORT)
			apcli_entry->wpa_supplicant_info.IEEE8021x_required_keys = FALSE;
			apcli_entry->wpa_supplicant_info.bRSN_IE_FromWpaSupplicant = FALSE;
			apcli_entry->wpa_supplicant_info.bLostAp = FALSE;
			apcli_entry->bConfigChanged = FALSE;
			apcli_entry->wpa_supplicant_info.DesireSharedKeyId = 0;
			apcli_entry->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_DISABLE;
			apcli_entry->wpa_supplicant_info.WpaSupplicantScanCount = 0;
			apcli_entry->wpa_supplicant_info.pWpsProbeReqIe = NULL;
			apcli_entry->wpa_supplicant_info.WpsProbeReqIeLen = 0;
			apcli_entry->wpa_supplicant_info.pWpaAssocIe = NULL;
			apcli_entry->wpa_supplicant_info.WpaAssocIeLen = 0;
			apcli_entry->SavedPMKNum = 0;
			RTMPZeroMemory(apcli_entry->SavedPMK, (PMKID_NO * sizeof(BSSID_INFO)));
#endif/*APCLI_CFG80211_SUPPORT*/
#ifdef APCLI_CONNECTION_TRIAL
			apcli_entry->TrialCh = 0;/* if the channel is 0, AP will connect the rootap is in the same channel with ra0. */
#endif /* APCLI_CONNECTION_TRIAL */
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
			/* for vendor ie */
			pAd->StaCfg[j].apcli_vendor_ie.length = 0;
			pAd->StaCfg[j].apcli_vendor_ie.pointer = NULL;
			NdisAllocateSpinLock(pAd, &pAd->StaCfg[j].apcli_vendor_ie.vendor_ie_lock);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
		}
#endif /* APCLI_SUPPORT */

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
		NdisAllocateSpinLock(pAd, &pAd->ScanCtrl.ScanTab.event_bss_entry_lock);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
		NdisAcquireSpinLock(pAd->MacTabLock);
		pAd->ApCfg.EntryClientCount = 0;
		NdisReleaseSpinLock(pAd->MacTabLock);
#if defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)
		pAd->bIGMPperiodicQuery = TRUE;
		pAd->IgmpQuerySendTick = QUERY_SEND_PERIOD;
		pAd->bMLDperiodicQuery = TRUE;
		pAd->MldQuerySendTick = QUERY_SEND_PERIOD;
#endif
		pAd->CommonCfg.bUseShortSlotTime = TRUE;
		pAd->CommonCfg.SlotTime = 9;
	}
	pAd->ApCfg.ObssGBandChanBitMap = 0;
#ifdef VOW_SUPPORT
	pAd->vow_cfg.mcli_sch_cfg.tcp_cnt_th = 0;
	pAd->vow_cfg.mcli_sch_cfg.mcli_tcp_num = 0;
	pAd->vow_cfg.mcli_sch_cfg.dl_wrr_en = FALSE;
	pAd->vow_cfg.mcli_sch_cfg.schedule_cond_running = 0x00;
	pAd->vow_cfg.mcli_sch_cfg.cwmin[VOW_MCLI_DL_MODE] = DL_MULTI_CLIENT_CWMIN;
	pAd->vow_cfg.mcli_sch_cfg.cwmax[VOW_MCLI_DL_MODE] = DL_MULTI_CLIENT_CWMAX;
	pAd->vow_cfg.mcli_sch_cfg.cwmin[VOW_MCLI_UL_MODE] = UL_MULTI_CLIENT_CWMIN;
	pAd->vow_cfg.mcli_sch_cfg.cwmax[VOW_MCLI_UL_MODE] = UL_MULTI_CLIENT_CWMAX;
#endif

#endif /* CONFIG_AP_SUPPORT */
#ifdef ETH_CONVERT_SUPPORT

	if (pAd->OpMode == OPMODE_STA) {
		NdisZeroMemory(pAd->EthConvert.EthCloneMac, MAC_ADDR_LEN);
		pAd->EthConvert.ECMode = ETH_CONVERT_MODE_DISABLE;
		pAd->EthConvert.CloneMacVaild = FALSE;
		/*pAd->EthConvert.nodeCount = 0;*/
		NdisZeroMemory(pAd->EthConvert.SSIDStr, MAX_LEN_OF_SSID);
		pAd->EthConvert.SSIDStrLen = 0;
		pAd->EthConvert.macAutoLearn = FALSE;
	}

#endif /* ETH_CONVERT_SUPPORT */
	/*
		part IV. others
	*/
	/* dynamic BBP R66:sensibity tuning to overcome background noise*/
	pAd->BbpTuning.bEnable = TRUE;
	pAd->BbpTuning.FalseCcaLowerThreshold = 100;
	pAd->BbpTuning.FalseCcaUpperThreshold = 512;
	pAd->BbpTuning.R66Delta = 4;
	pAd->Mlme.bEnableAutoAntennaCheck = TRUE;
	/* Also initial R66CurrentValue, RTUSBResumeMsduTransmission might use this value.*/
	/* if not initial this value, the default value will be 0.*/
	pAd->BbpTuning.R66CurrentValue = 0x38;
	/*RTMPInitTimer(pAd, &pAd->RECBATimer, RECBATimerTimeout, pAd, TRUE);*/
	/*RTMPSetTimer(&pAd->RECBATimer, REORDER_EXEC_INTV);*/
	pAd->CommonCfg.bWiFiTest = FALSE;
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	FT_CfgInitial(pAd);
#endif /* DOT11R_FT_SUPPORT */
	pAd->ApCfg.set_ch_async_flag = FALSE;
	pAd->ApCfg.iwpriv_event_flag = FALSE;
	RTMP_OS_INIT_COMPLETION(&pAd->ApCfg.set_ch_aync_done);
#ifdef DATA_TXPWR_CTRL
	pAd->ApCfg.data_pwr_cmd_flag = FALSE;
	RTMP_OS_INIT_COMPLETION(&pAd->ApCfg.get_tx_pwr_aync_done);
#endif
#endif /* CONFIG_AP_SUPPORT */

	ChannelOpCtrlInit(pAd);

	ChannelPriorityOpCtrlInit(pAd);

	pAd->RxAnt.Pair1PrimaryRxAnt = 0;
	pAd->RxAnt.Pair1SecondaryRxAnt = 1;
	pAd->RxAnt.EvaluatePeriod = 0;
	pAd->RxAnt.RcvPktNumWhenEvaluate = 0;
	pAd->MaxTxPwr = 27;

#ifdef CONFIG_AP_SUPPORT
	pAd->RxAnt.Pair1AvgRssiGroup1[0] = pAd->RxAnt.Pair1AvgRssiGroup1[1] = 0;
	pAd->RxAnt.Pair1AvgRssiGroup2[0] = pAd->RxAnt.Pair1AvgRssiGroup2[1] = 0;
#endif /* CONFIG_AP_SUPPORT */
#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	{

		BSS_TABLE *ScanTab = &pAd->ScanCtrl.ScanTab;

		for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++) {
			BSS_ENTRY *pBssEntry = &ScanTab->BssEntry[i];

			if (pAd->ProbeRespIE[i].pIe)
				pBssEntry->pVarIeFromProbRsp = pAd->ProbeRespIE[i].pIe;
			else
				pBssEntry->pVarIeFromProbRsp = NULL;
		}
	}
#endif /* defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT) */
		pAd->ScanCtrl.dfs_ch_utilization = TRUE;

#ifdef WSC_INCLUDED
	NdisZeroMemory(&pAd->CommonCfg.WscStaPbcProbeInfo, sizeof(WSC_STA_PBC_PROBE_INFO));
	pAd->CommonCfg.WscPBCOverlap = FALSE;
#endif /* WSC_INCLUDED */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
	pAd->WOW_Cfg.bEnable = FALSE;
	pAd->WOW_Cfg.bWOWFirmware = FALSE;	/* load normal firmware */
	pAd->WOW_Cfg.bInBand = TRUE;		/* use in-band signal */
	pAd->WOW_Cfg.nSelectedGPIO = 2;
	pAd->WOW_Cfg.nDelay = 3; /* (3+1)*3 = 12 sec */
	pAd->WOW_Cfg.nHoldTime = 1000;	/* unit is us */
	pAd->WOW_Cfg.nWakeupInterface = cap->nWakeupInterface; /* WOW_WAKEUP_BY_USB; */
	pAd->WOW_Cfg.bGPIOHighLow = WOW_GPIO_LOW_TO_HIGH;
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		"WOW Enable %d, WOWFirmware %d\n", pAd->WOW_Cfg.bEnable,
			 pAd->WOW_Cfg.bWOWFirmware);
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT) */

	/* Clear channel ctrl buffer */
	hc_init_ChCtrl(pAd);
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		"\x1b[1;33m Clear channel ctrl buffer \x1b[m\n");
	pAd->CommonCfg.ChGrpEn = 0;
	NdisZeroMemory(pAd->CommonCfg.ChGrpChannelList, (MAX_NUM_OF_CHANNELS)*sizeof(UCHAR));
	pAd->CommonCfg.ChGrpChannelNum = 0;

	init_ch_chg_info(pAd);

#ifdef WIFI_MD_COEX_SUPPORT
	pAd->idcState = TRUE;
	LteSafeChannelInit(pAd);
#endif

#ifdef MT_DFS_SUPPORT
	DfsParamInit(pAd);/* Jelly20150311 */
#endif
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#ifdef MT_DFS_SUPPORT
		pAd->Dot11_H.ChannelMode = CHAN_SILENCE_MODE;
#else
		pAd->Dot11_H.ChannelMode = CHAN_NORMAL_MODE;
#endif
#endif

	pAd->Dot11_H.CSCount = 0;
	pAd->Dot11_H.CSPeriod = 10;
	//TGX_TEST
	pAd->Dot11_H.MaxChannelSwitchTime = MAX_CHANNEL_SWITCH_TIME;
	pAd->Dot11_H.wdev_count = 0;
	pAd->Dot11_H.csa_ap_bitmap = 0;
	pAd->Dot11_H.cac_time = 65;
	pAd->Dot11_H.bDFSIndoor = 1;
	pAd->Dot11_H.disconn_after_ch_switch = FALSE;


	scan_partial_init(pAd);
#ifdef APCLI_SUPPORT
#ifdef APCLI_AUTO_CONNECT_SUPPORT

	for (i = 0; i < MAX_APCLI_NUM; i++) {
		pAd->StaCfg[i].ApCliAutoConnectType = TRIGGER_SCAN_BY_USER; /* User Trigger SCAN by default */
		pAd->StaCfg[i].ApCliAutoConnectRunning = FALSE;
	}

	pAd->ApCfg.ApCliAutoConnectChannelSwitching = FALSE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	for (i = 0; i < MAX_APCLI_NUM; i++) {
		pAd->ApCfg.bPartialScanEnable[i] = FALSE;
		pAd->ApCfg.bPartialScanning[i] = FALSE;
	}
#endif /* APCLI_SUPPORT */

#ifdef MICROWAVE_OVEN_SUPPORT

	if (pAd->OpMode == OPMODE_AP)
		pAd->CommonCfg.MO_Cfg.bEnable = TRUE;
	else
		pAd->CommonCfg.MO_Cfg.bEnable = FALSE;

	pAd->CommonCfg.MO_Cfg.nFalseCCATh = MO_FALSE_CCA_TH;
#endif /* MICROWAVE_OVEN_SUPPORT */
#ifdef DYNAMIC_VGA_SUPPORT
	pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable = TRUE;
	pAd->CommonCfg.lna_vga_ctl.nFalseCCATh = 600;
	pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh = 100;
#endif /* DYNAMIC_VGA_SUPPORT */
#ifdef DOT11_VHT_AC
	pAd->CommonCfg.bNonVhtDisallow = FALSE;
#endif /* DOT11_VHT_AC */
#ifdef MT_MAC
	cap->TmrEnable = 0;
#endif
#ifdef SNIFFER_SUPPORT
	pAd->monitor_ctrl.CurrentMonitorMode = 0;
	pAd->monitor_ctrl.FrameType = FC_TYPE_RSVED;
	pAd->monitor_ctrl.FilterSize = RX_DATA_BUFFER_SIZE + sizeof(struct mtk_radiotap_header);
#endif /* SNIFFER_SUPPORT */

#ifdef SNIFFER_RADIOTAP_SUPPORT
	pAd->monitor_ctrl.bMonitorOn = FALSE;
#endif

	pAd->bPS_Retrieve = 1;
	pAd->CommonCfg.bTXRX_RXV_ON = 0;
	pAd->parse_rxv_stat_enable = 0;
	pAd->rx_stat_rxv.rxv_cnt = 0;
	pAd->rxv_raw_data.rxv_pkt = NULL;
	pAd->rxv_raw_data.rxv_byte_cnt = 0;
	pAd->rxv_entry_sta_cnt = 0;

	pAd->AccuOneSecRxBand0FcsErrCnt = 0;
	pAd->AccuOneSecRxBand0MdrdyCnt = 0;
	pAd->AccuOneSecRxBand1FcsErrCnt = 0;
	pAd->AccuOneSecRxBand1MdrdyCnt = 0;
	pAd->CommonCfg.ManualTxop = 0;
	pAd->CommonCfg.ManualTxopThreshold = 10; /* Mbps */
	pAd->CommonCfg.ManualTxopUpBound = 20; /* Ratio */
	pAd->CommonCfg.ManualTxopLowBound = 5; /* Ratio */
#ifdef CONFIG_AP_SUPPORT
	vow_variable_reset(pAd);
#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
	pAd->ApCfg.bRoamingEnhance = FALSE;
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	pAd->rts_retrylimit = 0;

	for (i = 0; i < 16; i++)
		pAd->retrylimit[i] = 0;
	pAd->retrylimit_wcid = cap->wtbl_max_entries;

	pAd->cp_support = 3;
#ifdef CONFIG_AP_SUPPORT
	pAd->multi_cli_nums_eap_th = MULTI_CLIENT_NUMS_EAP_TH;
	pAd->aggManualEn = TRUE;
	pAd->per_dn_th = PER_DN_TH;
	pAd->per_up_th = PER_UP_TH;
	pAd->winsize_kp_idx = WINSIZE_KP_IDX;
#endif /* CONFIG_AP_SUPPORT */
#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
	pAd->ICapMode = 0;
	pAd->SpectrumEventCnt = 0;
	pAd->ICapStatus = 0;
	pAd->ICapEventCnt = 0;
	pAd->ICapDataCnt = 0;
	pAd->ICapIdx = 0;
	pAd->ICapCapLen = 0;
	pAd->ICapL32Cnt = 0;
	pAd->ICapM32Cnt = 0;
	pAd->ICapH32Cnt = 0;
	pAd->pL32Bit = NULL;
	pAd->pM32Bit = NULL;
	pAd->pH32Bit = NULL;
	pAd->pSrc_IQ = "/tmp/WifiSpectrum_IQ.txt";
	pAd->pSrc_Gain = "/tmp/WifiSpectrum_LNA_LPF.txt";
	pAd->pSrc_InPhySniffer = "/tmp/InPhySniffer.txt";
	/* Dynamic allocate memory for pIQ_Array buffer */
	{
		UINT32 retval, Len;
		RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

		Len = pChipCap->ICapMaxIQCnt * sizeof(RBIST_IQ_DATA_T);
		retval = os_alloc_mem(pAd, (UCHAR **)&pAd->pIQ_Array, Len);
		if (retval != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"Not enough memory for dynamic allocating !!\n");
		}
		os_zero_mem(pAd->pIQ_Array, Len);
	}
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

#ifdef PHY_ICS_SUPPORT
	pAd->PhyIcsFlag = 0;
#endif /* PHY_ICS_SUPPORT */

	/* ===================================================== */
#ifdef CONFIG_STA_SUPPORT
	/* Following code is needed for both STA mode and ApCli Mode */
	if ((IF_COMBO_HAVE_AP_STA(pAd)) || (IF_COMBO_HAVE_STA(pAd))) {
		pAd->MSTANum = 1;
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_DIRECT);
			RX_FILTER_CLEAR_FLAG(pAd, fRX_FILTER_ACCEPT_MULTICAST);
			RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_BROADCAST);
			RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_ALL_MULTICAST);
			pAd->CommonCfg.NdisRadioStateOff = FALSE;
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WAKEUP_NOW);
#ifdef PROFILE_STORE
			pAd->bWriteDat = FALSE;
#endif /* PROFILE_STORE */
			pAd->RxAnt.Pair1AvgRssi[0] = pAd->RxAnt.Pair1AvgRssi[1] = 0;

			pAd->Dot11_H.ChannelMode = CHAN_NORMAL_MODE;
		}

		for (i = 0; i < MAX_MULTI_STA; i++) {
			PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[i];
			struct adhoc_info *adhocInfo = &pStaCfg->adhocInfo;
			struct wifi_dev *wdev = &pStaCfg->wdev;
			SCAN_INFO *ScanInfo = &wdev->ScanInfo;

			pStaCfg->PwrMgmt.bDoze = FALSE;
			pStaCfg->CountDowntoPsm = 0;
#ifdef ETH_CONVERT_SUPPORT
#ifdef IP_ASSEMBLY
			pStaCfg->bFragFlag = TRUE;
#endif /* #ifdef IP_ASSEMBLY */
#endif
			pStaCfg->PwrMgmt.Psm = PWR_ACTIVE;
			CLEAR_CIPHER(pStaCfg->PairwiseCipher);
			CLEAR_CIPHER(pStaCfg->GroupCipher);
			/* 802.1x port control*/
			pStaCfg->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
			pStaCfg->LastMicErrorTime = 0;
			pStaCfg->MicErrCnt        = 0;
			pStaCfg->bBlockAssoc      = FALSE;
			pStaCfg->WpaState         = SS_NOTUSE;
			pStaCfg->RssiTrigger = 0;
			NdisZeroMemory(&pStaCfg->RssiSample, sizeof(RSSI_SAMPLE));
			pStaCfg->RssiTriggerMode = RSSI_TRIGGERED_UPON_BELOW_THRESHOLD;
			adhocInfo->AtimWin = 0;
			pStaCfg->DefaultListenCount = 3;/*default listen count;*/
			pStaCfg->BssType = BSS_INFRA;  /* BSS_INFRA or BSS_ADHOC or BSS_MONITOR*/
			pStaCfg->bSkipAutoScanConn = FALSE;
			pStaCfg->bAutoConnectIfNoSSID = FALSE;
#ifdef EXT_BUILD_CHANNEL_LIST
			pStaCfg->IEEE80211dClientMode = Rt802_11_D_None;
#endif /* EXT_BUILD_CHANNEL_LIST */
			STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_INFRA_ON);
			/* user desired power mode*/
			pStaCfg->WindowsPowerMode = Ndis802_11PowerModeCAM;
			pStaCfg->WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
			pStaCfg->bWindowsACCAMEnable = FALSE;
			pStaCfg->bHwRadio = TRUE; /* Default Hardware Radio status is On*/
			pStaCfg->bSwRadio = TRUE; /* Default Software Radio status is On*/
			pStaCfg->bRadio = TRUE; /* bHwRadio && bSwRadio*/
			pStaCfg->bHardwareRadio = FALSE;        /* Default is OFF*/
			pStaCfg->bShowHiddenSSID = FALSE;       /* Default no show*/
			/* Nitro mode control*/
#ifdef RT_CFG80211_SUPPORT
			pStaCfg->bAutoReconnect = FALSE;
#else /* RT_CFG80211_SUPPORT */
			pStaCfg->bAutoReconnect = TRUE;
#endif /* !RT_CFG80211_SUPPORT */
			/* Save the init time as last scan time, the system should do scan after 2 seconds.*/
			/* This patch is for driver wake up from standby mode, system will do scan right away.*/
			NdisGetSystemUpTime(&pStaCfg->LastScanTime);

			if (pStaCfg->LastScanTime > 10 * OS_HZ)
				pStaCfg->LastScanTime -= (10 * OS_HZ);

			NdisZeroMemory(pAd->nickname, IW_ESSID_MAX_SIZE + 1);

			NdisZeroMemory(pStaCfg->ReplayCounter, 8);
#ifdef DOT11R_FT_SUPPORT
			NdisZeroMemory(&pStaCfg->Dot11RCommInfo, sizeof(DOT11R_CMN_STRUC));
#endif /* DOT11R_FT_SUPPORT */
			pStaCfg->bAutoConnectByBssid = FALSE;
			pStaCfg->BeaconLostTime = BEACON_LOST_TIME;
			NdisZeroMemory(pStaCfg->WpaPassPhrase, 64);
			pStaCfg->WpaPassPhraseLen = 0;
			pStaCfg->bAutoRoaming = FALSE;
			pStaCfg->bForceTxBurst = FALSE;
			pStaCfg->bNotFirstScan = FALSE;
			ScanInfo->bImprovedScan = FALSE;
#ifdef DOT11_N_SUPPORT
			adhocInfo->bAdhocN = TRUE;
#endif /* DOT11_N_SUPPORT */
			pStaCfg->bFastConnect = FALSE;
			adhocInfo->bAdhocCreator = FALSE;
			pStaCfg->MlmeAux.OldChannel = 0;
#ifdef IP_ASSEMBLY
			pStaCfg->bFragFlag = TRUE;
#endif /* IP_ASSEMBLY */
		}
	}
#endif

#ifdef SMART_CARRIER_SENSE_SUPPORT
	/* SCS Variable initialization */
	pAd->SCSCtrl.SCSEnable = SCS_DISABLE;
	pAd->SCSCtrl.SCSTrafficThreshold = TriggerTrafficeTh; /* 2M */
	pAd->SCSCtrl.SCSStatus = 0;
	pAd->SCSCtrl.OneSecTxByteCount = 0;
	pAd->SCSCtrl.OneSecRxByteCount = 0;
	pAd->SCSCtrl.CckPdBlkTh = PdBlkCckThDefault;
	pAd->SCSCtrl.OfdmPdBlkTh = PdBlkOfmdThDefault;
	pAd->SCSCtrl.SCSThTolerance = ThTolerance;
	pAd->SCSCtrl.SCSMinRssiTolerance = MinRssiTolerance;
	pAd->SCSCtrl.OfdmPdSupport = TRUE;
	pAd->SCSCtrl.CckFalseCcaUpBond = FalseCcaUpBondDefault;
	pAd->SCSCtrl.CckFalseCcaLowBond = FalseCcaLowBondDefault;
	pAd->SCSCtrl.OfdmFalseCcaUpBond = FalseCcaUpBondDefault;
	pAd->SCSCtrl.OfdmFalseCcaLowBond = FalseCcaLowBondDefault;
	pAd->SCSCtrl.CckFixedRssiBond = CckFixedRssiBondDefault;
	pAd->SCSCtrl.OfdmFixedRssiBond = OfdmFixedRssiBondDefault;
	/* SCSGen6 */
	pAd->SCSCtrl.LastETput = 0;
	pAd->SCSCtrl.SCSEnable = SCS_ENABLE;
#endif /* SMART_CARRIER_SENSE_SUPPORT */

	pAd->CCI_ACI_TxOP_Value = 0;

	pAd->g_mode_txop_wdev = NULL;
	pAd->G_MODE_INFRA_TXOP_RUNNING = FALSE;
	pAd->MUMIMO_TxOP_Value = 0;
	pAd->partial_mib_show_en = 0;

	pAd->txop_ctl.multi_client_nums = 0;
	pAd->txop_ctl.multi_tcp_nums = 0;
	pAd->txop_ctl.cur_wdev = NULL;
	pAd->txop_ctl.multi_cli_txop_running = 0x00;
	pAd->txop_ctl.near_far_txop_running = FALSE;
#ifdef PEAK_ENHANCE
	pAd->txop_ctl.mlo_entry = FALSE;
#endif /* PEAK_ENHANCE */

#ifdef PKT_BUDGET_CTRL_SUPPORT
	pAd->pbc_bound[PBC_AC_BE] = PBC_WMM_UP_DEFAULT_BE;
	pAd->pbc_bound[PBC_AC_BK] = PBC_WMM_UP_DEFAULT_BK;
	pAd->pbc_bound[PBC_AC_VO] = PBC_WMM_UP_DEFAULT_VO;
	pAd->pbc_bound[PBC_AC_VI] = PBC_WMM_UP_DEFAULT_VI;
	pAd->pbc_bound[PBC_AC_MGMT] = PBC_WMM_UP_DEFAULT_MGMT;
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
#ifdef TX_AGG_ADJUST_WKR
	pAd->TxAggAdjsut = TRUE;
#endif /* TX_AGG_ADJUST_WKR */
#ifdef HTC_DECRYPT_IOT
	pAd->HTC_ICV_Err_TH = 15;
#endif /* HTC_DECRYPT_IOT */
#ifdef DHCP_UC_SUPPORT
	pAd->DhcpUcEnable = FALSE;
#endif /* DHCP_UC_SUPPORT */

	/*Initial EDCCA Enable/Mode*/
	pAd->CommonCfg.u1EDCCACtrl = TRUE; /* EDCCA default is ON. */
	pAd->CommonCfg.u1EDCCAMode = FALSE; /* EDCCAMode default is OFF. */
	pAd->CommonCfg.u1EDCCACfgMode = FALSE; /* EDCCAMode default OFF. */
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		" edcca band %d set to true\n", hc_get_hw_band_idx(pAd));

	/*Initial EDCCA threshold value*/
	{
		UINT8 u1bw_idx;
		for (u1bw_idx = 0; u1bw_idx < EDCCA_MAX_BW_NUM ; u1bw_idx++)
			pAd->CommonCfg.u1EDCCAThreshold[u1bw_idx] = 0x7f;
	}

#ifdef DSCP_PRI_SUPPORT
	{
		UINT8 bss_idx;
		/*fill default dscp value, overwrite by profile param or iwpriv command*/
		for (bss_idx = 0; bss_idx < MAX_BEACON_NUM; bss_idx++) {
			for (i = 0; i < 64; i++)
				pAd->ApCfg.MBSSID[bss_idx].dscp_pri_map[i] = i >> 3;
		}
	}
#endif /*DSCP_PRI_SUPPORT*/
#ifdef AIR_MONITOR
	pAd->MntRuleBitMap = DEFAULT_MNTR_RULE;
#endif /* AIR_MONITOR */
#if defined(MBO_SUPPORT) || defined(DOT11_SAE_SUPPORT)
	pAd->reg_domain = REG_GLOBAL;
#endif /* MBO_SUPPORT */
#ifdef WAPP_SUPPORT
	pAd->bss_load_info.high_thrd = MAX_BSSLOAD_THRD;
#endif /* WAPP_SUPPORT */
#ifdef FW_LOG_DUMP
	for (i = 0; i < BIN_DBG_LOG_NUM; i++)
		FwLogCtrl->debug_level_ctrl[i] = 0;
	for (i = 0; i < MAC_ADDR_LEN; i++)
		FwLogCtrl->fw_log_server_mac[i] = 0xFF;
	memset(FwLogCtrl->fw_log_arg, 0, 10);


	FwLogCtrl->fw_log_server_ip = 0xFFFFFFFF;
	FwLogCtrl->fw_log_serialID_count = 0x00;
	ret_val = snprintf(FwLogCtrl->fw_log_dest_dir, 32, "%s", DEFAULT_FW_LOG_DESTINATION);

	if (os_snprintf_error(32, ret_val)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"FW_LOG_DUMP snprintf error!\n");
	}
#endif /* FW_LOG_DUMP */
#ifdef RATE_PRIOR_SUPPORT
	DlListInit(&pAd->LowRateCtrl.BlackList);
	NdisAllocateSpinLock(pAd, &pAd->LowRateCtrl.BlackListLock);
#endif/*RATE_PRIOR_SUPPORT*/

#ifdef CFG_SUPPORT_FALCON_MURU
	pAd->CommonCfg.bShowMuEdcaParam = FALSE;
#endif
	/* fw core dump is dumped */
	pAd->bIsBeenDumped = FALSE;

	/* fwcmd timeout dump */
	pAd->FwCmdTimeoutCnt = 0;
	pAd->FwCmdTimeoutPrintCnt = FW_CMD_TO_PRINT_CNT;
	NdisZeroMemory(pAd->FwCmdTimeoutRecord, sizeof(pAd->FwCmdTimeoutRecord));
#ifdef PER_PKT_CTRL_FOR_CTMR
	pAd->PerPktCtrlEnable = FALSE;
#endif

#ifdef ANTENNA_DIVERSITY_SUPPORT
	band_idx = hc_get_hw_band_idx(pAd);
	ant_diversity_ctrl_init(pAd, band_idx);
	ant_diversity_ctrl_reset(pAd);
#endif

#ifdef DABS_QOS
	NdisAllocateSpinLock(pAd, &qos_param_table_lock);
	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	memset(&qos_param_table[0], 0, sizeof(struct qos_param_rec)*MAX_QOS_PARAM_TBL);
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
#endif

	pAd->CommonCfg.need_fallback = 0;

#ifdef IXIA_C50_MODE
	pAd->ixia_ctl.chkTmr = 2; /*check ixia mode per 2 sec*/
	pAd->ixia_ctl.pktthld = 50;
	pAd->ixia_ctl.DeltaRssiTh = 10; /*initial as 10dBm*/
	pAd->ixia_ctl.MinRssiTh = -65;
	pAd->ixia_ctl.BA_timeout = (100 * OS_HZ)/1000; /*flash one time out*/
	pAd->ixia_ctl.max_BA_timeout = (1500 * OS_HZ)/1000; /*flash all time out*/
#endif

#ifdef DELAY_TCP_ACK_V2 /*for panther*/
	if (IS_MT7986(pAd))
		mt_cmd_wo_query(pAd, WO_CMD_RXCNT_CTRL, 0x1, PEAK_TP_WO_REPORT_TIME); /*only need do one time, 6*150ms report*/
#endif /* DELAY_TCP_ACK_V2 */

#ifdef DOT11_EHT_BE
#ifdef CFG_SUPPORT_FALCON_PP
	pp_black_list_reset(pAd);
#endif
#endif
	pAd->CommonCfg.BcnDataRate = 0xFFFF;
#ifdef SW_CONNECT_SUPPORT
	NdisZeroMemory(&pAd->dummy_obj, sizeof(pAd->dummy_obj));
#endif /* SW_CONNECT_SUPPORT */
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "<--\n");
}


/* IRQL = PASSIVE_LEVEL*/
UCHAR BtoH(RTMP_STRING ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';        /* Handle numerals*/

	if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 0xA;  /* Handle capitol hex digits*/

	if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 0xA;  /* Handle small hex digits*/

	return 255;
}


/*
	FUNCTION: AtoH(char *, UCHAR *, int)

	PURPOSE:  Converts ascii string to network order hex

	PARAMETERS:
		src    - pointer to input ascii string
		dest   - pointer to output hex
		destlen - size of dest

	COMMENTS:

		2 ascii bytes make a hex byte so must put 1st ascii byte of pair
		into upper nibble and 2nd ascii byte of pair into lower nibble.

	IRQL = PASSIVE_LEVEL
*/
void AtoH(RTMP_STRING *src, PUCHAR dest, int destlen)
{
	RTMP_STRING *srcptr;
	PUCHAR destTemp;

	srcptr = src;
	destTemp = (PUCHAR) dest;

	while (destlen--) {
		*destTemp = (UCHAR)(BtoH(*srcptr++) << 4); /* Put 1st ascii byte in upper nibble.*/
		*destTemp += BtoH(*srcptr++);      /* Add 2nd ascii byte to above.*/
		destTemp++;
	}
}

/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pTimer				Timer structure
		Value				Timer value in milliseconds

	Return Value:
		None

	Note:
		To use this routine, must call RTMPInitTimer before.

	========================================================================
*/
VOID RTMPSetTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value)
{
	if (!pTimer || !pTimer->timer_lock)
		return;

	OS_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid) {
		RTMP_ADAPTER *pAd;

		pAd = (RTMP_ADAPTER *)pTimer->pAd;

		if (pAd &&
			RTMP_TEST_FLAG(pAd,
				fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)) {
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
				"Failed, Halt in Progress!\n");
			OS_SEM_UNLOCK(pTimer->timer_lock);
			return;
		}

		pTimer->TimerValue = Value;
		pTimer->State      = FALSE;

		if (pTimer->PeriodicType == TRUE) {
			pTimer->Repeat = TRUE;
			RTMP_SetPeriodicTimer(&pTimer->TimerObj, Value);
		} else {
			pTimer->Repeat = FALSE;
			RTMP_OS_Add_Timer(&pTimer->TimerObj, Value);
		}

		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG, "%lx\n", (ULONG)pTimer);
	} else
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
			"Timer hasn't been initialize! <caller: %pS>\n",
			__builtin_return_address(0));

	OS_SEM_UNLOCK(pTimer->timer_lock);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pTimer				Timer structure
		Value				Timer value in milliseconds

	Return Value:
		None

	Note:
		To use this routine, must call RTMPInitTimer before.

	========================================================================
*/
VOID RTMPModTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value)
{
	BOOLEAN	Cancel;

	if (!pTimer || !pTimer->timer_lock)
		return;

	OS_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid) {
		pTimer->TimerValue = Value;
		pTimer->State      = FALSE;

		if (pTimer->PeriodicType == TRUE) {
			OS_SEM_UNLOCK(pTimer->timer_lock);
			RTMPCancelTimer(pTimer, &Cancel);
			RTMPSetTimer(pTimer, Value);
		} else {
			RTMP_OS_Mod_Timer(&pTimer->TimerObj, Value);
			OS_SEM_UNLOCK(pTimer->timer_lock);
		}

		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG, "%lx\n", (ULONG)pTimer);
	} else {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
			"Timer hasn't been initialize!\n");
		OS_SEM_UNLOCK(pTimer->timer_lock);
	}
}


/*
	========================================================================

	Routine Description:
		Cancel timer objects

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:
		1.) To use this routine, must call RTMPInitTimer before.
		2.) Reset NIC to initial state AS IS system boot up time.

	========================================================================
*/
VOID RTMPCancelTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled)
{
#ifdef RTMP_TIMER_TASK_SUPPORT
	struct _RTMP_ADAPTER *mac_ad = NULL;
#endif /* RTMP_TIMER_TASK_SUPPORT */

	if (!pTimer || !pTimer->timer_lock)
		goto end;

	OS_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid) {
		if (pTimer->State == FALSE)
			pTimer->Repeat = FALSE;

		OS_SEM_UNLOCK(pTimer->timer_lock);
		RTMP_OS_Del_Timer(&pTimer->TimerObj, pCancelled);
		OS_SEM_LOCK(pTimer->timer_lock);

		if (*pCancelled == TRUE)
			pTimer->State = TRUE;

#ifdef RTMP_TIMER_TASK_SUPPORT
		mac_ad = (struct _RTMP_ADAPTER *)pTimer->pAd;
		if (mac_ad
			&& IS_ASIC_CAP(mac_ad, fASIC_CAP_MGMT_TIMER_TASK)) {
			/* We need to go-through the TimerQ to findout this timer handler and remove it if */
			/*		it's still waiting for execution.*/
			RtmpTimerQRemove(mac_ad, pTimer);
		}
#endif /* RTMP_TIMER_TASK_SUPPORT */
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG, "%lx\n", (ULONG)pTimer);
	}

	OS_SEM_UNLOCK(pTimer->timer_lock);
	return;
end:
	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG,
		"Failed, Timer hasn't been initialize!\n");
}

VOID RTMPReleaseTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled)
{
	struct physical_device *ph_dev = NULL;

	if (!pTimer || !pTimer->timer_lock)
		goto end;

	OS_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid) {
		if (pTimer->State == FALSE)
			pTimer->Repeat = FALSE;

		OS_SEM_UNLOCK(pTimer->timer_lock);
		RTMP_OS_Del_Timer(&pTimer->TimerObj, pCancelled);
		OS_SEM_LOCK(pTimer->timer_lock);

		if (*pCancelled == TRUE)
			pTimer->State = TRUE;

		/* release timer */
		pTimer->Valid = FALSE;
		ph_dev = (struct physical_device *)pTimer->ph_dev;
		physical_device_timer_list_release(ph_dev, pTimer);
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG, "%lx\n", (ULONG)pTimer);
	}

	OS_SEM_UNLOCK(pTimer->timer_lock);
end:
	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_DEBUG,
		"Timer hasn't been initialize!\n");
}


/*
	========================================================================

	Routine Description:
		Enable RX

	Arguments:
		pAd						Pointer to our adapter

	Return Value:
		None

	IRQL <= DISPATCH_LEVEL

	Note:
		Before Enable RX, make sure you have enabled Interrupt.
	========================================================================
*/
VOID RTMPEnableRxTx(RTMP_ADAPTER *pAd)
{
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "==>\n");

	if (pAd->CommonCfg.bTXRX_RXV_ON)
		AsicSetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, TRUE);
	else
		AsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "<==\n");
}


void CfgInitHook(RTMP_ADAPTER *pAd)
{
	/*pAd->bBroadComHT = TRUE;*/
}

PNET_DEV get_netdev_from_bssid(RTMP_ADAPTER *pAd, UCHAR wdev_idx)
{
	PNET_DEV dev_p = NULL;

	if ((wdev_idx < WDEV_NUM_MAX) && (pAd->wdev_list[wdev_idx] != NULL))
		dev_p = pAd->wdev_list[wdev_idx]->if_dev;

	if (dev_p == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"dev_p=NULL,caller:%pS\n", OS_TRACE);
	}

	return dev_p;
}


INT RtmpRaDevCtrlInit(VOID *pAdSrc, RTMP_INF_TYPE infType)
{
	RTMP_ADAPTER *pAd = (PRTMP_ADAPTER)pAdSrc;
#ifdef FW_DUMP_SUPPORT
	pAd->fw_dump_max_size = MAX_FW_DUMP_SIZE;
	RTMP_OS_FWDUMP_PROCINIT(pAd);
#endif
	/* Assign the interface type. We need use it when do register/EEPROM access.*/
	pAd->infType = infType;
#ifdef CONFIG_STA_SUPPORT
	pAd->OpMode = OPMODE_STA;
	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_INFO,
		"STA Driver version-%s\n", STA_DRIVER_VERSION);
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	pAd->OpMode = OPMODE_AP;
	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_INFO,
		"AP Driver version-%s\n", AP_DRIVER_VERSION);
#endif /* CONFIG_AP_SUPPORT */
	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_INFO,
		"pAd->infType=%d\n", pAd->infType);
	pAd->iface_combinations = 0;
#ifdef CONFIG_STA_SUPPORT
	pAd->iface_combinations |= HAVE_STA_INF;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	pAd->iface_combinations |= HAVE_AP_INF;
#endif /* CONFIG_AP_SUPPORT */

	RTMP_SEM_EVENT_INIT(&(pAd->AutoRateLock), &pAd->RscSemMemList);

	pAd->IoctlHandleFlag = FALSE;

#ifdef WIFI_MD_COEX_SUPPORT
	NdisAllocateSpinLock(pAd, &pAd->LteSafeChCtrl.SafeChDbLock);
#endif

	/*prepeare hw resource depend on chipcap*/
	hdev_resource_init(pAd->hdev_ctrl);

	/*initial wlan hook module*/
	WLAN_HOOK_INIT();

#ifdef CONFIG_CSO_SUPPORT

	if (IS_ASIC_CAP(pAd, fASIC_CAP_CSO))
		RTMP_SET_MORE_FLAG(pAd, fASIC_CAP_CSO);

#endif /* CONFIG_CSO_SUPPORT */
#ifdef MCS_LUT_SUPPORT

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCS_LUT)) {
		if (hc_get_chip_wtbl_max_num(pAd) < 128)
			RTMP_SET_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT);
		else {
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_WARN,
				"MCS_LUT not used because MacTb size(%d) > 128!\n",
				hc_get_chip_wtbl_max_num(pAd));
		}
	}

#endif /* MCS_LUT_SUPPORT */
	pAd->NopListBk = NULL;
	pAd->MaxMSTANum = 1;
	return 0;
}


BOOLEAN RtmpRaDevCtrlExit(IN VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	INT index;
	RTMP_SEM_EVENT_DESTORY(&(pAd->AutoRateLock));

#ifdef WIFI_MD_COEX_SUPPORT
	NdisFreeSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
#endif

	pAd->IoctlHandleFlag = FALSE;

	/*
		Free ProbeRespIE Table
	*/
	for (index = 0; index < MAX_LEN_OF_BSS_TABLE; index++) {
		if (pAd->ProbeRespIE[index].pIe)
			os_free_mem(pAd->ProbeRespIE[index].pIe);
	}

#ifdef FW_DUMP_SUPPORT
	RTMP_OS_FWDUMP_PROCREMOVE(pAd);

	if (pAd->fw_dump_buffer) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO, "Free FW dump buffer\n");
		os_free_mem(pAd->fw_dump_buffer);
		pAd->fw_dump_buffer = 0;
		pAd->fw_dump_size = 0;
		pAd->fw_dump_read = 0;
	}

#endif
	if (pAd->NopListBk) {
		os_free_mem(pAd->NopListBk);
	}

	wpf_config_exit(pAd);
	RTMPFreeAdapter(pAd);
	return TRUE;
}


#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID RTMP_11N_D3_TimerInit(RTMP_ADAPTER *pAd)
{
	RTMPInitTimer(pAd, &pAd->CommonCfg.Bss2040CoexistTimer, GET_TIMER_FUNCTION(Bss2040CoexistTimeOut), pAd, FALSE);
}

VOID RTMP_11N_D3_TimerRelease(RTMP_ADAPTER *pAd)
{
	BOOLEAN Cancel;

	RTMPReleaseTimer(&pAd->CommonCfg.Bss2040CoexistTimer, &Cancel);
}

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CFG_SUPPORT_CSI

VOID csi_support_init(RTMP_ADAPTER *pAd)
{
	/* init CSI wait queue */
	init_waitqueue_head(&pAd->rCSIInfo.waitq);
	/* inti lock */
	NdisAllocateSpinLock(pAd, &pAd->rCSIInfo.CSIBufferLock);
	NdisAllocateSpinLock(pAd, &pAd->rCSIInfo.CSIStaListLock);
	/*init csi sta list*/
	DlListInit(&pAd->rCSIInfo.CSIStaList);
	/* init proc fs*/
	csi_proc_init(pAd);
	/* init csi timer for nlmsg report*/
	RTMPInitTimer(pAd, &pAd->rCSIInfo.csi_report_timer, GET_TIMER_FUNCTION(csi_report_timer_handle), pAd, TRUE);
	/*init csi total buffer*/
	os_zero_mem(pAd->rCSIInfo.arCSIBuffer, CSI_RING_SIZE * sizeof(struct CSI_DATA_T));
}

VOID csi_support_deinit(RTMP_ADAPTER *pAd)
{
	PCSI_STA pCSISta = NULL, tmp = NULL;
	struct CSI_INFO_T *prCSIInfo;
	BOOLEAN Cancelled;

	prCSIInfo = &pAd->rCSIInfo;

	/* release csi timer*/
	RTMPCancelTimer(&prCSIInfo->csi_report_timer, &Cancelled);
	RTMPReleaseTimer(&prCSIInfo->csi_report_timer, &Cancelled);

	/*clear the list*/
	NdisAcquireSpinLock(&prCSIInfo->CSIStaListLock);
		DlListForEach(pCSISta, &prCSIInfo->CSIStaList, CSI_STA, List) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_INFO,
					"Remove csi sta, "MACSTR"\n", MAC2STR(pCSISta->Addr));
			tmp = pCSISta;
			pCSISta = DlListEntry(pCSISta->List.Prev, CSI_STA, List);
			DlListDel(&(tmp->List));
			os_free_mem(tmp);
	}
	NdisReleaseSpinLock(&prCSIInfo->CSIStaListLock);

	csi_clean_ring_buffer(pAd);

	/* deinti lock */
	NdisFreeSpinLock(&pAd->rCSIInfo.CSIBufferLock);
	NdisFreeSpinLock(&pAd->rCSIInfo.CSIStaListLock);
	/* deinit proc fs*/
	csi_proc_deinit(pAd);
}
#endif

