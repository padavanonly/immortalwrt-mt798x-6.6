/*
 * Copyright (c) [2020], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation "MediaTek Software") are
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
	txpwr.c
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
#ifdef TX_POWER_CONTROL_SUPPORT_V2

CHAR *gPwrBstString[POWER_UP_CATE_V2_NUM] = {"PowerBoostCCK", "PowerBoostOFDM", "PowerBoostHT20",
	"PowerBoostHT40", "PowerBoostVHT20", "PowerBoostVHT40", "PowerBoostVHT80",
	"PowerBoostVHT160", "PowerBoostHE26", "PowerBoostHE52", "PowerBoostHE106",
	"PowerBoostHE242", "PowerBoostHE484", "PowerBoostHE996", "PowerBoostHE996X2",
	"PowerBoostEHT26", "PowerBoostEHT52", "PowerBoostEHT106", "PowerBoostEHT242",
	"PowerBoostEHT484", "PowerBoostEHT996", "PowerBoostEHT996X2", "PowerBoostEHT996X4",
	"PowerBoostEHT26_52", "PowerBoostEHT26_106", "PowerBoostEHT484_242",
	"PowerBoostEHT996_484", "PowerBoostEHT996_484_242", "PowerBoostEHT996X2_484",
	"PowerBoostEHT996X3", "PowerBoostEHT996X3_484"};

#endif /* TX_POWER_CONTROL_SUPPORT_V2 */

/*******************************************************************************
 *    EXTERNAL FUNCTION PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/
INT32 MtCmdPwrLimitTblUpdate(
	RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT8 u1Type,
	UINT8 u1ChannelBand,
	UINT8 u1ControlChannel,
	UINT8 u1CentralChannel
)
{
	INT32 ret = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	CMD_POWER_LIMIT_TABLE_CTRL_T rPwrLimitTblCtrl;

	pAd->CommonCfg.SKUBandIdx = u1BandIdx;
	pAd->CommonCfg.SKUChannelBand = u1ChannelBand;
	pAd->CommonCfg.SKUControlChannel = u1ControlChannel;
	pAd->CommonCfg.SKUCentralChannel = u1CentralChannel;

#ifdef WIFI_UNIFIED_COMMAND

	os_zero_mem(&rPwrLimitTblCtrl, sizeof(CMD_POWER_LIMIT_TABLE_CTRL_T));

	if (cap->txpower_type == TX_POWER_TYPE_V1)
		rPwrLimitTblCtrl.u1PowerCtrlFormatId = POWER_LIMIT_TABLE_CTRL;
	rPwrLimitTblCtrl.u1PwrLimitType = u1Type;
	rPwrLimitTblCtrl.u1BandIdx = u1BandIdx;
#ifdef SINGLE_SKU_V2
	if (cap->txpower_type == TX_POWER_TYPE_V1)
		MtPwrFillLimitParam(pAd, u1ChannelBand, u1ControlChannel,
				u1CentralChannel, &rPwrLimitTblCtrl.uPwrLimitTbl, u1Type);
#endif
	ret = UniCmdPwrLimitTblUpdate(pAd, &rPwrLimitTblCtrl);
#else

	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_POWER, DBG_LVL_INFO,
		"u1Type: %d, u1BandIdx: %d, u1ChannelBand: %d, u1ControlChannel: %d, u1CentralChannel: %d\n",
		u1Type, u1BandIdx, u1ChannelBand, u1ControlChannel, u1CentralChannel);
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_POWER_LIMIT_TABLE_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rPwrLimitTblCtrl, sizeof(CMD_POWER_LIMIT_TABLE_CTRL_T));
	if (cap->txpower_type == TX_POWER_TYPE_V1)
		rPwrLimitTblCtrl.u1PowerCtrlFormatId = POWER_LIMIT_TABLE_CTRL;
	rPwrLimitTblCtrl.u1PwrLimitType 	 = u1Type;
	rPwrLimitTblCtrl.u1BandIdx			 = u1BandIdx;

#ifdef SINGLE_SKU_V2
	if (cap->txpower_type == TX_POWER_TYPE_V1)
	/* Fill Power Limit Parameters to CMD payload */
		MtPwrFillLimitParam(pAd, u1ChannelBand, u1ControlChannel,
				u1CentralChannel, &rPwrLimitTblCtrl.uPwrLimitTbl, u1Type);
#endif

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&rPwrLimitTblCtrl,
						sizeof(CMD_POWER_LIMIT_TABLE_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_POWER, DBG_LVL_INFO, "(ret = %d)\n", ret);
#endif
	return ret;
}

#if defined(TX_POWER_CONTROL_SUPPORT) || defined(TX_POWER_CONTROL_SUPPORT_V2)
INT32 MtCmdTxPwrUpCtrl(
	RTMP_ADAPTER *pAd,
	INT8          ucBandIdx,
	CHAR          cPwrUpCat,
	signed char   *cPwrUpValue,
	UCHAR			cPwrUpValLen)
{
	INT32 ret = 0;
#ifdef WIFI_UNIFIED_COMMAND
	ret = UniCmdTxPwrUpCtrl(pAd, (uint8_t)ucBandIdx, cPwrUpCat, cPwrUpValue);
#else
	struct cmd_msg *msg;
	CMD_POWER_BOOST_TABLE_CTRL_T TxPwrUpTblCtrl;
	UCHAR i = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_POWER, DBG_LVL_INFO,
		"ucBandIdx: %d, cPwrUpCat: %d, cPwrUpValLen: %d\n",
		ucBandIdx, cPwrUpCat, cPwrUpValLen);

	for (i = 0; i < cPwrUpValLen; i++)
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_POWER, DBG_LVL_INFO,
			"cPwrUpValue: %d\n", *(cPwrUpValue + i));

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_POWER_BOOST_TABLE_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/* init buffer structure */
	os_zero_mem(&TxPwrUpTblCtrl, sizeof(CMD_POWER_BOOST_TABLE_CTRL_T));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		TxPwrUpTblCtrl.ucPowerCtrlFormatId = TXPOWER_UP_TABLE_CTRL_V0;
	else
		TxPwrUpTblCtrl.ucPowerCtrlFormatId = TXPOWER_UP_TABLE_CTRL_V1;
	TxPwrUpTblCtrl.ucBandIdx           = ucBandIdx;
	TxPwrUpTblCtrl.cPwrUpCat           = cPwrUpCat;

	/* update Power Up Table value to buffer structure */
	os_move_mem(TxPwrUpTblCtrl.cPwrUpValue, cPwrUpValue,
			sizeof(CHAR) * cPwrUpValLen);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&TxPwrUpTblCtrl,
		sizeof(CMD_POWER_BOOST_TABLE_CTRL_T));

	ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_POWER, DBG_LVL_INFO, "(ret = %d)\n", ret);
#endif
	return ret;
}
#endif /* TX_POWER_CONTROL_SUPPORT || TX_POWER_CONTROL_SUPPORT_V2 */

