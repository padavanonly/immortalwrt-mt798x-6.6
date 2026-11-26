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
 ***************************************************************************

	Module Name:
	phystate.c
*/

/*******************************************************************************
 *    INCLUDED COMMON FILES
 ******************************************************************************/

#include "rt_config.h"

/*******************************************************************************
 *    INCLUDED EXTERNAL FILES
 ******************************************************************************/


/*******************************************************************************
 *    INCLUDED INTERNAL FILES
 ******************************************************************************/


/*******************************************************************************
 *   PRIVATE DEFINITIONS
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE TYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *    PRIVATE DATA
 ******************************************************************************/


/*******************************************************************************
 *    PUBLIC DATA
 ******************************************************************************/


/*******************************************************************************
 *    EXTERNAL DATA
 ******************************************************************************/


/*******************************************************************************
 *    EXTERNAL FUNCTION PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/
INT PhyStatGetRssi(
	RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	CHAR *rssi,
	UINT8 *len
	)
{
	RX_STATISTIC_RXV *rx_stat = NULL;
	UINT8 path_idx = 0;
	UINT16 i = 0;
	PMAC_TABLE_ENTRY pEntry = NULL, pCurrEntry = NULL;
	CHAR rssi_tmp[MAX_RSSI_LEN] = {0};
	UINT16 wcid = 0;
	UINT32 ent_type = ENTRY_CLIENT;
	BOOLEAN fgTestMode = FALSE;

	if (!rssi)
		goto error0;

	if (!len)
		goto error0;

	if (band_idx >= CFG_WIFI_RAM_BAND_NUM)
		goto error1;

	rx_stat = &pAd->rx_stat_rxv;

#ifdef CONFIG_WLAN_SERVICE
	if (ATE_ON(pAd))
		fgTestMode = TRUE;
	else
		fgTestMode = FALSE;
#else
	fgTestMode = FALSE;
#endif /* CONFIG_WLAN_SERVICE */

	if (fgTestMode) {
		*len = MAX_RSSI_LEN;
		for (path_idx = 0; path_idx < *len; path_idx++)
			*(rssi + path_idx) = rx_stat->RSSI[path_idx];

		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"rssi: %d, %d, %d, %d.\n",
			rx_stat->RSSI[0], rx_stat->RSSI[1], rx_stat->RSSI[2], rx_stat->RSSI[3]);
	} else {
		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pEntry = entry_get(pAd, i);
			/* dump MacTable entries which match the EntryType */
			if (pEntry->EntryType != ent_type)
				continue;

			if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
					&& (pEntry->Sst != SST_ASSOC))
				continue;

			if (band_idx != HcGetBandByWdev(pEntry->wdev))
				continue;

			pCurrEntry = pEntry;
			break;
		}

		*len = MAX_RSSI_LEN;
		if (!pCurrEntry)
			goto error2;

		wcid = pCurrEntry->wcid;
		chip_get_rssi(pAd, wcid, rssi_tmp);

		for (i = 0; i < *len; i++)
			*(rssi + i) = rssi_tmp[i];

		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"wcid: %d, rssi: %d, %d, %d, %d, %d.\n",
			pCurrEntry->wcid, rssi_tmp[0], rssi_tmp[1], rssi_tmp[2], rssi_tmp[3], rssi_tmp[4]);
	}

	return 0;

error0:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
		"null pointer for content buffer.\n");
	return 1;

error1:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
		"invalid band index(%d).\n", band_idx);
	return 1;

error2:
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"invalid entry. no station link up.\n");
	return 1;
}

INT PhyStatGetCnInfo(
	RTMP_ADAPTER *pAd,
	UINT8 ucband_idx,
	UINT16 *pCnInfo
	)
{
	if (ucband_idx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;

	chip_get_cninfo(pAd, ucband_idx, pCnInfo);
	return TRUE;
}

INT ShowTxPhyRate(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	BOOLEAN fgset
)
{
	INT status = FALSE;
	UINT8 ucBandIdx;
	struct  wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	   apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;
	if (wdev == NULL)
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	if (wdev == NULL)
		wdev = &pAd->StaCfg[0].wdev;
#endif /* CONFIG_STA_SUPPORT */

	if (wdev)
		ucBandIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;

	MTWF_PRINT("(PHY STATE INFO)\n");

	/* READ */
	if (fgset == FALSE) {
		if (MtCmdPhyShowInfo(pAd, CMD_PHY_STATE_TX_PHYRATE, ucBandIdx) == RETURN_STATUS_TRUE)
			status = TRUE;
	}

	return status;
}

INT ShowLastRxPhyRate(
	RTMP_ADAPTER *pAd,
	UINT8 ucBandIdx,
	UINT16 u2Wcid,
	UINT32 *rx_rate
)
{
	INT status = FALSE;
	EXT_EVENT_PHY_STATE_RX_RATE rRxRateInfo;

	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "(PHY STATE INFO)\n");
	os_zero_mem(&rRxRateInfo, sizeof(rRxRateInfo));

	/* READ */
	if (MtCmdPhyGetRxRate(pAd, CMD_PHY_STATE_CONTENTION_RX_PHYRATE, ucBandIdx, u2Wcid, (UINT32 *)&rRxRateInfo) == RETURN_STATUS_TRUE)
		status = TRUE;

	*rx_rate = 0;

	/* Mode [19:16]     GI [15:14]     Rate [13:8]     BW [7:5]     STBC [4]     Coding [3]     Nsts [2:0]  */
	*rx_rate = ((rRxRateInfo.u1RxMode & 0xF) << 16) | ((rRxRateInfo.u1Gi & 0x3) << 14) |
				((rRxRateInfo.u1RxRate & 0x3F) << 8) | ((rRxRateInfo.u1BW & 0x7) << 5) |
				((rRxRateInfo.u1Stbc & 0x1) << 4) | ((rRxRateInfo.u1Coding & 0x1) << 3) |
				(rRxRateInfo.u1RxNsts & 0x7);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "contention-based:\n");
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "Wcid:%u Rate::%u Mode:%u Nsts:%u\n",
			u2Wcid, rRxRateInfo.u1RxRate, rRxRateInfo.u1RxMode, rRxRateInfo.u1RxNsts);

	return status;
}

