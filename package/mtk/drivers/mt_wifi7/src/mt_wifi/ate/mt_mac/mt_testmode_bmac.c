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

#ifdef COMPOS_TESTMODE_WIN
#include "config.h"
#else
#include "rt_config.h"
#endif



INT32 mtb_ate_ipg_cr_restore(RTMP_ADAPTER *ad, UCHAR band_idx)
{
	if (band_idx == TESTMODE_BAND0) {
		MtTestModeRestoreCr(ad, BN0_WF_AGG_TOP_ATCR1_ADDR);
		MtTestModeRestoreCr(ad, BN0_WF_AGG_TOP_ATCR3_ADDR);
		MtTestModeRestoreCr(ad, BN0_WF_TMAC_TOP_TRCR0_ADDR);
		/* IFS CR, for SIFS/SLOT time control */
		MtTestModeRestoreCr(ad, BN0_WF_TMAC_TOP_ICR0_ADDR);
		/* For fixing backoff random number ARB_DRNGR0 */
		MtTestModeRestoreCr(ad, BN0_WF_ARB_TOP_DRNGR0_ADDR);
		/* For fixing backoff random number ARB_DRNGR1 */
		MtTestModeRestoreCr(ad, BN0_WF_ARB_TOP_DRNGR1_ADDR);
	} else if (band_idx == TESTMODE_BAND1) {
		MtTestModeRestoreCr(ad, BN1_WF_AGG_TOP_ATCR1_ADDR);
		MtTestModeRestoreCr(ad, BN1_WF_AGG_TOP_ATCR3_ADDR);
		MtTestModeRestoreCr(ad, BN1_WF_TMAC_TOP_TRCR0_ADDR);
		/* IFS CR, for SIFS/SLOT time control */
		MtTestModeRestoreCr(ad, BN1_WF_TMAC_TOP_ICR0_ADDR);
		/* For fixing backoff random number ARB_DRNGR0 */
		MtTestModeRestoreCr(ad, BN1_WF_ARB_TOP_DRNGR0_ADDR);
		/* For fixing backoff random number ARB_DRNGR1 */
		MtTestModeRestoreCr(ad, BN1_WF_ARB_TOP_DRNGR1_ADDR);
	} else if (band_idx == TESTMODE_BAND2) {
		MtTestModeRestoreCr(ad, IP1_BN0_WF_AGG_TOP_ATCR1_ADDR);
		MtTestModeRestoreCr(ad, IP1_BN0_WF_AGG_TOP_ATCR3_ADDR);
		MtTestModeRestoreCr(ad, IP1_BN0_WF_TMAC_TOP_TRCR0_ADDR);
		/* IFS CR, for SIFS/SLOT time control */
		MtTestModeRestoreCr(ad, IP1_BN0_WF_TMAC_TOP_ICR0_ADDR);
		/* For fixing backoff random number ARB_DRNGR0 */
		MtTestModeRestoreCr(ad, IP1_BN0_WF_ARB_TOP_DRNGR0_ADDR);
		/* For fixing backoff random number ARB_DRNGR1 */
		MtTestModeRestoreCr(ad, IP1_BN0_WF_ARB_TOP_DRNGR1_ADDR);
	}

	return 0;
}

