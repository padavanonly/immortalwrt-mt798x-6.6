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
 ***************************************************************************

	Module Name:
	hw_ctrl.c
*/
#include	"rt_config.h"

extern NDIS_STATUS HwCtrlEnqueueCmd(
	RTMP_ADAPTER *pAd,
	HW_CTRL_TXD HwCtrlTxd);


/*Only can used in this file*/
static INT32 HW_CTRL_BASIC_ENQ(RTMP_ADAPTER *pAd, UINT32 CmdType, UINT32 CmdId, UINT32 Len, VOID *pBuffer)
{
	HW_CTRL_TXD HwCtrlTxd;
	UINT32 ret;

	os_zero_mem(&HwCtrlTxd, sizeof(HW_CTRL_TXD));
	HwCtrlTxd.CmdType = CmdType;
	HwCtrlTxd.CmdId = CmdId;
	HwCtrlTxd.NeedWait = FALSE;
	HwCtrlTxd.wait_time = 0;
	HwCtrlTxd.InformationBufferLength = Len;
	HwCtrlTxd.pInformationBuffer = pBuffer;
	HwCtrlTxd.pRespBuffer = NULL;
	HwCtrlTxd.RespBufferLength = 0;
	HwCtrlTxd.CallbackFun = NULL;
	HwCtrlTxd.CallbackArgs = NULL;
	ret = HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
	return ret;
}

#define HW_CTRL_TXD_BASIC(_pAd, _CmdType, _CmdId, _Len, _pBuffer, _HwCtrlTxd) \
	{ \
		_HwCtrlTxd.CmdType = _CmdType; \
		_HwCtrlTxd.CmdId = _CmdId; \
		_HwCtrlTxd.NeedWait = FALSE; \
		_HwCtrlTxd.wait_time = 0;\
		_HwCtrlTxd.InformationBufferLength = _Len; \
		_HwCtrlTxd.pInformationBuffer = _pBuffer; \
		_HwCtrlTxd.pRespBuffer = NULL; \
		_HwCtrlTxd.RespBufferLength = 0; \
		_HwCtrlTxd.CallbackFun = NULL; \
		_HwCtrlTxd.CallbackArgs = NULL; \
	}

#define HW_CTRL_TXD_RSP(_pAd, _RspLen, _RspBuffer, _wait_time, _HwCtrlTxd) \
	{ \
		_HwCtrlTxd.NeedWait = TRUE; \
		_HwCtrlTxd.wait_time = _wait_time;\
		_HwCtrlTxd.pRespBuffer = _RspBuffer; \
		_HwCtrlTxd.RespBufferLength = _RspLen; \
	}

#define HW_CTRL_TXD_CALLBACK(_pAd, _CallbackFun, _CallbackArgs, _HwCtrlTxd) \
	{ \
		_HwCtrlTxd.CallbackFun = _CallbackFun; \
		_HwCtrlTxd.CallbackArgs = _CallbackArgs; \
	}



/*CMD Definition Start*/
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
#else

#ifdef CONFIG_AP_SUPPORT

VOID RTMP_AP_ADJUST_EXP_ACK_TIME(PRTMP_ADAPTER pAd)
{
	UINT32 ret;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_AP_ADJUST_EXP_ACK_TIME, 0, NULL);
}

VOID RTMP_AP_RECOVER_EXP_ACK_TIME(PRTMP_ADAPTER pAd)
{
	UINT32 ret;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_AP_RECOVER_EXP_ACK_TIME, 0, NULL);
}

#endif /* CONFIG_AP_SUPPORT */


VOID RTMP_SET_LED_STATUS(PRTMP_ADAPTER pAd, UCHAR Status, CHAR BandIdx)
{
	MT_SET_LED_STS led_sts;
	UINT32 ret;

	led_sts.Status = Status;
	led_sts.BandIdx = BandIdx;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_PERIPHERAL, HWCMD_ID_SET_LED_STATUS, sizeof(MT_SET_LED_STS), &led_sts);
}

VOID RTMP_SET_LED(PRTMP_ADAPTER pAd, UINT32 WPSLedMode10, CHAR BandIdx)
{
	MT_SET_LED_STS led_sts;
	UINT32 ret;

	led_sts.Status = WPSLedMode10;
	led_sts.BandIdx = BandIdx;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_PERIPHERAL, HWCMD_ID_LED_WPS_MODE10, sizeof(MT_SET_LED_STS), &led_sts);
}
#endif  /*defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT) */

VOID RTMP_LED_GPIO_MAP(RTMP_ADAPTER *pAd, UINT8 led_index, UINT16 map_index, BOOLEAN ctr_type)
{
	MT_LED_GPIO_MAP led_gpio_map;

	led_gpio_map.led_index = led_index;
	led_gpio_map.map_index = map_index;
	led_gpio_map.ctr_type = ctr_type;
	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_PERIPHERAL, HWCMD_ID_LED_GPIO_MAP, sizeof(MT_LED_GPIO_MAP), &led_gpio_map);
}

/* PCI, USB use the same LP function */
#ifdef CONFIG_STA_SUPPORT
VOID RTMP_PWR_MGT_BIT_WIFI(PRTMP_ADAPTER pAd, UINT16 u2WlanIdx, UINT8 ucPwrMgtBit)
{
	MT_PWR_MGT_BIT_WIFI_T rPwtMgtBitWiFi = {0};
	MTWF_DBG(NULL, DBG_CAT_PS, CATPS_LP, DBG_LVL_INFO, "--->\n");
	rPwtMgtBitWiFi.u2WlanIdx = u2WlanIdx;
	rPwtMgtBitWiFi.ucPwrMgtBit = ucPwrMgtBit;

	if (HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_PS, HWCMD_ID_PWR_MGT_BIT_WIFI, sizeof(MT_PWR_MGT_BIT_WIFI_T),
						  &rPwtMgtBitWiFi) != NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_PS, CATPS_LP, DBG_LVL_ERROR, "Failed to enqueue cmd\n");

	MTWF_DBG(NULL, DBG_CAT_PS, CATPS_LP, DBG_LVL_INFO, "<---\n");
}

VOID HW_ENTER_PS_NULL(PRTMP_ADAPTER pAd, PSTA_ADMIN_CONFIG pStaCfg)
{
	MT_STA_CFG_PTR_T	rStaCfgPtr = {0};
	rStaCfgPtr.pStaCfg = pStaCfg;
	MTWF_DBG(NULL, DBG_CAT_PS, CATPS_LP, DBG_LVL_INFO, "--->\n");

	if (HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_PS, HWCMD_ID_ENTER_PS_NULL, sizeof(MT_STA_CFG_PTR_T),
						  &rStaCfgPtr) != NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_PS, CATPS_LP, DBG_LVL_ERROR, "Failed to enqueue cmd\n");

	MTWF_DBG(NULL, DBG_CAT_PS, CATPS_LP, DBG_LVL_INFO, "<---\n");
}

VOID RTMP_FORCE_WAKEUP(PRTMP_ADAPTER pAd, PSTA_ADMIN_CONFIG pStaCfg)
{
	MT_STA_CFG_PTR_T	rStaCfgPtr = {0};
	struct wifi_dev *wdev = &pStaCfg->wdev;
	rStaCfgPtr.pStaCfg = pStaCfg;

	MTWF_DBG(pAd, DBG_CAT_PS, CATPS_LP, DBG_LVL_INFO, "%s):caller=%pS\n",
		wdev->if_dev->name, OS_TRACE);

	if (HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_PS, HWCMD_ID_FORCE_WAKE_UP, sizeof(MT_STA_CFG_PTR_T),
						  &rStaCfgPtr) != NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_PS, CATPS_LP, DBG_LVL_ERROR, "Failed to enqueue cmd\n");
}

