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
	cmm_single_sku.c
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
#include	"txpwr/PowerLimit_sku.h"

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
struct pwr_table_list *pcptrSkuTbl[] = {Sku_01, Sku_02, Sku_03, Sku_04, Sku_05,
	Sku_06, Sku_07, Sku_08, Sku_09, Sku_10,
	Sku_11, Sku_12, Sku_13, Sku_14, Sku_15,
	Sku_16, Sku_17, Sku_18, Sku_19, Sku_20, Sku_21};

struct pwr_table_list *pcptrBackoffTbl[] = {Backoff_01, Backoff_02, Backoff_03, Backoff_04, Backoff_05,
	Backoff_06, Backoff_07, Backoff_08, Backoff_09, Backoff_10,
	Backoff_11, Backoff_12, Backoff_13, Backoff_14, Backoff_15,
	Backoff_16, Backoff_17, Backoff_18, Backoff_19, Backoff_20, Backoff_21};

UINT8 powerLimitDataRowNum[SINGLE_SKU_TABLE_TYPE_NUM] = {
	SINGLE_SKU_TABLE_CCK_LENGTH, SINGLE_SKU_TABLE_OFDM_LENGTH, SINGLE_SKU_TABLE_HT_20_LENGTH,
	SINGLE_SKU_TABLE_HT_40_LENGTH, SINGLE_SKU_TABLE_VHT_LENGTH, SINGLE_SKU_TABLE_VHT_LENGTH, SINGLE_SKU_TABLE_VHT_LENGTH,
	SINGLE_SKU_TABLE_VHT_LENGTH, SINGLE_SKU_TABLE_HE_LENGTH, SINGLE_SKU_TABLE_HE_LENGTH, SINGLE_SKU_TABLE_HE_LENGTH,
	SINGLE_SKU_TABLE_HE_LENGTH,	SINGLE_SKU_TABLE_HE_LENGTH, SINGLE_SKU_TABLE_HE_LENGTH, SINGLE_SKU_TABLE_HE_LENGTH
	, SINGLE_SKU_TABLE_EHT_LENGTH, SINGLE_SKU_TABLE_EHT_LENGTH, SINGLE_SKU_TABLE_EHT_LENGTH, SINGLE_SKU_TABLE_EHT_LENGTH,
	SINGLE_SKU_TABLE_EHT_LENGTH, SINGLE_SKU_TABLE_EHT_LENGTH, SINGLE_SKU_TABLE_EHT_LENGTH, SINGLE_SKU_TABLE_EHT_LENGTH,
	SINGLE_SKU_TABLE_EHT_LENGTH, SINGLE_SKU_TABLE_EHT_LENGTH, SINGLE_SKU_TABLE_EHT_LENGTH, SINGLE_SKU_TABLE_EHT_LENGTH,
	SINGLE_SKU_TABLE_EHT_LENGTH, SINGLE_SKU_TABLE_EHT_LENGTH, SINGLE_SKU_TABLE_EHT_LENGTH, SINGLE_SKU_TABLE_EHT_LENGTH};
UINT8 backOffDataRowNum[BF_BACKOFF_TYPE_NUM] = {
	BF_BACKOFF_CCK_LENGTH, BF_BACKOFF_OFDM_OFF_LENGTH, BF_BACKOFF_OFDM_ON_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH
	, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH
	, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH
	, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH
	, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH
	, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH, BF_BACKOFF_LENGTH
	};
static const CHAR SkuParseTypeName[BF_BACKOFF_TYPE_NUM][20] = {
		"CCK", "OFDM", "HT20", "HT40", "VHT20", "VHT40", "VHT80", "VHT160", "HERU26", "HERU52", "HERU106", "HERU242", "HERU484"
		, "HERU996", "HERU996X2", "EHTRU26", "EHTRU52", "EHTRU106", "EHTRU242", "EHTRU484", "EHTRU996", "EHTRU996X2", "EHTRU996X4"
		, "EHTRU26_52", "EHTRU26_106", "EHTRU484_242", "EHTRU996_484", "EHTRU996_484_242", "EHTRU996X2_484", "EHTRU996X3", "EHTRU996X3_484"};
static const CHAR BackoffParseTypeName[BF_BACKOFF_TYPE_NUM][15] = {
		"BFOFF_CCK", "BF_OFF_OFDM", "BF_ON_OFDM", "BF_OFF_RU26", "BF_ON_RU26", "BF_OFF_RU52", "BF_ON_RU52", "BF_OFF_RU78", "BF_ON_RU78"
		, "BF_OFF_RU106", "BF_ON_RU106", "BF_OFF_RU132", "BF_ON_RU132", "BF_OFF_VHT20", "BF_ON_VHT20", "BF_OFF_VHT40", "BF_ON_VHT40"
		, "BF_OFF_RU726", "BF_ON_RU726", "BF_OFF_VHT80", "BF_ON_VHT80", "BF_OFF_RU1480", "BF_ON_RU1480", "BF_OFF_RU1722", "BF_ON_RU1722"
		, "BF_OFF_VHT160", "BF_ON_VHT160", "BF_OFF_RU2476", "BF_ON_RU2476", "BF_OFF_RU2988", "BF_ON_RU2988", "BF_OFF_RU3472", "BF_ON_RU3472"
		, "BF_OFF_EHT320", "BF_ON_EHT320"};


/*******************************************************************************
 *    EXTERNAL DATA
 ******************************************************************************/

extern RTMP_STRING *__rstrtok;

