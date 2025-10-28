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
	rt_led.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#include	"rt_config.h"

#ifdef LED_CONTROL_SUPPORT

#if defined(MT7990)
static LED_INIT_TABLE mt7990_led_init_table[LED_MAX_NUM] = {
	/*DEFAULT*/
	{LED_WLAN_2G_init, LED_BAND_2G, LED_IDX_0, GPIO(20), HW_LED, FALSE},
	{LED_WLAN_5G_init, LED_BAND_5G, LED_IDX_1, GPIO(21), HW_LED, FALSE},
	{LED_WLAN_6G_init, LED_BAND_6G, LED_IDX_2, GPIO(22), HW_LED, FALSE}
};
#endif /* defined(MT7990) */

#if defined(MT7992)
static LED_INIT_TABLE mt7992_led_init_table[LED_MAX_NUM] = {
	/*DEFAULT*/
	{LED_WLAN_2G_init, LED_BAND_2G, LED_IDX_0, GPIO(6), HW_LED, FALSE},
	{LED_WLAN_5G_init, LED_BAND_5G, LED_IDX_1, GPIO(7), HW_LED, FALSE}
};
#endif /* defined(MT7992) */

#if defined(MT7993)
static LED_INIT_TABLE mt7993_led_init_table[LED_MAX_NUM] = {
	/*DEFAULT*/
	{LED_WLAN_2G_init, LED_BAND_2G, LED_IDX_0, GPIO(6), HW_LED, FALSE},
	{LED_WLAN_5G_init, LED_BAND_5G_6G, LED_IDX_1, GPIO(7), HW_LED, FALSE}
};
#endif /* defined(MT7993) */


