/****************************************************************************
* Mediatek Inc.
* 5F., No.5, Taiyuan 1st St., Zhubei City,
* Hsinchu County 302, Taiwan, R.O.C.
* (c) Copyright 2014, Mediatek, Inc.
*
* All rights reserved. Mediatek's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code
* contains confidential trade secret material of Mediatek. Any attemp
* or participation in deciphering, decoding, reverse engineering or in any
* way altering the source code is stricitly prohibited, unless the prior
* written consent of Mediatek, Inc. is obtained.
****************************************************************************

	Module Name:
	ftm.h

	Abstract:
	802.11mc FTM protocol function prototype declaration.

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	annie		 2014.11.22   Initial version.
*/

#ifndef __FTM_H__
#define __FTM_H__

enum FTM_ISTA_ACTION {
	ACTION_GO_TO,
	ACTION_SWITCH_BACK
};

VOID ftm_set_last_wdev(struct wifi_dev *wdev);
struct wifi_dev *ftm_get_last_wdev(void);

struct _ftm_set_channel_cfg {
	PRTMP_ADAPTER pAd;
	struct wifi_dev *wdev;
	UCHAR channel;
};

struct _ftm_set_mc_burst {
	PRTMP_ADAPTER pAd;
	struct wifi_dev *wdev;
	UINT8 band_idx;
};

VOID ftm_mc_burst(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT8 band_idx);

VOID FtmMCBurst(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

VOID ftm_req_fw_mc_or_burst(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

VOID ftm_switch_channel(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR target_channel);

VOID ftm_create_mac_entry(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	VOID *pAddr,
	struct _FTM_PEER_INFO *pPeerInfo);

BOOLEAN FtmEntryDump(
	IN PRTMP_ADAPTER    pAd,
	IN struct wifi_dev	*wdev,
	IN PFTM_PEER_ENTRY	pEntry,
	IN UINT32			DbgLvl);

VOID FtmPidPendingQDump(
	IN PRTMP_ADAPTER    pAd,
	IN struct wifi_dev	*wdev,
	IN UINT32			DbgLvl);

UINT32 FtmMinDeltaToMS(
	IN UINT32			MinDelta);

UINT32 FtmBurstDurationToMS(
	IN UINT8			BurstDuration);

BOOLEAN FtmConvertTodToa(
	IN PRTMP_ADAPTER    pAd,
	IN UINT64 * pTOD,
	IN UINT64 * pTOA);

inline UINT8 FtmGetNewPid(
	IN PRTMP_ADAPTER    pAd);

VOID FtmAddPidPendingNode(
	IN PRTMP_ADAPTER    pAd,
	IN struct wifi_dev	*wdev,
	IN PFTM_PEER_ENTRY	pEntry,
	IN UINT8			PID);

INT FtmDeqPidPendingNode(
	IN PRTMP_ADAPTER    pAd,
	IN PFTM_PEER_ENTRY	pEntry);

PFTM_PEER_ENTRY FtmGetPidPendingNode(
	IN PRTMP_ADAPTER    pAd,
	IN UINT8    PID);

VOID FtmMgmtInit(
	IN PRTMP_ADAPTER    pAd);

VOID FtmMgmtInitByWdev(
	IN PRTMP_ADAPTER	pAd);

VOID FtmMgmtExit(
	IN PRTMP_ADAPTER    pAd);

VOID FtmPeerTableInit(
	IN PRTMP_ADAPTER    pAd);

PFTM_PEER_ENTRY FtmEntrySearch(
	IN PRTMP_ADAPTER    pAd,
	IN struct wifi_dev		*wdev,
	IN UINT8	*Addr);

UCHAR
FtmGetBWSetting(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR format_and_bw);

PFTM_PEER_ENTRY FtmEntryGet(
	IN PRTMP_ADAPTER    pAd,
	IN struct wifi_dev		*wdev,
	IN UINT8	*Addr);

VOID FtmEntryNegoDoneAction(
	IN PRTMP_ADAPTER	pAd,
	IN PFTM_PEER_ENTRY	pEntry);

BOOLEAN FtmEntryCntDownAction(
	IN PRTMP_ADAPTER    pAd,
	IN struct wifi_dev		*wdev,
	IN PFTM_PEER_ENTRY  pEntry);


BOOLEAN FtmEntryTerminate(
	IN PRTMP_ADAPTER	pAd,
	IN struct wifi_dev	*wdev,
	IN PFTM_PEER_ENTRY	pEntry,
	IN BOOLEAN			bTxFTM);

INT FtmSwitchChannel(
	struct _RTMP_ADAPTER *mac_ad,
	struct wifi_dev *wdev,
	UCHAR channel,
	enum FTM_ISTA_ACTION action);

VOID SendFTM(
	IN PRTMP_ADAPTER    pAd,
	IN struct wifi_dev	*wdev,
	IN UINT8			*Addr,
	IN CHAR				Reason);

VOID ReceiveFTMReq(
	IN PRTMP_ADAPTER	pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID ReceiveFTM(
	IN PRTMP_ADAPTER	pAd,
	IN MLME_QUEUE_ELEM * Elem);

INT Set_FtmLciValue_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);

INT Set_FtmLciLat_ThisAP_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);

INT Set_FtmLciLng_ThisAP_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);

INT Set_FtmLciAlt_ThisAP_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);

INT Set_FtmLciLat_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg,
	IN PLCI_FIELD pLci);

INT Set_FtmLciLng_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg,
	IN PLCI_FIELD pLci);

INT Set_FtmLciAlt_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg,
	IN PLCI_FIELD pLci);

INT Set_FtmLciKnown_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);

INT Set_FtmLciFlag_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);

INT Set_FtmCivicKnown_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);

INT Set_FtmCivicFlag_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);

INT Set_FtmZFlag_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);

INT Show_FtmEntry_Proc(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev	*wdev,
	IN RTMP_STRING * arg);

INT Show_FtmLciValue_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);

VOID FtmMapSigmaCmdToLocLCI(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * tmpbuf,
	IN PLCI_FIELD pLci);

VOID FtmProfileNeighborApParse(
	IN RTMP_ADAPTER * pAd,
	IN UINT NeighborIdx,
	IN RTMP_STRING * tmpbuf);

INT Send_ANQP_Req_For_Test(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);

INT Send_NeighborReq_For_Test(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);

INT Set_FtmRMRandomizationInterval_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);

INT Set_FtmRMMinimumApCount_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);

INT Send_RadioMeasurement_Req_For_Test(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);


#endif /* __FTM_H__ */

