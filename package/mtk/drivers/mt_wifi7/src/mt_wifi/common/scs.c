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
	scs.c
*/
#ifdef SMART_CARRIER_SENSE_SUPPORT
#include "rt_config.h"

VOID SmartCarrierSense_Gen6(RTMP_ADAPTER *pAd)
{
	PSMART_CARRIER_SENSE_CTRL    pSCSCtrl;
	UCHAR	Ret = NDIS_STATUS_FAILURE;
	UINT8	BandIdx;
	RTMP_STRING	str[8] = {};
	int ret;

	pSCSCtrl = &pAd->SCSCtrl;
	BandIdx = hc_get_hw_band_idx(pAd);
	ret = snprintf(str, sizeof(str), "%d", BandIdx);
	if (os_snprintf_error(sizeof(str), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_ERROR,
			"snprintf error!\n");
		return;
	}

	if (pSCSCtrl->SCSEnable == SCS_ENABLE) {
#ifdef WIFI_UNIFIED_COMMAND
		Ret = UNI_SendSCSDataProc(pAd, str);
#endif
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_DEBUG,
			 "(Ret = %d_\n", Ret);
}

#ifdef WIFI_UNIFIED_COMMAND
static VOID check_scs_glo(struct _RTMP_ADAPTER *pAd, VOID *pData)
{
	P_DRV_SCS_GLO pDrvGlo = &pAd->CommonCfg.rScsGloInfo;
	P_EVENT_SCS_GLO pFwGlo = (P_EVENT_SCS_GLO)pData;
	UINT_32 DriverSize = 0, j;

	for (j = 0; j < 2; j++) {
		pDrvGlo->rscsband[j].u4Addr = pFwGlo->rscsband[j].u4Addr;
		pDrvGlo->rscsband[j].fgError =
		(pFwGlo->rscsband[j].u4Size != sizeof(SMART_CARRIER_SENSE_CTRL_GEN2_T))?1:0;

		if (pDrvGlo->rscsband[j].fgError) {
			DriverSize = sizeof(SMART_CARRIER_SENSE_CTRL_GEN2_T);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_ERROR,
				"\x1b[31m is not synced.\x1b[0m");
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_ERROR,
				"Drive Size = %d, FW Size = %d",
				DriverSize, pFwGlo->rscsband[j].u4Size);
		}
	}
}

VOID UniEventSCSGetGloAddrHandler(struct cmd_msg *msg, char *rsp_payload)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)msg->priv;
	P_UNI_EVENT_GET_SCS_GLO_ADDR pEntry = (P_UNI_EVENT_GET_SCS_GLO_ADDR)rsp_payload;
	P_EVENT_SCS_GLO pFwGlo = NULL;

	pFwGlo = &pEntry->rGloInfo;
	check_scs_glo(pAd, (VOID *)pFwGlo);
}
#endif /* WIFI_UNIFIED_COMMAND */

INT ShowSCSinfo_ver2_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->show_scs_info)
		ops->show_scs_info(pAd);
	else
		return FALSE;

	return TRUE;
}

VOID SCS_init(RTMP_ADAPTER *pAd)
{
	INT32 Ret;

#ifdef WIFI_UNIFIED_COMMAND
	Ret = UNI_Set_SCSEnable_Proc(pAd, "1");
#else
	Ret = NDIS_STATUS_FAILURE;
#endif /* WIFI_UNIFIED_COMMAND */

	MTWF_PRINT("%s(): SCS Default Enable, Ret=%d\n", __func__, Ret);
}
#endif /* SMART_CARRIER_SENSE_SUPPORT */