VOID RTMP_SLEEP_FORCE_AUTO_WAKEUP(PRTMP_ADAPTER pAd, PSTA_ADMIN_CONFIG pStaCfg)
{
	MT_STA_CFG_PTR_T	rStaCfgPtr = {0};
	struct wifi_dev *wdev = &pStaCfg->wdev;
	rStaCfgPtr.pStaCfg = pStaCfg;

	MTWF_DBG(pAd, DBG_CAT_PS, CATPS_LP, DBG_LVL_INFO, "(%s):caller=%pS\n",
		wdev->if_dev->name, OS_TRACE);

	if (HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_PS, HWCMD_ID_FORCE_SLEEP_AUTO_WAKEUP, sizeof(MT_STA_CFG_PTR_T),
						  &rStaCfgPtr) != NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_PS, CATPS_LP, DBG_LVL_ERROR, "Failed to enqueue cmd\n");
}
#endif /* CONFIG_STA_SUPPORT */

#ifdef HOST_RESUME_DONE_ACK_SUPPORT
VOID rtmp_host_resume_done_ack(struct _RTMP_ADAPTER *pAd)
{
	if (HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_PS, HWCMD_ID_HOST_RESUME_DONE_ACK, 0, NULL) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PS, CATPS_LP, DBG_LVL_ERROR, "Failed to enqueue cmd\n");
	}
}
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */

VOID RTMP_GET_TEMPERATURE(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT32 *pTemperature)
{
	UINT32 ret;
	HW_CTRL_TXD HwCtrlTxd;
	UINT8 uBandIdx =  ucDbdcIdx;
	os_zero_mem(&HwCtrlTxd, sizeof(HW_CTRL_TXD));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "uBandIdx:%d\n", uBandIdx);
	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_GET_TEMPERATURE, sizeof(UINT8), &uBandIdx, HwCtrlTxd);
	HW_CTRL_TXD_RSP(pAd, sizeof(UINT32), pTemperature, 0, HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}

VOID RTMP_RADIO_ON_OFF_CTRL(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 ucRadio)
{
	HW_CTRL_TXD HwCtrlTxd;
	RADIO_ON_OFF_T RadioOffOn = {0};
	os_zero_mem(&HwCtrlTxd, sizeof(HW_CTRL_TXD));
	RadioOffOn.ucDbdcIdx = ucDbdcIdx;
	RadioOffOn.ucRadio = ucRadio;
	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_RADIO_ON_OFF, sizeof(RADIO_ON_OFF_T), &RadioOffOn, HwCtrlTxd);
	HW_CTRL_TXD_RSP(pAd, 0, NULL, HWCTRL_CMD_WAITTIME, HwCtrlTxd);
	HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}

VOID RTMP_FREE_RRO_SETBL(RTMP_ADAPTER *pAd, UINT16 seid)
{
	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_FREE_RRO_SETBL, sizeof(seid), &seid);
}

#ifdef PRE_CFG_SUPPORT
VOID RTMP_SET_PRECFG_CMD(void *pPreCfgTimerCmdCtrl_v)
{
	UINT32 ret;
	struct PRECFG_TIMER_CMD_CTRL *pPreCfgTimerCmdCtrl = pPreCfgTimerCmdCtrl_v;
	struct _RTMP_ADAPTER *ad = NULL;

	ad = pPreCfgTimerCmdCtrl->pAd;

	ret = HW_CTRL_BASIC_ENQ(ad, HWCMD_TYPE_RADIO, HWCMD_ID_SET_PRECFG_CMD,
		sizeof(struct PRECFG_TIMER_CMD_CTRL), pPreCfgTimerCmdCtrl);
}
#endif

int HW_GET_TSF(
	struct wifi_dev *wdev,
	UINT32 *current_tsf)
{
	struct _RTMP_ADAPTER *ad = NULL;
	HW_CTRL_TXD HwCtrlTxd = {0};
	EXTRA_ARG_TSF_T tsf_arg = {0};

	if (!wdev)
		return NDIS_STATUS_FAILURE;

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (!ad)
		return NDIS_STATUS_FAILURE;

	tsf_arg.ucHwBssidIndex = wdev->OmacIdx;
	HW_CTRL_TXD_BASIC(ad, HWCMD_TYPE_RADIO, HWCMD_ID_GET_TSF, sizeof(tsf_arg), &tsf_arg, HwCtrlTxd);
	HW_CTRL_TXD_RSP(ad, sizeof(UINT32)*2, current_tsf, 0, HwCtrlTxd);

	return HwCtrlEnqueueCmd(ad, HwCtrlTxd);
}

int HW_GET_TSF_DIFF(
	struct wifi_dev *wdev,
	UINT8 BssIdx0,
	UINT8 BssIdx1,
	UINT32 *tsf_diff)
{
	struct _RTMP_ADAPTER *ad = NULL;
	HW_CTRL_TXD HwCtrlTxd = {0};
	struct EXTRA_ARG_TSF_DIFF_T tsf_diff_arg = {0};

	if (!wdev)
		return NDIS_STATUS_FAILURE;

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (!ad)
		return NDIS_STATUS_FAILURE;

	tsf_diff_arg.ucBssIdx0 = BssIdx0;
	tsf_diff_arg.ucBssIdx1 = BssIdx1;
	HW_CTRL_TXD_BASIC(ad, HWCMD_TYPE_RADIO, HWCMD_ID_GET_TSF_DIFF, sizeof(tsf_diff_arg), &tsf_diff_arg, HwCtrlTxd);
	HW_CTRL_TXD_RSP(ad, sizeof(UINT32)*4, tsf_diff, 0, HwCtrlTxd);

	return HwCtrlEnqueueCmd(ad, HwCtrlTxd);
}

#ifdef LINK_TEST_SUPPORT
VOID RTMP_AUTO_LINK_TEST(PRTMP_ADAPTER pAd)
{
	UINT32 ret;
	HW_CTRL_TXD HwCtrlTxd;

	os_zero_mem(&HwCtrlTxd, sizeof(HW_CTRL_TXD));

	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_AUTO_LINK_TEST, 0, NULL, HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}
#endif /* LINK_TEST_SUPPORT */

#ifdef MBO_SUPPORT
VOID RTMP_BSS_TERMINATION(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	HW_CTRL_TXD HwCtrlTxd;

	os_zero_mem(&HwCtrlTxd, sizeof(HW_CTRL_TXD));
	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_BSS_TERMINATION, sizeof(struct wifi_dev), wdev, HwCtrlTxd);
	HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}
#endif /* MBO_SUPPORT */

#ifdef GREENAP_SUPPORT
VOID RTMP_GREENAP_ON_OFF_CTRL(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN ucGreenAP)
{
	GREENAP_ON_OFF_T GreenAPCtrl = {0};
	GreenAPCtrl.ucDbdcIdx = ucDbdcIdx;
	GreenAPCtrl.ucGreenAPOn = ucGreenAP;

	if (HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_GREENAP_ON_OFF, sizeof(GREENAP_ON_OFF_T),
						  &GreenAPCtrl) != NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_PS, CATPS_GREENAP, DBG_LVL_ERROR, "Failed to enqueue cmd\n");
}
#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
VOID rtmp_pcie_aspm_dym_ctrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN fgL1Enable, BOOLEAN fgL0sEnable)
{
	PCIE_ASPM_DYM_CTRL_T pcie_aspm_dym_ctrl = {0};

	pcie_aspm_dym_ctrl.ucDbdcIdx = ucDbdcIdx;
	pcie_aspm_dym_ctrl.fgL1Enable = fgL1Enable;
	pcie_aspm_dym_ctrl.fgL0sEnable = fgL0sEnable;

	MTWF_DBG(NULL, DBG_CAT_PS, CATPS_LP, DBG_LVL_INFO,
		"%s::b(%d),L1(%d),L0s(%d)\n",
		__func__,
		ucDbdcIdx,
		fgL1Enable,
		fgL0sEnable);

	if (HW_CTRL_BASIC_ENQ(pAd,
		HWCMD_TYPE_PS,
		HWCMD_ID_PCIE_ASPM_DYM_CTRL,
		sizeof(PCIE_ASPM_DYM_CTRL_T),
		&pcie_aspm_dym_ctrl) != NDIS_STATUS_SUCCESS)
	MTWF_DBG(pAd, DBG_CAT_PS, CATPS_LP, DBG_LVL_ERROR, "Failed to enqueue cmd\n");
}
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
VOID rtmp_twt_agrt_update(
	struct _RTMP_ADAPTER *ad,
	struct TWT_AGRT_PARA_T *agrt_para)
{
	asic_twt_agrt_update(ad, agrt_para);
}