INT32 mtb_ate_mac_cr_restore(RTMP_ADAPTER *ad)
{
	if (IS_BELLWETHER(ad)) {
		/* Data frame protection CR recover */
		MtTestModeRestoreCr(ad, BN0_WF_AGG_TOP_PCR0_ADDR);
		MtTestModeRestoreCr(ad, BN0_WF_AGG_TOP_PCR1_ADDR);
		/* BA related CR recover */
		MtTestModeRestoreCr(ad, BN0_WF_AGG_TOP_AWSCR0_ADDR);
		MtTestModeRestoreCr(ad, BN0_WF_AGG_TOP_AWSCR1_ADDR);
		MtTestModeRestoreCr(ad, BN0_WF_AGG_TOP_AWSCR2_ADDR);
		MtTestModeRestoreCr(ad, BN0_WF_AGG_TOP_AWSCR3_ADDR);
		/* Enable HW BAR feature */
		MtTestModeRestoreCr(ad, BN0_WF_AGG_TOP_MRCR_ADDR);
		/* TCR */
		MtTestModeRestoreCr(ad, BN0_WF_TMAC_TOP_TCR0_ADDR);
		/* TFCR0 */
		MtTestModeRestoreCr(ad, BN0_WF_TMAC_TOP_TFCR0_ADDR);
		/* IPG related CRs resotre */
		MtTestModeRestoreCr(ad, BN0_WF_AGG_TOP_ATCR1_ADDR);
		MtTestModeRestoreCr(ad, BN0_WF_AGG_TOP_ATCR3_ADDR);
		MtTestModeRestoreCr(ad, BN0_WF_TMAC_TOP_TRCR0_ADDR);
		/* IFS CR, for SIFS/SLOT time control */
		MtTestModeRestoreCr(ad, BN0_WF_TMAC_TOP_ICR0_ADDR);
		/* For fixing backoff random number ARB_DRNGR0 */
		MtTestModeRestoreCr(ad, BN0_WF_ARB_TOP_DRNGR0_ADDR);
		/* For fixing backoff random number ARB_DRNGR1 */
		MtTestModeRestoreCr(ad, BN0_WF_ARB_TOP_DRNGR1_ADDR);
		/* For testmode default disable MLO */
		MtTestModeRestoreCr(ad, WF_MLO_TOP_SCR0_ADDR);
#ifdef DBDC_MODE
		if (IS_ATE_DBDC(ad)) {
			/* Data frame protection CR recover */
			MtTestModeRestoreCr(ad, BN1_WF_AGG_TOP_PCR0_ADDR);
			MtTestModeRestoreCr(ad, BN1_WF_AGG_TOP_PCR1_ADDR);
			/* BA related CR recover */
			MtTestModeRestoreCr(ad, BN1_WF_AGG_TOP_AWSCR0_ADDR);
			MtTestModeRestoreCr(ad, BN1_WF_AGG_TOP_AWSCR1_ADDR);
			MtTestModeRestoreCr(ad, BN1_WF_AGG_TOP_AWSCR2_ADDR);
			MtTestModeRestoreCr(ad, BN1_WF_AGG_TOP_AWSCR3_ADDR);
			/* Enable HW BAR feature */
			MtTestModeRestoreCr(ad, BN1_WF_AGG_TOP_MRCR_ADDR);
			/* TCR */
			MtTestModeRestoreCr(ad, BN1_WF_TMAC_TOP_TCR0_ADDR);
			/* TFCR0 */
			MtTestModeRestoreCr(ad, BN1_WF_TMAC_TOP_TFCR0_ADDR);
			/* IPG related CRs resotre */
			MtTestModeRestoreCr(ad, BN1_WF_AGG_TOP_ATCR1_ADDR);
			MtTestModeRestoreCr(ad, BN1_WF_AGG_TOP_ATCR3_ADDR);
			MtTestModeRestoreCr(ad, BN1_WF_TMAC_TOP_TRCR0_ADDR);
			/* IFS CR, for SIFS/SLOT time control */
			MtTestModeRestoreCr(ad, BN1_WF_TMAC_TOP_ICR0_ADDR);
			/* For fixing backoff random number ARB_DRNGR0 */
			MtTestModeRestoreCr(ad, BN1_WF_ARB_TOP_DRNGR0_ADDR);
			/* For fixing backoff random number ARB_DRNGR1 */
			MtTestModeRestoreCr(ad, BN1_WF_ARB_TOP_DRNGR1_ADDR);
		}
#endif /* DBDC_MODE */

		/* for Band2 config */
		/* Data frame protection CR recover */
		MtTestModeRestoreCr(ad, IP1_BN0_WF_AGG_TOP_PCR0_ADDR);
		MtTestModeRestoreCr(ad, IP1_BN0_WF_AGG_TOP_PCR1_ADDR);

		/* Enable HW BAR feature */
		MtTestModeRestoreCr(ad, IP1_BN0_WF_AGG_TOP_MRCR_ADDR);
		/* TCR */
		MtTestModeRestoreCr(ad, IP1_BN0_WF_TMAC_TOP_TCR0_ADDR);
		/* TFCR0 */
		MtTestModeRestoreCr(ad, IP1_BN0_WF_TMAC_TOP_TFCR0_ADDR);
		/* IPG related CRs resotre */
		MtTestModeRestoreCr(ad, IP1_BN0_WF_AGG_TOP_ATCR1_ADDR);
		MtTestModeRestoreCr(ad, IP1_BN0_WF_AGG_TOP_ATCR3_ADDR);
		MtTestModeRestoreCr(ad, IP1_BN0_WF_TMAC_TOP_TRCR0_ADDR);
		/* IFS CR, for SIFS/SLOT time control */
		MtTestModeRestoreCr(ad, IP1_BN0_WF_TMAC_TOP_ICR0_ADDR);
		/* For fixing backoff random number ARB_DRNGR0 */
		MtTestModeRestoreCr(ad, IP1_BN0_WF_ARB_TOP_DRNGR0_ADDR);
		/* For fixing backoff random number ARB_DRNGR1 */
		MtTestModeRestoreCr(ad, IP1_BN0_WF_ARB_TOP_DRNGR1_ADDR);

	}

	return 0;
}