INT ShowRxPhyRate(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	BOOLEAN fgset
)
{
	INT status = FALSE;
	UINT8 ucBandIdx;
	struct	wifi_dev *wdev = NULL;
	UINT8 rx_rate = 0, rx_nsts = 0, rx_mode = 0, buf = 0;
	CHAR str[6][20] = {"CCK", "OFDM", "HT Mix-Mode", "HT Green-Field", "VHT", "HE"};
	CHAR rx_mode_str[20] = {'\0'};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	   apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;
	if (wdev == NULL)
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	if (wdev == NULL)
		wdev = &pAd->StaCfg[0].wdev;
#endif /* CONFIG_STA_SUPPORT */

	if (wdev)
		ucBandIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;

	MTWF_PRINT("(PHY STATE INFO)\n");

	/* READ */
	if (fgset == FALSE) {
		if (MtCmdPhyShowInfo(pAd, CMD_PHY_STATE_RX_PHYRATE, ucBandIdx) == RETURN_STATUS_TRUE)
			status = TRUE;
	}

	/* contention-based rx rate parsing */
	buf = (pAd->phy_stat_elem.rx_raw & BITS(0, 6)) >> 0;
	rx_nsts = (pAd->phy_stat_elem.rx_raw & BITS(7, 9)) >> 7;
	rx_mode = (pAd->phy_stat_elem.rx_raw2 & BITS(4, 7)) >> 4;

	/* contention-based rx nsts handle */
	rx_nsts += 1;

	switch (rx_mode) {
	case 0:
		os_move_mem(rx_mode_str, *(str + 0), 20);
		rx_rate = buf & BITS(0, 1);
		break;
	case 1:
		os_move_mem(rx_mode_str, *(str + 1), 20);
		if ((buf & BITS(0, 3)) == 0xb)
			rx_rate = 0;
		else if ((buf & BITS(0, 3)) == 0xf)
			rx_rate = 1;
		else if ((buf & BITS(0, 3)) == 0xa)
			rx_rate = 2;
		else if ((buf & BITS(0, 3)) == 0xe)
			rx_rate = 3;
		else if ((buf & BITS(0, 3)) == 0x9)
			rx_rate = 4;
		else if ((buf & BITS(0, 3)) == 0xd)
			rx_rate = 5;
		else if ((buf & BITS(0, 3)) == 0x8)
			rx_rate = 6;
		else if ((buf & BITS(0, 3)) == 0xc)
			rx_rate = 7;
		break;
	case 2:
		os_move_mem(rx_mode_str, *(str + 2), 20);
		rx_rate = (buf & BITS(0, 6));
		break;
	case 3:
		os_move_mem(rx_mode_str, *(str + 3), 20);
		rx_rate = (buf & BITS(0, 6));
		break;
	case 4:
		os_move_mem(rx_mode_str, *(str + 4), 20);
		rx_rate = (buf & BITS(0, 3));
		break;
	case 8:
		os_move_mem(rx_mode_str, *(str + 5), 20);
		rx_rate = (buf & BITS(0, 3));
		break;
	case 9:
		os_move_mem(rx_mode_str, *(str + 5), 20);
		rx_rate = (buf & BITS(0, 3));
		break;
	case 10:
		os_move_mem(rx_mode_str, *(str + 5), 20);
		rx_rate = (buf & BITS(0, 3));
		break;
	case 11:
		os_move_mem(rx_mode_str, *(str + 5), 20);
		rx_rate = (buf & BITS(0, 3));
		break;
	default:
		break;
	}

	MTWF_PRINT("contention-based:\n");
	MTWF_PRINT("%s::rx rate: %s(%dNss M%d)\n", __func__, rx_mode_str, rx_nsts, rx_rate);

	return status;
}

INT32 MtCmdPhyGetRxRate(
	RTMP_ADAPTER *pAd,
	UCHAR ucPhyStateInfoCatg,
	UINT8 ucBandIdx,
	UINT16 u2Wcid,
	UINT32 *prRxRateInfo
)
{
	INT32 ret = 0;
#ifdef WIFI_UNIFIED_COMMAND
	UniCmdPerStaGetRxRate(
		pAd, UNI_CMD_PER_STA_CONTENTION_RX_PHYRATE, ucBandIdx,
		u2Wcid, prRxRateInfo);
#else
	struct cmd_msg *msg;
	CMD_PHY_STATE_SHOW_INFO_T PhyStateShowInfoCtrl;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PHY_STATE_SHOW_INFO_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&PhyStateShowInfoCtrl, sizeof(CMD_PHY_STATE_SHOW_INFO_T));
	PhyStateShowInfoCtrl.ucPhyStateInfoCatg = ucPhyStateInfoCatg;
	PhyStateShowInfoCtrl.ucBandIdx = ucBandIdx;
	PhyStateShowInfoCtrl.u2Wcid = u2Wcid;
