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
	hw_init.c
*/

#include "rt_config.h"
#include "hdev/hdev.h"

/*HW related init*/
static INT32 WfEPROMHwInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	NICInitAsicFromEEPROM(pAd);
	return ret;
}


/*Common Part for externl*/
static INT mac_init(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "-->\n");

	/* re-set specific MAC registers for individual chip */
	if (ops->AsicMacInit != NULL)
		ops->AsicMacInit(pAd);

	/* auto-fall back settings */
	AsicAutoFallbackInit(pAd);
	AsicSetMacMaxLen(pAd);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "<--\n");
	return TRUE;
}

INT32 WfMacInit(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	UCHAR idx = UNI_CMD_FR_TABLE_CCK_11M_IDX;
#endif
#ifdef CONFIG_3_WIRE_SUPPORT
	struct physical_device *ph_dev = pAd->physical_dev;
	UINT8 threeWire_function_enable = 0;
#endif

#ifdef DMA_SCH_SUPPORT
	AsicDMASchedulerInit(pAd, DMA_SCH_LMAC);
#endif
	mac_init(pAd);
	asic_init_wtbl(pAd, TRUE);
#ifdef HDR_TRANS_RX_SUPPORT
	AsicRxHeaderTransCtl(pAd, TRUE, FALSE, FALSE, TRUE, FALSE);
	AsicRxHeaderTaranBLCtl(pAd, 0, TRUE, ETH_TYPE_EAPOL);
	AsicRxHeaderTaranBLCtl(pAd, 1, TRUE, ETH_TYPE_WAI);
	AsicRxHeaderTaranBLCtl(pAd, 2, TRUE, ETH_TYPE_FASTROAMING);
#endif

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
		AsicAutoBATrigger(pAd, TRUE, BA_TRIGGER_OFFLOAD_TIMEOUT);

	if (hwifi_get_inf_num(pAd) == 0) {
#ifdef WIFI_UNIFIED_COMMAND
		uni_cmd_set_nic_cap_query(pAd); /* only read nic ap once */
		do {
			uni_cmd_fr_table_add(pAd, idx);
			idx = idx + 2;
		} while (idx < UNI_CMD_FR_TABLE_MAX_IDX);
#endif /* WIFI_UNIFIED_COMMAND */
		chip_do_extra_action(pAd, NULL, NULL,
			CHIP_EXTRA_ACTION_PROFILE_CORRECT, NULL, NULL);
		chip_set_rro_timeout(pAd);
		chip_set_rro_rss_map(pAd);
#ifdef CONFIG_3_WIRE_SUPPORT
#ifdef WIFI_UNIFIED_COMMAND
		if (ph_dev->threeWire_function_enable) {
			threeWire_function_enable = ph_dev->threeWire_function_enable;
			UniCmd3wireFunctionSet(pAd, threeWire_function_enable);
		}
#endif /* WIFI_UNIFIED_COMMAND */
#endif /* CONFIG_3_WIRE_SUPPORT */
	}
#ifdef WIFI_UNIFIED_COMMAND
	uni_cmd_fr_table_add_for_duptx(pAd);
	if (!pAd->BarRetryLimitCnt)
		UniCmdSetBarRetry(pAd, 1);
	else
		UniCmdSetBarRetry(pAd, pAd->BarRetryLimitCnt);
#endif /* WIFI_UNIFIED_COMMAND */
	ret = chip_init_hw_ring_setting(pAd);
	return ret;
}

INT32 WfEPROMInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	ret = WfEPROMSysInit(pAd, true, pAd->CommonCfg.dbdc_mode);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err;

	WfEPROMHwInit(pAd);
	return ret;
err:
	WfEPROMSysExit(pAd);
	return ret;
}

INT32 WfPhyInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	NICInitBBP(pAd);
	return ret;
}

/*SW related init*/

INT32 WfSysPreInit(RTMP_ADAPTER *pAd)
{


#ifdef MT7990

	if (IS_MT7990(pAd))
		mt7990_init(pAd);

#endif

#ifdef MT7992

	if (IS_MT7992(pAd))
		mt7992_init(pAd);

#endif

#ifdef MT7993
	
	if (IS_MT7993(pAd))
		mt7993_init(pAd);
	
#endif

	wifi_sup_list_register(pAd, WIFI_CAP_CHIP);
	wifi_sup_list_register(pAd, WIFI_CAP_SEC);
	wifi_sup_list_register(pAd, WIFI_CAP_FEATURE);

	return 0;
}

INT32 WfSysCfgInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	return ret;
}


INT32 WfSysCfgExit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	return ret;
}