INT32 mtb_ate_mac_cr_backup_and_set(RTMP_ADAPTER *ad)
{
	struct _ATE_CTRL *ate_ctrl = &ad->ATECtrl;

	NdisZeroMemory(&ate_ctrl->bk_cr, sizeof(struct _TESTMODE_BK_CR)*MAX_TEST_BKCR_NUM);

	if (IS_BELLWETHER(ad)) {
		UINT32 val = 0;
		/* Disable data frame protection for test mode */
		MtTestModeBkCr(ad, BN0_WF_AGG_TOP_PCR0_ADDR, TEST_MAC_BKCR);
		MAC_IO_READ32(ad->hdev_ctrl, BN0_WF_AGG_TOP_PCR0_ADDR, &val);
		val &= ~(BN0_WF_AGG_TOP_PCR0_MM_PROTECTION0_MASK | BN0_WF_AGG_TOP_PCR0_GF_PROTECTION0_MASK | BN0_WF_AGG_TOP_PCR0_ERP_PROTECTION0_MASK | BN0_WF_AGG_TOP_PCR0_VHT_PROTECTION0_MASK);
		val &= ~(BN0_WF_AGG_TOP_PCR0_BW20_PROTECTION0_MASK | BN0_WF_AGG_TOP_PCR0_BW40_PROTECTION0_MASK | BN0_WF_AGG_TOP_PCR0_BW80_PROTECTION0_MASK | BN0_WF_AGG_TOP_PCR0_BW160_PROTECTION0_MASK);
		val |= BN0_WF_AGG_TOP_PCR0_PROTECTION_DIS_IN_PTA_WIN0_MASK;
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_PCR0_ADDR, val);

		MtTestModeBkCr(ad, BN0_WF_AGG_TOP_PCR1_ADDR, TEST_MAC_BKCR);
		MAC_IO_READ32(ad->hdev_ctrl, BN0_WF_AGG_TOP_PCR1_ADDR, &val);
		/* Setting RTS length/numb threshold to max value to aviod send RTS in test mode */
		val = (BN0_WF_AGG_TOP_PCR1_RTS0_PKT_NUM_THRESHOLD_MASK | BN0_WF_AGG_TOP_PCR1_RTS0_PKT_LEN_THRESHOLD_MASK);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_PCR1_ADDR, val);
		/* BA related CR backup */
		MtTestModeBkCr(ad, BN0_WF_AGG_TOP_AWSCR0_ADDR, TEST_MAC_BKCR);
		MtTestModeBkCr(ad, BN0_WF_AGG_TOP_AWSCR1_ADDR, TEST_MAC_BKCR);
		MtTestModeBkCr(ad, BN0_WF_AGG_TOP_AWSCR2_ADDR, TEST_MAC_BKCR);
		MtTestModeBkCr(ad, BN0_WF_AGG_TOP_AWSCR3_ADDR, TEST_MAC_BKCR);
		/* HW BAR feature */
		MtTestModeBkCr(ad, BN0_WF_AGG_TOP_MRCR_ADDR, TEST_MAC_BKCR);
		MAC_IO_READ32(ad->hdev_ctrl, BN0_WF_AGG_TOP_MRCR_ADDR, &val);
		val &= ~(BN0_WF_AGG_TOP_MRCR_BAR_TX_CNT_LIMIT_MASK);
		val &= ~(BN0_WF_AGG_TOP_MRCR_LAST_RTS_AS_CTS_EN_MASK);
		val &= ~(BN0_WF_AGG_TOP_MRCR_RTS_FAIL_CNT_LIMIT_MASK | BN0_WF_AGG_TOP_MRCR_TXCMD_RTS_FAIL_CNT_LIMIT_MASK);
		val |= ((0x1 << BN0_WF_AGG_TOP_MRCR_RTS_FAIL_CNT_LIMIT_SHFT) | (0x1 << BN0_WF_AGG_TOP_MRCR_TXCMD_RTS_FAIL_CNT_LIMIT_SHFT));
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_MRCR_ADDR, val);
		/* TFCR */
		MtTestModeBkCr(ad, BN0_WF_TMAC_TOP_TFCR0_ADDR, TEST_MAC_BKCR);
		//MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_ADDR, 0);
		/* TCR */
		MtTestModeBkCr(ad, BN0_WF_TMAC_TOP_TCR0_ADDR, TEST_MAC_BKCR);
		MAC_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TCR0_ADDR, &val);
		val &= ~(BN0_WF_TMAC_TOP_TCR0_TBTT_TX_STOP_CONTROL_MASK);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TCR0_ADDR, val);

		/* IPG related CR back up */
		MtTestModeBkCr(ad, BN0_WF_AGG_TOP_ATCR1_ADDR, TEST_MAC_BKCR);
		MtTestModeBkCr(ad, BN0_WF_AGG_TOP_ATCR3_ADDR, TEST_MAC_BKCR);
		MtTestModeBkCr(ad, BN0_WF_TMAC_TOP_TRCR0_ADDR, TEST_MAC_BKCR);
		/* IFS CR, for SIFS/SLOT time control */
		MtTestModeBkCr(ad, BN0_WF_TMAC_TOP_ICR0_ADDR, TEST_MAC_BKCR);
		/* For fixing backoff random number ARB_DRNGR0 */
		MtTestModeBkCr(ad, BN0_WF_ARB_TOP_DRNGR0_ADDR, TEST_MAC_BKCR);
		/* For fixing backoff random number ARB_DRNGR1 */
		MtTestModeBkCr(ad, BN0_WF_ARB_TOP_DRNGR1_ADDR, TEST_MAC_BKCR);
		/* For testmode default disable MLO */
		MtTestModeBkCr(ad, WF_MLO_TOP_SCR0_ADDR, TEST_MAC_BKCR);
		MAC_IO_READ32(ad->hdev_ctrl, WF_MLO_TOP_SCR0_ADDR, &val);
		val &= ~(WF_MLO_TOP_SCR0_LINK0_MLO_EN_MASK |
			WF_MLO_TOP_SCR0_LINK1_MLO_EN_MASK |
			WF_MLO_TOP_SCR0_LINK2_MLO_EN_MASK |
			WF_MLO_TOP_SCR0_EMLSR_USR_RMV_EN_MASK |
			WF_MLO_TOP_SCR0_EMLSR_VLD_CHK_EN_MASK);
		MAC_IO_WRITE32(ad->hdev_ctrl, WF_MLO_TOP_SCR0_ADDR, val);
