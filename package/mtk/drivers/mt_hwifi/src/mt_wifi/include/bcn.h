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
    bcn.h

    Abstract:
    bcn related fucntions definition.

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Carter      2014-1121     Created.
*/

#ifndef __BCN_H__
#define __BCN_H__

#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
#define MAX_BEACONV2_LENGTH       (sizeof(HEADER_802_11) + \
									 TIMESTAMP_FIELD_LEN + \
									 BEACON_INTERVAL_FIELD_LEN + \
									 CAP_INFO_FIELD_LEN + \
									 MAX_IE_V2_LENGTH)
#endif
#define MAX_BEACON_LENGTH       (sizeof(HEADER_802_11) + \
								 TIMESTAMP_FIELD_LEN + \
								 BEACON_INTERVAL_FIELD_LEN + \
								 CAP_INFO_FIELD_LEN + \
								 MAX_IE_LENGTH)



#define BCN_REASON_MASK            BITS(0, 5)
/* move enum BCN_UPDATE_CRIT_UPD to high bit[6] */
#define BCN_CRIT_UPD_MASK        BIT(6)
/* rsv */
#define BCN_RSV_MASK        BIT(7)
#define BCN_SET_CRIT_UPD(_var) ((_var) | (BCN_CRIT_UPD_MASK))
#define BCN_CLEAR_CRIT_UPD(_var) ((_var) & (~BCN_CRIT_UPD_MASK))
#define IS_BCN_CRIT_UPD(_var) ((_var & BCN_CRIT_UPD_MASK))
#define BCN_REASON(_var) ((_var & BCN_REASON_MASK))
#define BCN_REASON_EQUAL(_var, _var2) ((_var & BCN_REASON_MASK) == _var2)

typedef enum _BCN_UPDATE_REASON {
	BCN_UPDATE_INIT		= 0,	/* beacon resource initial. */
	BCN_UPDATE_IF_STATE_CHG	= 1,	/* there is interface up or down, check related TXD handle. */
	BCN_UPDATE_IE_CHG		= 2,	/* simple IE change, just update the corresponding interface content. */
	BCN_UPDATE_ALL_AP_RENEW	= 3,	/* prepared All beacon for All active interface. */
	BCN_UPDATE_PRETBTT		= 4,	/* update function routine, source could from INT isr or timer or event notify. */
	BCN_UPDATE_ENABLE_TX	= 5,	/* Enable Beacon TX. */
	BCN_UPDATE_DISABLE_TX	= 6,	/* Disable Beacon TX. */
	BCN_UPDATE_TIM		= 7,	/* TIM preparing */
	BCN_UPDATE_CSA		= 8,	/* CSA (Channel Switch Announcement) */
	BCN_UPDATE_BMGR		= 9,	/* BSS Manager triggered */
	BCN_UPDATE_BTWT_IE	= 10,	/* bTWT element */
	/* 11 ~ 62 */ /* for new features */
	BCN_UPDATE_MAX	= 63, /* UINT8 max  BITS(0, 5) = 63 , BIT(6) for CRIT_UPD usage */
} BCN_UPDATE_REASON;



enum BCN_BPCC_IE_TYPE {
	BCN_BPCC_CSA		= 0,/*35.3.11*/
	BCN_BPCC_ECSA		= 1,/*35.3.11*/
	BCN_BPCC_EDCA		= 2,
	BCN_BPCC_QUIET		= 3,/*35.3.11*/
	BCN_BPCC_DSSS		= 4,
	BCN_BPCC_HTOP		= 5,
	BCN_BPCC_WBCS		= 6,
	BCN_BPCC_CSW		= 7,
	BCN_BPCC_OMN		= 8,
	BCN_BPCC_QUIETC		= 9,/*35.3.11*/
	BCN_BPCC_VHTOP		= 10,
	BCN_BPCC_HEOP		= 11,
	BCN_BPCC_BTWT		= 12,
	BCN_BPCC_BTWTSET	= 13,
	BCN_BPCC_BSSCCA		= 14,
	BCN_BPCC_MUEDCA		= 15,
	BCN_BPCC_SR			= 16,
	BCN_BPCC_UORA		= 17,
	BCN_BPCC_EHTOP		= 18,
	BCN_BPCC_CH_CHANGED	= 19,
	BCN_BPCC_RECONFIG	= 20,
	BCN_BPCC_ADD_LINK	= 21,
	BCN_BPCC_AT2LM		= 22,
	BCN_BPCC_LEGACY_BCN,
	BCN_BPCC_MAX
};