#ifdef CFG_BIG_ENDIAN
	PhyStateShowInfoCtrl.u2Wcid = cpu2le16(PhyStateShowInfoCtrl.u2Wcid);
#endif
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PHY_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(*prRxRateInfo));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, prRxRateInfo);
	SET_CMD_ATTR_RSP_HANDLER(attr, phyStateEventDispatcher);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&PhyStateShowInfoCtrl,
						sizeof(CMD_PHY_STATE_SHOW_INFO_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_PHY, DBG_LVL_INFO,
			"(ret = %d)\n", ret);
#endif
	return ret;
}

INT32 MtCmdPhyGetMutliRxRate(
	RTMP_ADAPTER *pAd,
	PHY_STATE_RX_RATE_PAIR *RxRatePair,
	UINT32 Num
)
{
	INT32 ret = 0;
#ifdef WIFI_UNIFIED_COMMAND
	UINT32 index;
	UINT16 *pu2Wcid;

	os_alloc_mem_suspend(pAd, (UCHAR **)&pu2Wcid, sizeof(*pu2Wcid) * Num);

	if (!pu2Wcid)
		return NDIS_STATUS_RESOURCES;

	os_zero_mem(pu2Wcid, sizeof(*pu2Wcid) * Num);
	for (index = 0; index < Num; index++)
		pu2Wcid[index] = RxRatePair[index].u2Wcid;

	UniCmdMultiStaGetRxRate(pAd, UNI_CMD_PER_STA_CONTENTION_RX_PHYRATE, pu2Wcid, (UINT32 *)RxRatePair, Num);

	if (pu2Wcid)
		os_free_mem(pu2Wcid);
#else
	struct cmd_msg *msg;
	CMD_PHY_STATE_SHOW_INFO_T PhyStateShowInfoCtrl;
	UINT32 index;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PHY_STATE_SHOW_INFO_T) * Num);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PHY_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_PHY_STATE_RX_RATE) * Num);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, RxRatePair);
	SET_CMD_ATTR_RSP_HANDLER(attr, phyMutliStateEventDispatcher);
	AndesInitCmdMsg(msg, attr);

	for (index = 0; index < Num; index++) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_PHY, DBG_LVL_INFO,
			"ucPhyStateInfoCatg=0x%8x, ucBandIdx=%d u2Wcid=%d\n",
			RxRatePair[index].ucPhyStateInfoCatg, RxRatePair[index].ucBandIdx, RxRatePair[index].u2Wcid);
		os_zero_mem(&PhyStateShowInfoCtrl, sizeof(CMD_PHY_STATE_SHOW_INFO_T));
		PhyStateShowInfoCtrl.ucPhyStateInfoCatg = RxRatePair[index].ucPhyStateInfoCatg;
		PhyStateShowInfoCtrl.ucBandIdx = RxRatePair[index].ucBandIdx;
		PhyStateShowInfoCtrl.u2Wcid = RxRatePair[index].u2Wcid;
#ifdef CFG_BIG_ENDIAN
		PhyStateShowInfoCtrl.u2Wcid = cpu2le16(PhyStateShowInfoCtrl.u2Wcid);
#endif
		AndesAppendCmdMsg(msg, (char *)&PhyStateShowInfoCtrl,
							sizeof(CMD_PHY_STATE_SHOW_INFO_T));
	}
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_PHY, DBG_LVL_INFO,
			"(ret = %d)\n", ret);
#endif
	return ret;
}


/* Send command to firmware */
INT32 MtCmdPhyShowInfo(
	RTMP_ADAPTER *pAd,
	UCHAR ucPhyStateInfoCatg,
	UINT8 ucBandIdx
)
{
	INT32 ret = 0;
#ifdef WIFI_UNIFIED_COMMAND
	UINT16 u2StateCmdTag;

	if (ucPhyStateInfoCatg == CMD_PHY_STATE_TX_PHYRATE)
		u2StateCmdTag = UNI_CMD_STATE_TX_RATE;
	else if (ucPhyStateInfoCatg == CMD_PHY_STATE_RX_PHYRATE)
		u2StateCmdTag = UNI_CMD_STATE_RX_RATE;
	else
		return NDIS_STATUS_INVALID_DATA;

	UniCmdStateGetTxRxPhyRate(
		pAd, u2StateCmdTag, ucBandIdx);
#else
	struct cmd_msg *msg;
	CMD_PHY_STATE_SHOW_INFO_T PhyStateShowInfoCtrl;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_PHY, DBG_LVL_INFO,
			"ucPhyStateInfoCatg: %d, BandIdx: %d \n", ucPhyStateInfoCatg, ucBandIdx);
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PHY_STATE_SHOW_INFO_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&PhyStateShowInfoCtrl, sizeof(CMD_PHY_STATE_SHOW_INFO_T));
	PhyStateShowInfoCtrl.ucPhyStateInfoCatg = ucPhyStateInfoCatg;
	PhyStateShowInfoCtrl.ucBandIdx = ucBandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PHY_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&PhyStateShowInfoCtrl,
						sizeof(CMD_PHY_STATE_SHOW_INFO_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_PHY, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
#endif
	return ret;
}