#ifdef DBDC_MODE
		/* RTS threshold disable for band1 */
		if (IS_ATE_DBDC(ad)) {
			/* Disable data frame protection for test mode */
			MtTestModeBkCr(ad, BN1_WF_AGG_TOP_PCR0_ADDR, TEST_MAC_BKCR);
			MAC_IO_READ32(ad->hdev_ctrl, BN1_WF_AGG_TOP_PCR0_ADDR, &val);
			val &= ~(BN1_WF_AGG_TOP_PCR0_MM_PROTECTION0_MASK | BN1_WF_AGG_TOP_PCR0_GF_PROTECTION0_MASK | BN1_WF_AGG_TOP_PCR0_ERP_PROTECTION0_MASK | BN1_WF_AGG_TOP_PCR0_VHT_PROTECTION0_MASK);
			val &= ~(BN1_WF_AGG_TOP_PCR0_BW20_PROTECTION0_MASK | BN1_WF_AGG_TOP_PCR0_BW40_PROTECTION0_MASK | BN1_WF_AGG_TOP_PCR0_BW80_PROTECTION0_MASK);
			val |= BN1_WF_AGG_TOP_PCR0_PROTECTION_DIS_IN_PTA_WIN0_MASK;
			MAC_IO_WRITE32(ad->hdev_ctrl, BN1_WF_AGG_TOP_PCR0_ADDR, val);

			MtTestModeBkCr(ad, BN1_WF_AGG_TOP_PCR1_ADDR, TEST_MAC_BKCR);
			/* Setting RTS length/numb threshold to */
			/* max value to aviod send RTS in test mode */
			val = (BN1_WF_AGG_TOP_PCR1_RTS0_PKT_NUM_THRESHOLD_MASK | BN1_WF_AGG_TOP_PCR1_RTS0_PKT_LEN_THRESHOLD_MASK);
			MAC_IO_WRITE32(ad->hdev_ctrl, BN1_WF_AGG_TOP_PCR1_ADDR, val);
			/* BA related CR backup */
			MtTestModeBkCr(ad, BN1_WF_AGG_TOP_AWSCR0_ADDR, TEST_MAC_BKCR);
			MtTestModeBkCr(ad, BN1_WF_AGG_TOP_AWSCR1_ADDR, TEST_MAC_BKCR);
			MtTestModeBkCr(ad, BN1_WF_AGG_TOP_AWSCR2_ADDR, TEST_MAC_BKCR);
			MtTestModeBkCr(ad, BN1_WF_AGG_TOP_AWSCR3_ADDR, TEST_MAC_BKCR);
			/* HW BAR feature */
			MtTestModeBkCr(ad, BN1_WF_AGG_TOP_MRCR_ADDR, TEST_MAC_BKCR);
			MAC_IO_READ32(ad->hdev_ctrl, BN1_WF_AGG_TOP_MRCR_ADDR, &val);
			val &= ~(BN1_WF_AGG_TOP_MRCR_BAR_TX_CNT_LIMIT_MASK);
			val &= ~(BN1_WF_AGG_TOP_MRCR_LAST_RTS_AS_CTS_EN_MASK);
			val &= ~(BN1_WF_AGG_TOP_MRCR_RTS_FAIL_CNT_LIMIT_MASK | BN0_WF_AGG_TOP_MRCR_TXCMD_RTS_FAIL_CNT_LIMIT_MASK);
			val |= ((0x1 << BN1_WF_AGG_TOP_MRCR_RTS_FAIL_CNT_LIMIT_SHFT) | (0x1 << BN1_WF_AGG_TOP_MRCR_TXCMD_RTS_FAIL_CNT_LIMIT_SHFT));
			MAC_IO_WRITE32(ad->hdev_ctrl, BN1_WF_AGG_TOP_MRCR_ADDR, val);
			/* TFCR */
			MtTestModeBkCr(ad, BN1_WF_TMAC_TOP_TFCR0_ADDR, TEST_MAC_BKCR);
			//MAC_IO_WRITE32(ad->hdev_ctrl, BN1_WF_TMAC_TOP_TFCR0_ADDR, 0);
			/* TCR */
			MtTestModeBkCr(ad, BN1_WF_TMAC_TOP_TCR0_ADDR, TEST_MAC_BKCR);
			MAC_IO_READ32(ad->hdev_ctrl, BN1_WF_TMAC_TOP_TCR0_ADDR, &val);
			val &= ~(BN1_WF_TMAC_TOP_TCR0_TBTT_TX_STOP_CONTROL_MASK);
			MAC_IO_WRITE32(ad->hdev_ctrl, BN1_WF_TMAC_TOP_TCR0_ADDR, val);
			/* IPG related CR back up */
			MtTestModeBkCr(ad, BN1_WF_AGG_TOP_ATCR1_ADDR, TEST_MAC_BKCR);
			MtTestModeBkCr(ad, BN1_WF_AGG_TOP_ATCR3_ADDR, TEST_MAC_BKCR);
			MtTestModeBkCr(ad, BN1_WF_TMAC_TOP_TRCR0_ADDR, TEST_MAC_BKCR);
			/* IFS CR, for SIFS/SLOT time control */
			MtTestModeBkCr(ad, BN1_WF_TMAC_TOP_ICR0_ADDR, TEST_MAC_BKCR);
			/* For fixing backoff random number ARB_DRNGR0 */
			MtTestModeBkCr(ad, BN1_WF_ARB_TOP_DRNGR0_ADDR, TEST_MAC_BKCR);
			/* For fixing backoff random number ARB_DRNGR1 */
			MtTestModeBkCr(ad, BN1_WF_ARB_TOP_DRNGR1_ADDR, TEST_MAC_BKCR);
		}
