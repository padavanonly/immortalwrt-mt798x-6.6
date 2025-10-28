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

*/


#ifndef __CMM_ASIC_MT_FW_H__
#define __CMM_ASIC_MT_FW_H__


struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _CIPHER_KEY;
struct _MT_TX_COUNTER;
struct _EDCA_PARM;
struct _EXT_CMD_CHAN_SWITCH_T;
struct _BCTRL_INFO_T;
struct _BSS_INFO_ARGUMENT_T;


/* Function by FW */
INT32 MtAsicSetDevMacByFw(
	struct _RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT64 EnableFeature);

INT32 MtAsicSetBssidByFw(
	struct _RTMP_ADAPTER *pAd,
	struct _BSS_INFO_ARGUMENT_T *bss_info_argument);

#ifdef DOT11_EHT_BE
INT32 MtAsicUpdateDscbInfo(
	struct _RTMP_ADAPTER *pAd,
	struct CMD_STATIC_PP_DSCB_T *dscb);
#endif

INT32 MtAsicSetStaRecByFw(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaCfg);

INT32 MtAsicUpdateStaRecBaByFw(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_BA_CFG_T StaRecBaCfg);

INT32 MtAsicUpdateStaRecPsmByFw(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 Wcid,
	IN UINT8 Psm);

#ifdef HTC_DECRYPT_IOT
/*
	CONNAC F/W cmd PATH:
*/
INT32 MtAsicUpdateStaRecAadOmByFw(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 Wcid,
	IN UINT8 AadOm);
#endif /* HTC_DECRYPT_IOT */

VOID MtAsicDelWcidTabByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT16 wcid_idx);

#ifdef HTC_DECRYPT_IOT
VOID MtAsicSetWcidAAD_OMByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT16 wcid_idx,
	IN UCHAR value);
#endif /* HTC_DECRYPT_IOT */

#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
VOID MtAsicSetWcid4Addr_HdrTransByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT16 wcid_idx,
	IN UCHAR IsEnable,
	IN UCHAR IsApcliEntry);
#endif

VOID MtAsicUpdateRxWCIDTableByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN MT_WCID_TABLE_INFO_T WtblInfo);

INT32 MtAsicUpdateBASessionByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN MT_BA_CTRL_T BaCtrl);

INT32 MtAsicUpdateBASessionOffloadByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN MT_BA_CTRL_T BaCtrl);

VOID MtAsicAddRemoveKeyTabByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _ASIC_SEC_INFO *pInfo);

VOID MtAsicSetSMPSByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	IN UCHAR Smps);

VOID MtAsicGetTxTscByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT32 pn_type_mask,
	OUT UCHAR *pTxTsc);
#ifdef ZERO_PKT_LOSS_SUPPORT
UINT8 mtf_read_skip_tx(IN struct _RTMP_ADAPTER *pAd, UINT16 wcid);
VOID mtf_update_skip_tx(IN struct _RTMP_ADAPTER *pAd, UINT16 wcid, UINT8 set);
#endif

VOID mt_wtbltlv_debug(struct _RTMP_ADAPTER *pAd,
		      UINT16 u2Wcid,
		      UCHAR ucCmdId,
		      UCHAR ucAtion,
		      union _wtbl_debug_u *debug_u);

VOID MtAsicUpdateRtsThldByFw(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR pkt_num, UINT32 length);

VOID MtSetTmrCRByFw(struct _RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx);
#define BA_TRIGGER_OFFLOAD_TIMEOUT 1000
VOID AsicAutoBATrigger(struct _RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Timeout);

INT MtAsicSetRDGByFw(struct _RTMP_ADAPTER *pAd, MT_RDG_CTRL_T *Rdg);

INT MtAsicGetTsfTimeByFirmware(
	struct _RTMP_ADAPTER *pAd,
	UINT32 *high_part,
	UINT32 *low_part,
	UCHAR HwBssidIdx);

INT MtAsicGetTsfDiffTime(
	RTMP_ADAPTER *pAd,
	UINT8 BssIdx0,
	UINT8 BssIdx1,
	UINT32 *Tsf0Bit0_31,
	UINT32 *Tsf0Bit63_32,
	UINT32 *Tsf1Bit0_31,
	UINT32 *Tsf1Bit63_32);

INT32 MtAsicSetAid(
	struct _RTMP_ADAPTER *pAd,
	UINT16 Aid,
	UINT8 OmacIdx);

UINT32 MtAsicGetChBusyCntByFw(struct _RTMP_ADAPTER *pAd, UCHAR ch_idx);
UINT32 MtAsicGetCCACnt(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx);
UINT32 MtAsicGetWmmParamByFw(struct _RTMP_ADAPTER *pAd, UINT32 AcNum, UINT32 EdcaType);

INT32 MtAsicSetMacTxRxByFw(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx);
INT32 MtAsicSetRxvFilter(RTMP_ADAPTER *pAd, BOOLEAN Enable, UCHAR BandIdx);


VOID MtAsicDisableSyncByFw(struct _RTMP_ADAPTER *pAd, UCHAR HWBssidIdx);
VOID MtAsicEnableBssSyncByFw(
	struct _RTMP_ADAPTER *pAd,
	USHORT BeaconPeriod,
	UCHAR HWBssidIdx,
	UCHAR OPMode);

VOID MtAsicSetEdcaParm(struct _RTMP_ADAPTER *pAd, UCHAR idx, UCHAR tx_mode, struct _EDCA_PARM *pEdcaParm);
#ifdef WIFI_UNIFIED_COMMAND
VOID MtAsicUniCmdSetEdcaParm(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR idx, UCHAR tx_mode, PEDCA_PARM pEdcaParm);
#endif /* WIFI_UNIFIED_COMMAND */
INT MtAsicSetWmmParam(RTMP_ADAPTER *pAd, UCHAR idx, UINT32 AcNum, UINT32 EdcaType, UINT32 EdcaValue);
#ifdef WIFI_UNIFIED_COMMAND
INT MtAsicUniCmdSetWmmParam(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR idx, UINT32 AcNum, UINT32 EdcaType, UINT32 EdcaValue);
#endif /* WIFI_UNIFIED_COMMAND */

INT32 MtAsicSetLpi(struct _RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT8 PsdLimit, UCHAR BandIdx);


#ifdef TXRX_STAT_SUPPORT
UINT32 mt_asic_get_mib_txrx_cnts(RTMP_ADAPTER *pAd, UINT8 ucBandIdx);
UINT32 mt_asic_get_all_rate_cnts(RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT8 ucDirection);
UINT32 mt_asic_get_stbc_cnts(RTMP_ADAPTER *pAd, UINT8 ucBandIdx);
UINT32 mt_asic_get_gi_cnts(RTMP_ADAPTER *pAd, UINT8 ucBandIdx);
#endif /* TXRX_STAT_SUPPORT*/
#endif
