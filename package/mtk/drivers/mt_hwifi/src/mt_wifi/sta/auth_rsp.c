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
	auth_rsp.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	John		2004-10-1		copy from RT2560
*/
#include "rt_config.h"

/*
 *    ==========================================================================
 *    Description:
 *	authentication state machine init procedure
 *    Parameters:
 *	Sm - the state machine
 *
 *	IRQL = PASSIVE_LEVEL
 *
 *    ==========================================================================
 */
VOID AuthRspStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN PSTATE_MACHINE Sm,
	IN STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, Trans, MAX_AUTH_RSP_STATE, MAX_AUTH_RSP_MSG,
					 (STATE_MACHINE_FUNC) Drop, AUTH_RSP_IDLE,
					 AUTH_RSP_MACHINE_BASE);
	/* column 1 */
	StateMachineSetAction(Sm, AUTH_RSP_IDLE, MT2_PEER_DEAUTH,
						  (STATE_MACHINE_FUNC) PeerDeauthAction);
	/* column 2 */
	StateMachineSetAction(Sm, AUTH_RSP_WAIT_CHAL, MT2_PEER_DEAUTH,
						  (STATE_MACHINE_FUNC) PeerDeauthAction);
}

/*
 *    ==========================================================================
 *    Description:
 *
 *	IRQL = DISPATCH_LEVEL
 *
 *    ==========================================================================
 */
VOID PeerDeauthAction(
	IN PRTMP_ADAPTER pAd,
	IN PMLME_QUEUE_ELEM Elem)
{
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR Addr3[MAC_ADDR_LEN];
	USHORT Reason;
	BOOLEAN bDoIterate = FALSE;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	if (PeerDeauthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr1, Addr2, Addr3, &Reason)) {
		if (INFRA_ON(pStaCfg)
			&& (MAC_ADDR_EQUAL(Addr1, pStaCfg->wdev.if_addr)
				|| MAC_ADDR_EQUAL(Addr1, BROADCAST_ADDR))
			&& MAC_ADDR_EQUAL(Addr2, pStaCfg->Bssid)
			&& MAC_ADDR_EQUAL(Addr3, pStaCfg->Bssid)
#ifdef DOT11R_FT_SUPPORT
			&& ((pStaCfg->Dot11RCommInfo.bInMobilityDomain == FALSE)
				|| (pStaCfg->Dot11RCommInfo.FtRspSuccess == 0))
#endif /* DOT11R_FT_SUPPORT */
		   ) {
			struct wifi_dev *wdev = &pStaCfg->wdev;

			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
					 "AUTH_RSP - receive DE-AUTH from our AP (Reason=%d)\n",
					  Reason);

			if (Reason == REASON_4_WAY_TIMEOUT)
				RTMPSendWirelessEvent(pAd,
									  IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG,
									  NULL, 0, 0);

			if (Reason == REASON_GROUP_KEY_HS_TIMEOUT)
				RTMPSendWirelessEvent(pAd,
									  IW_GROUP_HS_TIMEOUT_EVENT_FLAG,
									  NULL, 0, 0);

			/* send wireless event - for deauthentication */
			RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, NULL,
								  BSS0, 0);
			/*
			 *   Some customer would set AP1 & AP2 same SSID, AuthMode & EncrypType but different WPAPSK,
			 *   therefore we need to do iterate here.
			 */
			if ((wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
				&&
				(IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap)
				 || IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap))
#ifdef WSC_STA_SUPPORT
				&& (wdev->WscControl.WscState < WSC_STATE_LINK_UP)
#endif /* WSC_STA_SUPPORT */
			   )
				bDoIterate = TRUE;

			LinkDown(pAd, TRUE, wdev, Elem);

			if (bDoIterate) {
				pStaCfg->MlmeAux.BssIdx++;
				IterateOnBssTab(pAd, wdev);
			}
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
				 "AUTH_RSP - PeerDeauthAction() sanity check fail\n");
	}
}