VOID EventPhyStatHandler(PRTMP_ADAPTER pAd, UINT8 *Data, UINT_32 Length)
{
	/* Event ID */
	UINT8 u1PhyStateEventId;

	/* Get Event Category ID */
	u1PhyStateEventId = *Data;

	/**Prevent legitimate but wrong ID **/
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_PHY, DBG_LVL_DEBUG,
		 "u1PhyStateEventId = %d\n", u1PhyStateEventId);

	/* Event Handle for different Category ID */
	switch (u1PhyStateEventId) {
	case EVENT_PHY_STATE_TX_PHYRATE:
		EventPhyStatTxRate(pAd, Data, Length);
		break;

	case EVENT_PHY_STATE_RX_PHYRATE:
		EventPhyStatRxRate(pAd, Data, Length);
		break;

	case EVENT_PHY_STATE_PER:
		EventPhyStatPER(pAd, Data, Length);
		break;

	default:
		break;
	}
}

VOID EventPhyStatTxRate(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_PHY_STATE_TX_RATE prEventPhyStatTxRate = NULL;
	UINT8 tx_rate = 0, tx_nsts = 0, tx_mode = 0;
	CHAR str[6][20] = {"CCK", "OFDM", "HT Mix-Mode", "HT Green-Field", "VHT", "HE"};
	CHAR tx_mode_str[20];

	prEventPhyStatTxRate = (P_EXT_EVENT_PHY_STATE_TX_RATE)Data;
	tx_rate = prEventPhyStatTxRate->u1TxRate;
	tx_mode = prEventPhyStatTxRate->u1TxMode;
	tx_nsts = prEventPhyStatTxRate->u1TxNsts;
	os_move_mem(tx_mode_str, *(str + tx_mode), 20);

	MTWF_PRINT("%s: tx rate: %s(%dNss M%d)\n", __func__, tx_mode_str, tx_nsts, tx_rate);
}

VOID EventPhyStatRxRate(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_PHY_STATE_RX_RATE prEventPhyStatRxRate = NULL;
	UINT8 rx_rate = 0, rx_nsts = 0, rx_mode = 0;
	CHAR str[6][20] = {"CCK", "OFDM", "HT Mix-Mode", "HT Green-Field", "VHT", "HE"};
	CHAR rx_mode_str[20];

	prEventPhyStatRxRate = (P_EXT_EVENT_PHY_STATE_RX_RATE)Data;
	rx_rate = prEventPhyStatRxRate->u1RxRate;
	rx_mode = prEventPhyStatRxRate->u1RxMode;
	rx_nsts = prEventPhyStatRxRate->u1RxNsts;
	os_move_mem(rx_mode_str, *(str + rx_mode), 20);

	MTWF_PRINT("%s: trigger-based\n", __func__);
	MTWF_PRINT("rx rate: %s(%dNss M%d)\n", rx_mode_str, rx_nsts, rx_rate);
}

VOID EventPhyStatPER(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_PHY_STATE_PER prEventPhyStatPER = NULL;
	UINT8 u1rx_PER = 0;

	prEventPhyStatPER = (P_EXT_EVENT_PHY_STATE_PER)Data;
	u1rx_PER = prEventPhyStatPER->u1PER;
	MTWF_PRINT("%s: PER: %d\n", __func__, u1rx_PER);
}

INT32 MtCmdGetRssi(
	RTMP_ADAPTER *pAd,
	UINT16 u2WlanIdx,
	UINT32 *rssi_rpt
)
{
	INT32 ret = 0;
#ifdef WIFI_UNIFIED_COMMAND
	UniCmdPerStaGetRssi(pAd, u2WlanIdx, rssi_rpt);
#else
	struct cmd_msg *msg;
	CMD_PHY_STATE_SHOW_RSSI_T PhyStatRssi;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_PHY, DBG_LVL_DEBUG,
			"wcid: %d\n", u2WlanIdx);
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PHY_STATE_SHOW_RSSI_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(&PhyStatRssi, sizeof(CMD_PHY_STATE_SHOW_RSSI_T));
	PhyStatRssi.u1PhyStateInfoCatg = CMD_PHY_STATE_RSSI;
	PhyStatRssi.u2WlanIdx = u2WlanIdx;
#ifdef CFG_BIG_ENDIAN
	PhyStatRssi.u2WlanIdx = cpu2le16(PhyStatRssi.u2WlanIdx);
#endif
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PHY_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_PHY_STATE_RSSI));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, rssi_rpt);
	SET_CMD_ATTR_RSP_HANDLER(attr, phy_stat_rssi_rsp_handle);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&PhyStatRssi,
						sizeof(CMD_PHY_STATE_SHOW_RSSI_T));
	ret = chip_cmd_tx(pAd, msg);
	return ret;

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_PHY, DBG_LVL_ERROR,
			 "ret = %d\n", ret);
#endif
	return ret;
}

INT32 MtCmdGetPER(
	RTMP_ADAPTER *pAd,
	UINT16 u2WlanIdx,
	PUINT8 u1PER
)
{
	INT32 ret = 0;
#ifdef WIFI_UNIFIED_COMMAND
	UniCmdPerStaGetPER(pAd, u2WlanIdx, u1PER);
#else
	struct cmd_msg *msg;
	CMD_PHY_STATE_SHOW_PER_T PhyStatPER;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_PHY, DBG_LVL_DEBUG,
			"wcid: %d\n", u2WlanIdx);
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PHY_STATE_SHOW_PER_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(&PhyStatPER, sizeof(CMD_PHY_STATE_SHOW_PER_T));
	PhyStatPER.u1PhyStateInfoCatg = CMD_PHY_STATE_PER;
	PhyStatPER.u2WlanIdx = u2WlanIdx;
