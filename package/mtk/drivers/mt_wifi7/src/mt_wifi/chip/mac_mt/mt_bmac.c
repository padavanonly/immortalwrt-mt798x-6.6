/*
 * Copyright (c) [2021], MediaTek Inc. All rights reserved.
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

#ifdef MT_BMAC

#include "rt_config.h"
#include "mac_mt/fmac/mt_fmac.h"
#include "mac_mt/bmac/mt_bmac.h"

static const  UCHAR bmac_wmm_swq_2_hw_ac_que[4][4] = {
	{
		TxQ_IDX_AC0, /* 0: QID_AC_BK */
		TxQ_IDX_AC1, /* 1: QID_AC_BE */
		TxQ_IDX_AC2, /* 2: QID_AC_VI */
		TxQ_IDX_AC3, /* 3: QID_AC_VO */
	},
	{
		TxQ_IDX_AC10,
		TxQ_IDX_AC11,
		TxQ_IDX_AC12,
		TxQ_IDX_AC13,
	},
	{
		TxQ_IDX_AC20,
		TxQ_IDX_AC21,
		TxQ_IDX_AC22,
		TxQ_IDX_AC23,
	},
	{
		TxQ_IDX_AC30,
		TxQ_IDX_AC31,
		TxQ_IDX_AC32,
		TxQ_IDX_AC33,
	}
};

VOID mtb_dump_rmac_info(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	union _RMAC_RXD_0_UNION *rxd_0;
	UINT32 pkt_type;

	rxd_0 = (union _RMAC_RXD_0_UNION *)rmac_info;
	pkt_type = RMAC_RX_PKT_TYPE(rxd_0->word);
	hex_dump_always("RawData", rmac_info, 50);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG,
		"RMAC_RXD Header Format :%s\n",
		rxd_pkt_type_str(pkt_type));
/* TODO #1
* switch (pkt_type) {
* case RMAC_RX_PKT_TYPE_RX_TXS:
* dump_rmac_info_txs(pAd, rmac_info);
break;
* case RMAC_RX_PKT_TYPE_RX_TXRXV:
* dump_rmac_info_rxv(pAd, rmac_info);
break;
* case RMAC_RX_PKT_TYPE_RX_NORMAL:
break;
* case RMAC_RX_PKT_TYPE_RX_DUP_RFB:
* dump_rmac_info_rfb(pAd, rmac_info);
break;
* case RMAC_RX_PKT_TYPE_RX_TMR:
* dump_rmac_info_tmr(pAd, rmac_info);
break;
default:
break;
}
*/
}

VOID mtb_dump_rxinfo(RTMP_ADAPTER *pAd, UCHAR *rxinfo)
{
	struct _RXINFO_STRUC *pRxInfo = (struct _RXINFO_STRUC *) rxinfo;

	hex_dump("RxInfo Raw Data", (UCHAR *)pRxInfo, sizeof(struct _RXINFO_STRUC));
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "RxInfo Fields:\n");
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tBA=%d\n", pRxInfo->BA);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tDATA=%d\n", pRxInfo->DATA);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tNULLDATA=%d\n", pRxInfo->NULLDATA);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tFRAG=%d\n", pRxInfo->FRAG);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tU2M=%d\n", pRxInfo->U2M);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tMcast=%d\n", pRxInfo->Mcast);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tBcast=%d\n", pRxInfo->Bcast);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tMyBss=%d\n", pRxInfo->MyBss);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tCrc=%d\n", pRxInfo->Crc);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tCipherErr=%d\n", pRxInfo->CipherErr);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tAMSDU=%d\n", pRxInfo->AMSDU);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tHTC=%d\n", pRxInfo->HTC);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tRSSI=%d\n", pRxInfo->RSSI);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tL2PAD=%d\n", pRxInfo->L2PAD);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tAMPDU=%d\n", pRxInfo->AMPDU);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tDecrypted=%d\n", pRxInfo->Decrypted);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tBssIdx3=%d\n", pRxInfo->BssIdx3);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\twapi_kidx=%d\n", pRxInfo->wapi_kidx);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tpn_len=%d\n", pRxInfo->pn_len);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tsw_fc_type0=%d\n", pRxInfo->sw_fc_type0);
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG
		, "\tsw_fc_type1=%d\n", pRxInfo->sw_fc_type1);
}