#endif /* DBDC_MODE */

		/* for Band2 config */
		/* Disable data frame protection for test mode */
		MtTestModeBkCr(ad, IP1_BN0_WF_AGG_TOP_PCR0_ADDR, TEST_MAC_BKCR);
		MAC_IO_READ32(ad->hdev_ctrl, IP1_BN0_WF_AGG_TOP_PCR0_ADDR, &val);
		val &= ~(IP1_BN0_WF_AGG_TOP_PCR0_MM_PROTECTION0_MASK | IP1_BN0_WF_AGG_TOP_PCR0_GF_PROTECTION0_MASK | IP1_BN0_WF_AGG_TOP_PCR0_ERP_PROTECTION0_MASK | IP1_BN0_WF_AGG_TOP_PCR0_VHT_PROTECTION0_MASK);
		val &= ~(IP1_BN0_WF_AGG_TOP_PCR0_BW20_PROTECTION0_MASK | IP1_BN0_WF_AGG_TOP_PCR0_BW40_PROTECTION0_MASK | IP1_BN0_WF_AGG_TOP_PCR0_BW80_PROTECTION0_MASK | IP1_BN0_WF_AGG_TOP_PCR0_BW160_PROTECTION0_MASK);
		val |= IP1_BN0_WF_AGG_TOP_PCR0_PROTECTION_DIS_IN_PTA_WIN0_MASK;
		MAC_IO_WRITE32(ad->hdev_ctrl, IP1_BN0_WF_AGG_TOP_PCR0_ADDR, val);

		MtTestModeBkCr(ad, IP1_BN0_WF_AGG_TOP_PCR1_ADDR, TEST_MAC_BKCR);
		/* Setting RTS length/numb threshold to max value to aviod send RTS in test mode */
		val = (IP1_BN0_WF_AGG_TOP_PCR1_RTS0_PKT_NUM_THRESHOLD_MASK | IP1_BN0_WF_AGG_TOP_PCR1_RTS0_PKT_LEN_THRESHOLD_MASK);
		MAC_IO_WRITE32(ad->hdev_ctrl, IP1_BN0_WF_AGG_TOP_PCR1_ADDR, val);

		/* HW BAR feature */
		MtTestModeBkCr(ad, IP1_BN0_WF_AGG_TOP_MRCR_ADDR, TEST_MAC_BKCR);
		MAC_IO_READ32(ad->hdev_ctrl, IP1_BN0_WF_AGG_TOP_MRCR_ADDR, &val);
		val &= ~(IP1_BN0_WF_AGG_TOP_MRCR_BAR_TX_CNT_LIMIT_MASK);
		val &= ~(IP1_BN0_WF_AGG_TOP_MRCR_LAST_RTS_AS_CTS_EN_MASK);
		val &= ~(IP1_BN0_WF_AGG_TOP_MRCR_RTS_FAIL_CNT_LIMIT_MASK | IP1_BN0_WF_AGG_TOP_MRCR_TXCMD_RTS_FAIL_CNT_LIMIT_MASK);
		val |= ((0x1 << IP1_BN0_WF_AGG_TOP_MRCR_RTS_FAIL_CNT_LIMIT_SHFT) | (0x1 << IP1_BN0_WF_AGG_TOP_MRCR_TXCMD_RTS_FAIL_CNT_LIMIT_SHFT));
		MAC_IO_WRITE32(ad->hdev_ctrl, IP1_BN0_WF_AGG_TOP_MRCR_ADDR, val);
		/* TFCR */
		MtTestModeBkCr(ad, IP1_BN0_WF_TMAC_TOP_TFCR0_ADDR, TEST_MAC_BKCR);
		//MAC_IO_WRITE32(ad->hdev_ctrl, IP1_BN0_WF_TMAC_TOP_TFCR0_ADDR, 0);
		/* TCR */
		MtTestModeBkCr(ad, IP1_BN0_WF_TMAC_TOP_TCR0_ADDR, TEST_MAC_BKCR);
		MAC_IO_READ32(ad->hdev_ctrl, IP1_BN0_WF_TMAC_TOP_TCR0_ADDR, &val);
		val &= ~(IP1_BN0_WF_TMAC_TOP_TCR0_TBTT_TX_STOP_CONTROL_MASK);
		MAC_IO_WRITE32(ad->hdev_ctrl, IP1_BN0_WF_TMAC_TOP_TCR0_ADDR, val);
		/* IPG related CR back up */
		MtTestModeBkCr(ad, IP1_BN0_WF_AGG_TOP_ATCR1_ADDR, TEST_MAC_BKCR);
		MtTestModeBkCr(ad, IP1_BN0_WF_AGG_TOP_ATCR3_ADDR, TEST_MAC_BKCR);
		MtTestModeBkCr(ad, IP1_BN0_WF_TMAC_TOP_TRCR0_ADDR, TEST_MAC_BKCR);
		/* IFS CR, for SIFS/SLOT time control */
		MtTestModeBkCr(ad, IP1_BN0_WF_TMAC_TOP_ICR0_ADDR, TEST_MAC_BKCR);
		/* For fixing backoff random number ARB_DRNGR0 */
		MtTestModeBkCr(ad, IP1_BN0_WF_ARB_TOP_DRNGR0_ADDR, TEST_MAC_BKCR);
		/* For fixing backoff random number ARB_DRNGR1 */
		MtTestModeBkCr(ad, IP1_BN0_WF_ARB_TOP_DRNGR1_ADDR, TEST_MAC_BKCR);




