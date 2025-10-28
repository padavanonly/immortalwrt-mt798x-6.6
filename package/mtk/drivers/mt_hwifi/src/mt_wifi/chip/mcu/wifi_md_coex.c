#include "rt_config.h"
#include "hw_ctrl.h"

/* For wifi and md coex in colgin project*/
#ifdef WIFI_MD_COEX_SUPPORT
/*---------------------------------------------------------------------*/
/* WIFI and MD Coexistence Realize                                     */
/*---------------------------------------------------------------------*/
/* Handle IDC info event sent by fw, update to sw_db. */
VOID Coex_IDC_Info_Handle(RTMP_ADAPTER *pAd, struct COEX_IDC_INFO *pEventIdcInfo)
{
	MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_INFO, "\n");
	NdisCopyMemory(&pAd->idcInfo, pEventIdcInfo, sizeof(pAd->idcInfo));
}

/* Set IDC state: 1: enable, 0: disable. */
static VOID update_idc_state(RTMP_ADAPTER *pAd)
{
	MtCmdIdcStateUpdate(pAd);

	if (pAd->idcState) {
		/* Query safe channel info when enable IDC */
		if (pAd->LteSafeChCtrl.bEnabled && !pAd->LteSafeChCtrl.bQueryLteDone) {
			HW_QUERY_LTE_SAFE_CHANNEL(pAd);
			pAd->LteSafeChCtrl.bQueryLteDone = TRUE;
		}
	} else {
		/* clear all unsafe info when disable IDC */
		pAd->LteSafeChCtrl.bQueryLteDone = FALSE;
		NdisFillMemory(&(pAd->LteSafeChCtrl.SafeChn6GBitmask),
						sizeof(pAd->LteSafeChCtrl.SafeChn6GBitmask), 0xFF);
		NdisFillMemory(&(pAd->LteSafeChCtrl.PwrChn6GBitmask),
						sizeof(pAd->LteSafeChCtrl.PwrChn6GBitmask), 0xFF);
		NdisFillMemory(pAd->LteSafeChCtrl.SafeChnBitmask,
						sizeof(pAd->LteSafeChCtrl.SafeChnBitmask), 0xFF);
		NdisFillMemory(pAd->LteSafeChCtrl.PwrChnBitmask,
						sizeof(pAd->LteSafeChCtrl.PwrChnBitmask), 0xFF);
	}
}

INT Set_Idc_TxPwrBackOff(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 maxTxPwrLimit;

	MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_NOTICE, "\n");
	if (arg == NULL || strlen(arg) == 0)
		return FALSE;

	maxTxPwrLimit = os_str_tol(arg, 0, 10);
	if (maxTxPwrLimit == 0) {
		MTWF_PRINT("Wrong param input.\n");
		return FALSE;
	}

	MtCmdIdcSetTxPwrLimit(pAd, maxTxPwrLimit);
	MTWF_PRINT("Set maxTxPwrLimit to %d.\n", maxTxPwrLimit);
	return TRUE;
}

/* Set IDC state: 1: enable, 0: disable. */
INT Set_Idc_State(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN	bEnable;

	MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_NOTICE, "\n");
	if (arg == NULL || strlen(arg) == 0)
		return FALSE;

	bEnable = os_str_tol(arg, 0, 10);
	if (bEnable != 0 && bEnable != 1) {
		MTWF_PRINT("Wrong param input.\n");
		return FALSE;
	}

	if (pAd->idcState != bEnable) {
		pAd->idcState = bEnable;
		update_idc_state(pAd);
		MTWF_PRINT("IDC state change to %s.\n", bEnable?"Enable":"Disable");
	}

	return TRUE;
}