/*******************************************************************************
 *    EXTERNAL FUNCTION PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/

NDIS_STATUS MtPwrLimitLoadParamHandle(RTMP_ADAPTER *pAd, UINT8 u1Type)
{
	/* enable flag for Read Power limit table pass */
		pAd->fgPwrLimitRead[u1Type] = TRUE;

	/* print out power limit table info */
	if (MtShowPwrLimitTable(pAd, u1Type, DBG_LVL_DEBUG) != NDIS_STATUS_SUCCESS)
		goto error2;

	return NDIS_STATUS_SUCCESS;


error2:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR, "Show Power Table Error!!\n");
	return NDIS_STATUS_FAILURE;

}

NDIS_STATUS MtPwrLimitUnloadParamHandle(RTMP_ADAPTER *pAd, UINT8 u1Type)
{
	/* free allocated memory for power limit table */
	if (pAd->fgPwrLimitRead[u1Type]) {
		/* disable flag for Read Power limit table pass */
		pAd->fgPwrLimitRead[u1Type] = FALSE;
	}

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS MtPwrLimitParse(RTMP_ADAPTER *pAd, PUINT8 pi1PwrLimitNewCh, UINT8 u1ChBand, UINT8 u1Type)
{
	UINT8 u1ColIdx, u1ParamType, u1ParamIdx;
	INT8  *pu1ParamTypeLen = NULL, *pu1ChBandNeedParse = NULL;
	PCHAR pcToken;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 u1TypeParseNum[TABLE_PARSE_TYPE_NUM] = {pChipCap->single_sku_type_parse_num, pChipCap->backoff_type_parse_num};

	/* sanity check for null pointer */
	if (!pi1PwrLimitNewCh)
		goto error0;

	/* update power limit value raw data type number */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_RAW_DATA_LENGTH, (PVOID *)&pu1ParamTypeLen);

	/* sanity check for null pointer */
	if (!pu1ParamTypeLen)
		goto error1;

	/* update power limit value channel band need parsing bit-field */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_CH_BAND_NEED_PARSE_BITFIELD, (PVOID *)&pu1ChBandNeedParse);

	/* sanity check for null pointer */
	if (!pu1ChBandNeedParse)
		goto error1;

	/* check if different info contents for different channel (CCK, OFDM, VHT20/40/80/160, RU26/52/106/242/484/996/996X2) */
	for (u1ParamType = 0, u1ParamIdx = 0; u1ParamType < u1TypeParseNum[u1Type]; u1ParamType++) {
		/* check if need to parse for specific channel band */
		if (*(pu1ChBandNeedParse + u1ParamType) & (u1ChBand + 1)) {
			for (u1ColIdx = 0; u1ColIdx < *(pu1ParamTypeLen + u1ParamType); u1ColIdx++) {
				/* toker update for next character parsing */
				pcToken = rstrtok(NULL, " ");
				if (!pcToken)
					break;
				/* config VHT20 Power Limit */
				MtPowerLimitFormatTrans(pAd, pi1PwrLimitNewCh + u1ColIdx + u1ParamIdx, pcToken);
			}
		}

		/* parameter index increment for different parameter type */
		u1ParamIdx += *(pu1ParamTypeLen + u1ParamType);
	}

	return NDIS_STATUS_SUCCESS;

error0:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR,
		"null pointer for buffer to update power limit table after parsing !!\n");
	return NDIS_STATUS_FAILURE;

error1:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR,
		"null pointer for parameter related to parse power limit table proc !!\n");
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtPwrLimitSimilarCheck(RTMP_ADAPTER *pAd, PUINT8 pi1PwrLimitStartCh, PUINT8 pi1PwrLimitNewCh, BOOLEAN *pfgSameContent, UINT8 u1ChBand, UINT8 u1Type)
{
	UINT8 u1ColIdx, u1ParamType, u1ParamIdx;
	INT8  *pu1ParamTypeLen = NULL, *pu1ChBandNeedParse = NULL;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 u1TypeParseNum[TABLE_PARSE_TYPE_NUM] = {pChipCap->single_sku_type_parse_num, pChipCap->backoff_type_parse_num};

	/* sanity check for null pointer */
	if (!pi1PwrLimitStartCh)
		goto error1;

	/* sanity check for null pointer */
	if (!pi1PwrLimitNewCh)
		goto error2;

	/* update power limit value raw data type number */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_RAW_DATA_LENGTH, (PVOID *)&pu1ParamTypeLen);

	/* sanity check for null pointer */
	if (!pu1ParamTypeLen)
		goto error3;

	/* update power limit value channel band need parsing bit-field */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_CH_BAND_NEED_PARSE_BITFIELD, (PVOID *)&pu1ChBandNeedParse);

	/* sanity check for null pointer */
	if (!pu1ChBandNeedParse)
		goto error3;

	/* same content flag init */
	*pfgSameContent = TRUE;

	/* check if different info contents for different channel (CCK, OFDM, VHT20/40/80/160, RU26/52/106/242/484/996/996X2) */
	for (u1ParamType = 0, u1ParamIdx = 0; u1ParamType < u1TypeParseNum[u1Type]; u1ParamType++) {
		/* check if need to parse for specific channel band */
		if (*(pu1ChBandNeedParse + u1ParamType) & (u1ChBand + 1)) {
			for (u1ColIdx = 0; u1ColIdx < *(pu1ParamTypeLen + u1ParamType); u1ColIdx++) {
				if (*(pi1PwrLimitStartCh + u1ColIdx + u1ParamIdx) != *(pi1PwrLimitNewCh + u1ColIdx + u1ParamIdx)) {
					*pfgSameContent = FALSE;
					return NDIS_STATUS_SUCCESS;
				}
			}
		}

		/* parameter index increment for different parameter type */
		u1ParamIdx += *(pu1ParamTypeLen + u1ParamType);
	}

	return NDIS_STATUS_SUCCESS;