VOID rtmp_twt_agrt_update_by_cmd_task(
	struct _RTMP_ADAPTER *ad,
	struct TWT_AGRT_PARA_T *agrt_para)
{
	if (HW_CTRL_BASIC_ENQ(ad,
		HWCMD_TYPE_PS,
		HWCMD_ID_TWT_AGRT_UPDATE,
		sizeof(struct TWT_AGRT_PARA_T),
		agrt_para) != NDIS_STATUS_SUCCESS)
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"Failed to enqueue cmd\n");
}

VOID rtmp_twt_agrt_mgmt(
	struct _RTMP_ADAPTER *ad,
	struct TWT_AGRT_PARA_T *agrt_para,
	struct TWT_AGRT_MGMT_T *agrt_mgmt)
{
	asic_twt_agrt_mgmt(ad, agrt_para, agrt_mgmt);
}

VOID rtmp_mgmt_frame_offload(
	struct _RTMP_ADAPTER *ad,
	UINT8 bssinfo_idx,
	UINT16 wcid,
	UINT8 token,
	UINT8 mlo_en,
	UINT16 mld_sta_idx,
	UINT8 action_linkid,
	UINT16 all_linkid_bitmap,
	UINT8 *buf,
	UINT32 len)
{
	struct MGMT_FRAME_OFFLOAD_T offload_para = {0};
	UINT8 *temp_buf = NULL;
	UINT16 temp_len = len + ((len % 4) ? (4 - (len % 4)) : 0);

	os_alloc_mem(ad, (UINT8 **)&temp_buf, temp_len);
	if (!temp_buf) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR, "mem alloc fail\n");
		return;
	}
	os_move_mem(temp_buf, buf, len);

	offload_para.bssinfo_idx = bssinfo_idx;
	offload_para.wcid = wcid;
	offload_para.token = token;
	offload_para.buf = temp_buf;
	offload_para.len = len;
	/* mlo */
	offload_para.mlo_en = mlo_en;
	offload_para.action_linkid = action_linkid;
	offload_para.all_linkid_bitmap = all_linkid_bitmap;
	offload_para.mld_sta_idx = mld_sta_idx;
	asic_twt_mgmt_frame_offload(ad, &offload_para);
}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

VOID RTMP_UPDATE_RAW_COUNTER(PRTMP_ADAPTER pAd)
{
	UINT32 ret;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_UPDATE_DAW_COUNTER, 0, NULL);
}

VOID RTMP_UPDATE_MIB_COUNTER(PRTMP_ADAPTER pAd)
{
	UINT32 ret;

	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_UPDATE_MIB_COUNTER, 0, NULL);
}

#ifdef ZERO_PKT_LOSS_SUPPORT
VOID RTMP_UPDATE_CHANNEL_STATS(PRTMP_ADAPTER pAd)
{
	UINT32 ret;

	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_UPDATE_CHANNEL_STATS, 0, NULL);
}

VOID HANDLE_NULL_ACK_EVENT(PRTMP_ADAPTER pAd, UINT16 data)
{
	UINT32 ret;
	UINT16 wcid = data;
	/*pass wcid as UINT16 so size of */
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_HANDLE_NULL_ACK_EVENT, sizeof(UINT16), (void *)&wcid);
}

/*HANDLE_STA_NULL_ACK_TIMEOUT*/
VOID HANDLE_STA_NULL_ACK_TIMEOUT(PRTMP_ADAPTER pAd, UINT32 len,
		void *ctrl)
{
	UINT32 ret;
	/*pass wcid as UINT16 so size of */
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_HANDLE_STA_NULL_ACK_TIMEOUT,
			len, ctrl);
}
#endif /*ZERO_PKT_LOSS_SUPPORT*/
VOID RTMP_UPDATE_CHANNEL_INFO(PRTMP_ADAPTER pAd, UCHAR ch_index)
{
	UINT32 ret;
	UCHAR index = ch_index;

	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_UPDATE_CHANNEL_INFO, sizeof(UCHAR), (void *)&index);
}
VOID RTMP_SMART_CARRIER_SENSE(PRTMP_ADAPTER pAd)
{
	UINT32 ret;

	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_SMART_CARRIER_SENSE, 0, NULL);
}

VOID RTMP_FWLog2Host(PRTMP_ADAPTER pAd, UINT8 McuDest, UINT32 FWLog2HostCtrl)
{
	UINT32 ret;
	struct CMD_FwLog2Host Info = {0};

	Info.McuDest = McuDest;
	Info. FWLog2HostCtrl = FWLog2HostCtrl;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_FWLog2Host, sizeof(struct CMD_FwLog2Host), &Info);
}

VOID RTMP_SET_RRO_RECOVER(PRTMP_ADAPTER pAd)
{
	UINT32 ret;

	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_SET_RRO_RECOVER, 0, NULL);
}

VOID RTMP_GET_ALL_STA_STATS(PRTMP_ADAPTER pAd, UINT32 eventTypeBitmap)
{
	UINT32 ret;
	struct CMD_STAT_EVENT_TYPE Info = {0};

	Info.eventTypeBitmap = eventTypeBitmap;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_ALL_STA_STATS, sizeof(struct CMD_STAT_EVENT_TYPE), &Info);
}

#ifdef MT_MAC

VOID RTMP_HANDLE_PRETBTT_INT_EVENT(PRTMP_ADAPTER pAd)
{
	UpdateBeaconHandler(pAd, get_default_wdev(pAd), BCN_REASON(BCN_UPDATE_PRETBTT));
}

VOID HW_ADDREMOVE_KEYTABLE(struct _RTMP_ADAPTER *pAd, struct _ASIC_SEC_INFO *pInfo)
{
	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_SECURITY, HWCMD_ID_ADDREMOVE_ASIC_KEY, sizeof(ASIC_SEC_INFO), pInfo);
}

VOID HW_SET_DEL_ASIC_WCID(PRTMP_ADAPTER pAd, ULONG Wcid)
{
	RT_SET_ASIC_WCID	SetAsicWcid;
	SetAsicWcid.WCID = Wcid;
	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_HT_CAP, HWCMD_ID_DEL_ASIC_WCID, sizeof(RT_SET_ASIC_WCID), &SetAsicWcid);
}


#ifdef HTC_DECRYPT_IOT
VOID HW_SET_ASIC_WCID_AAD_OM(PRTMP_ADAPTER pAd, ULONG Wcid, UCHAR value)
{
	RT_SET_ASIC_AAD_OM SetAsicAAD_OM;
	SetAsicAAD_OM.WCID = Wcid;
	SetAsicAAD_OM.Value = value;
	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_HT_CAP, HWCMD_ID_SET_ASIC_AAD_OM, sizeof(RT_SET_ASIC_AAD_OM), &SetAsicAAD_OM);
}
#endif /* HTC_DECRYPT_IOT */