#ifdef CFG_BIG_ENDIAN
	PhyStatPER.u2WlanIdx = cpu2le16(PhyStatPER.u2WlanIdx);
#endif
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PHY_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_PHY_STATE_PER));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, u1PER);
	SET_CMD_ATTR_RSP_HANDLER(attr, phy_stat_per_rsp_handle);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&PhyStatPER,
						sizeof(CMD_PHY_STATE_SHOW_PER_T));
	ret = chip_cmd_tx(pAd, msg);
	return ret;

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_PHY, DBG_LVL_ERROR,
			 "ret = %d\n", ret);
#endif
	return ret;
}

INT32 MtCmdMultiRssi(
	RTMP_ADAPTER *pAd,
	RSSI_PAIR *RssiPair,
	UINT32 Num
)
{
	INT32 ret = 0;
#ifdef WIFI_UNIFIED_COMMAND
	UniCmdPerStaGetMultiRssi(pAd, (UINT_8 *)RssiPair, Num);
#else
	struct cmd_msg *msg;
	CMD_PHY_STATE_SHOW_RSSI_T PhyStatRssi;
	UINT32 index;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PHY_STATE_SHOW_RSSI_T) * Num);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PHY_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_PHY_STATE_RSSI) * Num);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, RssiPair);
	SET_CMD_ATTR_RSP_HANDLER(attr, phy_stat_multi_rssi_rsp_handle);
	AndesInitCmdMsg(msg, attr);

	for (index = 0; index < Num; index++) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_PHY, DBG_LVL_DEBUG,
			"wcid: %d\n", RssiPair[index].u2WlanIdx);
		os_zero_mem(&PhyStatRssi, sizeof(CMD_PHY_STATE_SHOW_RSSI_T));
		PhyStatRssi.u1PhyStateInfoCatg = CMD_PHY_STATE_RSSI;
		PhyStatRssi.u2WlanIdx = RssiPair[index].u2WlanIdx;
#ifdef CFG_BIG_ENDIAN
		PhyStatRssi.u2WlanIdx = cpu2le16(PhyStatRssi.u2WlanIdx);
#endif
		AndesAppendCmdMsg(msg, (char *)&PhyStatRssi, sizeof(CMD_PHY_STATE_SHOW_RSSI_T));
	}
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_PHY, DBG_LVL_DEBUG,
			 "(ret = %d)\n", ret);
#endif
	return ret;
}

INT32 MtCmdGetCnInfo(
	RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT16 *u2cninfo
)
{
	INT32 ret = 0;
#ifdef WIFI_UNIFIED_COMMAND
	UniCmdStateGetCnInfo(pAd, u1BandIdx, u2cninfo);
#else
	struct cmd_msg *msg;
	CMD_PHY_STATE_SHOW_OFDMLQ_CN_T PhyStatCnInfo;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_PHY, DBG_LVL_DEBUG,
			"Bandidx: %d\n", u1BandIdx);
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PHY_STATE_SHOW_OFDMLQ_CN_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(&PhyStatCnInfo, sizeof(CMD_PHY_STATE_SHOW_OFDMLQ_CN_T));
	PhyStatCnInfo.u1PhyStateInfoCatg = CMD_PHY_STATE_OFDMLQ_CNINFO;
	PhyStatCnInfo.u1BandIdx = u1BandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PHY_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_PHY_STATE_RX_RATE));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, u2cninfo);
	SET_CMD_ATTR_RSP_HANDLER(attr, phy_stat_cninfo_rsp_handle);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&PhyStatCnInfo,
						sizeof(CMD_PHY_STATE_SHOW_OFDMLQ_CN_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_PHY, DBG_LVL_INFO, "(ret = %d)\n", ret);
#endif
	return ret;
}

#ifdef SPECIAL_11B_OBW_FEATURE
INT32 MtCmdSetTxTdCck(
	RTMP_ADAPTER *pAd,
	UINT8 u1Enable
)
{
	INT32 ret = 0;
#ifdef WIFI_UNIFIED_COMMAND
	UniCmdStateSetTxTdCck(pAd, u1Enable);
#else
	struct cmd_msg *msg;
	CMD_PHY_SET_TXTD_CCK_T PhysetTxTd;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PHY_SET_TXTD_CCK_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(&PhysetTxTd, sizeof(CMD_PHY_SET_TXTD_CCK_T));
	PhysetTxTd.u1PhyStateInfoCatg = CMD_PHY_STATE_TX_TD_CCK;
	PhysetTxTd.u1Enable = u1Enable;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PHY_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&PhysetTxTd,
						sizeof(CMD_PHY_SET_TXTD_CCK_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_PHY, DBG_LVL_INFO, "(ret = %d)\n", ret);
#endif
	return ret;
}
#endif /* SPECIAL_11B_OBW_FEATURE */

#ifdef TXRX_STAT_SUPPORT
INT32 MtCmdGetAllRateInfo(RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT8 ucDirection)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	UniCmdStateAllRateCntGet(pAd, ucBandIdx, ucDirection);
#endif /* WIFI_UNIFIED_COMMAND */
	return ret;
}