error1:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR,
		"null pointer for pointer to power limit table start channel for check !!\n");
	return NDIS_STATUS_FAILURE;

error2:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR,
		"null pointer for pointer to power limit table current channel for check !!\n");
	return NDIS_STATUS_FAILURE;

error3:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR,
		"null pointer for parameter related to power limit table proc similar check !!\n");
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtShowPwrLimitTable(RTMP_ADAPTER *pAd, UINT8 u1Type, UINT8 u1DebugLevel)
{
	UINT8 u1ColIdx, u1ParamType;
	UINT32 u1ParamIdx;
	struct pwr_table_list *pTbl = NULL;
	UINT8 sku_tbl_idx = 0;
	UINT8 rowNum = 0, i =0;
	UINT8 u1TypeParseNum[TABLE_PARSE_TYPE_NUM] = {SINGLE_SKU_TABLE_TYPE_NUM, BF_BACKOFF_TYPE_NUM};

	if (u1DebugLevel != 0)
		return NDIS_STATUS_SUCCESS;

	/* query sku table index */
	chip_get_sku_tbl_idx(pAd, &sku_tbl_idx);
	if (sku_tbl_idx >= TABLE_SIZE)
	goto error1;

#if defined(CONFIG_6G_SUPPORT) && defined(CONFIG_6G_AFC_SUPPORT) && defined(DOT11_HE_AX)
		if (pAd->afc_ctrl.AfcStateMachine.CurrState == AFC_RUN) {
			MTWF_PRINT("Device is standard power\n");

			sku_tbl_idx = AFC_STD_PWR_SKUTABLE_IDX;
		}
#endif /*CONFIG_6G_SUPPORT &&*/

	if (u1Type == POWER_LIMIT_TABLE_TYPE_SKU) {
		pTbl = pcptrSkuTbl[sku_tbl_idx];
		rowNum = ARRAY_SIZE(Sku_01);
	} else if (u1Type == POWER_LIMIT_TABLE_TYPE_BACKOFF) {
		pTbl = pcptrBackoffTbl[sku_tbl_idx];
		rowNum = ARRAY_SIZE(Backoff_01);
	}

	MTWF_DBG_NP(DBG_CAT_POWER, CATPOWER_SKU, u1DebugLevel,
		"-----------------------------------------------------------------\n");
	MTWF_DBG_NP(DBG_CAT_POWER, CATPOWER_SKU, u1DebugLevel,
		"SKU table index: %d\n", sku_tbl_idx);

	for (i = 0; i < rowNum; i++) {
		MTWF_DBG_NP(DBG_CAT_POWER, CATPOWER_SKU, u1DebugLevel,
			"channel: %d\n", pTbl[i].channel);
		MTWF_DBG_NP(DBG_CAT_POWER, CATPOWER_SKU, u1DebugLevel,
			"Band: %d\n", pTbl[i].chBand);

		/* check if different info contents for different channel (CCK, OFDM, VHT20/40/80/160, RU26/52/106/242/484/996/996X2) */
		for (u1ParamType = 0, u1ParamIdx = 0; u1ParamType < u1TypeParseNum[u1Type]; u1ParamType++) {

			if (u1Type == POWER_LIMIT_TABLE_TYPE_SKU) {
				MTWF_DBG_NP(DBG_CAT_POWER, CATPOWER_SKU,
					u1DebugLevel, "%s: ", SkuParseTypeName[u1ParamType]);
				for (u1ColIdx = 0; u1ColIdx < powerLimitDataRowNum[u1ParamType]; u1ColIdx++, u1ParamIdx++)
					MTWF_DBG_NP(DBG_CAT_POWER, CATPOWER_SKU,
						u1DebugLevel, "%d ", pTbl[i].pwrLimit[u1ParamIdx]);
			} else if (u1Type == POWER_LIMIT_TABLE_TYPE_BACKOFF) {
				MTWF_DBG_NP(DBG_CAT_POWER, CATPOWER_SKU,
					u1DebugLevel, "%s: ", BackoffParseTypeName[u1ParamType]);
				for (u1ColIdx = 0; u1ColIdx < backOffDataRowNum[u1ParamType]; u1ColIdx++, u1ParamIdx++)
					MTWF_DBG_NP(DBG_CAT_POWER, CATPOWER_SKU, u1DebugLevel,
						"%d ", pTbl[i].pwrLimit[u1ParamIdx]);
			}
			MTWF_DBG_NP(DBG_CAT_POWER, CATPOWER_SKU, u1DebugLevel, "\n");
	}
	}
	MTWF_DBG_NP(DBG_CAT_POWER, CATPOWER_SKU, u1DebugLevel,
		"-----------------------------------------------------------------\n");

	return NDIS_STATUS_SUCCESS;

error1:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR, "invalid sku table index !!\n");
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtPwrGetPwrLimitInstanceSku(RTMP_ADAPTER *pAd, ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE eInstanceIdx, PVOID *ppvBuffer)
{
	switch (eInstanceIdx) {
	case POWER_LIMIT_LINK_LIST:
		*ppvBuffer = &(pAd->PwrLimitSkuList);
		break;
	case POWER_LIMIT_RAW_DATA_LENGTH:
		*ppvBuffer = pAd->u1SkuParamLen;
		break;
	case POWER_LIMIT_RAW_DATA_OFFSET:
		*ppvBuffer = pAd->u1SkuParamTransOffset;
		break;
	case POWER_LIMIT_DATA_LENGTH:
		*ppvBuffer = pAd->u1SkuFillParamLen;
		break;
	case POWER_LIMIT_CH_BAND_NEED_PARSE_BITFIELD:
		*ppvBuffer = pAd->u1SkuChBandNeedParse;
		break;
	default:
		goto error;
	}

	return NDIS_STATUS_SUCCESS;

error:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR, "invalid instance for sku !!\n");
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtPwrGetPwrLimitInstanceBackoff(RTMP_ADAPTER *pAd, ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE eInstanceIdx, PVOID *ppvBuffer)
{
	switch (eInstanceIdx) {
	case POWER_LIMIT_LINK_LIST:
		*ppvBuffer = &(pAd->PwrLimitBackoffList);
		break;
	case POWER_LIMIT_RAW_DATA_LENGTH:
		*ppvBuffer = pAd->u1BackoffParamLen;
		break;
	case POWER_LIMIT_RAW_DATA_OFFSET:
		*ppvBuffer = pAd->u1BackoffParamTransOffset;
		break;
	case POWER_LIMIT_DATA_LENGTH:
		*ppvBuffer = pAd->u1BackoffFillParamLen;
		break;
	case POWER_LIMIT_CH_BAND_NEED_PARSE_BITFIELD:
		*ppvBuffer = pAd->u1BackoffChBandNeedParse;
		break;
	default:
		goto error;
	}

	return NDIS_STATUS_SUCCESS;

error:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR,
		"invalid instance for backoff !!\n");
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtPwrGetPwrLimitInstance(RTMP_ADAPTER *pAd, POWER_LIMIT_TABLE u1Type, ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE eInstanceIdx, PVOID *ppvBuffer)
{
	/* get pointer of link list address */
	switch (u1Type) {
	case POWER_LIMIT_TABLE_TYPE_SKU:
		MtPwrGetPwrLimitInstanceSku(pAd, eInstanceIdx, ppvBuffer);
		break;
	case POWER_LIMIT_TABLE_TYPE_BACKOFF:
		MtPwrGetPwrLimitInstanceBackoff(pAd, eInstanceIdx, ppvBuffer);
		break;
	default:
		goto error;
	}

	return NDIS_STATUS_SUCCESS;

error:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR, "invalid instance type !!\n");
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtPowerLimitFormatTrans(RTMP_ADAPTER *pAd, PUINT8 pu1Value, PCHAR pcRawData)
{
	CHAR *cBuffer = NULL;
	CHAR *cToken = NULL;
	UINT8 u1NonInteValue = 0;

	/* sanity check for null pointer */
	if (!pu1Value)
		goto error1;

	/* sanity check for null poitner */
	if (!pcRawData)
		goto error2;

	/* neglect multiple spaces for content parsing */
	pcRawData += strspn(pcRawData, " ");

	/* decimal point existence check */
	if (!strchr(pcRawData, '.'))
		*pu1Value = (UINT8)os_str_tol(pcRawData, 0, 10) * 2;
	else {
		/* backup pointer to string of parser function */
		cBuffer = __rstrtok;

		/* parse integer part */
		cToken = rstrtok(pcRawData, ".");

		/* sanity check for null pointer */
		if (!cToken)
			goto error3;

		/* transform integer part unit to (0.5) */
		*pu1Value = (UINT8)os_str_tol(cToken, 0, 10) * 2;

		/* parse non-integer part */
		cToken = rstrtok(NULL, ".");

		/* sanity check for null pointer */
		if (!cToken)
			goto error4;

		/* get non-integer part */
		u1NonInteValue = (UINT8)os_str_tol(cToken, 0, 10);

		/* increment for non-zero non-integer part */
		if (u1NonInteValue >= 5)
			(*pu1Value) += 1;

		/* backup pointer to string of parser function */
		__rstrtok = cBuffer;
	}

	return NDIS_STATUS_SUCCESS;

error1:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR,
		"null pointer for buffer to update transform result !!\n");
	return NDIS_STATUS_FAILURE;