#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
VOID HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(PRTMP_ADAPTER pAd, ULONG Wcid, UCHAR IsEnable)
{
	RT_ASIC_4ADDR_HDR_TRANS Update_4Addr_Hdr_Trans;
	Update_4Addr_Hdr_Trans.Wcid = Wcid;
	Update_4Addr_Hdr_Trans.Enable = IsEnable;
	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_UPDATE_4ADDR_HDR_TRANS, sizeof(RT_ASIC_4ADDR_HDR_TRANS), &Update_4Addr_Hdr_Trans);
}
#endif

VOID HW_SET_BCN_OFFLOAD(RTMP_ADAPTER *pAd,
						UINT8 WdevIdx,
						ULONG WholeLength,
						BOOLEAN Enable,
						UCHAR OffloadPktType,
						ULONG TimIePos,
						ULONG CsaIePos)
{
	UINT32 ret;
	MT_SET_BCN_OFFLOAD rMtSetBcnOffload;
	os_zero_mem(&rMtSetBcnOffload, sizeof(MT_SET_BCN_OFFLOAD));
	rMtSetBcnOffload.WdevIdx = WdevIdx;
	rMtSetBcnOffload.WholeLength = WholeLength;
	rMtSetBcnOffload.Enable = Enable;
	rMtSetBcnOffload.OffloadPktType = OffloadPktType;
	rMtSetBcnOffload.TimIePos = TimIePos;
	rMtSetBcnOffload.CsaIePos = CsaIePos;
	ret = HW_CTRL_BASIC_ENQ(pAd,
							HWCMD_TYPE_RADIO,
							HWCMD_ID_SET_BCN_OFFLOAD,
							sizeof(rMtSetBcnOffload),
							&rMtSetBcnOffload);
}

VOID HW_SET_FRAME_OFFLOAD(RTMP_ADAPTER *pAd,
				UINT8 WdevIdx,
				ULONG WholeLength,
				BOOLEAN Enable,
				UINT16 wcid,
				UINT8 TxType,
				UINT8 TxMode,
				UINT8 TxInterval,
				UCHAR *Buf)
{
	UINT32 ret;
	PMT_SET_FRAME_OFFLOAD pMtSetFrameOffload;

	os_alloc_mem(pAd, (UCHAR **)&pMtSetFrameOffload, sizeof(MT_SET_FRAME_OFFLOAD));

	if (!pMtSetFrameOffload) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATAP_DISC, DBG_LVL_ERROR,
		"Mem alloc fail!\n");
		return;
	}

	os_zero_mem(pMtSetFrameOffload, sizeof(MT_SET_FRAME_OFFLOAD));

	pMtSetFrameOffload->WdevIdx = WdevIdx;
	pMtSetFrameOffload->ucEnable = Enable;
	pMtSetFrameOffload->ucTxType = TxType;
	pMtSetFrameOffload->ucTxMode = TxMode;
	pMtSetFrameOffload->ucTxInterval = TxInterval;
	pMtSetFrameOffload->u2Wcid = wcid;

	pMtSetFrameOffload->u2PktLength = WholeLength;
	if (pMtSetFrameOffload->ucEnable)
		os_move_mem(pMtSetFrameOffload->acPktContent, Buf, pMtSetFrameOffload->u2PktLength);

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_DISC, DBG_LVL_INFO,
					"[%02d] [oper] enable:%d, iob type=%d, interval=%d, (SU/MU)mode=%d, pkt_len=%d\n",
					WdevIdx, pMtSetFrameOffload->ucEnable, pMtSetFrameOffload->ucTxType,
					pMtSetFrameOffload->ucTxInterval, pMtSetFrameOffload->ucTxMode,
					pMtSetFrameOffload->u2PktLength);

	ret = HW_CTRL_BASIC_ENQ(pAd,
							HWCMD_TYPE_RADIO,
							HWCMD_ID_SET_FRAME_OFFLOAD,
							sizeof(MT_SET_FRAME_OFFLOAD),
							pMtSetFrameOffload);

	if (pMtSetFrameOffload)
		os_free_mem(pMtSetFrameOffload);

	return;
}

VOID HW_UPDATE_BSSINFO(RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *BssInfoArgs)
{
	UINT32 ret;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_UPDATE_BSSINFO,
		sizeof(BSS_INFO_ARGUMENT_T), BssInfoArgs);
}

#ifdef DOT11_EHT_BE
VOID HW_UPDATE_DSCBINFO(
	RTMP_ADAPTER *pAd,
	UINT8 BandIdx,
	UINT8 omac_idx,
	UINT8 DscbPresent,
	UINT16 dscb_bitmap,
	UINT8 Ctrl,
	UINT8 Ccfs0,
	UINT8 Ccfs1)
{
	struct CMD_STATIC_PP_DSCB_T dscb;
	UINT32 ret;

	dscb.ucBandIdx = BandIdx;
	dscb.ucOmacIdx = omac_idx;
	dscb.fgIsEhtOpPresent = TRUE;
	dscb.fgIsEhtDscpPresent = DscbPresent;
	dscb.u2DscbBitmap = dscb_bitmap;
	dscb.ucEhtCtrl = Ctrl;
	dscb.ucEhtCcfs0 = Ccfs0;
	dscb.ucEhtCcfs1 = Ccfs1;

	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_DSCB_UPDATE,
		sizeof(struct CMD_STATIC_PP_DSCB_T), &dscb);
}

int HW_GET_AT2LM_RES(
	struct wifi_dev *wdev,
	struct at2lm_res_req_t *req,
	struct at2lm_res_rsp_t *rsp
)
{
	int ret = 0;

	struct _RTMP_ADAPTER *ad = NULL;
	HW_CTRL_TXD HwCtrlTxd = {0};

	if (!wdev)
		return NDIS_STATUS_FAILURE;

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (!ad)
		return NDIS_STATUS_FAILURE;

	HW_CTRL_TXD_BASIC(ad, HWCMD_TYPE_RADIO, HWCMD_ID_AT2LM_RES,
		sizeof(struct at2lm_res_req_t), req, HwCtrlTxd);
	HW_CTRL_TXD_RSP(ad, sizeof(struct at2lm_res_rsp_t), rsp, 0, HwCtrlTxd);

	HwCtrlEnqueueCmd(ad, HwCtrlTxd);

	return ret;
}

int HW_SET_NT2LM(
	struct wifi_dev *wdev,
	struct nt2lm_req_t *req
)
{
	int ret = 0;
	struct _RTMP_ADAPTER *ad = NULL;

	if (!wdev)
		return NDIS_STATUS_FAILURE;

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (!ad)
		return NDIS_STATUS_FAILURE;

	ret = HW_CTRL_BASIC_ENQ(ad, HWCMD_TYPE_RADIO, HWCMD_ID_NT2LM,
		sizeof(struct nt2lm_req_t), req);

	return ret;
}

int HW_SET_RECONFIG_TMR(
	struct wifi_dev *wdev,
	struct reconfig_set_tmr_t *reconfig_info
)
{
	struct _RTMP_ADAPTER *ad = NULL;
	HW_CTRL_TXD HwCtrlTxd = {0};

	if (!wdev)
		return NDIS_STATUS_FAILURE;

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (!ad)
		return NDIS_STATUS_FAILURE;

	HW_CTRL_TXD_BASIC(ad, HWCMD_TYPE_RADIO, HWCMD_ID_RECONFIG_TMR,
		sizeof(struct reconfig_set_tmr_t), reconfig_info, HwCtrlTxd);
	HW_CTRL_TXD_RSP(ad, 0, NULL, 0, HwCtrlTxd);

	return HwCtrlEnqueueCmd(ad, HwCtrlTxd);
}

