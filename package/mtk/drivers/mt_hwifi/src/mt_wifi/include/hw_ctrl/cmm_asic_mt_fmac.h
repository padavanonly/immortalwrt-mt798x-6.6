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
	cmm_asic_mt.h

	Abstract:
	Ralink Wireless Chip HW related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __CMM_ASIC_MT_FMAC_H__
#define __CMM_ASIC_MT_FMAC_H__

struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _CIPHER_KEY;
struct _MT_TX_COUNTER;
struct _EDCA_PARM;
struct _EXT_CMD_CHAN_SWITCH_T;
struct _BCTRL_INFO_T;
struct _RX_BLK;
struct _BSS_INFO_ARGUMENT_T;

VOID mtf_wrap_protinfo_in_bssinfo(struct _RTMP_ADAPTER *ad, VOID *cookie);
VOID MtfAsicSwitchChannel(struct _RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg);
INT MtfAsicSetRxFilter(struct _RTMP_ADAPTER *pAd, MT_RX_FILTER_CTRL_T RxFilter);
INT MtfAsicSetGPTimer(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout);
#ifndef WIFI_UNIFIED_COMMAND
INT MtfAsicGetTsfTimeByDriver(struct _RTMP_ADAPTER *pAd, UINT32 *high_part, UINT32 *low_part, UCHAR HwBssidIdx);
#endif /* !WIFI_UNIFIED_COMMAND */
VOID MtfAsicUpdateRxWCIDTable(struct _RTMP_ADAPTER *pAd, MT_WCID_TABLE_INFO_T WtblInfo);
UINT32 MtfAsicGetRxStat(struct _RTMP_ADAPTER *pAd, UINT type);
VOID MtfDmacSetExtTTTTHwCRSetting(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
VOID MtfDmacSetMbssHwCRSetting(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
VOID MtfDmacSetExtMbssEnableCR(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
INT32 MtfAsicSetBssidByDriver(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info_argument);
INT MtfAsicTOPInit(struct _RTMP_ADAPTER *pAd);
INT mtf_asic_rts_on_off(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT32 rts_num, UINT32 rts_len, BOOLEAN rts_en);
INT mtf_asic_set_agglimit(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UCHAR ac, struct wifi_dev *wdev, UINT32 agg_limit);
INT mtf_asic_set_rts_retrylimit(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT32 limit);
VOID MtfSetTmrCal(struct _RTMP_ADAPTER *pAd, UCHAR TmrType, UCHAR Channel, UCHAR Bw);
#ifdef DOT11_VHT_AC
INT mtf_asic_set_rts_signal_ta(struct _RTMP_ADAPTER *ad, UINT8 band_idx, BOOLEAN enable);
#endif
#ifdef AIR_MONITOR
INT mtf_set_air_monitor_enable(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR band_idx);
INT mtf_set_air_monitor_rule(struct _RTMP_ADAPTER *pAd, UCHAR *rule, UCHAR band_idx);
INT mtf_set_air_monitor_idx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR mnt_idx, UCHAR band_idx);
#endif

#endif
