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
	cmm_txbf_cal_mt.c
*/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Cmm_txbf_cal_mt.h"
#endif
#elif defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif

#ifdef TXBF_SUPPORT

UINT_16 au2IBfCalEEPROMOffset[9] = {
	IBF_LNA_PHASE_G0_ADDR,
	IBF_LNA_PHASE_G1_ADDR,
	IBF_LNA_PHASE_G2_ADDR,
	IBF_LNA_PHASE_G3_ADDR,
	IBF_LNA_PHASE_G4_ADDR,
	IBF_LNA_PHASE_G5_ADDR,
	IBF_LNA_PHASE_G6_ADDR,
	IBF_LNA_PHASE_G7_ADDR,
	IBF_LNA_PHASE_G8_ADDR
};

VOID E2pMemWrite(IN PRTMP_ADAPTER pAd,
		 IN UINT16 u2MemAddr,
		 IN UCHAR  ucInput_L,
		 IN UCHAR  ucInput_H)
{
	UINT16 u2Value;

	if ((u2MemAddr & 0x1) != 0)
		u2Value = (ucInput_L << 8) | ucInput_H;
	else
		u2Value = (ucInput_H << 8) | ucInput_L;

	RT28xx_EEPROM_WRITE16(pAd, u2MemAddr, u2Value);
}

VOID eBFPfmuMemAlloc(IN PRTMP_ADAPTER pAd, IN PCHAR pPfmuMemRow, IN PCHAR pPfmuMemCol)
{
	pPfmuMemRow[0] = 0;
	pPfmuMemRow[1] = 1;
	pPfmuMemRow[2] = 2;
	pPfmuMemRow[3] = 3;

	pPfmuMemCol[0] = 0;
	pPfmuMemCol[1] = 0;
	pPfmuMemCol[2] = 0;
	pPfmuMemCol[3] = 0;
}

VOID iBFPfmuMemAlloc(IN PRTMP_ADAPTER pAd, IN PCHAR pPfmuMemRow, IN PCHAR pPfmuMemCol)
{
	pPfmuMemRow[0] = 4;
	pPfmuMemRow[1] = 5;
	pPfmuMemRow[2] = 6;
	pPfmuMemRow[3] = 7;

	pPfmuMemCol[0] = 0;
	pPfmuMemCol[1] = 0;
	pPfmuMemCol[2] = 0;
	pPfmuMemCol[3] = 0;
}

VOID iBFPhaseComp(IN PRTMP_ADAPTER pAd, IN UCHAR ucGroup, IN PCHAR pCmdBuf)
{
	UINT_8 u1IbfCalPhaseStructLenG0 = 0;
	UINT_8 u1IbfCalPhaseStructLenGx = 0;


	u1IbfCalPhaseStructLenG0 = sizeof(struct IBF_PHASE_G0_T);
	u1IbfCalPhaseStructLenGx = sizeof(struct IBF_PHASE_Gx_T);

	if (!pAd) {
		if (!pCmdBuf)
			return;

		NdisZeroMemory(pCmdBuf, u1IbfCalPhaseStructLenGx);
		return;
	}

	if (ucGroup == 0)
		os_move_mem(pCmdBuf, pAd->piBfPhaseG0, u1IbfCalPhaseStructLenG0);
	else
		os_move_mem(pCmdBuf, &pAd->piBfPhaseGx[(ucGroup - 1) * u1IbfCalPhaseStructLenGx], u1IbfCalPhaseStructLenGx);

}

VOID iBFPhaseFreeMem(IN PRTMP_ADAPTER pAd)
{
	/* Group 0 */
	if (pAd->piBfPhaseG0 != NULL) {
		os_free_mem(pAd->piBfPhaseG0);
	}

	/* Group 1 ~ 8 */
	if (pAd->piBfPhaseGx != NULL) {
		os_free_mem(pAd->piBfPhaseGx);
	}
}