int HW_REQ_RECONFIG_RM_LINK(
	struct wifi_dev *wdev,
	struct reconfig_rm_link_req_t *req
)
{
	int ret = 0;
	struct _RTMP_ADAPTER *ad = NULL;

	if (!wdev)
		return NDIS_STATUS_FAILURE;

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (!ad)
		return NDIS_STATUS_FAILURE;

	ret = HW_CTRL_BASIC_ENQ(ad, HWCMD_TYPE_RADIO, HWCMD_ID_RECONFIG_RM_LINK,
		sizeof(struct reconfig_rm_link_req_t), req);

	return ret;
}
#endif

VOID HW_SET_BA_REC(
	RTMP_ADAPTER *pAd,
	UINT16 wcid,
	UCHAR tid,
	UINT16 sn,
	UINT16 basize,
	BOOLEAN isAdd,
	INT ses_type,
	UCHAR amsdu)
{
	UINT32 ret;
	MT_BA_CTRL_T SetBaRec;
	os_zero_mem(&SetBaRec, sizeof(MT_BA_CTRL_T));
	SetBaRec.Wcid = wcid;
	SetBaRec.Tid = tid;
	SetBaRec.Sn = sn;
	SetBaRec.BaWinSize = basize;
	SetBaRec.isAdd = isAdd;
	SetBaRec.BaSessionType = ses_type;
	SetBaRec.amsdu = amsdu;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_SET_BA_REC, sizeof(MT_BA_CTRL_T), &SetBaRec);
}

#ifdef ERR_RECOVERY
VOID RTMP_MAC_RECOVERY(struct _RTMP_ADAPTER *pAd, UINT32 Status)
{
	UINT32 value = 0;
	HW_CTRL_T *hw_ctrl = &pAd->HwCtrl;
	INT func_stat = 0;

	value = Status & ERROR_DETECT_MASK;

	/* Trigger error recovery process with fw reload. */
	NdisAcquireSpinLock(&hw_ctrl->ser_lock);
	func_stat = hw_ctrl->ser_func_state;
	NdisReleaseSpinLock(&hw_ctrl->ser_lock);

	if (func_stat != RTMP_TASK_STAT_RUNNING) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				 "ERR! SER func not ready(%d)\n",
				  func_stat);
		/* TODO: do we may hit this case? */
		return;
	}

	if (value != pAd->HwCtrl.ser_status) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_WARN, "Status(0x%x)\n", Status);
		pAd->HwCtrl.ser_status = value;
		RTCMDUp(&pAd->HwCtrl.ser_task);
	} else {
		/* TODO: do we may hit this case? */
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				 "ERR! prev state=%x, new stat=%x\n",
				  pAd->HwCtrl.ser_status, value);
	}
}

INT IsStopingPdma(struct _ERR_RECOVERY_CTRL_T *pErrRecoveryCtl)
{
#ifdef HWIFI_SUPPORT
	return (ser_is_idle_state(pErrRecoveryCtl->ad) ? FALSE : TRUE);
#else
	return (pErrRecoveryCtl->errRecovState == ERR_RECOV_STAGE_STOP_IDLE) ?
			   FALSE : TRUE;
#endif
}

ERR_RECOVERY_STAGE ErrRecoveryCurStage(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl)
{
	if (pErrRecoveryCtl == NULL)
		return ERR_RECOV_STAGE_STOP_IDLE;

	return pErrRecoveryCtl->errRecovStage;
}
#endif /* ERR_RECOVERY */
#endif /*MT_MAC*/

VOID RTMP_SET_STA_DWRR(PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *pEntry)
{
	UINT32 ret;
	MT_VOW_STA_GROUP VoW;
	HW_CTRL_TXD HwCtrlTxd;
	os_zero_mem(&HwCtrlTxd, sizeof(HW_CTRL_TXD));
	os_zero_mem(&VoW, sizeof(MT_VOW_STA_GROUP));
	VoW.StaIdx = pEntry->wcid;

	if (pEntry->wdev->func_idx < 0 || pEntry->wdev->func_idx >= VOW_GROUP_TABLE_MAX) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				 "invalid wdev->func_idx:%d\n", pEntry->wdev->func_idx);
		return;
	}

#ifdef VOW_SUPPORT
	if (pAd->vow_cfg.en_bw_ctrl) {
		if ((pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG) &&
			 (pAd->bss_group.bw_group_idx[pEntry->wdev->func_idx] < VOW_MAX_GROUP_NUM))
			VoW.GroupIdx = pAd->bss_group.bw_group_idx[pEntry->wdev->func_idx];
		else
			VoW.GroupIdx = pAd->bss_group.group_idx[pEntry->wdev->func_idx];
	}
	else
#endif
		VoW.GroupIdx = pEntry->wdev->func_idx % pAd->max_bssgroup_num;

	if ((pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG) || (pAd->vow_cfg.en_bw_ctrl))
		MTWF_PRINT("%s::GroupID:%u,func_idx:%u, func_tb_idx:%u,wcid:%u\n", __func__,
			VoW.GroupIdx, pEntry->wdev->func_idx, pEntry->func_tb_idx, pEntry->wcid);

	VoW.WmmIdx = HcGetWmmIdx(pAd, pEntry->wdev);
	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_SET_STA_DWRR, sizeof(MT_VOW_STA_GROUP), &VoW, HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}

#ifdef VOW_SUPPORT
VOID RTMP_SET_STA_DWRR_QUANTUM(PRTMP_ADAPTER pAd, BOOLEAN restore, UCHAR quantum)
{
	UINT32 ret;
	MT_VOW_STA_QUANTUM VoW;
	HW_CTRL_TXD HwCtrlTxd;

	os_zero_mem(&HwCtrlTxd, sizeof(HW_CTRL_TXD));
	os_zero_mem(&VoW, sizeof(MT_VOW_STA_QUANTUM));

	VoW.restore = restore;
	VoW.quantum = quantum;
	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_SET_STA_DWRR_QUANTUM, sizeof(MT_VOW_STA_GROUP), &VoW, HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}
#endif /* VOW_SUPPORT */

VOID RTMP_SET_THERMAL_RADIO_OFF(PRTMP_ADAPTER pAd)
{
	UINT32 ret;
	HW_CTRL_TXD HwCtrlTxd;
	os_zero_mem(&HwCtrlTxd, sizeof(HW_CTRL_TXD));
	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_THERMAL_PROTECTION_RADIOOFF, 0, NULL, HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}

VOID RTMP_SET_UPDATE_PER(PRTMP_ADAPTER pAd, UINT16 Wcid)
{
	UINT32 ret;
	UINT16 u2WlanIdx = Wcid;
	HW_CTRL_TXD HwCtrlTxd;
	os_zero_mem(&HwCtrlTxd, sizeof(HW_CTRL_TXD));
	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_UPDATE_TX_PER, sizeof(UINT16), &u2WlanIdx, HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}

VOID RTMP_SET_UPDATE_RSSI(PRTMP_ADAPTER pAd)
{
	UINT32 ret;
	HW_CTRL_TXD HwCtrlTxd;
	os_zero_mem(&HwCtrlTxd, sizeof(HW_CTRL_TXD));
	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_UPDATE_RSSI, 0, NULL, HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}


#ifdef	ETSI_RX_BLOCKER_SUPPORT /* RX Blocker Solution */
VOID RTMP_CHECK_RSSI(PRTMP_ADAPTER pAd)
{
	UINT32 ret;
    HW_CTRL_TXD HwCtrlTxd;

	os_zero_mem(&HwCtrlTxd, sizeof(HW_CTRL_TXD));

	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_RX_CHECK_RSSI, 0, NULL, HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}
#endif /* end ETSI_RX_BLOCKER_SUPPORT */