error2:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR,
		"null pointer for raw data buffer !!\n");
	return NDIS_STATUS_FAILURE;

error3:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR,
		"null pointer for integer value parsing !!\n");
	return NDIS_STATUS_FAILURE;

error4:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR,
		"null pointer for decimal value parsing !!\n");
	return NDIS_STATUS_FAILURE;
}

CHAR SKUTxPwrOffsetGet(RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT8 ucBW, UINT8 ucPhymode, UINT8 ucMCS, UINT8 ucNss, BOOLEAN fgSE)
{
	CHAR   cPowerOffset = 0;
	UINT8  ucRateOffset = 0;
	UINT8  ucNSS = 1;
	UINT8  BW_OFFSET[4] = {VHT20_OFFSET, VHT40_OFFSET, VHT80_OFFSET, VHT160C_OFFSET};
#ifdef CONFIG_ATE
	struct	_ATE_CTRL	*ATECtrl = &(pAd->ATECtrl);
#endif

	/* Compute MCS rate and Nss for HT mode */
	if ((ucPhymode == MODE_HTMIX) || (ucPhymode == MODE_HTGREENFIELD)) {
		ucNss = (ucMCS >> 3) + 1;
		ucMCS &= 0x7;
	}

	switch (ucPhymode) {
	case MODE_CCK:
		ucRateOffset = SKU_CCK_OFFSET;

		switch (ucMCS) {
		case MCS_0:
		case MCS_1:
			ucRateOffset = SKU_CCK_RATE_M01;
			break;

		case MCS_2:
		case MCS_3:
			ucRateOffset = SKU_CCK_RATE_M23;
			break;

		default:
			break;
		}

		break;

	case MODE_OFDM:
		ucRateOffset = SKU_OFDM_OFFSET;

		switch (ucMCS) {
		case MCS_0:
		case MCS_1:
			ucRateOffset = SKU_OFDM_RATE_M01;
			break;

		case MCS_2:
		case MCS_3:
			ucRateOffset = SKU_OFDM_RATE_M23;
			break;

		case MCS_4:
		case MCS_5:
			ucRateOffset = SKU_OFDM_RATE_M45;
			break;

		case MCS_6:
			ucRateOffset = SKU_OFDM_RATE_M6;
			break;

		case MCS_7:
			ucRateOffset = SKU_OFDM_RATE_M7;
			break;

		default:
			break;
		}

		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		ucRateOffset = SKU_HT_OFFSET + BW_OFFSET[ucBW];

		switch (ucMCS) {
		case MCS_0:
			ucRateOffset += SKU_HT_RATE_M0;
			break;

		case MCS_1:
		case MCS_2:
			ucRateOffset += SKU_HT_RATE_M12;
			break;

		case MCS_3:
		case MCS_4:
			ucRateOffset += SKU_HT_RATE_M34;
			break;

		case MCS_5:
			ucRateOffset += SKU_HT_RATE_M5;
			break;

		case MCS_6:
			ucRateOffset += SKU_HT_RATE_M6;
			break;

		case MCS_7:
			ucRateOffset += SKU_HT_RATE_M7;
			break;
		}

		break;

	case MODE_VHT:
		ucRateOffset = SKU_VHT_OFFSET + BW_OFFSET[ucBW];

		switch (ucMCS) {
		case MCS_0:
			ucRateOffset += SKU_VHT_RATE_M0;
			break;

		case MCS_1:
		case MCS_2:
			ucRateOffset += SKU_VHT_RATE_M12;
			break;

		case MCS_3:
		case MCS_4:
			ucRateOffset += SKU_VHT_RATE_M34;
			break;

		case MCS_5:
		case MCS_6:
			ucRateOffset += SKU_VHT_RATE_M56;
			break;

		case MCS_7:
			ucRateOffset += SKU_VHT_RATE_M7;
			break;

		case MCS_8:
			ucRateOffset += SKU_VHT_RATE_M8;
			break;

		case MCS_9:
			ucRateOffset += SKU_VHT_RATE_M9;
			break;

		default:
			break;
		}

		break;
	}

	/* Update Power offset by look up Tx Power Compensation Table */
	cPowerOffset = (fgSE) ? (pAd->CommonCfg.cTxPowerCompBackup[ucRateOffset][ucNSS - 1])
				: (pAd->CommonCfg.cTxPowerCompBackup[ucRateOffset][3]);

	/* Debug log for SKU Power offset to compensate */
	MTWF_DBG(NULL, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_INFO,
		KBLU "ucBW: %d, ucPhymode: %d, ucMCS: %d, ucNss: %d, fgSPE: %d !!!\n"
		KNRM, ucBW, ucPhymode, ucMCS,
		ucNss, fgSE);
	MTWF_DBG(NULL, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_INFO,
		KBLU "cPowerOffset: 0x%x (%d) !!!\n"
		KNRM, cPowerOffset, cPowerOffset);

#ifdef CONFIG_ATE
	/* Check if Single SKU is disabled */
	if (!ATECtrl->tx_pwr_sku_en)
		cPowerOffset = 0;
#endif
	return cPowerOffset;
}