INT LED_Array[16][16] = {
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ LED_SOLID_OFF,   LED_TX_BLINKING,    LED_SOLID_OFF,    LED_TX_BLINKING,   LED_SOLID_OFF,  -1,  LED_SOLID_ON, -1,   LED_BLINKING_170MS_ON_170MS_OFF, -1, -1, LED_WPS_5S_ON_3S_OFF_THEN_BLINKING, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{  3,  2,   -1,   -1,   -1, -1, 16,  1,  5, -1, -1, 17, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{  1,  2,    1,   -1,   -1, -1,  3, -1,  6, -1, -1,  0, -1, -1, -1, -1},
	{  1,  2,    1,   -1,   -1, -1, -1,  1,  4, -1, -1, 18, -1, -1, -1, -1}
};

/*
	========================================================================

	Routine Description:
		Set LED Status

	Arguments:
		pAd						Pointer to our adapter
		Status					LED Status

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPSetLEDStatus(RTMP_ADAPTER *pAd, UCHAR Status, UCHAR BandIdx)
{
	/*ULONG	data; */
	UCHAR LinkStatus = 0;
	UCHAR LedMode;
	BOOLEAN	bIgnored = FALSE;
	INT	LED_CMD = -1;
	struct wifi_dev *wdev = NULL;
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
	PWSC_CTRL pWscControl = NULL;
#ifdef CONFIG_AP_SUPPORT
	pWscControl = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.WscControl;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	pWscControl = &pAd->StaCfg[0].wdev.WscControl;
#endif /* CONFIG_STA_SUPPORT */
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef CONFIG_ATE

	/*
		In ATE mode of RT2860 AP/STA, we have erased 8051 firmware.
		So LED mode is not supported when ATE is running.
	*/
	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */
	LedMode = LED_MODE(pAd);

	if (IS_MT7915(pAd) ||
		IS_MT7990(pAd) ||
		IS_MT7992(pAd) ||
		IS_MT7993(pAd)) {
		LedMode = 1;
#if defined(WSC_INCLUDED) && defined(WSC_LED_SUPPORT)

		if (Status > LED_WPS_SUCCESS)
#else
		if (Status > LED_POWER_UP)
#endif /* defined(WSC_INCLUDED) && defined(WSC_LED_SUPPORT) */
			return;
		else
			LED_CMD = LED_Array[LedMode][Status];
	}

	/* #endif MT76x0 */
	switch (Status) {
	case LED_LINK_DOWN:
		LinkStatus = LINK_STATUS_LINK_DOWN;
		pAd->LedCntl.LedIndicatorStrength = 0;
		break;

	case LED_LINK_UP:
		wdev = get_default_wdev(pAd);

		if (!wdev) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"wdev is NULL!\n");
			return;
		}

		if (WMODE_CAP_6G(wdev->PhyMode))
			LinkStatus = LINK_STATUS_6G_BAND_LINK_UP;
		else if (WMODE_CAP_5G(wdev->PhyMode))
			LinkStatus = LINK_STATUS_5G_BAND_LINK_UP;
		else
			LinkStatus = LINK_STATUS_2G_BAND_LINK_UP;

		break;

	case LED_RADIO_ON:
		LinkStatus = LINK_STATUS_RADIO_ON;
		break;

	case LED_HALT:
		LedMode = 0; /* Driver sets MAC register and MAC controls LED */
		fallthrough;

	case LED_RADIO_OFF:
		LinkStatus = LINK_STATUS_RADIO_OFF;
		break;

	case LED_WPS:
		LinkStatus = LINK_STATUS_WPS;
		break;

	case LED_ON_SITE_SURVEY:
		LinkStatus = LINK_STATUS_ON_SITE_SURVEY;
		break;

	case LED_POWER_UP:
		LinkStatus = LINK_STATUS_POWER_UP;
		break;
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT

	case LED_WPS_IN_PROCESS:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_WPS_IN_PROCESS;
			pWscControl->WscLEDMode = LED_WPS_IN_PROCESS;
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
				"LED_WPS_IN_PROCESS\n");
		} else {
			bIgnored = TRUE;
		}

		break;

	case LED_WPS_ERROR:
		if (WscSupportWPSLEDMode(pAd)) {
			/* In the case of LED mode 9, the error LED should be turned on only after WPS walk time expiration. */
			if ((pWscControl->bWPSWalkTimeExpiration == FALSE) &&
				(LED_MODE(pAd) == WPS_LED_MODE_9))
				/* do nothing. */
			else
				LinkStatus = LINK_STATUS_WPS_ERROR;

			pWscControl->WscLEDMode = LED_WPS_ERROR;
			pWscControl->WscLastWarningLEDMode = LED_WPS_ERROR;
		} else {
			bIgnored = TRUE;
		}

		break;

	case LED_WPS_SESSION_OVERLAP_DETECTED:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_WPS_SESSION_OVERLAP_DETECTED;
			pWscControl->WscLEDMode = LED_WPS_SESSION_OVERLAP_DETECTED;
			pWscControl->WscLastWarningLEDMode = LED_WPS_SESSION_OVERLAP_DETECTED;
		} else {
			bIgnored = TRUE;
		}

		break;

	case LED_WPS_SUCCESS:
		if (WscSupportWPSLEDMode(pAd)) {
			if ((LED_MODE(pAd) == WPS_LED_MODE_7) ||
				(LED_MODE(pAd) == WPS_LED_MODE_11) ||
				(LED_MODE(pAd) == WPS_LED_MODE_12)
			   ) {
				/* In the WPS LED mode 7, 11 and 12, the blue LED would last 300 seconds regardless of the AP's security settings. */
				LinkStatus = LINK_STATUS_WPS_SUCCESS_WITH_SECURITY;
				pWscControl->WscLEDMode = LED_WPS_SUCCESS;
				/* Turn off the WPS successful LED pattern after 300 seconds. */
				RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
			} else if (LED_MODE(pAd) == WPS_LED_MODE_8) { /* The WPS LED mode 8 */
				if (WscAPHasSecuritySetting(pAd, pWscControl)) { /* The WPS AP has the security setting. */
					LinkStatus = LINK_STATUS_WPS_SUCCESS_WITH_SECURITY;
					pWscControl->WscLEDMode = LED_WPS_SUCCESS;
					/* Turn off the WPS successful LED pattern after 300 seconds. */
					RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
				} else { /* The WPS AP does not have the secuirty setting. */
					LinkStatus = LINK_STATUS_WPS_SUCCESS_WITHOUT_SECURITY;
					pWscControl->WscLEDMode = LED_WPS_SUCCESS;
					/* Turn off the WPS successful LED pattern after 300 seconds. */
					RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
				}
			} else if (LED_MODE(pAd) == WPS_LED_MODE_9) { /* The WPS LED mode 9. */
				/* Always turn on the WPS blue LED for 300 seconds. */
				LinkStatus = LINK_STATUS_WPS_BLUE_LED;
				pWscControl->WscLEDMode = LED_WPS_SUCCESS;
				/* Turn off the WPS successful LED pattern after 300 seconds. */
				RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
			} else {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
					"LED_WPS_SUCCESS (Incorrect LED mode = %d)\n", LED_MODE(pAd));
				ASSERT(FALSE);
			}
		} else
			bIgnored = TRUE;

		break;

	case LED_WPS_TURN_LED_OFF:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_WPS_TURN_LED_OFF;
			pWscControl->WscLEDMode = LED_WPS_TURN_LED_OFF;
		} else {
			bIgnored = TRUE;
		}

		break;

	case LED_WPS_TURN_ON_BLUE_LED:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_WPS_BLUE_LED;
			pWscControl->WscLEDMode = LED_WPS_SUCCESS;
		} else {
			bIgnored = TRUE;
		}

		break;

	case LED_NORMAL_CONNECTION_WITHOUT_SECURITY:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_NORMAL_CONNECTION_WITHOUT_SECURITY;
			pWscControl->WscLEDMode = LED_WPS_SUCCESS;
		} else {
			bIgnored = TRUE;
		}

		break;

	case LED_NORMAL_CONNECTION_WITH_SECURITY:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_NORMAL_CONNECTION_WITH_SECURITY;
			pWscControl->WscLEDMode = LED_WPS_SUCCESS;
		} else {
			bIgnored = TRUE;
		}

		break;

	/*WPS LED Mode 10 */
	case LED_WPS_MODE10_TURN_ON:
		if (WscSupportWPSLEDMode10(pAd))
			LinkStatus = LINK_STATUS_WPS_MODE10_TURN_ON;
		else
			bIgnored = TRUE;

		break;

	case LED_WPS_MODE10_FLASH:
		if (WscSupportWPSLEDMode10(pAd))
			LinkStatus = LINK_STATUS_WPS_MODE10_FLASH;
		else
			bIgnored = TRUE;

		break;

	case LED_WPS_MODE10_TURN_OFF:
		if (WscSupportWPSLEDMode10(pAd))
			LinkStatus = LINK_STATUS_WPS_MODE10_TURN_OFF;
		else
			bIgnored = TRUE;

		break;
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */

	default:
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
			"RTMPSetLED::Unknown Status 0x%x\n", Status);
		break;
	}

	if (IS_MT7915(pAd) ||
		IS_MT7990(pAd) ||
		IS_MT7992(pAd) ||
		IS_MT7993(pAd)) {
		UCHAR led_idx = 0;

		if (IS_MT7915(pAd)) {
			led_idx = BandIdx;
		} else {
			if (pAd->LedCntl.Led_Init_Ops)
				led_idx = pAd->LedCntl.Led_Init_Ops->led_idx;
		}

		if ((Status == LED_RADIO_OFF) || (Status == LED_LINK_DOWN)) {
			UCHAR i = 0;
			UCHAR wdev_same_band_cnt = 0;
			struct wifi_dev *wdev_temp = NULL;

			for (i = 0; i < WDEV_NUM_MAX; i++) {
				wdev_temp = pAd->wdev_list[i];
				if (wdev_temp && wdev_temp->if_up_down_state) {
					if (HcGetBandByWdev(wdev_temp) == BandIdx) {
						wdev_same_band_cnt++;
						break;
					}
				}
			}

			if ((Status == LED_RADIO_OFF) && (wdev_same_band_cnt == 0)) { /* Interface down case */
				AndesLedEnhanceOP(pAd, led_idx, LED_TX_OVER_BLINK_DISABLE, LED_REVERSE_POLARITY_DISABLE,
					BandIdx, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
			} else if ((Status == LED_LINK_DOWN) && (wdev_same_band_cnt == 1)) { /* Radio On/Off case */
				AndesLedEnhanceOP(pAd, led_idx, LED_TX_OVER_BLINK_DISABLE, LED_REVERSE_POLARITY_DISABLE,
						BandIdx, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
			}
		} else {
			if (IS_MT7990(pAd) ||
				IS_MT7992(pAd) ||
				IS_MT7993(pAd)) {
				if ((Status == LED_LINK_UP) || (Status == LED_RADIO_ON))
					AndesLedEnhanceOP(pAd, led_idx, LED_TX_OVER_BLINK_DISABLE, LED_REVERSE_POLARITY_DISABLE,
										BandIdx, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
			} else {
				AndesLedEnhanceOP(pAd, led_idx, LED_TX_OVER_BLINK_DISABLE, LED_REVERSE_POLARITY_DISABLE,
								BandIdx, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
			}
		}
	}

	/*
		Keep LED status for LED SiteSurvey mode.
		After SiteSurvey, we will set the LED mode to previous status.
	*/
	if ((Status != LED_ON_SITE_SURVEY) && (Status != LED_POWER_UP) && (bIgnored == FALSE))
		pAd->LedCntl.LedStatus = Status;
}


/*
	========================================================================

	Routine Description:
		Set LED Signal Stregth

	Arguments:
		pAd						Pointer to our adapter
		Dbm						Signal Stregth

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		Can be run on any IRQL level.

		According to Microsoft Zero Config Wireless Signal Stregth definition as belows.
		<= -90  No Signal
		<= -81  Very Low
		<= -71  Low
		<= -67  Good
		<= -57  Very Good
		 > -57  Excellent
	========================================================================
*/
VOID RTMPSetSignalLED(RTMP_ADAPTER *pAd, NDIS_802_11_RSSI Dbm)
{
	UCHAR		nLed = 0;

	if (pAd->LedCntl.MCULedCntl.field.LedMode == LED_MODE_SIGNAL_STREGTH) {
		if (Dbm <= -90)
			nLed = 0;
		else if (Dbm <= -81)
			nLed = 1;
		else if (Dbm <= -71)
			nLed = 3;
		else if (Dbm <= -67)
			nLed = 7;
		else if (Dbm <= -57)
			nLed = 15;
		else
			nLed = 31;

		/* */
		/* Update Signal Stregth to firmware if changed. */
		/* */
		if (pAd->LedCntl.LedIndicatorStrength != nLed) {
			AsicSendCommandToMcu(pAd, MCU_SET_LED_GPIO_SIGNAL_CFG,
							0xff, nLed, pAd->LedCntl.MCULedCntl.field.Polarity, FALSE);
			pAd->LedCntl.LedIndicatorStrength = nLed;
		}
	}
}


#ifdef WSC_STA_SUPPORT
#ifdef WSC_LED_SUPPORT
/*
	LED indication for normal connection start.
*/
VOID LEDConnectionStart(RTMP_ADAPTER *pAd)
{
	/* LED indication. */
	/*if (pAd->StaCfg[0].WscControl.bWPSSession == FALSE) */
	/*if (pAd->StaCfg[0].WscControl.WscConfMode != WSC_DISABLE && pAd->StaCfg[0].WscControl.bWscTrigger) */
	if (pAd->StaCfg[0].wdev.WscControl.WscConfMode == WSC_DISABLE) {
		if (LED_MODE(pAd) == WPS_LED_MODE_9) { /* LED mode 9. */
			UCHAR WPSLEDStatus = 0;

			/* The AP uses OPEN-NONE. */
			if ((IS_AKM_OPEN(pAd->StaCfg[0].wdev.SecConfig.AKMMap))
				&& (IS_CIPHER_NONE(pAd->StaCfg[0].wdev.SecConfig.PairwiseCipher)))
				WPSLEDStatus = LED_WPS_TURN_LED_OFF;
			else /* The AP uses an encryption algorithm. */
				WPSLEDStatus = LED_WPS_IN_PROCESS;

			RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(&pAd->StaCfg[0].wdev));
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%d\n", WPSLEDStatus);
		}
	}
}


/*
	LED indication for normal connection completion.
*/
VOID LEDConnectionCompletion(RTMP_ADAPTER *pAd, BOOLEAN bSuccess)
{
	/* LED indication. */
	/*if (pAd->StaCfg[0].WscControl.bWPSSession == FALSE) */
	if (pAd->StaCfg[0].wdev.WscControl.WscConfMode == WSC_DISABLE) {
		if (LED_MODE(pAd) == WPS_LED_MODE_9) { /* LED mode 9. */
			UCHAR WPSLEDStatus = 0;

			if (bSuccess == TRUE) { /* Successful connenction. */
				/* The AP uses OPEN-NONE. */
				if ((IS_AKM_OPEN(pAd->StaCfg[0].wdev.SecConfig.AKMMap))
					&& (IS_CIPHER_NONE(pAd->StaCfg[0].wdev.SecConfig.PairwiseCipher)))
					WPSLEDStatus = LED_NORMAL_CONNECTION_WITHOUT_SECURITY;
				else /* The AP uses an encryption algorithm. */
					WPSLEDStatus = LED_NORMAL_CONNECTION_WITH_SECURITY;
			} else /* Connection failure. */
				WPSLEDStatus = LED_WPS_TURN_LED_OFF;

			RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(&pAd->StaCfg[0].wdev));
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%d\n", WPSLEDStatus);
		}
	}
}
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_STA_SUPPORT */


void rtmp_read_led_setting_from_eeprom(RTMP_ADAPTER *pAd)
{
	USHORT Value = 0;
	PLED_CONTROL pLedCntl = &pAd->LedCntl;
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, Value);
		pLedCntl->MCULedCntl.word = (Value >> 8);
		RT28xx_EEPROM_READ16(pAd, EEPROM_LEDAG_CONF_OFFSET, Value);
		pLedCntl->LedAGCfg = Value;
		RT28xx_EEPROM_READ16(pAd, EEPROM_LEDACT_CONF_OFFSET, Value);
		pLedCntl->LedACTCfg = Value;
		RT28xx_EEPROM_READ16(pAd, EEPROM_LED_POLARITY_OFFSET, Value);
		pLedCntl->LedPolarity = Value;
	}
}


