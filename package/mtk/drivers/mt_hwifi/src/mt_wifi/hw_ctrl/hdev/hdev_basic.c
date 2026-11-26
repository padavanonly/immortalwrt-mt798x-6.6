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
	hdev.c
*/
#include "rt_config.h"
#include "hdev/hdev.h"

/*Local functions*/
VOID HdevHwResourceInit(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pHwResourceCfg = &ctrl->HwResourceCfg;

	os_zero_mem(pHwResourceCfg, sizeof(HD_RESOURCE_CFG));
	/*initial radio control*/
	rc_init(ctrl);
	/*initial wmm control*/
	wmm_ctrl_init(ctrl, &pHwResourceCfg->wmm_ctrl);
}

VOID HdevHwResourceExit(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pHwResourceCfg = &ctrl->HwResourceCfg;

	HdevExit(ctrl);
	wmm_ctrl_exit(&pHwResourceCfg->wmm_ctrl);
}


static VOID HdevHwResourceShow(HD_RESOURCE_CFG *pHwResourceCfg)
{
	RcRadioShow(pHwResourceCfg);
}


/*
*
*/
VOID HdevObjShow(struct mtk_mac_bss *obj)
{
	MTWF_PRINT("omac_idx\t: %d\n", obj->omac_idx);
	MTWF_PRINT("bss_type\t: %d\n", obj->if_cfg.if_type);
	MTWF_PRINT("bss_idx\t: %d\n", obj->bss_idx);
	MTWF_PRINT("mld_group_idx\t: %d\n", obj->if_cfg.mld_group_idx);
	MTWF_PRINT("mld_type\t: %d\n", obj->if_cfg.mld_type);
	MTWF_PRINT("mld_addr_idx\t: %d\n", obj->mld_addr_idx);
	MTWF_PRINT("mld_group_addr_idx\t: %d\n", obj->mld_group_addr_idx);
	MTWF_PRINT("mld_remap_idx\t: %d\n", obj->mld_remap_idx);
	MTWF_PRINT("wmm_idx\t: %d\n", obj->wmm_idx);
	MTWF_PRINT("band_idx\t: %d\n", obj->band_idx);
	MTWF_PRINT("if_addr\t: %pM\n", obj->if_cfg.if_addr);
	MTWF_PRINT("mld_addr\t: %pM\n", obj->if_cfg.mld_addr);
	MTWF_PRINT("bmc_sta ptr\t: 0x%p\n", obj->bmc_sta);
	MTWF_PRINT("mac_phy ptr\t: 0x%p\n", obj->mac_phy);
}

/*
*
*/

INT32 HdevInit(struct hdev_ctrl *ctrl, RADIO_CTRL *pRadioCtrl)
{
	struct radio_dev *rdev = NULL;

	rdev = &ctrl->rdev;
	os_zero_mem(rdev, sizeof(struct radio_dev));
	rdev->pRadioCtrl = pRadioCtrl;
	return 0;
}


/*
*
*/

INT32 HdevExit(struct hdev_ctrl *ctrl)
{
	struct radio_dev *rdev = NULL;

	rdev = &ctrl->rdev;
	os_zero_mem(rdev, sizeof(struct radio_dev));
	return 0;
}

/*
*
*/
VOID HdevCfgShow(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	HdevHwResourceShow(pHwResource);
}

/*
*
*/
BOOLEAN hdev_obj_state_ready(struct mtk_mac_bss *obj)
{
	return (obj && obj->mac_phy) ? TRUE : FALSE;
}
