
/*
 * Copyright (c) [2021], MediaTek Inc. All rights reserved.
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
#include "rt_config.h"
#include "mgmt/be_internal.h"
#ifdef DOT11_EHT_BE
int wlan_operate_get_mld_addr(struct wifi_dev *wdev, UINT8 *mld_addr)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			" op NULL\n");
		return WLAN_OPER_FAIL;
	}

	NdisMoveMemory(mld_addr, op->eht_oper.mld_addr, MAC_ADDR_LEN);
	return 0;
}
VOID wlan_operate_set_mld_addr(struct wifi_dev *wdev, UINT8 *mld_addr)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			" op NULL\n");
		return;
	}

	NdisMoveMemory(op->eht_oper.mld_addr, mld_addr, MAC_ADDR_LEN);
}

INT32 wlan_operate_set_eht_bw(struct wifi_dev *wdev, UCHAR eht_bw)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;
	UCHAR cap_eht_bw = wlan_config_get_eht_bw(wdev);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	INT32 ret = WLAN_OPER_OK;
	struct freq_cfg cfg;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			" op NULL\n");
		return WLAN_OPER_FAIL;
	}

	if (pAd && pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport &&
		hc_get_hw_band_idx(pAd) == pAd->CommonCfg.DfsParameter.ZeroWaitBandidx &&
		pAd->CommonCfg.DfsParameter.ZwAdjBwFlag)
		return ret;

	if (eht_bw == op->eht_oper.bw)
		return ret;

	if (eht_bw  > cap_eht_bw) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_DEBUG,
				 "new vht_bw:%d > cap_vht_bw: %d, correct to cap_vht_bw\n",
				  eht_bw,
				  cap_eht_bw
				 );
		eht_bw = cap_eht_bw;
		ret = WLAN_OPER_FAIL;
	}

	/*configure loader*/
	os_zero_mem(&cfg, sizeof(cfg));
	phy_freq_get_cfg(wdev, &cfg);
	cfg.eht_bw = eht_bw;
	operate_loader_phy(wdev, &cfg);
	return ret;
}

UINT8 wlan_operate_get_eht_bw(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *)wdev->wpf_op;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			" op NULL\n");
		return 0;
	}

	return op->eht_oper.bw;
}
#endif