void RTMPStartLEDMode(IN RTMP_ADAPTER *pAd)
{
}


void RTMPInitLEDMode(IN RTMP_ADAPTER *pAd)
{
	UINT8 i;
	PLED_CONTROL pLedCntl = &pAd->LedCntl;
	PLED_INIT_TABLE pled_table = NULL, cur_pled_table = NULL;
	UINT8 cur_band = hc_get_hw_band_idx(pAd);
	UINT8 led_table_count = 0;

	if (cur_band == INVALID_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"hc_get_hw_band_idx(pAd) is wrong!\n");
		return;
	}
	pAd->LedCntl.Led_Init_Ops = NULL;
	pAd->LedCntl.Led_WPS_Init_Ops = NULL;


#if defined(MT7990)
	if (IS_MT7990(pAd)) {
		pled_table = &mt7990_led_init_table[0];
		led_table_count = ARRAY_SIZE(mt7990_led_init_table);
	}
#endif /* defined(MT7990) */

#if defined(MT7992)
	if (IS_MT7992(pAd)) {
		pled_table = &mt7992_led_init_table[0];
		led_table_count = ARRAY_SIZE(mt7992_led_init_table);
	}
#endif /* defined(MT7992) */

#if defined(MT7993)
		if (IS_MT7993(pAd)) {
			pled_table = &mt7993_led_init_table[0];
			led_table_count = ARRAY_SIZE(mt7993_led_init_table);
		}