/*
	========================================================================

	Routine Description:
		Read statistical counters from hardware registers and record them
		in software variables for later on query

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	========================================================================
*/
VOID NICUpdateRawCountersNew(
	IN PRTMP_ADAPTER pAd)
{
	if (HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_PS, HWCMD_ID_PERODIC_CR_ACCESS_NIC_UPDATE_RAW_COUNTERS, 0,
						  NULL) != NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_PS, CATPS_LP, DBG_LVL_ERROR, "Failed to enqueue cmd\n");
}

/*----------------------------------------------------------------------------*/
/*!
* \brief     This routine calculates the acumulated TxPER of eaxh TxRate. And
*            according to the calculation result, change CommonCfg.TxRate which
*            is the stable TX Rate we expect the Radio situation could sustained.
*
* \param[in] pAd
*
* \return    None
*/
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_STA_SUPPORT
VOID MlmeDynamicTxRateSwitchingNew(
	IN PRTMP_ADAPTER pAd)
{
	if (HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_PS, HWCMD_ID_PERODIC_CR_ACCESS_MLME_DYNAMIC_TX_RATE_SWITCHING, 0,
						  NULL) != NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_PS, CATPS_LP, DBG_LVL_ERROR, "Failed to enqueue cmd\n");
}
#endif /* CONFIG_STA_SUPPORT */


VOID HW_SET_SLOTTIME(RTMP_ADAPTER *pAd, BOOLEAN bUseShortSlotTime, UCHAR Channel, struct wifi_dev *wdev)
{
	UINT32 ret;
	SLOT_CFG SlotCfg;
	SlotCfg.bUseShortSlotTime = bUseShortSlotTime;
	SlotCfg.Channel = Channel;
	SlotCfg.wdev = wdev;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_SET_SLOTTIME, sizeof(SLOT_CFG), (VOID *)&SlotCfg);
}


VOID HW_SET_TX_BURST(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					 UINT8 ac_type, UINT8 prio,
					 UINT16 level, UINT8 enable)
{
	struct _tx_burst_cfg txop_cfg;
	UINT32 ret;

	if (wdev == NULL)
		return;

	txop_cfg.wdev = wdev;
	txop_cfg.ac_type = ac_type;
	txop_cfg.prio = prio;
	txop_cfg.txop_level = level;
	txop_cfg.enable = enable;
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_TXOP, DBG_LVL_INFO,
		"<caller: %pS>\n -%s: enable=%x, ac_type=%x, prio=%x, txop=%x\n",
		__builtin_return_address(0), __func__,
		txop_cfg.enable, txop_cfg.ac_type,
		txop_cfg.prio, txop_cfg.txop_level);
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_SET_TX_BURST,
		sizeof(struct _tx_burst_cfg), (VOID *)&txop_cfg);
}

#ifdef FTM_SUPPORT
VOID HW_SET_FTM_CHANNEL(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
						UCHAR channel)
{
	struct _ftm_set_channel_cfg ftm_set_channel_cfg;
	UINT32 ret;

	if (wdev == NULL)
		return;

	ftm_set_channel_cfg.pAd = pAd;
	ftm_set_channel_cfg.wdev = wdev;
	ftm_set_channel_cfg.channel = channel;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"channel=%hhu\n", channel);

	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_FTM_SET_CHANNEL,
		sizeof(struct _ftm_set_channel_cfg), (VOID *)&ftm_set_channel_cfg);
}

VOID HW_SET_FTM_MC_BURST(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct _ftm_set_mc_burst ftm_set_mc_burst;
	UINT32 ret;

	if (wdev == NULL)
		return;

	ftm_set_mc_burst.pAd = pAd;
	ftm_set_mc_burst.wdev = wdev;
	ftm_set_mc_burst.band_idx = hc_get_hw_band_idx(pAd);

	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_FTM_SET_MC_BURST,
		sizeof(struct _ftm_set_mc_burst), (VOID *)&ftm_set_mc_burst);

}
#endif /* FTM_SUPPORT */

VOID HW_SET_PART_WMM_PARAM(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					 UCHAR wmm_idx, UINT32 AcNum, UINT32 EdcaType, UINT32 EdcaValue)
{
	struct _part_wmm_cfg part_wmm_cfg;
	UINT32 ret;

	part_wmm_cfg.wmm_idx = wmm_idx;
	part_wmm_cfg.ac_num = AcNum;
	part_wmm_cfg.edca_type = EdcaType;
	part_wmm_cfg.edca_value = EdcaValue;
	part_wmm_cfg.wdev = wdev;
	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_WMM, DBG_LVL_INFO,
			"wmm_idx=%x, ac_num=%x, edca_type=%x, edca_value=%x\n",
			part_wmm_cfg.wmm_idx, part_wmm_cfg.ac_num,
			part_wmm_cfg.edca_type, part_wmm_cfg.edca_value);
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_WMM, HWCMD_ID_PART_SET_WMM,
							sizeof(struct _part_wmm_cfg), (VOID *)&part_wmm_cfg);
}

#ifdef VOW_SUPPORT
VOID HW_SET_VOW_SCHEDULE_CTRL(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN apply_sch_ctrl,
	UINT8 sch_type,
	UINT8 sch_policy)
{
	VOW_SCH_CFG_T vow_sch_cfg;
	UINT32 ret;

	if (apply_sch_ctrl == pAd->vow_sch_cfg.apply_sch_ctrl
		&& sch_type == pAd->vow_sch_cfg.sch_type
		&& sch_policy == pAd->vow_sch_cfg.sch_policy) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_VOW, DBG_LVL_INFO,
			"SCH Unchanging, Don't Apply!");
		return;
	}

	vow_sch_cfg.apply_sch_ctrl = apply_sch_ctrl;
	vow_sch_cfg.sch_type = sch_type;
	vow_sch_cfg.sch_policy = sch_policy;

	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_VOW, DBG_LVL_INFO,
			"apply_sch_ctrl=%x, sch_type=%x, sch_policy=%x\n",
			vow_sch_cfg.apply_sch_ctrl, vow_sch_cfg.sch_type,
			vow_sch_cfg.sch_policy);
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_SET_VOW_SCHEDULE_CTRL,
							sizeof(VOW_SCH_CFG_T), (VOID *)&vow_sch_cfg);
}
#endif
#ifdef DABS_QOS
NDIS_STATUS HW_UPDATE_QOS_PARAM(struct _RTMP_ADAPTER *pAd, UINT32 idx, BOOLEAN set_del)
{
	NDIS_STATUS ret;
	struct qos_param_set_del qos_param_set_del;
	struct qos_param_rec *pqos_param;

	pqos_param = &qos_param_table[idx];
	qos_param_set_del.idx = idx;
	qos_param_set_del.sel_del = set_del;
	qos_param_set_del.qos_setting.u2WlanIdx = pqos_param->wlan_idx;
	qos_param_set_del.qos_setting.u1AC = up_to_ac_mapping[pqos_param->priority];
	qos_param_set_del.qos_setting.u1ForceAC = pqos_param->force_ac;
	qos_param_set_del.qos_setting.u2DelayReq = pqos_param->delay_req;
	qos_param_set_del.qos_setting.u2DelayBound = pqos_param->delay_bound;
	qos_param_set_del.qos_setting.u1DelayWeight = pqos_param->delay_weight;
	qos_param_set_del.qos_setting.u4DataRate = pqos_param->data_rate;
	qos_param_set_del.qos_setting.u2BWReq = pqos_param->bw_req;
	qos_param_set_del.qos_setting.u1Dir = pqos_param->dir;
	qos_param_set_del.qos_setting.u2DropThres = pqos_param->drop_thres;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_WMM, HWCMD_ID_SET_DEL_QOS,
							sizeof(struct qos_param_set_del), (VOID *)&qos_param_set_del);
	if ((ret == NDIS_STATUS_SUCCESS) && set_del)
		pqos_param->in_use = TRUE;
	else
		pqos_param->in_use = FALSE;

	return ret;
}
#endif