/* Show IDC info, for debug use. */
INT Show_Idc_Info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct COEX_IDC_INFO *pEventIdcInfo = &pAd->idcInfo;
	UINT8 idx;

	MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_INFO, "\n");

	MtCmdIdcInfoQuery(pAd);
	MTWF_PRINT("IDC Info\n---------\n");

	MTWF_PRINT("idcEnable=%d\n", pEventIdcInfo->idcEnable);
	if (!pEventIdcInfo->idcEnable)
		return TRUE;

	MTWF_PRINT("MD LTE operfreq:\n");
	for (idx = 0; idx < MAX_TG_NUM_LTE; idx++)
		MTWF_PRINT("  lte_oper_band[%d]=%d, lte_dl_freq[%d]=%d, lte_ul_freq[%d]=%d\n",
					idx, pEventIdcInfo->lte_oper_band[idx],
					idx, pEventIdcInfo->lte_dl_freq[idx],
					idx, pEventIdcInfo->lte_ul_freq[idx]);

	MTWF_PRINT("MD NR operfreq:\n");
	for (idx = 0; idx < MAX_TG_NUM_NR; idx++)
		MTWF_PRINT("  nr_oper_band[%d]=%d, nr_dl_freq[%d]=%d, nr_ul_freq[%d]=%d\n",
					idx, pEventIdcInfo->nr_oper_band[idx],
					idx, pEventIdcInfo->nr_dl_freq[idx],
					idx, pEventIdcInfo->nr_ul_freq[idx]);

	MTWF_PRINT("MD channel bitmask:\n");
	MTWF_PRINT("  u4SafeChannelBitmask:");
	for (idx = 0; idx < WIFI_CH_MASK_IDX_NUM; idx++)
		MTWF_PRINT(" [%d]=0x%x", idx, pEventIdcInfo->u4SafeChannelBitmask[idx]);

	MTWF_PRINT("\n  u4TdmChannelBitmask:");
	for (idx = 0; idx < WIFI_CH_MASK_IDX_NUM; idx++)
		MTWF_PRINT(" [%d]=0x%x", idx, pEventIdcInfo->u4TdmChannelBitmask[idx]);

	MTWF_PRINT("\n  u4PwrChannelBitmask:");
	for (idx = 0; idx < WIFI_CH_MASK_IDX_NUM; idx++)
		MTWF_PRINT(" [%d]=0x%x", idx, pEventIdcInfo->u4PwrChannelBitmask[idx]);

	MTWF_PRINT("\n  u4FdmChannelBitmask:");
	for (idx = 0; idx < WIFI_CH_MASK_IDX_NUM; idx++)
		MTWF_PRINT(" [%d]=0x%x", idx, pEventIdcInfo->u4FdmChannelBitmask[idx]);

	MTWF_PRINT("\nIDC Solution (Free run(0) / Power backoff(1) / TDM(2) / FDM(3))");
	MTWF_PRINT("\n  u4WiFiFinalSoluation:");

	for (idx = 0; idx < WIFI_RAM_BAND_NUM; idx++)
		MTWF_PRINT(" [%d]=%d", idx, pEventIdcInfo->u4WiFiFinalSoluation[idx]);

	MTWF_PRINT("\n  u4MDFinalSoluation:");
	for (idx = 0; idx < WIFI_RAM_BAND_NUM; idx++)
		MTWF_PRINT(" [%d]=%d", idx, pEventIdcInfo->u4MDFinalSoluation[idx]);

	MTWF_PRINT("\nMD Connected mode / Idle mode\n");
	MTWF_PRINT(" lteTxExist=%d, nrTxExist=%d\n", pEventIdcInfo->lteTxExist, pEventIdcInfo->nrTxExist);

	MTWF_PRINT("MD Pwr flag\n");
	MTWF_PRINT("  u2LtePwrBackBmp=0x%x, u2NrPwrBackBmp=0x%x\n",
				pEventIdcInfo->u2LtePwrBackBmp, pEventIdcInfo->u2NrPwrBackBmp);

	MTWF_PRINT("MD / Wi-Fi TDM time\n");
	MTWF_PRINT("  tdm_lte_window=%d, tdm_lte_conn_window=%d,",
		pEventIdcInfo->tdm_lte_window, pEventIdcInfo->tdm_lte_conn_window);
	MTWF_PRINT(" tdm_nr_window=%d, tdm_conn_window=%d\n",
		pEventIdcInfo->tdm_nr_window, pEventIdcInfo->tdm_conn_window);
	return TRUE;
}

/* Do COEX init */
VOID init_wifi_md_coex(struct _RTMP_ADAPTER *pAd)
{
	MtCmdIdcStateUpdate(pAd);
}

/* Do COEX de-init */
VOID deinit_wifi_md_coex(struct _RTMP_ADAPTER *pAd)
{
	pAd->idcState = FALSE;
}

#endif /* WIFI_MD_COEX_SUPPORT */