#endif /* defined(MT7993) */


	if (pled_table == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"pled_table is NULL!\n");
		return;
	}

	/* Init WPS LED Ops */
	for (i = 0; i < led_table_count; i++) {
		if (pled_table[i].is_wps_led) {
			if (((pled_table[i].band_idx != LED_BAND_NONE) &&
				(cur_band == pled_table[i].band_idx)) ||
				(pled_table[i].band_idx == LED_BAND_NONE)) {
				pAd->LedCntl.Led_WPS_Init_Ops = &pled_table[i];
				break;
			}
		}
	}

	/* Init Normal LED Ops */
	for (i = 0; i < led_table_count; i++) {
		if ((pled_table[i].band_idx != LED_BAND_NONE) &&
			(cur_band == pled_table[i].band_idx)) {
			pAd->LedCntl.Led_Init_Ops = &pled_table[i];
			cur_pled_table = &pled_table[i];
			break;
		}
	}

	if (cur_pled_table == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"cur_pled_table is NULL!\n");
		return;
	}

	if (pLedCntl->MCULedCntl.word == 0xFF) {
		pLedCntl->MCULedCntl.word = 0x01;
		pLedCntl->LedAGCfg = 0x5555;
		pLedCntl->LedACTCfg = 0x2221;
#ifdef RTMP_MAC_PCI
		pLedCntl->LedPolarity = 0xA9F8;
#endif /* RTMP_MAC_PCI */
	}
	AsicSendCommandToMcu(pAd, MCU_SET_LED_AG_CFG, 0xff, (UCHAR)pLedCntl->LedAGCfg, (UCHAR)(pLedCntl->LedAGCfg >> 8), FALSE);
	AsicSendCommandToMcu(pAd, MCU_SET_LED_ACT_CFG, 0xff, (UCHAR)pLedCntl->LedACTCfg, (UCHAR)(pLedCntl->LedACTCfg >> 8), FALSE);
	AsicSendCommandToMcu(pAd, MCU_SET_LED_POLARITY, 0xff, (UCHAR)pLedCntl->LedPolarity, (UCHAR)(pLedCntl->LedPolarity >> 8), FALSE);
	AsicSendCommandToMcu(pAd, MCU_SET_LED_GPIO_SIGNAL_CFG, 0xff, 0, pLedCntl->MCULedCntl.field.Polarity, FALSE);
	pAd->LedCntl.LedIndicatorStrength = 0xFF;
	RTMPSetSignalLED(pAd, -100);	/* Force signal strength Led to be turned off, before link up */
	RTMPStartLEDMode(pAd);