#ifdef DBDC_MODE
		if (IS_ATE_DBDC(ad)) {
			MtTestModeBkCr(ad, BN1_WF_TMAC_TOP_TRCR0_ADDR, TEST_MAC_BKCR);
			MtTestModeBkCr(ad, BN1_WF_TMAC_TOP_ICR0_ADDR, TEST_MAC_BKCR);
			/* For fixing backoff random number ARB_DRNGR0/ARB_DRNGR0 */
			MtTestModeBkCr(ad, BN1_WF_ARB_TOP_DRNGR0_ADDR, TEST_MAC_BKCR);
			MtTestModeBkCr(ad, BN1_WF_ARB_TOP_DRNGR1_ADDR, TEST_MAC_BKCR);
		}
#endif /* DBDC_MODE */
	}

	return 0;
}


UINT32 bmac_bn0_agg_cnt_array[] = {BN0_WF_AGG_TOP_AALCR0_ADDR, BN0_WF_AGG_TOP_AALCR1_ADDR, BN0_WF_AGG_TOP_AALCR2_ADDR, BN0_WF_AGG_TOP_AALCR3_ADDR};
UINT32 bmac_bn1_agg_cnt_array[] = {BN1_WF_AGG_TOP_AALCR0_ADDR, BN1_WF_AGG_TOP_AALCR1_ADDR, BN1_WF_AGG_TOP_AALCR2_ADDR, BN1_WF_AGG_TOP_AALCR3_ADDR};
//FIXME miller
UINT32 bmac_bn2_agg_cnt_array[] = {IP1_BN0_WF_AGG_TOP_AALCR0_ADDR, IP1_BN0_WF_AGG_TOP_AALCR1_ADDR, IP1_BN0_WF_AGG_TOP_AALCR2_ADDR, IP1_BN0_WF_AGG_TOP_AALCR3_ADDR};


