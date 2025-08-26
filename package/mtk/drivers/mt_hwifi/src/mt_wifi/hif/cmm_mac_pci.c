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
 ***************************************************************************/



#ifdef RTMP_MAC_PCI
#include	"rt_config.h"

#ifdef LINUX
#if (KERNEL_VERSION(5, 4, 0) < LINUX_VERSION_CODE)
#define mmiowb()		do { } while (0)
#endif
#endif

/*
*
*/
VOID dump_txd(RTMP_ADAPTER *pAd, TXD_STRUC *pTxD)
{
	MTWF_PRINT("TxD:\n");
	MTWF_PRINT("\tSDPtr0=0x%x\n", pTxD->SDPtr0);
	MTWF_PRINT("\tSDLen0=0x%x\n", pTxD->SDLen0);
	MTWF_PRINT("\tLastSec0=0x%x\n", pTxD->LastSec0);
	MTWF_PRINT("\tSDPtr1=0x%x\n", pTxD->SDPtr1);
	MTWF_PRINT("\tSDLen1=0x%x\n", pTxD->SDLen1);
	MTWF_PRINT("\tLastSec1=0x%x\n", pTxD->LastSec1);
	MTWF_PRINT("\tDMADONE=0x%x\n", pTxD->DMADONE);
	MTWF_PRINT("\tBurst=0x%x\n", pTxD->Burst);
}

/***************************************************************************
  *
  *	register related procedures.
  *
  **************************************************************************/

#ifdef CONFIG_STA_SUPPORT

VOID RT28xxPciStaAsicWakeup(RTMP_ADAPTER *pAd, BOOLEAN bFromTx, PSTA_ADMIN_CONFIG pStaCfg)
{
	if (pStaCfg)
		RTMPOffloadPm(pAd, pStaCfg, PM4, EXIT_PM_STATE);
}


VOID RT28xxPciStaAsicSleepAutoWakeup(
	RTMP_ADAPTER *pAd,
	PSTA_ADMIN_CONFIG pStaCfg)
{
	if (pStaCfg)
		RTMPOffloadPm(pAd, pStaCfg, PM4, ENTER_PM_STATE);
}

#endif /* CONFIG_STA_SUPPORT */
#endif /* RTMP_MAC_PCI */