#if defined(MT7915) || defined(MT7990) || defined(MT7992) || defined(MT7993)
	/*led init setting*/
	if (cur_pled_table->gpio_inti_func)
		cur_pled_table->gpio_inti_func(pAd, cur_pled_table->led_idx);

	for (i = 0; i < led_table_count; i++) {
		if (pled_table[i].is_wps_led &&
			pled_table[i].gpio_inti_func)
			pled_table[i].gpio_inti_func(pAd, pled_table[i].led_idx);
	}
#endif /* defined(MT7915) || defined(MT7990) || defined(MT7992) || defined(MT7993) */
}


inline void RTMPExitLEDMode(IN RTMP_ADAPTER *pAd)
{
	UCHAR BandIdx = hc_get_hw_band_idx(pAd);

	if (BandIdx == INVALID_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"hc_get_hw_band_idx(pAd) is wrong!\n");
		return;
	}

	RTMPSetLED(pAd, LED_RADIO_OFF, BandIdx);

#ifdef CONFIG_CPE_SUPPORT
	wps_led_control(pAd, 1);/*turn OFF wps led  HIGH ACTIVE*/
#endif
	return;
}

void wps_led_control(struct _RTMP_ADAPTER *pAd, UCHAR flag)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->wps_led_control)
		ops->wps_led_control(pAd, flag);
	else
		AsicNotSupportFunc(pAd, __func__);
}