VOID HW_BEACON_UPDATE(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR UpdateReason)
{
	UINT32 ret;
	MT_UPDATE_BEACON rMtUpdateBeacon;
	rMtUpdateBeacon.wdev = wdev;
	rMtUpdateBeacon.UpdateReason = UpdateReason;
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO, "wdev(%d), Update reason = %x\n",
			 wdev->wdev_idx, UpdateReason);
	ret = HW_CTRL_BASIC_ENQ(pAd,
							HWCMD_TYPE_RADIO,
							HWCMD_ID_UPDATE_BEACON,
							sizeof(MT_UPDATE_BEACON),
							&rMtUpdateBeacon);
}


#ifdef PKT_BUDGET_CTRL_SUPPORT
VOID HW_SET_PBC_CTRL(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UCHAR type)
{
	struct pbc_ctrl pbc;
	UINT32 ret;
	pbc.entry = entry;
	pbc.wdev = wdev;
	pbc.type = type;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_WMM, HWCMD_ID_PBC_CTRL,
							sizeof(struct pbc_ctrl), (VOID *)&pbc);
}
#endif

/*
 * set RTS Threshold per wdev
 */
VOID HW_SET_RTS_THLD(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR pkt_num, UINT32 length)
{
	struct rts_thld rts;
	UINT32 ret;
	rts.wdev = wdev;
	rts.pkt_thld = pkt_num;
	rts.len_thld = length;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_PROTECT,
							HWCMD_ID_RTS_THLD, sizeof(rts), (VOID *)&rts);
}

VOID HW_SET_PROTECT(struct _RTMP_ADAPTER *pAd,
		    struct wifi_dev *wdev,
		    enum prot_service_type type,
		    UINT32 cookie1,
		    UINT32 cookie2)
{
	struct prot_info *prot = &wdev->prot;

	prot->wdev = wdev;
	prot->type = type;

	switch (type) {
	case PROT_PROTOCOL:
		prot->cookie.protect_mode = wdev->protection;
		HW_CTRL_BASIC_ENQ(pAd,
			  HWCMD_TYPE_PROTECT,
			  HWCMD_ID_HT_PROTECT,
			  sizeof(struct prot_info),
			  (VOID *)prot);
		break;
	case PROT_RTS_THLD:
		prot->cookie.rts.wdev = wdev;
		prot->cookie.rts.pkt_thld = (UCHAR)cookie1;/*volumn of pkt thld is UCHAR*/
		prot->cookie.rts.len_thld = cookie2;
		HW_CTRL_BASIC_ENQ(pAd,
			  HWCMD_TYPE_PROTECT,
			  HWCMD_ID_RTS_THLD,
			  sizeof(struct prot_info),
			  (VOID *)prot);
		break;
#ifdef DOT11_HE_AX
	case PROT_TXOP_DUR_BASE:
		prot->cookie.txop_dur_rts_thld = wlan_operate_get_he_txop_dur_rts_thld(wdev);
		wdev->bss_info_argument.he_bss.txop_dur_rts_thr =
				wlan_operate_get_he_txop_dur_rts_thld(wdev);
		HW_UPDATE_BSSINFO(pAd, &wdev->bss_info_argument);
		break;
#endif
	default:
		MTWF_DBG(pAd, DBG_CAT_HW,
			 CATHW_MAC,
			 DBG_LVL_ERROR,
			 "incorrect prot_service_type:%d\n",
			 type);
		return;
	}
}

/*WIFI_SYS related HwCtrl CMD*/
static UINT32 wifi_sys_queue_work(struct _RTMP_ADAPTER *ad, UINT32 id, struct WIFI_SYS_CTRL *wsys)
{
	UINT32 ret = 0;
	HW_CTRL_TXD HwCtrlTxd;

	if (in_interrupt()) {
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
			"do not equeue wifi sys layer API to dispatch context!\n");
		dump_stack();
		return ret;
	}

	HW_CTRL_TXD_BASIC(
		ad,
		HWCMD_TYPE_WIFISYS,
		id,
		sizeof(struct WIFI_SYS_CTRL),
		wsys,
		HwCtrlTxd
	);
	HW_CTRL_TXD_RSP(ad, 0, NULL, 3000, HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(ad, HwCtrlTxd);
	return ret;
}

/*
*
*/
UINT32 HW_WIFISYS_OPEN(
	RTMP_ADAPTER *pAd,
	struct WIFI_SYS_CTRL *wsys)
{
	return wifi_sys_queue_work(pAd, HWCMD_ID_WIFISYS_OPEN, wsys);
}


/*
*
*/
UINT32 HW_WIFISYS_CLOSE(
	RTMP_ADAPTER *pAd,
	struct WIFI_SYS_CTRL *wsys)
{
	return wifi_sys_queue_work(pAd, HWCMD_ID_WIFISYS_CLOSE, wsys);
}


/*
*
*/
UINT32 HW_WIFISYS_LINKDOWN(
	RTMP_ADAPTER *pAd,
	struct WIFI_SYS_CTRL *wsys)
{
	return wifi_sys_queue_work(pAd, HWCMD_ID_WIFISYS_LINKDOWN, wsys);
}


/*
*
*/
UINT32 HW_WIFISYS_LINKUP(
	RTMP_ADAPTER *pAd,
	struct WIFI_SYS_CTRL *wsys)
{
	return wifi_sys_queue_work(pAd, HWCMD_ID_WIFISYS_LINKUP, wsys);
}


/*
*
*/
UINT32 HW_WIFISYS_PEER_LINKUP(
	RTMP_ADAPTER *pAd,
	struct WIFI_SYS_CTRL *wsys)
{
	return wifi_sys_queue_work(pAd, HWCMD_ID_WIFISYS_PEER_LINKUP, wsys);
}


/*
*
*/
UINT32 HW_WIFISYS_PEER_LINKDOWN(
	RTMP_ADAPTER *pAd,
	struct WIFI_SYS_CTRL *wsys)
{
	return wifi_sys_queue_work(pAd, HWCMD_ID_WIFISYS_PEER_LINKDOWN, wsys);
}

/*
*
*/
UINT32 HW_WIFISYS_PEER_UPDATE(
	RTMP_ADAPTER *pAd,
	struct WIFI_SYS_CTRL *wsys)
{
	return wifi_sys_queue_work(pAd, HWCMD_ID_WIFISYS_PEER_UPDATE, wsys);
}

/*
* for ra update
*/
VOID HW_WIFISYS_RA_UPDATE(
	RTMP_ADAPTER *pAd,
	struct WIFI_SYS_CTRL *wsys)
{
	UINT32 ret;

	ret = HW_CTRL_BASIC_ENQ(pAd,
							HWCMD_TYPE_WIFISYS,
							HWCMD_ID_WIFISYS_PEER_UPDATE,
							sizeof(struct WIFI_SYS_CTRL),
							wsys);

	if (ret != NDIS_STATUS_SUCCESS)
		if (wsys->priv)
			os_free_mem(wsys->priv);
}

VOID HW_GET_TX_STATISTIC(
	struct _RTMP_ADAPTER *pAd,
	TX_STAT_STRUC *P_buf,
	UCHAR num)
{
	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_WIFISYS, HWCMD_ID_GET_TX_STATISTIC,
	sizeof(TX_STAT_STRUC) * num, (VOID *)P_buf);
}

#ifdef NF_SUPPORT_V2
VOID HW_NF_UPDATE(struct _RTMP_ADAPTER *pAd, UCHAR flag)
{
	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_GET_NF_BY_FW, sizeof(UCHAR), (VOID *)&flag);
}
#endif

