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
 ***************************************************************************

	Module Name:
	bf.h

	Abstract:
	beamform related setting and interface

*/
#ifndef __BF_H__
#define __BF_H__

struct _RTMP_ADAPTER;
struct wifi_dev;

/*
 * BF Chip Capability
 */
#define TXBF_HW_CAP         BIT(0)
#define TXBF_AID_HW_LIMIT   BIT(1)
#define TXBF_HW_2BF         BIT(2)
#define TXBF_HW_3BF         BIT(3)

struct bf_mode_en_ctrl {
	u8 u1BfNum;
	u8 u1BfBitmap;
	u8 u1BfSelBand[8];
};

struct bf_ctrl {
	bool bf_smth_intl_bypass;
	bool is_bf_enable;
};

bool bf_is_support(struct wifi_dev *wdev);
int bf_type_ctrl(struct _RTMP_ADAPTER *pAd);
int bf_phy_cfg_ctrl(struct _RTMP_ADAPTER *pAd, u8 band_idx);
int bf_enable_ctrl(struct _RTMP_ADAPTER *pAd, u8 band_idx);
int bf_init(struct _RTMP_ADAPTER *pAd);

#endif  /* __BF_H__ */