INT RTMPSetLED(RTMP_ADAPTER *pAd, UCHAR Status, UCHAR BandIdx)
{
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)

	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd) || IS_HWIFI_INF(pAd)) {
#ifdef LED_CONTROL_SUPPORT
		RTMPSetLEDStatus(pAd, Status, BandIdx);
#endif /* LED_CONTROL_SUPPORT */
	}

#endif /* defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT) */
	return TRUE;
}

#if defined(MT7915) || defined(MT7990) || defined(MT7992) || defined(MT7993)
INT rtmp_control_led_cmd(
	struct _RTMP_ADAPTER *pAd,
	UCHAR led_idx,
	UCHAR tx_over_blink,
	UCHAR reverse_polarity,
	UCHAR band,
	UCHAR blink_mode,
	UCHAR off_time,
	UCHAR on_time,
	UCHAR led_control_mode)
{
	INT ret = 0;
	BOOLEAN led_drv_ctrl = FALSE;
	BOOLEAN Cancelled;
	BOOLEAN is_wps_led = FALSE;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		"led_idx=%d,tx_over_blink=%d,reverse_polarity=%d,band=%d,blink_mode=%d,off_time=%d,on_time=%d,led_control_mode=%d\n",
		led_idx,
		tx_over_blink,
		reverse_polarity,
		band,
		blink_mode,
		off_time,
		on_time,
		led_control_mode);

	if (pAd->LedCntl.Led_WPS_Init_Ops &&
		(pAd->LedCntl.Led_WPS_Init_Ops->led_idx == led_idx))
		is_wps_led = TRUE;

	/*
		Only these mode need timer, so they are handled here.
		Other modes are still handled in AndesLedEnhanceOP
	*/
	switch (led_control_mode) {
	/* WPS control by normal led idx */
	case LED_WPS_3_BLINKING_PER_SECOND_FOR_4_SECONDS:
		ret = AndesLedEnhanceOP(pAd, led_idx, tx_over_blink, reverse_polarity, band,
				blink_mode, off_time, on_time, LED_BLINKING_170MS_ON_170MS_OFF);
		pAd->LedCntl.LEDActionType = LED_MODE16_ACTION_1;
		pAd->LedCntl.LEDIndex = led_idx;
		pAd->LedCntl.LEDBand = band;
		if (pAd->LedCntl.LEDControlTimerRunning)
			RTMPCancelTimer(&pAd->LedCntl.LEDControlTimer, &Cancelled);
		RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 4000);
		pAd->LedCntl.LEDControlTimerRunning = TRUE;
		led_drv_ctrl = TRUE;
		break;

	/* WPS control by normal led idx */
	case LED_WPS_5S_ON_3S_OFF_THEN_BLINKING:
		ret = AndesLedEnhanceOP(pAd, led_idx, tx_over_blink, reverse_polarity, band,
				blink_mode, off_time, on_time, LED_SOLID_ON);
		pAd->LedCntl.LEDActionType = LED_MODE17_ACTION_1;
		pAd->LedCntl.LEDIndex = led_idx;
		pAd->LedCntl.LEDBand = band;
		if (pAd->LedCntl.LEDControlTimerRunning)
			RTMPCancelTimer(&pAd->LedCntl.LEDControlTimer, &Cancelled);
		RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 5000);
		pAd->LedCntl.LEDControlTimerRunning = TRUE;
		led_drv_ctrl = TRUE;
		break;

	/* WPS control by normal led idx */
	case LED_WPS_5S_ON:
		ret = AndesLedEnhanceOP(pAd, led_idx, tx_over_blink, reverse_polarity, band,
				blink_mode, off_time, on_time, LED_SOLID_ON);
		pAd->LedCntl.LEDActionType = LED_MODE18_ACTION_1;
		pAd->LedCntl.LEDIndex = led_idx;
		pAd->LedCntl.LEDBand = band;
		if (pAd->LedCntl.LEDControlTimerRunning)
			RTMPCancelTimer(&pAd->LedCntl.LEDControlTimer, &Cancelled);
		RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 5000);
		pAd->LedCntl.LEDControlTimerRunning = TRUE;
		led_drv_ctrl = TRUE;
		break;

	/* WPS control by specified wps led flow */
	case LED_SOLID_ON:
		if (is_wps_led) {
			if (pAd->LedCntl.LEDControlTimerRunning == TRUE) {
				RTMPCancelTimer(&pAd->LedCntl.LEDControlTimer, &Cancelled);
				pAd->LedCntl.LEDControlTimerRunning = FALSE;
			}
			if (reverse_polarity)
				wps_led_control(pAd, WPS_LED_OFF);
			else
				wps_led_control(pAd, WPS_LED_ON);
			led_drv_ctrl = TRUE;
		}
		break;

	/* WPS control by specified wps led flow */
	case LED_SOLID_OFF:
		if (is_wps_led) {
			if (pAd->LedCntl.LEDControlTimerRunning) {
				RTMPCancelTimer(&pAd->LedCntl.LEDControlTimer, &Cancelled);
				pAd->LedCntl.LEDControlTimerRunning = FALSE;
			}
			if (reverse_polarity)
				wps_led_control(pAd, WPS_LED_ON);
			else
				wps_led_control(pAd, WPS_LED_OFF);
			led_drv_ctrl = TRUE;
		}
		break;

	/* WPS control by specified wps led flow */
	case LED_BLINKING_500MS_ON_500MS_OFF:
		if (is_wps_led) {
			wps_led_control(pAd, WPS_LED_ON);
			pAd->LedCntl.LEDIndex = led_idx;
			pAd->LedCntl.LEDActionType = LED_BLINK_ACTION_500;
			if (pAd->LedCntl.LEDControlTimerRunning)
				RTMPCancelTimer(&pAd->LedCntl.LEDControlTimer, &Cancelled);
			RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 500);
			pAd->LedCntl.LEDControlTimerRunning = TRUE;
			led_drv_ctrl = TRUE;
		}
		break;

	/* WPS control by specified wps led flow */
	case LED_BLINKING_1000MS_ON_1000MS_OFF:
		if (is_wps_led) {
			wps_led_control(pAd, WPS_LED_ON);
			pAd->LedCntl.LEDIndex = led_idx;
			pAd->LedCntl.LEDActionType = LED_BLINK_ACTION_1000;
			if (pAd->LedCntl.LEDControlTimerRunning)
				RTMPCancelTimer(&pAd->LedCntl.LEDControlTimer, &Cancelled);
			RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 1000);
			pAd->LedCntl.LEDControlTimerRunning = TRUE;
			led_drv_ctrl = TRUE;
		}
		break;

	/* WPS control by specified wps led flow */
	case LED_BLINKING_2000MS_ON_2000MS_OFF:
		if (is_wps_led) {
			wps_led_control(pAd, WPS_LED_ON);
			pAd->LedCntl.LEDIndex = led_idx;
			pAd->LedCntl.LEDActionType = LED_BLINK_ACTION_2000;
			if (pAd->LedCntl.LEDControlTimerRunning)
				RTMPCancelTimer(&pAd->LedCntl.LEDControlTimer, &Cancelled);
			RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 2000);
			pAd->LedCntl.LEDControlTimerRunning = TRUE;
			led_drv_ctrl = TRUE;
		}
		break;

	default:
		break;
	}

	if (!led_drv_ctrl)
		ret = AndesLedEnhanceOP(pAd, led_idx, tx_over_blink, reverse_polarity, band,
						blink_mode, off_time, on_time, led_control_mode);
	return ret;
}