INT32 MtCmdGetStbcInfo(RTMP_ADAPTER *pAd, UINT8 ucBandIdx)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	UniCmdStateStbcCntGet(pAd, ucBandIdx);
#endif /* WIFI_UNIFIED_COMMAND */
	return ret;
}

INT32 MtCmdGetGiInfo(RTMP_ADAPTER *pAd, UINT8 ucBandIdx)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	UniCmdStateGiCntGet(pAd, ucBandIdx);
#endif /* WIFI_UNIFIED_COMMAND */
	return ret;
}

#endif /* TXRX_STAT_SUPPORT */

INT Show_Phy_State_Dbg_Info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx = 0, CmdType = 0, tx_per = 0;
	UINT16 wcid = 0, PhyStateCat = 0, CnInfo = 0;
	struct wifi_dev *wdev;
	PCHAR pch = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	RSSI_PAIR RssiPair[MAX_RSSI_LEN];
	UINT8 u1Snr[MAX_RSSI_LEN] = {0};
	UINT32 Num, Value = 0, Index;
	EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
	TX_STATISTIC_RESULT_PAIR TxStatResultPair[2] = {0};
	TX_STAT_STRUC p_buff[2] = {0};
	UINT16 event_subtype = 0;
	RSSI_REPORT rssi_rpt;
	EXT_EVENT_PHY_STATE_RX_RATE rRxStatResult;

	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	if (wdev)
		BandIdx = HcGetBandByWdev(wdev);
	else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Invalid wdev!!\n");
		return FALSE;
	}
	/* sanity check for Band index */
	if (BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATHW_PHY, DBG_LVL_ERROR,
				"Invalid Band Index!!\n");
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Band Index:%d Channel:%d!!\n", BandIdx, wdev->channel);

	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATHW_PHY, DBG_LVL_ERROR,
				"Invalid parameters\n");
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		PhyStateCat = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"No parameters for PhyStateCat!!\n");
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		CmdType = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_FW, CATHW_PHY, DBG_LVL_ERROR,
				"No parameters for CmdType!!\n");
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"PhyStateCat:%d CmdType:%d!!\n", PhyStateCat, CmdType);

	switch (PhyStateCat) {
	case 0:
#ifdef WIFI_UNIFIED_COMMAND
		if (CmdType)
			UniCmdStateGetTxRxPhyRate(pAd, PhyStateCat, BandIdx);
		else
#endif
			MtCmdPhyShowInfo(pAd, PhyStateCat, BandIdx);
		break;

	case 1:
#ifdef WIFI_UNIFIED_COMMAND
		if (CmdType)
			UniCmdStateGetTxRxPhyRate(pAd, PhyStateCat, BandIdx);
		else
#endif
			MtCmdPhyShowInfo(pAd, PhyStateCat, BandIdx);
		break;

	case 2:
		pch = arg;
		if (pch != NULL)
			wcid = (UINT16) os_str_toul(pch, 0, 10);

		NdisZeroMemory(&rssi_rpt, sizeof(rssi_rpt));

		/* fw command to query rcpi/rssi */
#ifdef WIFI_UNIFIED_COMMAND
		if (CmdType)
			UniCmdPerStaGetRssi(pAd, wcid, (UINT32 *)&rssi_rpt);
		else
#endif
			MtCmdGetRssi(pAd, wcid, (UINT32 *)&rssi_rpt);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"%s(): wcid: %d, rssi: %d, %d, %d, %d\n", __func__, wcid,
				rssi_rpt.rssi[0], rssi_rpt.rssi[1], rssi_rpt.rssi[2], rssi_rpt.rssi[3]);
		break;

	case 3:
		pch = arg;
		if (pch != NULL)
			wcid = (UINT16) os_str_toul(pch, 0, 10);

		NdisZeroMemory(&rRxStatResult, sizeof(rRxStatResult));

#ifdef WIFI_UNIFIED_COMMAND
		if (CmdType)
			UniCmdPerStaGetRxRate(pAd, UNI_CMD_PER_STA_CONTENTION_RX_PHYRATE, BandIdx, wcid, (UINT32 *)&rRxStatResult);
		else
#endif
			MtCmdPhyGetRxRate(pAd, PhyStateCat, BandIdx, wcid, (UINT32 *)&rRxStatResult);

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "contention-based:\n");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"%s: Wcid:%u Rate::%u Mode:%u Nsts:%u\n",
			__func__, wcid, rRxStatResult.u1RxRate,
			rRxStatResult.u1RxMode, rRxStatResult.u1RxNsts);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"%s: GI:%u BW::%u STBC:%u Coding:%u\n",
			__func__, rRxStatResult.u1Gi, rRxStatResult.u1BW,
			rRxStatResult.u1Stbc, rRxStatResult.u1Coding);
		break;

	case 4:
#ifdef WIFI_UNIFIED_COMMAND
		if (CmdType)
			UniCmdStateGetCnInfo(pAd, BandIdx, &CnInfo);
		else
#endif
			MtCmdGetCnInfo(pAd, BandIdx, &CnInfo);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "%s: CnInfo:%u\n",
						__func__, CnInfo);
		break;

	case 5:
#ifdef SPECIAL_11B_OBW_FEATURE
		if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G) {
#ifdef WIFI_UNIFIED_COMMAND
			if (CmdType)
				UniCmdStateSetTxTdCck(pAd, TRUE);
			else
#endif
				MtCmdSetTxTdCck(pAd, TRUE);
		} else
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"%s: Channel should be >= 1 and <= 14\n", __func__);
#else
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "%s: Unsupported\n",
						__func__);