INT32 mtb_ate_ampdu_ba_limit(RTMP_ADAPTER *ad, uint8_t wmm_idx, uint8_t agg_limit, uint8_t band_idx)
{
	UINT32 value;
	/* UCHAR band_idx = hc_get_hw_band_idx(ad); */
	UINT32 band_offset = MAC_CR_PERBAND_OFFSET(band_idx);

	if (wmm_idx > 3) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: invalid WmmIdx=%d, set to all!\n",
			__func__, wmm_idx));
		wmm_idx = 0xFF;
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: WmmIdx=%d,agg_limit=%d\n", __func__, wmm_idx, agg_limit));

	value = ((agg_limit & BN0_WF_AGG_TOP_AALCR6_AC30_AGG_LIMIT_MASK) << BN0_WF_AGG_TOP_AALCR6_AC31_AGG_LIMIT_SHFT)
		| ((agg_limit & BN0_WF_AGG_TOP_AALCR6_AC30_AGG_LIMIT_MASK));

	switch (wmm_idx) {
	case 0:
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR0_ADDR + band_offset, value);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR1_ADDR + band_offset, value);
		break;
	case 1:
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR2_ADDR + band_offset, value);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR3_ADDR + band_offset, value);
		break;
	case 2:
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR4_ADDR + band_offset, value);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR5_ADDR + band_offset, value);
		break;
	case 3:
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR6_ADDR + band_offset, value);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR7_ADDR + band_offset, value);
		break;
	default:
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR0_ADDR + band_offset, value);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR1_ADDR + band_offset, value);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR2_ADDR + band_offset, value);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR3_ADDR + band_offset, value);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR4_ADDR + band_offset, value);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR5_ADDR + band_offset, value);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR6_ADDR + band_offset, value);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_AALCR7_ADDR + band_offset, value);
		break;
	}
	return 0;
}

