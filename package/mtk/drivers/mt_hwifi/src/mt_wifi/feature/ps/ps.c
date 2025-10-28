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

/****************************************************************************

    Abstract:

	All related POWER SAVE function body.

***************************************************************************/

#include "rt_config.h"

/*
	==========================================================================
	Description:
		This routine is used to clean up a specified power-saving queue. It's
		used whenever a wireless client is deleted.
	==========================================================================
 */
VOID RtmpCleanupPsQueue(
	RTMP_ADAPTER *pAd, struct _QUEUE_HEADER *pQueue)
{
	struct _QUEUE_ENTRY *pQEntry;
	PNDIS_PACKET pPacket;

	MTWF_DBG(NULL, DBG_CAT_PS, CATPS_CFG, DBG_LVL_INFO,
		"(0x%08lx)...\n", (ULONG)pQueue);

	while (pQueue->Head) {
		MTWF_DBG(NULL, DBG_CAT_PS, CATPS_CFG, DBG_LVL_INFO,
				 "%d...\n", pQueue->Number);
		pQEntry = RemoveHeadQueue(pQueue);
		/*pPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReservedEx); */
		pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		MTWF_DBG(NULL, DBG_CAT_PS, CATPS_CFG, DBG_LVL_INFO,
			"pkt = %lx...\n", (ULONG)pPacket);
	}
}


/*
  ========================================================================
  Description:
	This routine frees all packets in PSQ that's destined to a specific DA.
	BCAST/MCAST in DTIMCount=0 case is also handled here, just like a PS-POLL
	is received from a WSTA which has MAC address FF:FF:FF:FF:FF:FF
  ========================================================================
*/
VOID RtmpHandleRxPsPoll(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT wcid, BOOLEAN isActive)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->APPSMode == APPS_MODE2)
		return;
}


/*
 * ==========================================================================
 * Description:
 * Update the station current power save mode.
 * ==========================================================================
 */
BOOLEAN RtmpPsIndicate(RTMP_ADAPTER *pAd, UCHAR *pAddr, UINT16 wcid, UCHAR Psm)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->APPSMode == APPS_MODE2) {
		MAC_TABLE_ENTRY *entry;
		UCHAR old_psmode;
		STA_TR_ENTRY *tr_entry;

		if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
			return PWR_ACTIVE;

		entry = entry_get(pAd, wcid);
		tr_entry = tr_entry_get(pAd, wcid);

		old_psmode = entry->PsMode;
		if (old_psmode == PWR_ACTIVE && Psm == PWR_SAVE)
			NdisGetSystemUpTime(&entry->sleep_from);
		else if (old_psmode == PWR_SAVE && Psm == PWR_ACTIVE)
			entry->sleep_from = 0;

		entry->NoDataIdleCount = 0;
		entry->PsMode = Psm;
		tr_entry->PsMode = Psm;
	}
	return PWR_ACTIVE;
}


#ifdef CONFIG_STA_SUPPORT
/*
========================================================================
Routine Description:
    Check if PM of any packet is set.

Arguments:
	pAd		Pointer to our adapter

Return Value:
    TRUE	can set
	FALSE	can not set

Note:
========================================================================
*/
BOOLEAN RtmpPktPmBitCheck(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg)
{
	return (pStaCfg->PwrMgmt.Psm == PWR_SAVE);
}

VOID RtmpPsModeChange(RTMP_ADAPTER *pAd, UINT32 PsMode)
{
	MAC_TABLE_ENTRY *pEntry = GetAssociatedAPByWdev(pAd, &pAd->StaCfg[MAIN_MSTA_ID].wdev);

	if (pAd->StaCfg[0].BssType == BSS_INFRA) {
		/* reset ps mode */
		if (PsMode == Ndis802_11PowerModeMAX_PSP) {
			/* do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange() */
			/* to exclude certain situations. */
			/* MlmeSetPsm(pAd, PWR_SAVE); */
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

			if (pAd->StaCfg[0].bWindowsACCAMEnable == FALSE)
				pAd->StaCfg[0].WindowsPowerMode = Ndis802_11PowerModeMAX_PSP;

			pAd->StaCfg[0].WindowsBatteryPowerMode = Ndis802_11PowerModeMAX_PSP;
			pAd->StaCfg[0].DefaultListenCount = 5;
		} else if (PsMode == Ndis802_11PowerModeFast_PSP) {
			/* do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange() */
			/* to exclude certain situations. */
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

			if (pAd->StaCfg[0].bWindowsACCAMEnable == FALSE)
				pAd->StaCfg[0].WindowsPowerMode = Ndis802_11PowerModeFast_PSP;

			pAd->StaCfg[0].WindowsBatteryPowerMode = Ndis802_11PowerModeFast_PSP;
			pAd->StaCfg[0].DefaultListenCount = 3;
		} else if (PsMode == Ndis802_11PowerModeLegacy_PSP) {
			/* do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange() */
			/* to exclude certain situations. */
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

			if (pAd->StaCfg[0].bWindowsACCAMEnable == FALSE)
				pAd->StaCfg[0].WindowsPowerMode = Ndis802_11PowerModeLegacy_PSP;

			pAd->StaCfg[0].WindowsBatteryPowerMode = Ndis802_11PowerModeLegacy_PSP;
			pAd->StaCfg[0].DefaultListenCount = 3;
		} else {
			/* Default Ndis802_11PowerModeCAM */
			/* clear PSM bit immediately */
			RTMP_SET_PSM_BIT(pAd, &pAd->StaCfg[0], PWR_ACTIVE);
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

			if (pAd->StaCfg[0].bWindowsACCAMEnable == FALSE)
				pAd->StaCfg[0].WindowsPowerMode = Ndis802_11PowerModeCAM;

			pAd->StaCfg[0].WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
		}

		/* change ps mode */
		RTMPSendNullFrame(pAd, pEntry, pAd->CommonCfg.TxRate, TRUE, FALSE);
		MTWF_DBG(NULL, DBG_CAT_PS, CATPS_CFG, DBG_LVL_INFO,
			"PSMode=%ld\n", pAd->StaCfg[0].WindowsPowerMode);
	}
}

VOID EnqueuePsPoll(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg)
{
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */

	if (pStaCfg->WindowsPowerMode == Ndis802_11PowerModeLegacy_PSP)
		pAd->PsPollFrame.FC.PwrMgmt = PWR_SAVE;

	MiniportMMRequest(pAd, 0, (PUCHAR)&pAd->PsPollFrame, sizeof(PSPOLL_FRAME), NULL);
}

#endif /* CONFIG_STA_SUPPORT */