VOID MtPwrLimitTblChProc(RTMP_ADAPTER *pAd, UINT8 u1BandIdx, UINT8 u1ChannelBand, UINT8 u1ControlChannel, UINT8 u1CentralChannel)
{
	UINT8 u1Type;

	for (u1Type = POWER_LIMIT_TABLE_TYPE_SKU; u1Type < POWER_LIMIT_TABLE_TYPE_NUM; u1Type++) {
		if (pAd->fgPwrLimitRead[u1Type])
			MtCmdPwrLimitTblUpdate(pAd, u1BandIdx, u1Type, u1ChannelBand, u1ControlChannel, u1CentralChannel);
	}
}

NDIS_STATUS MtPwrFillLimitParam(RTMP_ADAPTER *pAd, UINT8 ChBand, UINT8 u1ControlChannel,
				UINT8 u1CentralChannel, VOID *pi1PwrLimitParam, UINT8 u1Type)
{
	UINT8 u1ChListIdx;
	UINT8 u1PwrLimitChannel;
	UINT8 rowNum = 0;
	UINT8 sku_tbl_idx = 0;
	struct pwr_table_list *pTbl = NULL;
#if defined(CONFIG_6G_SUPPORT) && defined(CONFIG_6G_AFC_SUPPORT) && defined(DOT11_HE_AX)
	UINT16 u2ParamIdx = 0, u2TblTypeLengh = 0;
	INT8 i1PwrValue = 0;
	UINT8 u1AfcChannelIdx;
#endif /*CONFIG_6G_SUPPORT &&*/
	UINT8 PsdLimit = CheckPSDLimitType(pAd);
	UINT16 sku_1_update_idx, sku_2_bf_off_update_idx, sku_2_bf_on_update_idx;
	UINT8 mcs_idx, bo_idx;

	/* sanity check for null pointer */
	if (!pi1PwrLimitParam)
		goto error0;

	/* query sku table index */
	chip_get_sku_tbl_idx(pAd, &sku_tbl_idx);
	if (sku_tbl_idx >= TABLE_SIZE)
		goto error1;

#if defined(CONFIG_6G_SUPPORT) && defined(CONFIG_6G_AFC_SUPPORT) && defined(DOT11_HE_AX)
		if (pAd->afc_ctrl.AfcStateMachine.CurrState == AFC_RUN) {
			MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR,
						"Device is standard power\n");

			sku_tbl_idx = AFC_STD_PWR_SKUTABLE_IDX;
		}