VOID iBFPhaseCalInit(IN PRTMP_ADAPTER pAd)
{
	UINT_8 u1IbfCalPhaseStructLenG0 = 0;
	UINT_8 u1IbfCalPhaseStructLenGx = 0;


	u1IbfCalPhaseStructLenG0 = sizeof(struct IBF_PHASE_G0_T);
	u1IbfCalPhaseStructLenGx = sizeof(struct IBF_PHASE_Gx_T);

	/* Free memory allocated by iBF phase calibration */
	iBFPhaseFreeMem(pAd);

	os_alloc_mem(pAd, (PUCHAR *)&pAd->piBfPhaseG0, u1IbfCalPhaseStructLenG0);
	os_alloc_mem(pAd, (PUCHAR *)&pAd->piBfPhaseGx, u1IbfCalPhaseStructLenGx * 8);

	if ((pAd->piBfPhaseG0 == NULL) || (pAd->piBfPhaseGx == NULL)) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
				"Fail to allocate memory!\n");
		return;
	}

	NdisZeroMemory(pAd->piBfPhaseG0, u1IbfCalPhaseStructLenG0);
	NdisZeroMemory(pAd->piBfPhaseGx, u1IbfCalPhaseStructLenGx * 8);

	pAd->fgCalibrationFail = FALSE;
	NdisZeroMemory(&pAd->fgGroupIdPassFailStatus[0], 9);
}

VOID iBFPhaseCalE2PInit(IN PRTMP_ADAPTER pAd)
{
	UINT_16 u2BfE2pOccupiedSize;
	UINT_8 u1IbfCalPhaseStructLenG0 = 0;
	UINT_8 u1IbfCalPhaseStructLenGx = 0;


	u1IbfCalPhaseStructLenG0 = sizeof(struct IBF_PHASE_G0_T);
	u1IbfCalPhaseStructLenGx = sizeof(struct IBF_PHASE_Gx_T);

	/* Group 0 */
	u2BfE2pOccupiedSize = u1IbfCalPhaseStructLenG0;
	RT28xx_EEPROM_READ_WITH_RANGE(
		pAd, au2IBfCalEEPROMOffset[GROUP_0], u2BfE2pOccupiedSize, pAd->piBfPhaseG0);

	/* Group 1 ~ 7 */
	u2BfE2pOccupiedSize = (u1IbfCalPhaseStructLenGx * 7);
	RT28xx_EEPROM_READ_WITH_RANGE(
		pAd, au2IBfCalEEPROMOffset[GROUP_1], u2BfE2pOccupiedSize, pAd->piBfPhaseGx);

	/* Group 8 */
	u2BfE2pOccupiedSize = u1IbfCalPhaseStructLenGx;
	RT28xx_EEPROM_READ_WITH_RANGE(
		pAd, au2IBfCalEEPROMOffset[GROUP_8], u2BfE2pOccupiedSize,
		(PUCHAR)&pAd->piBfPhaseGx[7 * u1IbfCalPhaseStructLenGx]);

	pAd->fgCalibrationFail = FALSE;
	NdisZeroMemory(&pAd->fgGroupIdPassFailStatus[0], 9);
}