enum BCN_BPCC_HEOP_TYPE {
	BCN_BPCC_HEOP_BSS_COLOR	= 0,
	BCN_BPCC_HEOP_BW_CHG	= 1,
	BCN_BPCC_HEOP_RTS_THLD	= 2,
	BCN_BPCC_HEOP_MAX
};

enum _BCN_MUEDCA_UPDATE_RSN {
	RSN_AP_MUEDCA,
	RSN_RFEATURE_DIS_MUEDCA,
	RSN_CFG_SET,
};

enum _BCN_BSSCCA_UPDATE_RSN {
	RSN_CFG80211_PATH,
	RSN_DBG_PATH,
	RSN_COLLISION_DETECT,
};

struct _BCN_MUEDCA_UPDATE_PARAM {
	UINT8 MUEDCAUpdateReason;
	UINT8 value;
	UINT8 param;
	UINT8 aci;
};

struct _BCN_BSSCCA_UPDATE_PARAM {
	UINT8 BSSCAUpdateReason;
	UCHAR next_color;
	UCHAR count;
};

INT bcn_buf_deinit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

INT bcn_buf_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

VOID bcn_bpcc_mld_id_log_sync(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 mld_id, BOOLEAN log);

INT bcn_bpcc_deinit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

VOID bcn_bpcc_ct_switch(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 ie_type, ...);

INT bcn_bpcc_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

BOOLEAN bcn_bpcc_op_lock(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN query_bssmngr, enum BCN_BPCC_IE_TYPE ie_type);

BOOLEAN bcn_bpcc_op_lock_by_bssmngr(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

void bcn_bpcc_op_lock_set_timer_by_bssmngr(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

VOID bcn_bpcc_op_unlock_by_hwctrl(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN query_bssmngr, UCHAR UpdateReason);

VOID bcn_bpcc_op_unlock_by_bssmngr(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

BOOLEAN bcn_bpcc_op_unlock(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN query_bssmngr);

VOID bcn_bpcc_TaskExit(IN RTMP_ADAPTER *pAd);

NDIS_STATUS bcn_bpcc_TaskInit(IN RTMP_ADAPTER *pAd);

INT clearHwBcnTxMem(RTMP_ADAPTER *pAd);

ULONG ComposeBcnPktHead(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pBeaconFrame);
VOID BcnCheck(RTMP_ADAPTER *pAd);
VOID BcnStopHandle(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN stop);
#ifdef BCN_EXTCAP_VAR_LEN
INT32 set_BeaconExtCapLength_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* BCN_EXTCAP_VAR_LEN */

#ifdef CONFIG_AP_SUPPORT
INT BcnTimAdd(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *ptr);
#endif
VOID ComposeBcnPktTail(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen,
	UCHAR *pBeaconFrame, BOOLEAN bcn);

UINT16 MakeBeacon(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BOOLEAN UpdateRoutine);

BOOLEAN BeaconTransmitRequired(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

VOID UpdateBeaconHandler(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BCN_UPDATE_REASON reason);

INT bcn_bpcc_dbg(
	RTMP_ADAPTER *pAd,
	RTMP_STRING		*arg);

INT BcnBPCC_Channel_Sync(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BCN_UPDATE_REASON reason);

VOID UpdateBeaconHandler_BPCC(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev	*wdev,
	BCN_UPDATE_REASON reason,
	enum BCN_BPCC_IE_TYPE ie_type,
	BOOLEAN ie_update);

BOOLEAN UpdateBeaconProc(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BOOLEAN UpdateRoutine,
	UCHAR UpdatePktType,
	BOOLEAN bMakeBeacon,
	UCHAR UpdateReason);

VOID updateBeaconRoutineCase(
	RTMP_ADAPTER *pAd,
	BOOLEAN UpdateAfterTim);

#if defined(CONFIG_HOTSPOT)
VOID MakeHotSpotIE(
	struct wifi_dev *wdev,
	ULONG *pFrameLen,
	UCHAR *pBeaconFrame);
#endif /* defined(CONFIG_HOTSPOT) */
#endif /* __BCN_H__ */