#endif
		break;

	case 6:
		pch = arg;
		if (pch != NULL)
			wcid = (UINT16) os_str_toul(pch, 0, 10);

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"wcid:%d!!\n", wcid);

#ifdef WIFI_UNIFIED_COMMAND
		if (CmdType)
			UniCmdPerStaGetPER(pAd, wcid, &tx_per);
		else
#endif
			MtCmdGetPER(pAd, wcid, &tx_per);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"%s: wcid(%d) tx per(%d)!\n", __func__, wcid, tx_per);
		break;

	case 7:
		pch = arg;
		if (pch != NULL)
			wcid = (UINT16) os_str_toul(pch, 0, 10);

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"wcid:%d!!\n", wcid);

#ifdef WIFI_UNIFIED_COMMAND
		if (CmdType)
			UniCmdPerStaGetSNR(pAd, wcid, u1Snr);
		else
#endif
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"Not supported in Ext Cmd!!\n");

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"%s: wcid(%d) SNR(%d:%d:%d:%d)!\n", __func__, wcid, u1Snr[0], u1Snr[1], u1Snr[2], u1Snr[3]);
		break;

	case 8:
		Num = MAX_RSSI_LEN;
		NdisZeroMemory(RssiPair, (sizeof(RSSI_PAIR) * Num));
		RssiPair[0].u2WlanIdx = 1;
		RssiPair[1].u2WlanIdx = 2;
		RssiPair[2].u2WlanIdx = 3;
		RssiPair[3].u2WlanIdx = 4;

#ifdef WIFI_UNIFIED_COMMAND
		if (CmdType)
			UniCmdPerStaGetMultiRssi(pAd, (UINT_8 *)RssiPair, Num);
		else