VOID LEDControlTimer(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	pAd->LedCntl.LEDControlTimerRunning = FALSE;

	switch (pAd->LedCntl.LEDActionType) { /* Last Control LED state. */
	/*Turn off LED*/
	case LED_MODE16_ACTION_1:
	case LED_MODE18_ACTION_1:
		AndesLedEnhanceOP(pAd, pAd->LedCntl.LEDIndex, 0, 0,
						pAd->LedCntl.LEDBand, 0, 0, 0, LED_SOLID_OFF);
		pAd->LedCntl.LEDActionType = LED_CONTROL_SUCCESS;
		break;

	/* Turn off LED for next 3 secs */
	case LED_MODE17_ACTION_1:
		AndesLedEnhanceOP(pAd, pAd->LedCntl.LEDIndex, 0, 0,
							pAd->LedCntl.LEDBand, 0, 0, 0, LED_SOLID_OFF);
		pAd->LedCntl.LEDActionType = LED_MODE17_ACTION_2;
		RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 3000);
		pAd->LedCntl.LEDControlTimerRunning = TRUE;
		break;

	/* Change to TX Blink mode */
	case LED_MODE17_ACTION_2:
		AndesLedEnhanceOP(pAd, pAd->LedCntl.LEDIndex, 0, 0,
						pAd->LedCntl.LEDBand, LED_TX_DATA_ONLY, 0, 0, LED_TX_BLINKING);
		pAd->LedCntl.LEDActionType = LED_CONTROL_SUCCESS;
		break;

	case LED_BLINK_ACTION_500:
		wps_led_control(pAd, WPS_LED_REVERSE);
		RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 500);
		pAd->LedCntl.LEDControlTimerRunning = TRUE;
		break;

	case LED_BLINK_ACTION_1000:
		wps_led_control(pAd, WPS_LED_REVERSE);
		RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 1000);
		pAd->LedCntl.LEDControlTimerRunning = TRUE;
		break;

	case LED_BLINK_ACTION_2000:
		wps_led_control(pAd, WPS_LED_REVERSE);
		RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 2000);
		pAd->LedCntl.LEDControlTimerRunning = TRUE;
		break;

	default:
		/* do nothing. */
		break;
	}
}