#endif /*CONFIG_6G_SUPPORT &&*/

	switch (u1Type) {
		case POWER_LIMIT_TABLE_TYPE_SKU:
			pTbl = pcptrSkuTbl[sku_tbl_idx];
			rowNum = ARRAY_SIZE(Sku_01);
			break;
		case POWER_LIMIT_TABLE_TYPE_BACKOFF:
			pTbl = pcptrBackoffTbl[sku_tbl_idx];
			rowNum = ARRAY_SIZE(Backoff_01);
			break;
		default:
			rowNum = 0;
			MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_WARN,
				"unknown power limit type\n");
			break;
	}

	/* Check G-Band/A-Band and power limit channel */
	u1PwrLimitChannel = (ChBand) ? (u1ControlChannel) : (u1CentralChannel);

	/* search for specific channel */
	for (u1ChListIdx = 0; u1ChListIdx < rowNum; u1ChListIdx++) {
		MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_INFO,
			"u1Pwrch:%u cntch:%u cench:%u chband:%u lisIdx:%u tblband:%u tblcha:%u\n",
			u1PwrLimitChannel
			, u1ControlChannel, u1CentralChannel, ChBand, u1ChListIdx, pTbl[u1ChListIdx].chBand, pTbl[u1ChListIdx].channel);
		/* check Channel Band and Channel */
		if ((ChBand == pTbl[u1ChListIdx].chBand) && (u1PwrLimitChannel == pTbl[u1ChListIdx].channel)) {

			if (u1Type == POWER_LIMIT_TABLE_TYPE_SKU) {
				memmove(pi1PwrLimitParam, pTbl[u1ChListIdx].pwrLimit,
					SINGLE_SKU_PARAM_NUM_V1);
			} else if (u1Type == POWER_LIMIT_TABLE_TYPE_BACKOFF) {
				memmove(pi1PwrLimitParam, pcptrBackoffTbl[sku_tbl_idx][u1ChListIdx].pwrLimit,
					BACKOFF_PARAM_NUM_V1);
			}

			/* stop channel list search loop */
			break;
		}
	}

#ifdef CONFIG_6G_AFC_SUPPORT
	if (ChBand == 2 && (pAd->afc_ctrl.AfcStateMachine.CurrState == AFC_RUN)){
		u1AfcChannelIdx = afc_get_channel_index(u1PwrLimitChannel);
		if (u1AfcChannelIdx == AFC_INVALID_CH_INDEX) {
			MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR,
			"AFC channel is not available !!\n");
			return NDIS_STATUS_FAILURE;
		}
		if (u1Type == POWER_LIMIT_TABLE_TYPE_SKU)
			u2TblTypeLengh = SINGLE_SKU_PARAM_NUM_V1;
		else if (u1Type == POWER_LIMIT_TABLE_TYPE_BACKOFF)
			u2TblTypeLengh = BACKOFF_PARAM_NUM_V1;
		for (u2ParamIdx = 0; u2ParamIdx < u2TblTypeLengh; u2ParamIdx++) {
			i1PwrValue = afc_pwr_calculation(pAd, u2ParamIdx, u1AfcChannelIdx, *((INT8 *)pi1PwrLimitParam + u2ParamIdx), u1Type);
			*((INT8 *)pi1PwrLimitParam + u2ParamIdx) = i1PwrValue;
		}
	} else