VOID iBFPhaseCalE2PUpdate(IN PRTMP_ADAPTER pAd,
			  IN UCHAR   ucGroup,
			  IN UINT8   fgSX2,
			  IN UCHAR   ucUpdateAllTye)
{
	UCHAR  ucGroupIdx, ucEndLoop;
	UCHAR  ucCounter;
	UINT_8 u1IbfCalPhaseStructLenG0 = 0;
	UINT_8 u1IbfCalPhaseStructLenGx = 0;
	struct IBF_PHASE_Gx_T iBfPhaseGx;
	struct IBF_PHASE_G0_T iBfPhaseG0;

#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test = NULL;


	if (!pAd)
		return;

	serv_test = (struct service_test *)pAd->serv.serv_handle;
#endif

	u1IbfCalPhaseStructLenG0 = sizeof(struct IBF_PHASE_G0_T);
	u1IbfCalPhaseStructLenGx = sizeof(struct IBF_PHASE_Gx_T);
	NdisZeroMemory(&iBfPhaseG0, u1IbfCalPhaseStructLenG0);
	NdisZeroMemory(&iBfPhaseGx, u1IbfCalPhaseStructLenGx);

	/* uint16_t u2Value; */
	/* IF phase calibration is for BW20/40/80/160 */
	ucGroupIdx = 0;

	switch (ucUpdateAllTye) {
	case IBF_PHASE_ONE_GROUP_UPDATE:
		ucGroupIdx = ucGroup - 1;
		ucEndLoop  = 0;

		if (ucGroup == GROUP_0) {
			RT28xx_EEPROM_WRITE_WITH_RANGE(pAd,
				au2IBfCalEEPROMOffset[ucGroup],
				u1IbfCalPhaseStructLenG0,
				pAd->piBfPhaseG0);
			NdisCopyMemory(&iBfPhaseG0,
				pAd->piBfPhaseG0,
				u1IbfCalPhaseStructLenG0);
		} else {
			RT28xx_EEPROM_WRITE_WITH_RANGE(pAd,
				au2IBfCalEEPROMOffset[ucGroup],
				u1IbfCalPhaseStructLenGx,
				&pAd->piBfPhaseGx[
					ucGroupIdx * u1IbfCalPhaseStructLenGx]);
			NdisCopyMemory(&iBfPhaseGx,
				&pAd->piBfPhaseGx[
					ucGroupIdx * u1IbfCalPhaseStructLenGx],
					u1IbfCalPhaseStructLenGx);
		}

		if (ucGroup == GROUP_0) {
			MTWF_PRINT("G%d and Group_M\n"
				"G%d_M_T0_H = %d\n G%d_M_T1_H = %d\n",
				ucGroup,
				ucGroup, iBfPhaseG0.ucG0_M_T0_H,
				ucGroup, iBfPhaseG0.ucG0_M_T1_H);

			MTWF_PRINT("G%d_M_T2_H = %d\n",
				ucGroup, iBfPhaseG0.ucG0_M_T2_H);

			MTWF_PRINT("G%d_R0_UH = %d\n G%d_R0_H = %d\n"
				"G%d_R0_M = %d\n G%d_R0_L = %d\n"
				"G%d_R1_UH = %d\n G%d_R1_H = %d\n"
				"G%d_R1_M = %d\n G%d_R1_L = %d\n",
				ucGroup, iBfPhaseG0.ucG0_R0_UH,
				ucGroup, iBfPhaseG0.ucG0_R0_H,
				ucGroup, iBfPhaseG0.ucG0_R0_M,
				ucGroup, iBfPhaseG0.ucG0_R0_L,
				ucGroup, iBfPhaseG0.ucG0_R1_UH,
				ucGroup, iBfPhaseG0.ucG0_R1_H,
				ucGroup, iBfPhaseG0.ucG0_R1_M,
				ucGroup, iBfPhaseG0.ucG0_R1_L);

			MTWF_PRINT("G%d_R2_UH = %d\n G%d_R2_H = %d\n"
				"G%d_R2_M = %d\n G%d_R2_L = %d\n",
				ucGroup, iBfPhaseG0.ucG0_R2_UH,
				ucGroup, iBfPhaseG0.ucG0_R2_H,
				ucGroup, iBfPhaseG0.ucG0_R2_M,
				ucGroup, iBfPhaseG0.ucG0_R2_L);

			MTWF_PRINT("G%d_R3_UH = %d\n G%d_R3_H = %d\n"
				"G%d_R3_M = %d\n"
				"G%d_R3_L = %d \n G%d_R3_UL = %d \n",
				ucGroup, iBfPhaseG0.ucG0_R3_UH,
				ucGroup, iBfPhaseG0.ucG0_R3_H,
				ucGroup, iBfPhaseG0.ucG0_R3_M,
				ucGroup, iBfPhaseG0.ucG0_R3_L,
				ucGroup, iBfPhaseG0.ucG0_R3_UL);

		} else {
			MTWF_PRINT("G%d and Group_M\n"
				"G%d_M_T0_H = %d\n G%d_M_T1_H = %d\n",
				ucGroup,
				ucGroup, iBfPhaseGx.ucGx_M_T0_H,
				ucGroup, iBfPhaseGx.ucGx_M_T1_H);

			MTWF_PRINT("G%d_M_T2_H = %d\n",
				ucGroup, iBfPhaseGx.ucGx_M_T2_H);

			MTWF_PRINT("G%d_R0_UH = %d\n G%d_R0_H = %d\n"
				"G%d_R0_M = %d\n G%d_R0_L = %d\n"
				"G%d_R1_UH = %d\n G%d_R1_H = %d\n"
				"G%d_R1_M = %d\n G%d_R1_L = %d\n",
				ucGroup, iBfPhaseGx.ucGx_R0_UH,
				ucGroup, iBfPhaseGx.ucGx_R0_H,
				ucGroup, iBfPhaseGx.ucGx_R0_M,
				ucGroup, iBfPhaseGx.ucGx_R0_L,
				ucGroup, iBfPhaseGx.ucGx_R1_UH,
				ucGroup, iBfPhaseGx.ucGx_R1_H,
				ucGroup, iBfPhaseGx.ucGx_R1_M,
				ucGroup, iBfPhaseGx.ucGx_R1_L);
		}

		MTWF_PRINT("G%d_R2_UH = %d\n G%d_R2_H = %d\n"
			"G%d_R2_M = %d\n G%d_R2_L = %d\n",
			ucGroup, iBfPhaseGx.ucGx_R2_UH,
			ucGroup, iBfPhaseGx.ucGx_R2_H,
			ucGroup, iBfPhaseGx.ucGx_R2_M,
			ucGroup, iBfPhaseGx.ucGx_R2_L);

		MTWF_PRINT("G%d_R3_UH = %d\n G%d_R3_H = %d\n"
			"G%d_R3_M = %d\n G%d_R3_L = %d \n G%d_R3_UL = %d \n",
			ucGroup, iBfPhaseGx.ucGx_R3_UH,
			ucGroup, iBfPhaseGx.ucGx_R3_H,
			ucGroup, iBfPhaseGx.ucGx_R3_M,
			ucGroup, iBfPhaseGx.ucGx_R3_L,
			ucGroup, iBfPhaseGx.ucGx_R3_UL);

		MTWF_PRINT("G%d_R3_UH = %d\n G%d_R3_H = %d\n"
			"G%d_R3_M = %d\n G%d_R3_L = %d \n G%d_R3_UL = %d \n",
			ucGroup, iBfPhaseGx.ucGx_R3_UH,
			ucGroup, iBfPhaseGx.ucGx_R3_H,
			ucGroup, iBfPhaseGx.ucGx_R3_M,
			ucGroup, iBfPhaseGx.ucGx_R3_L,
			ucGroup, iBfPhaseGx.ucGx_R3_UL);
		break;

	case IBF_PHASE_ALL_GROUP_UPDATE:
		if (pAd->fgCalibrationFail == FALSE) {
			MTWF_PRINT("All of groups can pass criterion and "
				"calibrated phases can be written into EEPROM\n"
				);
			if ((pAd->u1IbfCalPhase2G5GE2pClean == CLEAN_ALL) ||
				(pAd->u1IbfCalPhase2G5GE2pClean == CLEAN_2G)) {
				/* Write Group 0 into EEPROM */
				RT28xx_EEPROM_WRITE_WITH_RANGE(pAd,
					au2IBfCalEEPROMOffset[GROUP_0],
					u1IbfCalPhaseStructLenG0,
					pAd->piBfPhaseG0);
			}

			if ((pAd->u1IbfCalPhase2G5GE2pClean == CLEAN_ALL) ||
				(pAd->u1IbfCalPhase2G5GE2pClean == CLEAN_5G)) {
				/* Write Group 1 ~ 8 into EEPROM */
				RT28xx_EEPROM_WRITE_WITH_RANGE(pAd,
					au2IBfCalEEPROMOffset[GROUP_1],
					(u1IbfCalPhaseStructLenGx * 8),
					pAd->piBfPhaseGx);
			}
		} else {
			MTWF_PRINT("Calibrated phases can't be written into ");
			MTWF_PRINT("EEPROM because some groups can't pass ");
			MTWF_PRINT("criterion!!!\n");

			for (ucCounter = GROUP_0; ucCounter <= GROUP_8;
								ucCounter++) {
				MTWF_PRINT("Group%d = %s\n",
				ucCounter,
				(pAd->fgGroupIdPassFailStatus[ucCounter]
						== TRUE) ? "FAIL" : "PASS");
			}
		}

		break;

	case IBF_PHASE_ALL_GROUP_ERASE:
		NdisZeroMemory(pAd->piBfPhaseG0, u1IbfCalPhaseStructLenG0);
		NdisZeroMemory(pAd->piBfPhaseGx, u1IbfCalPhaseStructLenGx * 8);

		if ((pAd->u1IbfCalPhase2G5GE2pClean == CLEAN_ALL) ||
			(pAd->u1IbfCalPhase2G5GE2pClean == CLEAN_2G)) {
			RT28xx_EEPROM_WRITE_WITH_RANGE(pAd,
				au2IBfCalEEPROMOffset[GROUP_0],
				u1IbfCalPhaseStructLenG0,
				pAd->piBfPhaseG0);
		}

		if ((pAd->u1IbfCalPhase2G5GE2pClean == CLEAN_ALL) ||
			(pAd->u1IbfCalPhase2G5GE2pClean == CLEAN_5G)) {
			RT28xx_EEPROM_WRITE_WITH_RANGE(pAd,
				au2IBfCalEEPROMOffset[GROUP_1],
				(u1IbfCalPhaseStructLenGx * 8),
				pAd->piBfPhaseGx);
		}
		break;

	case IBF_PHASE_ALL_GROUP_READ_FROM_E2P:
		iBFPhaseCalE2PInit(pAd);
		break;

	default:
		ucGroupIdx = ucGroup - 1;
		ucEndLoop  = 0;

		NdisCopyMemory(&iBfPhaseGx,
		&pAd->piBfPhaseGx[ucGroupIdx * u1IbfCalPhaseStructLenGx],
		u1IbfCalPhaseStructLenGx);
		NdisCopyMemory(&iBfPhaseG0,
		pAd->piBfPhaseG0, u1IbfCalPhaseStructLenG0);
		break;
	}
}