INT32 mtb_ate_set_sta_pause_cr(RTMP_ADAPTER *ad, uint8_t ac_idx)
{
	INT32 ret = 0;
	return ret;
}

INT32 mtb_ate_set_ifs_cr(RTMP_ADAPTER *pAd, uint8_t band_idx)
{
	INT32 ret = 0;
	struct _ATE_IPG_PARAM *ipg_param = NULL;
	uint16_t slot_time, sifs_time;
	UINT32 txv_time = 0, i2t_chk_time = 0, tr2t_chk_time = 0;
	UINT32 value = 0;
	UINT32 band_offset = MAC_CR_PERBAND_OFFSET(band_idx);

	ipg_param = (struct _ATE_IPG_PARAM *)TESTMODE_GET_PADDR(pAd, TESTMODE_GET_BAND_IDX(pAd), ipg_param);

	slot_time = ipg_param->slot_time;
	sifs_time = ipg_param->sifs_time;
	/* in uint of ns */
	MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_TMAC_TOP_ATCR_ADDR + band_offset, &txv_time);
	txv_time &= BN0_WF_TMAC_TOP_ATCR_TXV_TOUT_MASK;
	txv_time *= NORMAL_CLOCK_TIME;

	i2t_chk_time = (UINT32)(slot_time * 1000) - txv_time - BBP_PROCESSING_TIME;
	tr2t_chk_time = (UINT32)(sifs_time * 1000) - txv_time - BBP_PROCESSING_TIME;
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s: txv_time=%dns, i2t_chk_time=%dns, tr2t_chk_time=%dns\n",
		__func__, txv_time, i2t_chk_time, tr2t_chk_time);

	i2t_chk_time /= NORMAL_CLOCK_TIME;
	tr2t_chk_time /= NORMAL_CLOCK_TIME;

	if (band_idx == TESTMODE_BAND0) {
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_TMAC_TOP_TRCR0_ADDR + band_offset, &value);
		value |= ((tr2t_chk_time << BN0_WF_TMAC_TOP_TRCR0_TR2T_CHK_SHFT) & BN0_WF_TMAC_TOP_TRCR0_TR2T_CHK_MASK);
		value |= ((i2t_chk_time << BN0_WF_TMAC_TOP_TRCR0_I2T_CHK_SHFT) & BN0_WF_TMAC_TOP_TRCR0_I2T_CHK_MASK);
		MAC_IO_WRITE32(pAd->hdev_ctrl, BN0_WF_TMAC_TOP_TRCR0_ADDR + band_offset, value);
	} else if (band_idx == TESTMODE_BAND1) {
		MAC_IO_READ32(pAd->hdev_ctrl, BN1_WF_TMAC_TOP_TRCR0_ADDR + band_offset, &value);
		value |= ((tr2t_chk_time << BN1_WF_TMAC_TOP_TRCR0_TR2T_CHK_SHFT) & BN1_WF_TMAC_TOP_TRCR0_TR2T_CHK_MASK);
		value |= ((i2t_chk_time << BN1_WF_TMAC_TOP_TRCR0_I2T_CHK_SHFT) & BN1_WF_TMAC_TOP_TRCR0_I2T_CHK_MASK);
		MAC_IO_WRITE32(pAd->hdev_ctrl, BN1_WF_TMAC_TOP_TRCR0_ADDR + band_offset, value);
	} else if (band_idx == TESTMODE_BAND2) {
		MAC_IO_READ32(pAd->hdev_ctrl, IP1_BN0_WF_TMAC_TOP_TRCR0_ADDR + band_offset, &value);
		value |= ((tr2t_chk_time << IP1_BN0_WF_TMAC_TOP_TRCR0_TR2T_CHK_SHFT) & IP1_BN0_WF_TMAC_TOP_TRCR0_TR2T_CHK_MASK);
		value |= ((i2t_chk_time << IP1_BN0_WF_TMAC_TOP_TRCR0_I2T_CHK_SHFT) & IP1_BN0_WF_TMAC_TOP_TRCR0_I2T_CHK_MASK);
		MAC_IO_WRITE32(pAd->hdev_ctrl, IP1_BN0_WF_TMAC_TOP_TRCR0_ADDR + band_offset, value);
	} else {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Invalid band_idx!!\n", __func__));
		return FALSE;
	}

	return ret;
}