#endif /*CONFIG_6G_SUPPORT &&*/
	{
	if (pAd->CommonCfg.SKU_DUP_Patch_enable) {
		/* Duplicate mode patch */
		/* Step 1. Get current operating BW. */
		/* if (6G && 6G BW320 center channel)
		   else if (6G && 6G BW160 center channel) || (5G && 5G BW160 center channel)
		   else if (6G && 6G BW80 center channel) || (5G && 5G BW80 center channel)
		   else if (u1ControlChannel != u1CentralChannel) => BW40
		   else => BW20
		*/
		if ((ChBand == 2) &&
			(u1CentralChannel == 31 || u1CentralChannel == 63
			|| u1CentralChannel == 95 || u1CentralChannel == 127
			|| u1CentralChannel == 159 || u1CentralChannel == 191)) {
			/* BW_320 */
			sku_1_update_idx = SKU_1_EHT996X4_IDX;
			sku_2_bf_off_update_idx = SKU_2_BW320_BF_OFF_IDX;
			sku_2_bf_on_update_idx = SKU_2_BW320_BF_ON_IDX;
		} else if (((ChBand == 2) &&
			(u1CentralChannel == 15 || u1CentralChannel == 47
			|| u1CentralChannel == 79 || u1CentralChannel == 111
			|| u1CentralChannel == 143 || u1CentralChannel == 175
			|| u1CentralChannel == 207))
			|| ((ChBand == 1) &&
			(u1CentralChannel == 50 || u1CentralChannel == 114 || u1CentralChannel == 163))) {
			/* BW_160 */
			sku_1_update_idx = SKU_1_EHT996X2_IDX;
			sku_2_bf_off_update_idx = SKU_2_BW160_BF_OFF_IDX;
			sku_2_bf_on_update_idx = SKU_2_BW160_BF_ON_IDX;
		} else if (((ChBand == 2) &&
			(u1CentralChannel == 7 || u1CentralChannel == 23 || u1CentralChannel == 39
			|| u1CentralChannel == 55 || u1CentralChannel == 71 || u1CentralChannel == 87
			|| u1CentralChannel == 103 || u1CentralChannel == 119 || u1CentralChannel == 135
			|| u1CentralChannel == 151 || u1CentralChannel == 167 || u1CentralChannel == 183
			|| u1CentralChannel == 199 || u1CentralChannel == 215))
			|| ((ChBand == 1) &&
			(u1CentralChannel == 42 || u1CentralChannel == 58 || u1CentralChannel == 106
			|| u1CentralChannel == 122 || u1CentralChannel == 138 || u1CentralChannel == 155
			|| u1CentralChannel == 171))) {
			/* BW_80 */
			sku_1_update_idx = SKU_1_EHT996_IDX;
			sku_2_bf_off_update_idx = SKU_2_BW80_BF_OFF_IDX;
			sku_2_bf_on_update_idx = SKU_2_BW80_BF_ON_IDX;
		} else if (u1ControlChannel != u1CentralChannel) {
			/* BW_40 */
			sku_1_update_idx = SKU_1_EHT484_IDX;
			sku_2_bf_off_update_idx = SKU_2_BW40_BF_OFF_IDX;
			sku_2_bf_on_update_idx = SKU_2_BW40_BF_ON_IDX;
		} else {
			/* BW_20 */
			sku_1_update_idx = SKU_1_EHT242_IDX;
			sku_2_bf_off_update_idx = SKU_2_BW20_BF_OFF_IDX;
			sku_2_bf_on_update_idx = SKU_2_BW20_BF_ON_IDX;
		}

		/* Duplicate mode patch */
		/* 2. Update SKU_1 & SKU_2 */
		/* Spectial Case Handle: if (6G && PDS_LIMIT) => do not update SKU_1 */
		if ((u1Type == POWER_LIMIT_TABLE_TYPE_SKU) && !((ChBand == 2) && (PsdLimit == 1))) {
			for (mcs_idx = 1; mcs_idx < SINGLE_SKU_TABLE_OFDM_LENGTH; mcs_idx++)
				*((INT8 *)pi1PwrLimitParam + SKU_1_OFDM_IDX + mcs_idx)
					= *((INT8 *)pi1PwrLimitParam + sku_1_update_idx + mcs_idx);
		} else if (u1Type == POWER_LIMIT_TABLE_TYPE_BACKOFF) {
			for (bo_idx = 0; bo_idx < BF_BACKOFF_OFDM_OFF_LENGTH; bo_idx++)
				*((INT8 *)pi1PwrLimitParam + SKU_2_OFDM_BF_OFF_IDX + bo_idx)
					= *((INT8 *)pi1PwrLimitParam + sku_2_bf_off_update_idx + bo_idx);
			for (bo_idx = 0; bo_idx < BF_BACKOFF_OFDM_ON_LENGTH; bo_idx++)
				*((INT8 *)pi1PwrLimitParam + SKU_2_OFDM_BF_ON_IDX + bo_idx)
					= *((INT8 *)pi1PwrLimitParam + sku_2_bf_on_update_idx + 1 + bo_idx);
		}

		/* Duplicate mode patch */
		/* 3. Special Case Handle for (2G && BW40)  */
		/* Refill by Using Control channel's table value*/
		if ((ChBand == 0) && (u1ControlChannel != u1CentralChannel)) {
			u1PwrLimitChannel = u1ControlChannel;
			for (u1ChListIdx = 0; u1ChListIdx < rowNum; u1ChListIdx++) {
				/* check Channel Band and Channel */
				if ((ChBand == pTbl[u1ChListIdx].chBand)
					&& (u1PwrLimitChannel == pTbl[u1ChListIdx].channel)
					&& (u1Type == POWER_LIMIT_TABLE_TYPE_SKU)) {
					/* Update CCK, HT20, VHT20, HE242 and EHT242 in SKU_1.*/
					for (mcs_idx = 0; mcs_idx < SINGLE_SKU_TABLE_CCK_LENGTH; mcs_idx++)
						*((INT8 *)pi1PwrLimitParam + SKU_1_CCK_IDX + mcs_idx)
							= pTbl[u1ChListIdx].pwrLimit[SKU_1_CCK_IDX + mcs_idx];
					for (mcs_idx = 0; mcs_idx < SINGLE_SKU_TABLE_HT_20_LENGTH; mcs_idx++)
						*((INT8 *)pi1PwrLimitParam + SKU_1_HT20_IDX + mcs_idx)
							= pTbl[u1ChListIdx].pwrLimit[SKU_1_HT20_IDX + mcs_idx];
					for (mcs_idx = 0; mcs_idx < SINGLE_SKU_TABLE_VHT_LENGTH; mcs_idx++)
						*((INT8 *)pi1PwrLimitParam + SKU_1_VHT20_IDX + mcs_idx)
							= pTbl[u1ChListIdx].pwrLimit[SKU_1_VHT20_IDX + mcs_idx];
					for (mcs_idx = 0; mcs_idx < SINGLE_SKU_TABLE_HE_LENGTH; mcs_idx++)
						*((INT8 *)pi1PwrLimitParam + SKU_1_HE242_IDX + mcs_idx)
							= pTbl[u1ChListIdx].pwrLimit[SKU_1_HE242_IDX + mcs_idx];
					for (mcs_idx = 0; mcs_idx < SINGLE_SKU_TABLE_EHT_LENGTH; mcs_idx++)
						*((INT8 *)pi1PwrLimitParam + SKU_1_EHT242_IDX + mcs_idx)
							= pTbl[u1ChListIdx].pwrLimit[SKU_1_EHT242_IDX + mcs_idx];
					/* stop channel list search loop */
					break;
				} else if ((ChBand == pTbl[u1ChListIdx].chBand)
					&& (u1PwrLimitChannel == pTbl[u1ChListIdx].channel)
					&& (u1Type == POWER_LIMIT_TABLE_TYPE_BACKOFF)) {
					/* Update CCK and BW20 in SKU_2. */
					for (bo_idx = 0; bo_idx < BF_BACKOFF_CCK_LENGTH; bo_idx++)
						*((INT8 *)pi1PwrLimitParam + SKU_2_CCK_BF_OFF_IDX + bo_idx)
							= pTbl[u1ChListIdx].pwrLimit[SKU_2_CCK_BF_OFF_IDX + bo_idx];
					for (bo_idx = 0; bo_idx < BF_BACKOFF_LENGTH; bo_idx++) {
						*((INT8 *)pi1PwrLimitParam + SKU_2_BW20_BF_OFF_IDX + bo_idx)
							= pTbl[u1ChListIdx].pwrLimit[SKU_2_BW20_BF_OFF_IDX + bo_idx];
						*((INT8 *)pi1PwrLimitParam + SKU_2_BW20_BF_ON_IDX + bo_idx)
							= pTbl[u1ChListIdx].pwrLimit[SKU_2_BW20_BF_ON_IDX + bo_idx];
					}
					/* stop channel list search loop */
					break;
				}

			}
		}
	}
	}
	return NDIS_STATUS_SUCCESS;