void LED_WLAN_2G_init(RTMP_ADAPTER *pAd, UINT8 led_index)
{
	PLED_INIT_TABLE pled_table = pAd->LedCntl.Led_Init_Ops;

	if (pled_table)
		RTMP_LED_GPIO_MAP(pAd, led_index, pled_table->map_idx, pled_table->control_type);
	/*for others init, TBD*/
}

void LED_WLAN_5G_init(RTMP_ADAPTER *pAd, UINT8 led_index)
{
	PLED_INIT_TABLE pled_table = pAd->LedCntl.Led_Init_Ops;

	if (pled_table)
		RTMP_LED_GPIO_MAP(pAd, led_index, pled_table->map_idx, pled_table->control_type);
	/*for others init, TBD*/
}

void LED_WLAN_6G_init(RTMP_ADAPTER *pAd, UINT8 led_index)
{
	PLED_INIT_TABLE pled_table = pAd->LedCntl.Led_Init_Ops;

	if (pled_table)
		RTMP_LED_GPIO_MAP(pAd, led_index, pled_table->map_idx, pled_table->control_type);
	/*for others init, TBD*/
}

void LED_WLAN_WPS_init(RTMP_ADAPTER *pAd, UINT8 led_index)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->wps_led_init)
		ops->wps_led_init(pAd);
	else
		AsicNotSupportFunc(pAd, __func__);
}
#endif /* defined(MT7915) || defined(MT7990) || defined(MT7992) || defined(MT7993) */

#endif /* LED_CONTROL_SUPPORT */
