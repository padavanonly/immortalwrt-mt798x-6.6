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
	rtmp_mcu.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include	"rt_config.h"

INT32 MCUSysExit(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops && ops->FwExit)
		ops->FwExit(pAd);

	MCU_CTRL_EXIT(pAd);
	return Ret;
}


VOID ChipOpsMCUHook(PRTMP_ADAPTER pAd, enum MCU_TYPE MCUType)
{
	RTMP_CHIP_OP *pChipOps = hc_get_chip_ops(pAd->hdev_ctrl);

	if (pChipOps && ((MCUType & ANDES) == ANDES)) {
		pChipOps->FwInit = hif_mcu_fw_init;
		pChipOps->FwExit = hif_mcu_fw_exit;
		pChipOps->MtCmdTx = AndesSendCmdMsg;
		pChipOps->MCUCtrlInit = AndesCtrlInit;
		pChipOps->MCUCtrlExit = AndesCtrlExit;
	}
}