/* For wifi and md coex in colgin project */
#ifdef WIFI_MD_COEX_SUPPORT
/*Tx Flow (APCCCI->COEX->WLAN_DRIVER->WLAN_FW)*/
VOID HW_WIFI_COEX_APCCCI2FW(struct _RTMP_ADAPTER *pAd, VOID *apccci2fw_msg)
{
	if (HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_WIFI_COEX_APCCCI2FW,
			      sizeof(MT_WIFI_COEX_APCCCI2FW), apccci2fw_msg))
		MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_ERROR,
				"Failed to enqueue cmd\n");
}

VOID HW_QUERY_LTE_SAFE_CHANNEL(struct _RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_NOTICE, "\n");
	if (HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_QUERY_LTE_SAFE_CHANNEL, 0, NULL) != NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_ERROR,
				"Failed to enqueue cmd\n");
}

#endif /* WIFI_MD_COEX_SUPPORT */

#ifdef CFG_SUPPORT_CSI
VOID HW_CSI_CTRL(struct _RTMP_ADAPTER *pAd, void *prCSICtrl)
{
	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_GET_CSI_RAW_DATA,
	sizeof(struct CMD_CSI_CONTROL_T), (VOID *)prCSICtrl);
}
#endif

VOID HW_WA_QUERY(struct _RTMP_ADAPTER *pAd, UINT32 arg0, UINT32 arg1, UINT32 arg2)
{
	struct CMD_WAQuery query = {0};

	query.arg0 = arg0;
	query.arg1 = arg1;
	query.arg2 = arg2;

	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_WA, HWCMD_ID_WA_QUERY, sizeof(struct CMD_WAQuery), &query);
}

VOID HW_WA_MULTIQUERY(struct _RTMP_ADAPTER *pAd, UINT32 arg0, UINT32 arg1, UINT32 arg2, struct _CR4_QUERY_STRUC *cr4_query_list)
{
	cr4_query_list->arg0 = arg0;
	cr4_query_list->arg1 = arg1;
	cr4_query_list->arg2 = arg2;

	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_WA, HWCMD_ID_WA_MULTIQUERY,
		sizeof(*cr4_query_list) + arg1 * sizeof(*cr4_query_list->list), cr4_query_list);
}

#ifdef IGMP_SNOOP_SUPPORT
BOOLEAN HW_WA_MCAST_ENTRY_ADD(struct _RTMP_ADAPTER *pAd, PUCHAR GrpAddr,
	UINT8 BssIdx, UINT8 Type, PUCHAR MemberAddr, PNET_DEV dev, UINT16 wcid)
{
	struct _CR4_MCAST_ENTRY mcast_info = {0};

	if (!GrpAddr || !MemberAddr)
		return NDIS_STATUS_FAILURE;

	mcast_info.pdev = dev;
	mcast_info.wcid = wcid;
	mcast_info.bss_idx = BssIdx;
	mcast_info.filter_type = Type;
	NdisCopyMemory(&mcast_info.group_addr[0], (UCHAR *)GrpAddr, HWCTRL_MCAST_ADDR_LEN);
	NdisCopyMemory(&mcast_info.member_addr[0], (UCHAR *)MemberAddr, MAC_ADDR_LEN);

	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_WA_MCAST, HWCMD_ID_WA_MCAST_ADD, sizeof(struct _CR4_MCAST_ENTRY), &mcast_info);
	/* caller doesn't care the return value */
	return NDIS_STATUS_SUCCESS;
}

BOOLEAN HW_WA_MCAST_ENTRY_DEL(struct _RTMP_ADAPTER *pAd, PUCHAR GrpAddr,
	UINT8 BssIdx, PUCHAR MemberAddr, PNET_DEV dev, UINT16 wcid)
{
	struct _CR4_MCAST_ENTRY mcast_info = {0};

	if (!GrpAddr || !MemberAddr)
		return NDIS_STATUS_FAILURE;

	mcast_info.pdev = dev;
	mcast_info.wcid = wcid;
	mcast_info.bss_idx = BssIdx;
	NdisCopyMemory(&mcast_info.group_addr[0], (UCHAR *)GrpAddr, HWCTRL_MCAST_ADDR_LEN);
	NdisCopyMemory(&mcast_info.member_addr[0], (UCHAR *)MemberAddr, MAC_ADDR_LEN);

	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_WA_MCAST, HWCMD_ID_WA_MCAST_DEL, sizeof(struct _CR4_MCAST_ENTRY), &mcast_info);
	/* caller doesn't care the return value */
	return NDIS_STATUS_SUCCESS;
}

#ifdef IGMP_SNOOPING_DENY_LIST

BOOLEAN HW_WA_MCAST_Deny_LIST(struct _RTMP_ADAPTER *pAd, UINT8 BssIdx,
							UINT8 entry_cnt, UINT8 add_to_list, UINT8 *pAddr, UINT8 *Prefix_list)
{
	struct _CR4_MCAST_DENY_LIST deny_list_info = {0};

	if (!pAddr)
		return NDIS_STATUS_FAILURE;

	deny_list_info.bss_idx = BssIdx;
	deny_list_info.entry_cnt = entry_cnt;
	deny_list_info.add_to_list = add_to_list;
	NdisCopyMemory(&deny_list_info.deny_list[0], (UCHAR *)pAddr, IGMP_DENY_TABLE_SIZE_MAX * IPV6_ADDR_LEN);
	NdisCopyMemory(&deny_list_info.Prefix_list[0], (UCHAR *)Prefix_list, IGMP_DENY_TABLE_SIZE_MAX);

	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_WA_MCAST, HWCMD_ID_WA_MCAST_DENY, sizeof(struct _CR4_MCAST_DENY_LIST), &deny_list_info);
	/* caller doesn't care the return value */
	return NDIS_STATUS_SUCCESS;
}
#endif

#endif

#ifdef QOS_R3
INT32 HW_UPDATE_QOS_CHARA_IE(struct _RTMP_ADAPTER *pAd, VOID *qosData)
{
	UINT32 ret;
	struct UNI_CMD_MURU_QOS_CFG *pQosCfg = (struct UNI_CMD_MURU_QOS_CFG *)qosData;

	if (!pAd || !pQosCfg) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_QOS_MAP, DBG_LVL_ERROR,
			"invalid pointer, pAd:%p, pQosCfg:%p\n", pAd, pQosCfg);
		return NDIS_STATUS_FAILURE;
	}

	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_WMM,
				HWCMD_ID_SET_QOS_CHARACTERISTICS_IE,
				sizeof(*pQosCfg), (VOID *)pQosCfg);

	return ret;
}
#endif
VOID HW_RX_FILTER_CTRL(struct _RTMP_ADAPTER *pAd, void *rxfilter)
{
	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_SET_RX_FILTER_CNTL,
	sizeof(UINT8), (VOID *)rxfilter);
}

VOID HW_GET_STA_SNR(struct _RTMP_ADAPTER *pAd, UINT8 wcid, UINT8 *pSnr)
{
	HW_CTRL_TXD HwCtrlTxd;
	UINT8 uwcid = wcid;

	os_zero_mem(&HwCtrlTxd, sizeof(HW_CTRL_TXD));
	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_GET_STA_SNR, sizeof(UINT8), &uwcid, HwCtrlTxd);
	HW_CTRL_TXD_RSP(pAd, sizeof(UINT8), pSnr, 0, HwCtrlTxd);
	HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}
#ifdef VLAN_SUPPORT
VOID HW_VLAN_BMC_VLANTAG_SET(struct _RTMP_ADAPTER *pAd, UINT8 BssIndex, UINT16 WlanIdx)
{
	struct VLAN_STA_REC vstarec;

	vstarec.u2WlanIdx = WlanIdx;
	vstarec.ucBssIndex = BssIndex;
	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_WIFISYS, HWCMD_ID_UPDATE_STARECVLAN, sizeof(struct VLAN_STA_REC), (VOID *)&vstarec);
}
#endif
