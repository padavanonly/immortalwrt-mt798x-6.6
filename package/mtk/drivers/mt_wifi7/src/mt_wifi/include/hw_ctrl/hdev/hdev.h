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

#ifndef __HDEV_H
#define __HDEV_H

#include "hdev/hdev_basic.h"

/*Radio Control*/
VOID rc_radio_init(struct hdev_ctrl *ctrl, UCHAR rfic, UCHAR dbdc_mode);
VOID rc_radio_exit(struct hdev_ctrl *ctrl, UCHAR dbdc_mode);
struct radio_dev *rc_init(struct hdev_ctrl *ctrl);
VOID RcRadioShow(HD_RESOURCE_CFG *pHwResourceCfg);


PHY_STATUS RcGetRadioCurStat(struct radio_dev *rdev);
VOID RcSetRadioCurStat(struct radio_dev *rdev, PHY_STATUS CurStat);

INT32 wmm_ctrl_init(struct hdev_ctrl *ctrl, struct wmm_ctrl *wctrl);
INT32 wmm_ctrl_exit(struct wmm_ctrl *ctrl);
VOID wmm_ctrl_show_entry(struct wmm_ctrl *ctrl);
struct wmm_entry *wmm_ctrl_get_entry_by_idx(struct hdev_ctrl *ctrl, UINT32 Idx);

/*Omac Control*/
INT32 GetOmacIdx(struct hdev_ctrl *ctrl, UINT32 OmacType, struct radio_dev *rdev, INT8 Idx);
VOID ReleaseOmacIdx(struct hdev_ctrl *ctrl, UINT32 OmacType, struct radio_dev *rdev, UINT32 Idx);



#endif /*__HDEV_H*/
