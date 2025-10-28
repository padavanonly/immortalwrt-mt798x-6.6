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

*/

#include "rt_config.h"
#include "mgmt/be_internal.h"


/*
*vht phy related
*/
#ifdef DOT11_VHT_AC
VOID vht_oper_init(struct wifi_dev *wdev, struct vht_op *obj)
{
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (IS_APCLI_SYNC_PEER_DEAUTH_ENBL(pAd))
		obj->vht_bw = wlan_operate_get_vht_bw(wdev);
	else
#endif
		obj->vht_bw = VHT_BW_160;
}

VOID vht_oper_exit(struct vht_op *obj)
{
	os_zero_mem(obj, sizeof(*obj));
}


/*
* internal used configure loader
*/
/*
* exported operation function.
*/
VOID operate_loader_vht_bw(struct wlan_operate *op)
{
}

VOID operate_loader_vht_ldpc(struct wlan_operate *op, UCHAR vht_ldpc)
{
	op->vht_oper.vht_ldpc = vht_ldpc;
}
/*
* Set
*/
INT32 wlan_operate_set_vht_bw(struct wifi_dev *wdev, UCHAR vht_bw)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;
	UCHAR cap_vht_bw = wlan_config_get_vht_bw(wdev);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	INT32 ret = WLAN_OPER_OK;
	struct freq_cfg cfg;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_INFO,
		"(caller:%pS), vht_bw=%d\n",
		OS_TRACE, vht_bw);
	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_ERROR,
			" op NULL\n");
		return WLAN_OPER_FAIL;
	}

	if (pAd && pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport &&
		hc_get_hw_band_idx(pAd) == pAd->CommonCfg.DfsParameter.ZeroWaitBandidx &&
		pAd->CommonCfg.DfsParameter.ZwAdjBwFlag)
		return ret;

	if (vht_bw == op->vht_oper.vht_bw) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_INFO,
			"Same vht_bw.\n");
		return ret;
	}

	if (vht_bw  > cap_vht_bw) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_DEBUG,
				 "new vht_bw:%d > cap_vht_bw: %d, correct to cap_vht_bw\n",
				  vht_bw,
				  cap_vht_bw
				 );
		vht_bw = cap_vht_bw;
		ret = WLAN_OPER_FAIL;
	}

	/*configure loader*/
	os_zero_mem(&cfg, sizeof(cfg));
	phy_freq_get_cfg(wdev, &cfg);
	cfg.vht_bw = vht_bw;
	operate_loader_phy(wdev, &cfg);
	return ret;
}

INT32 wlan_operate_set_vht_ldpc(struct wifi_dev *wdev, UCHAR vht_ldpc)
{
	struct wlan_operate *op = NULL;
	INT32 ret = WLAN_OPER_OK;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_ERROR,
				" wdev is NULL!!!\n");
		return WLAN_OPER_FAIL;
	}

	op = (struct wlan_operate *)wdev->wpf_op;
	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_ERROR,
			" op NULL\n");
		return WLAN_OPER_FAIL;
	}

	if (wdev && wdev->wpf_op)
		operate_loader_vht_ldpc(op, vht_ldpc);

	return ret;
}
/*
* Get
*/
UCHAR wlan_operate_get_vht_bw(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *)wdev->wpf_op;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_ERROR,
			" op NULL\n");
		return 0;
	}

	return op->vht_oper.vht_bw;
}

UCHAR wlan_operate_get_vht_ldpc(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *)wdev->wpf_op;

	if (!op) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_ERROR,
			" op NULL\n");
		return 0;
	}

	return op->vht_oper.vht_ldpc;
}
#endif /* DOT11_VHT_AC */