VOID mtb_dump_rmac_info_for_ICVERR(
	RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
/* TODO #2
struct rxd_grp_0 *rxd_grp0 = (struct rxd_grp_0 *)(rmac_info);
int LogDbgLvl = DBG_LVL_ERROR;
if (!IS_HIF_TYPE(pAd, HIF_MT))
return;
MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, LogDbgLvl, "\tHTC/UC2ME/MC/BC = %d/%d/%d/%d\n",
(rxd_grp0->rxd_3 & RXD_HTC) ? 1 : 0,
(((rxd_grp0->rxd_3 & RXD_A1_TYPE_MASK) >> RXD_A1_TYPE_SHIFT) == 0x1)? 1 : 0,
(((rxd_grp0->rxd_3 & RXD_A1_TYPE_MASK) >> RXD_A1_TYPE_SHIFT) == 0x2)? 1 : 0,
(((rxd_grp0->rxd_3 & RXD_A1_TYPE_MASK) >> RXD_A1_TYPE_SHIFT) == 0x3)? 1 : 0);
MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, LogDbgLvl, ", WlanIndex=%d",
((rxd_grp0->rxd_1 & RXD_WLAN_IDX_MASK) >> RXD_WLAN_IDX_SHIFT));
MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, LogDbgLvl, ", SEC Mode=%d\n",
((rxd_grp0->rxd_1 & RXD_SEC_MODE_MASK) >> RXD_SEC_MODE_SHIFT));
MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, LogDbgLvl, "\tFCE Error(FC)=%d", (rxd_grp0->rxd_1 & RXD_FCS_ERR) ? 1 : 0);
MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, LogDbgLvl, ", CM=%d", (rxd_grp0->rxd_1 & RXD_CM) ? 1 : 0);
MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, LogDbgLvl, ", CLM=%d", (rxd_grp0->rxd_1 & RXD_CLM) ? 1 : 0);
MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, LogDbgLvl, ", I=%d", (rxd_grp0->rxd_1 & RXD_ICV_ERR) ? 1 : 0);
MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, LogDbgLvl, ", T=%d", (rxd_grp0->rxd_1 & RXD_TKIPMIC_ERR) ? 1 : 0);
MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, LogDbgLvl, "\tFragment Frame(FRAG)=%d\n", (rxd_grp0->rxd_2 & RXD_FRAG) ? 1 : 0);
*/
}

#ifdef TX_POWER_CONTROL_SUPPORT_V2
VOID mtb_txpower_boost_v2(struct _RTMP_ADAPTER *pAd, UCHAR ucBandIdx)
{
	UINT i = 0;
	struct POWER_BOOST_PARA_V2 *pwrBst = pAd->CommonCfg.PowerBoostParamV2;

	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_ERROR, "ucBandIdx incorrect!!!\n");
		return;
	}

	for (i = POWER_UP_CATE_V2_CCK; i < POWER_UP_CATE_V2_NUM; i++)
		/* config Power boost table via profile */
		TxPwrUpCtrl(pAd, ucBandIdx, i,
				pwrBst[i].pwrUpCatRateVal,
				pwrBst[i].catSize);
}

VOID mtb_txpower_boost_ctrl_v2(
	struct _RTMP_ADAPTER *pAd, CHAR cPwrUpCat, PUCHAR pcPwrUpValue)
{
	struct POWER_BOOST_PARA_V2 *pwrBst = pAd->CommonCfg.PowerBoostParamV2;

	if (cPwrUpCat >= POWER_UP_CATE_V2_CCK && cPwrUpCat < POWER_UP_CATE_V2_NUM) {
		os_move_mem(pwrBst[cPwrUpCat].pwrUpCatRateVal,
			pcPwrUpValue, sizeof(CHAR) * pwrBst[cPwrUpCat].catSize);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_ERROR, "set wrong parameters\n");
	}
}

BOOLEAN mtb_txpower_boost_info_v2(struct _RTMP_ADAPTER *pAd, enum POWER_BOOST_TABLE_CATEGORY_V2 ePowerBoostRateType)
{
#define MCS_STRLEN 16
	UINT i = 0, j = 0;
	UCHAR mcsStr[MCS_STRLEN];
	INT sRet = 0;
	struct POWER_BOOST_PARA_V2 *pwrBst = pAd->CommonCfg.PowerBoostParamV2;

	for (i = POWER_UP_CATE_V2_CCK; i < POWER_UP_CATE_V2_NUM; i++) {
		MTWF_PRINT("%s:\n", gPwrBstString[i]);
		for (j = 0; j < pwrBst[i].catSize; j++) {
			sRet = snprintf(mcsStr, MCS_STRLEN, "M%d", j);
			if (os_snprintf_error(MCS_STRLEN, sRet))
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_ERROR, "snprintf error!\n");

			MTWF_PRINT("%4s", mcsStr);
		}
		MTWF_PRINT("\n");
		for (j = 0; j < pwrBst[i].catSize; j++)
			MTWF_PRINT("%4d", pwrBst[i].pwrUpCatRateVal[j]);
		MTWF_PRINT("\n");
	}
	return TRUE;
}

VOID mtb_txpower_boost_profile_v2(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *pBuffer)
{
	UINT	i = 0, j = 0;
	CHAR *value = 0;
	long PwrBoostReg = 0;
	struct POWER_BOOST_PARA_V2 *pwrBst = pAd->CommonCfg.PowerBoostParamV2;

	for (i = POWER_UP_CATE_V2_CCK; i < POWER_UP_CATE_V2_NUM; i++) {
		if (RTMPGetKeyParameter(gPwrBstString[i], tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (j = 0, value = rstrtok(tmpbuf, ":"); (value) && (j < pwrBst[i].catSize); value = rstrtok(NULL, ":"), j++) {
				PwrBoostReg = 0;
				PwrBoostReg = os_str_tol(value, 0, 10);
				pwrBst[i].pwrUpCatRateVal[j] = PwrBoostReg;
			}

			if (value != NULL && j >= pwrBst[i].catSize)
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_ERROR, "[%s] Input %d parameters are more then needed %d !!\n", gPwrBstString[i], j, pwrBst[i].catSize);
		}
	}
}
#endif /* TX_POWER_CONTROL_SUPPORT_V2 */

#endif /* MT_BMAC */
