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
 /***************************************************************************
 ***************************************************************************


    Module Name:
    cmm_ie.h
*/

#ifndef __CMM_IE_H__
#define __CMM_IE_H__

int build_txpwr_envelope(
	struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _tx_pwr_env_ie *tpe_ie);

int build_txpwr_envelope_eirp(
	struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _tx_pwr_env_ie *tpe_ie);

int build_txpwr_envelope_eirp_psd(
	struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _tx_pwr_env_ie *tpe_ie,
	u8 u1TxPwrInterpretation, u8 u1TxpwrCategory);

int build_bw_indication_ie(
	struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _bw_ind_ie *bw_ind_ie, UINT8 bw);

#endif /* __CMM_IE_H__ */