VOID iBFPhaseCalReport(IN PRTMP_ADAPTER pAd,
		   IN UCHAR   ucGroupL_M_H,
		   IN UCHAR   ucGroup,
		   IN UCHAR   u1DbdcBandIdx,
		   IN UCHAR   ucStatus,
		   IN UCHAR   ucPhaseCalType,
		   IN PUCHAR  pBuf)
{
	UCHAR  ucGroupIdx;
	PUCHAR pucIBfPhaseG;
	UINT_8 u1IbfCalPhaseStructLenG0 = 0;
	UINT_8 u1IbfCalPhaseStructLenGx = 0;
	UINT_8 u1IbfPhaseOutStructLen   = 0;
	struct IBF_PHASE_OUT  iBfPhaseOut;
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test = NULL;
#endif

	if (pAd == NULL) {
		MTWF_PRINT("%s pAd NULL\n", __func__);
		return;
	}
#ifdef CONFIG_WLAN_SERVICE
	if (ucGroup == 0)
		pAd = physical_device_get_mac_adapter_by_band(pAd->physical_dev, 0);
	else
		pAd = physical_device_get_mac_adapter_by_band(pAd->physical_dev, 1);

	if (pAd == NULL) {
		MTWF_PRINT("%s pAd NULL\n", __func__);
		return;
	}

	serv_test = (struct service_test *)pAd->serv.serv_handle;
#endif
	u1IbfCalPhaseStructLenG0 = sizeof(struct IBF_PHASE_G0_T);
	u1IbfCalPhaseStructLenGx = sizeof(struct IBF_PHASE_Gx_T);
	u1IbfPhaseOutStructLen	 = sizeof(struct IBF_PHASE_OUT);

	MTWF_PRINT(" Calibrated iBF phases\n");
	pucIBfPhaseG = pBuf + u1IbfPhaseOutStructLen;
	NdisZeroMemory(&iBfPhaseOut, sizeof(struct IBF_PHASE_OUT));
	NdisCopyMemory(&iBfPhaseOut, pBuf, u1IbfPhaseOutStructLen);

	switch (ucPhaseCalType) {
	case IBF_PHASE_CAL_NOTHING: /* Do nothing */
		break;

	case IBF_PHASE_CAL_NORMAL_INSTRUMENT:
	case IBF_PHASE_CAL_NORMAL: /* Store calibrated phases with buffer mode */
		/* IF phase calibration is for BW20/40/80 */
		if (ucGroup == GROUP_0) {
			struct IBF_PHASE_G0_T *piBfPhaseG0;
			struct IBF_PHASE_Gx_T	iBfPhaseGx;

			piBfPhaseG0 = (struct IBF_PHASE_G0_T *)pAd->piBfPhaseG0;
			NdisCopyMemory(&iBfPhaseGx, pucIBfPhaseG, u1IbfCalPhaseStructLenGx);

			switch (ucGroupL_M_H) {
			case GROUP_L:
				break;

			case GROUP_M:
				piBfPhaseG0->ucG0_M_T0_H = iBfPhaseGx.ucGx_M_T0_H;
				piBfPhaseG0->ucG0_M_T1_H = iBfPhaseGx.ucGx_M_T1_H;
				piBfPhaseG0->ucG0_M_T2_H = iBfPhaseGx.ucGx_M_T2_H;
				piBfPhaseG0->ucG0_R0_UH  = iBfPhaseGx.ucGx_R0_UH;
				piBfPhaseG0->ucG0_R0_H   = iBfPhaseGx.ucGx_R0_H;
				piBfPhaseG0->ucG0_R0_M   = iBfPhaseGx.ucGx_R0_M;
				piBfPhaseG0->ucG0_R0_L   = iBfPhaseGx.ucGx_R0_L;
				piBfPhaseG0->ucG0_R1_UH  = iBfPhaseGx.ucGx_R1_UH;
				piBfPhaseG0->ucG0_R1_H   = iBfPhaseGx.ucGx_R1_H;
				piBfPhaseG0->ucG0_R1_M   = iBfPhaseGx.ucGx_R1_M;
				piBfPhaseG0->ucG0_R1_L   = iBfPhaseGx.ucGx_R1_L;
				piBfPhaseG0->ucG0_R2_UH  = iBfPhaseGx.ucGx_R2_UH;
				piBfPhaseG0->ucG0_R2_H   = iBfPhaseGx.ucGx_R2_H;
				piBfPhaseG0->ucG0_R2_M   = iBfPhaseGx.ucGx_R2_M;
				piBfPhaseG0->ucG0_R2_L   = iBfPhaseGx.ucGx_R2_L;
				piBfPhaseG0->ucG0_R3_UH  = iBfPhaseGx.ucGx_R3_UH;
				piBfPhaseG0->ucG0_R3_H	 = iBfPhaseGx.ucGx_R3_H;
				piBfPhaseG0->ucG0_R3_M	 = iBfPhaseGx.ucGx_R3_M;
				piBfPhaseG0->ucG0_R3_L	 = iBfPhaseGx.ucGx_R3_L;
				MTWF_PRINT("G0 and Group_M\n G0_M_T0_H = %d\n G0_M_T1_H = %d\n",
						 piBfPhaseG0->ucG0_M_T0_H,
						 piBfPhaseG0->ucG0_M_T1_H);

				MTWF_PRINT("G0_M_T2_H = %d\n",
						 piBfPhaseG0->ucG0_M_T2_H);

				MTWF_PRINT("G0_R0_UH = %d\n G0_R0_H = %d\n G0_R0_M = %d\n"
				" G0_R0_L = %d\n G0_R1_UH = %d\n G0_R1_H = %d\n"
				" G0_R1_M = %d\n G0_R1_L = %d\n",
						 piBfPhaseG0->ucG0_R0_UH,
						 piBfPhaseG0->ucG0_R0_H,
						 piBfPhaseG0->ucG0_R0_M,
						 piBfPhaseG0->ucG0_R0_L,
						 piBfPhaseG0->ucG0_R1_UH,
						 piBfPhaseG0->ucG0_R1_H,
						 piBfPhaseG0->ucG0_R1_M,
						 piBfPhaseG0->ucG0_R1_L);

				MTWF_PRINT(
				"G0_R2_UH = %d\n G0_R2_H = %d\n G0_R2_M = %d\n G0_R2_L = %d\n",
						 piBfPhaseG0->ucG0_R2_UH,
						 piBfPhaseG0->ucG0_R2_H,
						 piBfPhaseG0->ucG0_R2_M,
						 piBfPhaseG0->ucG0_R2_L);

				MTWF_PRINT("G0_R3_UH = %d\n G0_R3_H = %d\n G0_R3_M = %d\n"
				" G0_R3_L = %d\n G0_R3_UL = %d\n",
						piBfPhaseG0->ucG0_R3_UH,
						piBfPhaseG0->ucG0_R3_H,
						piBfPhaseG0->ucG0_R3_M,
						piBfPhaseG0->ucG0_R3_L,
						piBfPhaseG0->ucG0_R3_UL);

				MTWF_PRINT("Group : %d\n", ucGroup);
				MTWF_PRINT("Calibration == 1? or Verification == 2? : %d\n",
					ucPhaseCalType);
				MTWF_PRINT("Calibrated result = %d\n", ucStatus);
				MTWF_PRINT("0 : Means failed\n 1: means pass\n2: means ongoing\n");
				MTWF_PRINT("C0_UH:%d, C1_UH:%d, C2_UH:%d, C3_UH:%d\n"
					"C0_H:%d,  C1_H:%d,  C2_H:%d,  C3_H:%d\n"
					"C0_M:%d,  C1_M:%d,  C2_M:%d,  C3_M:%d\n"
				 "C0_L:%d,  C1_L:%d,  C2_L:%d,  C3_L:%d\n",
				iBfPhaseOut.ucC0_UH, iBfPhaseOut.ucC1_UH, iBfPhaseOut.ucC2_UH, iBfPhaseOut.ucC3_UH,
					iBfPhaseOut.ucC0_H,  iBfPhaseOut.ucC1_H,  iBfPhaseOut.ucC2_H,  iBfPhaseOut.ucC3_H,
					iBfPhaseOut.ucC0_M,  iBfPhaseOut.ucC1_M,  iBfPhaseOut.ucC2_M,  iBfPhaseOut.ucC3_M,
					iBfPhaseOut.ucC0_L,  iBfPhaseOut.ucC1_L,  iBfPhaseOut.ucC2_L,  iBfPhaseOut.ucC3_L);

				break;

			case GROUP_H:
				break;
			}
		} else {
			struct IBF_PHASE_Gx_T *piBfPhaseGx;

			ucGroupIdx = ucGroup - 1;
			piBfPhaseGx = (struct IBF_PHASE_Gx_T *)&pAd->piBfPhaseGx[ucGroupIdx * u1IbfCalPhaseStructLenGx];
			NdisCopyMemory(piBfPhaseGx, pucIBfPhaseG, u1IbfCalPhaseStructLenGx);

			switch (ucGroupL_M_H) {
			case GROUP_L:
				break;

			case GROUP_M:
				MTWF_PRINT("G%d and Group_M\n G%d_M_T0_H = %d\n G%d_M_T1_H = %d\n",
						 ucGroup,
						 ucGroup, piBfPhaseGx->ucGx_M_T0_H,
						 ucGroup, piBfPhaseGx->ucGx_M_T1_H);

				MTWF_PRINT("G%d_M_T2_H = %d\n",
						 ucGroup, piBfPhaseGx->ucGx_M_T2_H);

				MTWF_PRINT("G%d_R0_UH = %d\n G%d_R0_H = %d\n G%d_R0_MH =%d\n",
						 ucGroup, piBfPhaseGx->ucGx_R0_UH,
						 ucGroup, piBfPhaseGx->ucGx_R0_H,
						 ucGroup, piBfPhaseGx->ucGx_R0_MH);

				MTWF_PRINT("G%d_R0_M = %d\n G%d_R0_L = %d\n",
						 ucGroup, piBfPhaseGx->ucGx_R0_M,
						 ucGroup, piBfPhaseGx->ucGx_R0_L);

				MTWF_PRINT("G%d_R1_UH = %d\n G%d_R1_H = %d\n G%d_R1_MH = %d\nn",
						 ucGroup, piBfPhaseGx->ucGx_R1_UH,
						 ucGroup, piBfPhaseGx->ucGx_R1_H,
						 ucGroup, piBfPhaseGx->ucGx_R1_MH);

				MTWF_PRINT("G%d_R1_M = %d\n G%d_R1_L = %d\n",
						 ucGroup, piBfPhaseGx->ucGx_R1_M,
						 ucGroup, piBfPhaseGx->ucGx_R1_L);

				MTWF_PRINT(
						"G%d_R2_UH = %d\n G%d_R2_H = %d\n G%d_R2_MH = %d\n",
						 ucGroup, piBfPhaseGx->ucGx_R2_UH,
						 ucGroup, piBfPhaseGx->ucGx_R2_H,
						 ucGroup, piBfPhaseGx->ucGx_R2_MH);

				MTWF_PRINT(
						"G%d_R2_M = %d\n G%d_R2_L = %d\n",
						 ucGroup, piBfPhaseGx->ucGx_R2_M,
						 ucGroup, piBfPhaseGx->ucGx_R2_L);

				MTWF_PRINT("G%d_R3_UH = %d\n G%d_R3_H = %d\n G%d_R3_MH = %d\n",
						ucGroup, piBfPhaseGx->ucGx_R3_UH,
						ucGroup, piBfPhaseGx->ucGx_R3_H,
						ucGroup, piBfPhaseGx->ucGx_R3_MH);

				MTWF_PRINT("G%d_R3_M = %d\n G%d_R3_L = %d\n",
						ucGroup, piBfPhaseGx->ucGx_R3_M,
						ucGroup, piBfPhaseGx->ucGx_R3_L);

				MTWF_PRINT("G%d_R3_UL = %d\n", ucGroup, piBfPhaseGx->ucGx_R3_UL);

				MTWF_PRINT("Group : %d\n", ucGroup);
				MTWF_PRINT("Calibration == 1? or Verification == 2? : %d\n",
					ucPhaseCalType);
				MTWF_PRINT("Calibrated result = %d\n", ucStatus);
				MTWF_PRINT("0 : Means failed\n1: means pass\n 2: means ongoing\n");
				MTWF_PRINT("C0_UH : %d, C1_UH : %d, C2_UH : %d,\n"
					"C3_UH : %d\n"
					"C0_H : %d, C1_H : %d, C2_H : %d, C3_H : %d\n"
					"C0_MH : %d, C1_MH : %d, C2_MH : %d, C3_MH : %d\n"
					"C0_M : %d, C1_M : %d, C2_M : %d, C3_M : %d\n"
					"C0_L : %d, C1_L : %d, C2_L : %d,  C3_L : %d\n",
						iBfPhaseOut.ucC0_UH, iBfPhaseOut.ucC1_UH, iBfPhaseOut.ucC2_UH, iBfPhaseOut.ucC3_UH,
						iBfPhaseOut.ucC0_H,  iBfPhaseOut.ucC1_H,  iBfPhaseOut.ucC2_H,  iBfPhaseOut.ucC3_H,
						iBfPhaseOut.ucC0_MH, iBfPhaseOut.ucC1_MH, iBfPhaseOut.ucC2_MH, iBfPhaseOut.ucC3_MH,
						iBfPhaseOut.ucC0_M,  iBfPhaseOut.ucC1_M,  iBfPhaseOut.ucC2_M,  iBfPhaseOut.ucC3_M,
						iBfPhaseOut.ucC0_L,  iBfPhaseOut.ucC1_L,  iBfPhaseOut.ucC2_L,  iBfPhaseOut.ucC3_L);
				break;

			case GROUP_H:
				break;
			}
		}
	break;

	case IBF_PHASE_CAL_VERIFY: /* Show calibrated result only */
	case IBF_PHASE_CAL_VERIFY_INSTRUMENT:
		NdisCopyMemory(&iBfPhaseOut, pBuf, u1IbfPhaseOutStructLen);

		/* Update calibrated status */
		pAd->fgCalibrationFail |= ((ucStatus == 1) ? FALSE : TRUE);
		pAd->fgGroupIdPassFailStatus[ucGroup] = ((ucStatus == 1) ? FALSE : TRUE);
		MTWF_PRINT("Group : %d\n", ucGroup);
		MTWF_PRINT("Calibration == 1? or Verification == 2? : %d\n",
				ucPhaseCalType);
		MTWF_PRINT("Calibrated result = %d\n", ucStatus);
		MTWF_PRINT("0 : Means failed\n1: means pass\n 2: means ongoing\n");
		MTWF_PRINT("C0_UH : %d, C1_UH : %d, C2_UH : %d, C3_UH : %d\n"
				"C0_H : %d, C1_H : %d, C2_H : %d, C3_H : %d\n"
				"C0_MH : %d, C1_MH : %d, C2_MH : %d, C3_MH : %d\n"
				"C0_M : %d, C1_M : %d, C2_M : %d, C3_M : %d\n"
				"C0_L : %d, C1_L : %d,C2_L : %d, C3_L : %d\n",
				iBfPhaseOut.ucC0_UH, iBfPhaseOut.ucC1_UH, iBfPhaseOut.ucC2_UH, iBfPhaseOut.ucC3_UH,
				iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H, iBfPhaseOut.ucC3_H,
				iBfPhaseOut.ucC0_MH, iBfPhaseOut.ucC1_MH, iBfPhaseOut.ucC2_MH, iBfPhaseOut.ucC3_MH,
				iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M, iBfPhaseOut.ucC3_M,
				iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L, iBfPhaseOut.ucC3_L);
		break;

	default:
		break;
	}
}
#endif /* TXBF_SUPPORT */