#endif
			MtCmdMultiRssi(pAd, RssiPair, Num);

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"%s: wcid(%d) RSSI(%d:%d:%d:%d)!\n", __func__, RssiPair[0].u2WlanIdx,
			RssiPair[0].rssi[0], RssiPair[0].rssi[1], RssiPair[0].rssi[2], RssiPair[0].rssi[3]);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"%s: wcid(%d) RSSI(%d:%d:%d:%d)!\n", __func__, RssiPair[1].u2WlanIdx,
			RssiPair[1].rssi[0], RssiPair[1].rssi[1], RssiPair[1].rssi[2], RssiPair[1].rssi[3]);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"%s: wcid(%d) RSSI(%d:%d:%d:%d)!\n", __func__, RssiPair[2].u2WlanIdx,
			RssiPair[2].rssi[0], RssiPair[2].rssi[1], RssiPair[2].rssi[2], RssiPair[2].rssi[3]);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"%s: wcid(%d) RSSI(%d:%d:%d:%d)!\n", __func__, RssiPair[3].u2WlanIdx,
			RssiPair[3].rssi[0], RssiPair[3].rssi[1], RssiPair[3].rssi[2], RssiPair[3].rssi[3]);
		break;

	case 9:
		pch = strsep(&arg, ":");
		if (pch != NULL)
			Value = (UINT32) os_str_toul(pch, 0, 10);

		pch = arg;
		if (pch != NULL)
			wcid = (UINT16) os_str_toul(pch, 0, 10);

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"Value:0x%x wcid:%u!!\n", Value, wcid);
		NdisZeroMemory(&rTxStatResult, sizeof(rTxStatResult));

		MtCmdGetTxStatistic(pAd, Value, 0/*Don't Care*/, wcid, &rTxStatResult);

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"[%s] Field:0x%x Wlan:%u:%u Band:%u!\n",
			__func__, rTxStatResult.ucField, rTxStatResult.ucWlanIdxHnVer, rTxStatResult.ucWlanIdxL, rTxStatResult.ucBandIdx);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"TotalTx:%u-%u CurBwTx:%u-%u OBwTx:%u-%u EntryTx:%u-%u-%u!\n",
			rTxStatResult.u4TotalTxCount, rTxStatResult.u4TotalTxFailCount, rTxStatResult.u4CurrBwTxCnt,
			rTxStatResult.u4CurrBwTxFailCnt, rTxStatResult.u4OtherBwTxCnt, rTxStatResult.u4OtherBwTxFailCnt,
			rTxStatResult.u4EntryTxCount, rTxStatResult.u4EntryTxFailCount, rTxStatResult.ucEntryTxPer);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"LastTxRate:M:%u Flag:%x STBC:%u GI:%u BW:%u cod:%u mcs:%u nss:%u ltf:%u!\n",
			rTxStatResult.rLastTxRate.MODE, rTxStatResult.rLastTxRate.Flags, rTxStatResult.rLastTxRate.STBC,
			rTxStatResult.rLastTxRate.ShortGI, rTxStatResult.rLastTxRate.BW, rTxStatResult.rLastTxRate.ldpc,
			rTxStatResult.rLastTxRate.MCS, rTxStatResult.rLastTxRate.VhtNss, rTxStatResult.rLastTxRate.he_ltf);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"EntryTxRate:M:%u Flag:%x STBC:%u GI:%u BW:%u cod:%u mcs:%u nss:%u ltf:%u!\n",
			rTxStatResult.rEntryTxRate.MODE, rTxStatResult.rEntryTxRate.Flags, rTxStatResult.rEntryTxRate.STBC,
			rTxStatResult.rEntryTxRate.ShortGI, rTxStatResult.rEntryTxRate.BW, rTxStatResult.rEntryTxRate.ldpc,
			rTxStatResult.rEntryTxRate.MCS, rTxStatResult.rEntryTxRate.VhtNss, rTxStatResult.rEntryTxRate.he_ltf);
		break;

	case 10:
		pch = arg;
		if (pch != NULL)
			Value = (UINT32) os_str_toul(pch, 0, 10);

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"Value:0x%x!!\n", Value);

		Num = 2;

		TxStatResultPair[0].u2WlanIdx = 1;
		TxStatResultPair[0].ucBand = 0;
		TxStatResultPair[0].ucField = Value;

		TxStatResultPair[1].u2WlanIdx = 2;
		TxStatResultPair[1].ucBand = 1;
		TxStatResultPair[1].ucField = Value;

		MtCmdGetMutiTxStatistic(pAd, &TxStatResultPair[0], Num);

		for (Index = 0; Index < Num; Index++) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"[%s] Index:%u u2WlanIdx:%u ucBand:%u ucField:0x%x!\n",
				__func__, Index, TxStatResultPair[Index].u2WlanIdx, TxStatResultPair[Index].ucBand, TxStatResultPair[Index].ucField);
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Field:0x%x Wlan:%u:%u Band:%u!\n",
				TxStatResultPair[Index].txStatisticRes.ucField, TxStatResultPair[Index].txStatisticRes.ucWlanIdxHnVer,
				TxStatResultPair[Index].txStatisticRes.ucWlanIdxL, TxStatResultPair[Index].txStatisticRes.ucBandIdx);
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"TotalTx:%u-%u CurBwTx:%u-%u OBwTx:%u-%u EntryTx:%u-%u-%u!\n",
				TxStatResultPair[Index].txStatisticRes.u4TotalTxCount, TxStatResultPair[Index].txStatisticRes.u4TotalTxFailCount, TxStatResultPair[Index].txStatisticRes.u4CurrBwTxCnt,
				TxStatResultPair[Index].txStatisticRes.u4CurrBwTxFailCnt, TxStatResultPair[Index].txStatisticRes.u4OtherBwTxCnt, TxStatResultPair[Index].txStatisticRes.u4OtherBwTxFailCnt,
				TxStatResultPair[Index].txStatisticRes.u4EntryTxCount, TxStatResultPair[Index].txStatisticRes.u4EntryTxFailCount, TxStatResultPair[Index].txStatisticRes.ucEntryTxPer);
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"LastTxRate:M:%u Flag:%x STBC:%u GI:%u BW:%u cod:%u mcs:%u nss:%u ltf:%u!\n",
				TxStatResultPair[Index].txStatisticRes.rLastTxRate.MODE, TxStatResultPair[Index].txStatisticRes.rLastTxRate.Flags, TxStatResultPair[Index].txStatisticRes.rLastTxRate.STBC,
				TxStatResultPair[Index].txStatisticRes.rLastTxRate.ShortGI, TxStatResultPair[Index].txStatisticRes.rLastTxRate.BW, TxStatResultPair[Index].txStatisticRes.rLastTxRate.ldpc,
				TxStatResultPair[Index].txStatisticRes.rLastTxRate.MCS, TxStatResultPair[Index].txStatisticRes.rLastTxRate.VhtNss, TxStatResultPair[Index].txStatisticRes.rLastTxRate.he_ltf);
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"EntryTxRate:M:%u Flag:%x STBC:%u GI:%u BW:%u cod:%u mcs:%u nss:%u ltf:%u!\n",
				TxStatResultPair[Index].txStatisticRes.rEntryTxRate.MODE, TxStatResultPair[Index].txStatisticRes.rEntryTxRate.Flags, TxStatResultPair[Index].txStatisticRes.rEntryTxRate.STBC,
				TxStatResultPair[Index].txStatisticRes.rEntryTxRate.ShortGI, TxStatResultPair[Index].txStatisticRes.rEntryTxRate.BW, TxStatResultPair[Index].txStatisticRes.rEntryTxRate.ldpc,
				TxStatResultPair[Index].txStatisticRes.rEntryTxRate.MCS, TxStatResultPair[Index].txStatisticRes.rEntryTxRate.VhtNss, TxStatResultPair[Index].txStatisticRes.rEntryTxRate.he_ltf);
		}

		break;

	case 11:
		pch = arg;
		if (pch != NULL)
			Value = (UINT32) os_str_toul(pch, 0, 10);

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"Value:0x%x!!\n", Value);

		Num = 2;

		p_buff[0].Wcid = 1;
		p_buff[0].Band = 0;
		p_buff[0].Field = Value;

		p_buff[1].Wcid = 2;
		p_buff[1].Band = 1;
		p_buff[1].Field = Value;

		mt_cmd_get_sta_tx_statistic(pAd, &p_buff[0], Num);
		break;

	case 12:
		pch = arg;
		if (pch != NULL)
			event_subtype = (UINT16) os_str_toul(pch, 0, 10);

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"event_subtype:%u!!\n", event_subtype);

#ifdef WIFI_UNIFIED_COMMAND
		if (CmdType)
			UniCmdAllStaInfoGetInfo(pAd, event_subtype);
		else
#endif
			MtCmdGetAllStaStats(pAd, event_subtype);

		break;

	default:
		break;
	}

	return TRUE;
}