error0:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR,
		"null pointer for buffer to fill power limit table !!\n");
	return NDIS_STATUS_FAILURE;
error1:
	MTWF_DBG(pAd, DBG_CAT_POWER, CATPOWER_SKU, DBG_LVL_ERROR,
		"invalid sku table index !!\n");
	return NDIS_STATUS_FAILURE;

}

#ifdef SINGLE_SKU_V2
NDIS_STATUS RTMPSetSkuParam(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS ret;

	ret = MtPwrLimitLoadParamHandle(pAd, POWER_LIMIT_TABLE_TYPE_SKU);
	return ret;
}

NDIS_STATUS RTMPSetBackOffParam(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS ret;

	ret = MtPwrLimitLoadParamHandle(pAd, POWER_LIMIT_TABLE_TYPE_BACKOFF);
	return ret;
}

NDIS_STATUS RTMPResetSkuParam(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;

	ret = MtPwrLimitUnloadParamHandle(pAd, POWER_LIMIT_TABLE_TYPE_SKU);
	return ret;
}

NDIS_STATUS RTMPResetBackOffParam(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;

	ret = MtPwrLimitUnloadParamHandle(pAd, POWER_LIMIT_TABLE_TYPE_BACKOFF);
	return ret;
}

#define	SKU_PHYMODE_CCK_1M_2M				0
#define	SKU_PHYMODE_CCK_5M_11M				1
#define	SKU_PHYMODE_OFDM_6M_9M				2
#define	SKU_PHYMODE_OFDM_12M_18M			3
#define	SKU_PHYMODE_OFDM_24M_36M			4
#define	SKU_PHYMODE_OFDM_48M_54M			5
#define	SKU_PHYMODE_HT_MCS0_MCS1			6
#define	SKU_PHYMODE_HT_MCS2_MCS3			7
#define	SKU_PHYMODE_HT_MCS4_MCS5			8
#define	SKU_PHYMODE_HT_MCS6_MCS7			9
#define	SKU_PHYMODE_HT_MCS8_MCS9			10
#define	SKU_PHYMODE_HT_MCS10_MCS11			11
#define	SKU_PHYMODE_HT_MCS12_MCS13			12
#define	SKU_PHYMODE_HT_MCS14_MCS15			13
#define	SKU_PHYMODE_STBC_MCS0_MCS1			14
#define	SKU_PHYMODE_STBC_MCS2_MCS3			15
#define	SKU_PHYMODE_STBC_MCS4_MCS5			16
#define	SKU_PHYMODE_STBC_MCS6_MCS7			17


VOID InitSkuRateDiffTable(
	IN PRTMP_ADAPTER	pAd)
{
	USHORT		i, value = 0;
	CHAR		BasePwr, Pwr;

	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 4, value);
	BasePwr = (value >> 8) & 0xFF;
	BasePwr = (BasePwr > 0x1F) ? BasePwr - 0x40 : BasePwr;

	for (i = 0; i < 9; i++) {
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i * 2, value);
		Pwr = value & 0xFF;
		Pwr = (Pwr > 0x1F) ? Pwr - 0x40 : Pwr;
		pAd->SingleSkuRatePwrDiff[i * 2] = Pwr - BasePwr;
		Pwr = (value >> 8) & 0xFF;
		Pwr = (Pwr > 0x1F) ? Pwr - 0x40 : Pwr;
		pAd->SingleSkuRatePwrDiff[i * 2 + 1] = Pwr - BasePwr;
	}
}
#endif /* SINGLE_SKU_V2 */


