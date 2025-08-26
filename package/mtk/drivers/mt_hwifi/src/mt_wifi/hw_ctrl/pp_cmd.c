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
	pp_cmd.c
*/
#include "rt_config.h"

#ifdef CFG_SUPPORT_FALCON_PP

/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/

/*******************************************************************************
 *    PRIVATE TYPES
 ******************************************************************************/
/** FW & DRV sync with pp_cmd.c **/

/*******************************************************************************
 *    PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/
/* For Command*/

/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/

NDIS_STATUS pp_mbss_init(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev)
{
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	UINT_8 dbdc_idx = BAND0;
	PP_CMD_T pp_cmd_cap;
	UINT_8 pp_ctrl = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (wdev != NULL)
		dbdc_idx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	/* ap.c will call this command enable PP by profile */
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_INFO,
		"Preamble puncture initialize via profile.\n");

	/* Set PPEnable Part */

	pp_ctrl = pAd->CommonCfg.pp_enable;

	os_zero_mem(&pp_cmd_cap, sizeof(PP_CMD_T));
	/* Assign Cmd Id */
	pp_cmd_cap.cmd_sub_id = PP_CMD_SET_PP_CAP_CTRL;
	pp_cmd_cap.dbdc_idx = dbdc_idx;
	pp_cmd_cap.pp_ctrl = pp_ctrl;

	if (pp_ctrl == PP_CTRL_PP_EN)
		pp_cmd_cap.pp_auto_mode = TRUE;
	else
		pp_cmd_cap.pp_auto_mode = FALSE;

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_INFO,
		"pp_en %d, pp_auto_mode %d\n",
		pp_cmd_cap.pp_ctrl, pp_cmd_cap.pp_auto_mode);

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		status = UniCmdPPCapCtrl(pAd, &pp_cmd_cap, NULL, NULL, NULL);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		status = pp_cmd_cap_ctrl(pAd, &pp_cmd_cap);

	if (status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
			"Set pp_enable[%d]=%d Fail!\n", dbdc_idx,
			pAd->CommonCfg.pp_enable);
	}
	/* End - Set PPEnable Part */

	return NDIS_STATUS_SUCCESS;
}

#ifdef DOT11_EHT_BE
NDIS_STATUS pp_black_list_init(IN PRTMP_ADAPTER pAd)
{
	struct wifi_dev *wdev = NULL;
	UINT8 band_idx = 0;
	BOOLEAN bl_valid = FALSE;

	if (pAd->CommonCfg.pp_bl_valid == TRUE)
		bl_valid = TRUE;
#ifdef CONFIG_STA_SUPPORT
	if (pAd->StaCfg[0].pp_bl_valid == TRUE)
		bl_valid = TRUE;
#endif
	wdev = pAd->wdev_list[0];
	if (wdev) {
		band_idx = HcGetBandByWdev(wdev);
		if ((pAd->CommonCfg.pp_enable == 1) && (bl_valid == FALSE))
			set_pp_on_off_ctrl(pAd, band_idx, PP_ENABLE);
		else if ((pAd->CommonCfg.pp_enable == 2) && (bl_valid == FALSE))
			set_pp_on_off_ctrl(pAd, band_idx, PP_ENABLE_FBW);
		else
			set_pp_on_off_ctrl(pAd, band_idx, PP_DISABLE);
	}
	return NDIS_STATUS_SUCCESS;
}
#endif

NDIS_STATUS pp_profile_pp_en(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer)
{
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	UINT8 band_idx;
	RTMP_STRING *ptr;

	for (band_idx = BAND0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), band_idx++) {
		if (band_idx > CFG_WIFI_RAM_BAND_NUM)
			return NDIS_STATUS_INVALID_DATA;

		switch (pAd->CommonCfg.eDBDC_mode) {
		case ENUM_SingleBand:
			pAd->CommonCfg.pp_enable = simple_strtol(ptr, 0, 10);
			break;

		default:
			pAd->CommonCfg.pp_enable = simple_strtol(ptr, 0, 10);
			break;
		}
	}
	return status;
}

#ifdef DOT11_EHT_BE
void pp_black_list_reset(IN PRTMP_ADAPTER pAd)
{
	NdisZeroMemory(pAd->CommonCfg.pp_bl, (sizeof(struct  pp_black_list) * MAX_PP_BLACK_LIST_LEN));
	pAd->CommonCfg.pp_bl_valid = FALSE;
#ifdef CONFIG_STA_SUPPORT
	NdisZeroMemory(pAd->StaCfg[0].pp_bl, (sizeof(struct  pp_black_list) * MAX_PP_BLACK_LIST_LEN));
	pAd->StaCfg[0].pp_bl_valid = FALSE;
#endif
}

INT set_pp_on_off_ctrl(IN PRTMP_ADAPTER pAd, UINT8 band_idx, UINT8 enable)
{
	INT status = NDIS_STATUS_FAILURE;
	PP_CMD_T pp_cmd_cap;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (band_idx >= PP_BL_BAND_MAX_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
		"band_idx(%d) out of range!\n", band_idx);
		return NDIS_STATUS_FAILURE;
	}

	os_zero_mem(&pp_cmd_cap, sizeof(PP_CMD_T));
	/* Assign Cmd Id */
	pp_cmd_cap.cmd_sub_id = PP_CMD_SET_PP_CAP_CTRL;
	pp_cmd_cap.dbdc_idx = band_idx;
	pp_cmd_cap.pp_auto_mode = enable;
	pp_cmd_cap.pp_ctrl = 0;
	pp_cmd_cap.pp_bitmap = 0;
	pp_cmd_cap.pp_mgmt_mode = 0;
	pp_cmd_cap.pp_csa_enable = 0;
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		status = UniCmdPPCapCtrl(pAd, &pp_cmd_cap, NULL, NULL, NULL);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		status = NDIS_STATUS_FAILURE;

	if (status == NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_WARN,
		"band(%d),enable(%d) success!\n", band_idx, enable);
	else
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
		"band(%d),enable(%d) failed!\n", band_idx, enable);

	return status;
}

INT set_pp_mu_ctrl_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	INT status = NDIS_STATUS_FAILURE;
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 bitmap = 0;/*DL-MUMIMO:bit[3];UL-MUMIMO:bit[2];DL-OFDMA:bit[1];UL-OFDMA:bit[0];*/
	UINT32 value = 0;/*DL-MUMIMO:bit[3];UL-MUMIMO:bit[2];DL-OFDMA:bit[1];UL-OFDMA:bit[0];*/
	PP_CMD_T pp_cmd_cap;
	struct CMD_PP_MU_CTRL_T mu_ctrl;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_WARN, "-->\n");
	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d-%d", &(band_idx), &(bitmap), &(value));
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_WARN,
				"band_idx(%d), bitmap(%d), value(%d)\n", band_idx, bitmap, value);
		} while (0);

		if (i4Recv != 3) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
				("Format Error! Please enter in the following format\n"
					"band_idx-bitmap-value\n"));
			return NDIS_STATUS_FAILURE;
		}

		if (band_idx >= PP_BL_BAND_MAX_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
				"band_idx(%d) is out of range!\n", band_idx);
			status = NDIS_STATUS_FAILURE;
		} else if (i4Recv != 3) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
				"Wrong parameter!(format should be like: x-y-z)\n");
			status = NDIS_STATUS_FAILURE;
		} else if ((bitmap > 15) || (value > 15)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
				"Error!Parameter out of range!\n");
			status = NDIS_STATUS_FAILURE;
		} else {
			os_zero_mem(&pp_cmd_cap, sizeof(PP_CMD_T));
			pp_cmd_cap.cmd_sub_id = PP_CMD_SET_PP_MU_CTRL;
			mu_ctrl.ucBssIdx = band_idx;
			mu_ctrl.PpMuBitmap = bitmap;
			mu_ctrl.value = value;
#ifdef WIFI_UNIFIED_COMMAND
			if (cap->uni_cmd_support)
				status = UniCmdPPCapCtrl(pAd, &pp_cmd_cap, NULL, &mu_ctrl, NULL);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				status = NDIS_STATUS_FAILURE;

			if (status == NDIS_STATUS_SUCCESS)
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_WARN, "%s:: cmd success!\n", __func__);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR, "%s:: cmd failed!\n", __func__);
		}
	}
	return status;
}

INT pp_mu_ctrl_init(IN PRTMP_ADAPTER pAd)
{
	INT status = NDIS_STATUS_FAILURE;
	UINT8 band_idx = 0;
	UINT8 bitmap = 0;
	UINT8 value = 0;
	UINT8 i = 0;
	UCHAR pp_dl_ofdma = 0;
	UCHAR pp_ul_ofdma = 0;
	UCHAR pp_dl_mu_mimo = 0;
	UCHAR pp_ul_mu_mimo = 0;
	BOOLEAN valid = FALSE;
	struct wifi_dev *wdev = NULL;
	PP_CMD_T pp_cmd_cap;
	struct CMD_PP_MU_CTRL_T mu_ctrl;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_INFO, "%s:: -->\n", __func__);
	wdev = pAd->wdev_list[0];
	if (!wdev)
		return NDIS_STATUS_FAILURE;

	for (i = 0; i < PP_MU_OP_MAX_NUM; i++) {
		if (wlan_config_get_pp_mu_ctrl_valid(wdev, i) == TRUE) {
			valid = TRUE;
			switch (i) {
			case PP_DL_OFDMA:
				bitmap |= (1<<PP_DL_OFDMA);
				pp_dl_ofdma = wlan_config_get_pp_dl_ofdma(wdev);
				if (pp_dl_ofdma != 0)
					value |= (1<<PP_DL_OFDMA);
				break;
			case PP_UL_OFDMA:
				bitmap |= (1<<PP_UL_OFDMA);
				pp_ul_ofdma = wlan_config_get_pp_ul_ofdma(wdev);
				if (pp_ul_ofdma != 0)
					value |= (1<<PP_UL_OFDMA);
				break;
			case PP_DL_MUMIMO:
				bitmap |= (1<<PP_DL_MUMIMO);
				pp_dl_mu_mimo = wlan_config_get_pp_dl_mu_mimo(wdev);
				if (pp_dl_mu_mimo != 0)
					value |= (1<<PP_DL_MUMIMO);
				break;
			case PP_UL_MUMIMO:
				bitmap |= (1<<PP_UL_MUMIMO);
				pp_ul_mu_mimo = wlan_config_get_pp_ul_mu_mimo(wdev);
				if (pp_ul_mu_mimo != 0)
					value |= (1<<PP_UL_MUMIMO);
				break;
			default:
				break;
			}
		}
	}

	if (valid == FALSE)
		return NDIS_STATUS_FAILURE;

	band_idx = HcGetBandByWdev(wdev);
	if (band_idx >= PP_BL_BAND_MAX_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
			"band_idx(%d) is out of range!\n", band_idx);
		return NDIS_STATUS_FAILURE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_WARN, "%s:: bitmap(%d),value=(%d)\n", __func__, bitmap, value);

	os_zero_mem(&pp_cmd_cap, sizeof(PP_CMD_T));
	pp_cmd_cap.cmd_sub_id = PP_CMD_SET_PP_MU_CTRL;
	mu_ctrl.ucBssIdx = band_idx;
	mu_ctrl.PpMuBitmap = bitmap;
	mu_ctrl.value = value;
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		status = UniCmdPPCapCtrl(pAd, &pp_cmd_cap, NULL, &mu_ctrl, NULL);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		status = NDIS_STATUS_FAILURE;

	if (status == NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_WARN, "%s:: init success!\n", __func__);
	else
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR, "%s:: init failed!\n", __func__);

	return status;
}

INT change_pp_cap_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	INT status = NDIS_STATUS_FAILURE;
	INT32 i4Recv = 0;
	UINT32 value1 = 0;
	UINT32 value2 = 0;
	PP_CMD_T pp_cmd_cap;
	struct CMD_CHANGE_PP_CAP_CTRL_T pp_cap;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_WARN, "%s:: -->\n", __func__);
	if (arg) {
		do {
			i4Recv = sscanf(arg, "%x-%x", &(value1), &(value2));
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_WARN,
				"%s:: set value(%x,%x)\n", __func__, value1, value2);
		} while (0);

		if (i4Recv != 2) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
				"Wrong parameter!\n");
			status = NDIS_STATUS_FAILURE;
		} else {
			os_zero_mem(&pp_cmd_cap, sizeof(PP_CMD_T));
			pp_cmd_cap.cmd_sub_id = PP_CMD_CHANGE_PP_CAP_CTRL;
			pp_cap.value1 = value1;
			pp_cap.value2 = value2;
#ifdef WIFI_UNIFIED_COMMAND
			if (cap->uni_cmd_support)
				status = UniCmdPPCapCtrl(pAd, &pp_cmd_cap, NULL, NULL, &pp_cap);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				status = NDIS_STATUS_FAILURE;

			if (status == NDIS_STATUS_SUCCESS)
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_WARN, "%s:: cmd success!\n", __func__);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR, "%s:: cmd failed!\n", __func__);
		}
	}
	return status;
}
#endif

INT set_pp_cap_ctrl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT status;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 u1DbdcIdx = BAND0;
	PP_CMD_T pp_cmd_cap;
	INT32 recv = 0;
	UINT32 pp_ctrl = 0;
	UINT32 pp_auto_mode = 0;
	UINT32 pp_bitmap = 0; /* for BW160 */
	UINT32 pp_mgmt = 0;
	UINT32 pp_csa_enable = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */


	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	os_zero_mem(&pp_cmd_cap, sizeof(PP_CMD_T));

	/* Assign Cmd Id */
	pp_cmd_cap.cmd_sub_id = PP_CMD_SET_PP_CAP_CTRL;

	if (arg) {
		recv = sscanf(arg, "%d-%d-%d-%x-%d-%d", &(u1DbdcIdx), &(pp_auto_mode),
			&(pp_ctrl), &(pp_bitmap), &(pp_mgmt), &(pp_csa_enable));
		if (recv != 6) {
			/* from sigma case */
			recv = sscanf(arg, "%d-%d-%x-%d-%d", &(pp_auto_mode), &(pp_ctrl), &(pp_bitmap),
				&(pp_mgmt), &(pp_csa_enable));
			if (recv != 5) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
					"Format Error! Please enter in the following format\n"
						"Band-PpMgmtEn-PpCtrl-PpBitMap-PpMgmtMode-PPCsaEn\n");
				return TRUE;
			}
			u1DbdcIdx = HcGetBandByWdev(wdev);
		}
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_INFO,
			"%s: band %d, pp_mgmt_en %d, pp_ctrl %d, pp_bitmap 0x%x, pp_mgmt %d, pp_csa_en %d\n",
			__func__, u1DbdcIdx, pp_auto_mode, pp_ctrl, pp_bitmap, pp_mgmt, pp_csa_enable);

		pp_cmd_cap.dbdc_idx = (UINT_8)u1DbdcIdx;
		pp_cmd_cap.pp_auto_mode = (UINT_8)pp_auto_mode;
		pp_cmd_cap.pp_ctrl = (UINT_8)pp_ctrl;
		pp_cmd_cap.pp_bitmap = (UINT_16)pp_bitmap;
		pp_cmd_cap.pp_mgmt_mode = (UINT_8)pp_mgmt;
		pp_cmd_cap.pp_csa_enable = (BOOLEAN)pp_csa_enable;
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			status = UniCmdPPCapCtrl(pAd, &pp_cmd_cap, NULL, NULL, NULL);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			status = pp_cmd_cap_ctrl(pAd, &pp_cmd_cap);

		if (status == NDIS_STATUS_SUCCESS)
			status = TRUE;
		else
			status = FALSE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
			 "Arg is Null\n");
		status = FALSE;
	}

	if (status == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
			 "iwpriv rax0 set ppcapctrl=[Band]-[PpMgmgEn]-[PpEn]-[PpBitMap]-[PpMgmt]\n");
	}

	return status;
}

INT set_pp_alg_ctrl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT status = FALSE;
	INT32 i4Recv0 = 0, i4Recv1 = 0;
	UINT32 u4PpTimerIntv = 0, u4PpThrX2Val = 0, u4PpThrX2Shf = 0;
	UINT32 u4PpThrX3Val = 0, u4PpThrX3Shf = 0, u4PpThrX4Val = 0;
	UINT32 u4PpThrX4Shf = 0, u4PpThrX5Val = 0, u4PpThrX5Shf = 0;
	UINT32 u4PpThrX6Val = 0, u4PpThrX6Shf = 0, u4PpThrX7Val = 0;
	UINT32 u4PpThrX7Shf = 0, u4PpThrX8Val = 0, u4PpThrX8Shf = 0;
	UINT32 u4PpAction = 0, u4Reset = 0;
	UINT8 u1DbdcIdx = 0;
	struct PP_ALG_CTRL_T rPpAlgCtrl;

#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
			"Arg is Null\n");
		return TRUE;
	}

	i4Recv0 = sscanf(arg, "%d:", &u4PpAction);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
		"\x1b[32m u1PpAction = %d, i4Recv0 = %d\x1b[m\n", u4PpAction, i4Recv0);

	if (i4Recv0 != 1) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
			"Value of PpAction is Null\n");
		return TRUE;
	}

	os_zero_mem(&rPpAlgCtrl, sizeof(struct PP_ALG_CTRL_T));
	rPpAlgCtrl.u1PpAction = (UINT8)u4PpAction;
	arg += 2;
	u1DbdcIdx = hc_get_hw_band_idx(pAd);

	switch (u4PpAction) {
	case PP_ALG_SET_TIMER:
		i4Recv1 = sscanf(arg, "%d", &u4PpTimerIntv);

		if (i4Recv1 != 1) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
				"Format Error! Please enter in the following format\n"
					"iwpriv rax0 set ppalgctrl=[pAction]:[PpTimerIntv]\n");
			return TRUE;
		}

		rPpAlgCtrl.u1DbdcIdx = u1DbdcIdx;
		rPpAlgCtrl.u4PpTimerIntv = u4PpTimerIntv;

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u1PpAction = %d\x1b[m\n", rPpAlgCtrl.u1PpAction);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4PpTimerIntv = %d\x1b[m\n", rPpAlgCtrl.u4PpTimerIntv);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m i4Recv1 = %d\x1b[m\n", i4Recv1);

		break;

	case PP_ALG_SET_THR:
		i4Recv1 = sscanf(arg,
			"%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d",
			&u4PpThrX2Val,
			&u4PpThrX2Shf,
			&u4PpThrX3Val,
			&u4PpThrX3Shf,
			&u4PpThrX4Val,
			&u4PpThrX4Shf,
			&u4PpThrX5Val,
			&u4PpThrX5Shf,
			&u4PpThrX6Val,
			&u4PpThrX6Shf,
			&u4PpThrX7Val,
			&u4PpThrX7Shf,
			&u4PpThrX8Val,
			&u4PpThrX8Shf);

		if (i4Recv1 != 14) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
				"Format Error! Please enter in the following format (ThrX2~ThrX8)\n"
					"iwpriv rax0 set ppalgctrl=[pAction]:[ThrX2_Value]:[ThrX2_Shift]\n");
			return TRUE;
		}

		rPpAlgCtrl.u1DbdcIdx = u1DbdcIdx;
		rPpAlgCtrl.u4ThrX2_Value = u4PpThrX2Val;
		rPpAlgCtrl.u4ThrX2_Shift = u4PpThrX2Shf;
		rPpAlgCtrl.u4ThrX3_Value = u4PpThrX3Val;
		rPpAlgCtrl.u4ThrX3_Shift = u4PpThrX3Shf;
		rPpAlgCtrl.u4ThrX4_Value = u4PpThrX4Val;
		rPpAlgCtrl.u4ThrX4_Shift = u4PpThrX4Shf;
		rPpAlgCtrl.u4ThrX5_Value = u4PpThrX5Val;
		rPpAlgCtrl.u4ThrX5_Shift = u4PpThrX5Shf;
		rPpAlgCtrl.u4ThrX6_Value = u4PpThrX6Val;
		rPpAlgCtrl.u4ThrX6_Shift = u4PpThrX6Shf;
		rPpAlgCtrl.u4ThrX7_Value = u4PpThrX7Val;
		rPpAlgCtrl.u4ThrX7_Shift = u4PpThrX7Shf;
		rPpAlgCtrl.u4ThrX8_Value = u4PpThrX8Val;
		rPpAlgCtrl.u4ThrX8_Shift = u4PpThrX8Shf;

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4PpAction = %d\x1b[m\n", rPpAlgCtrl.u1PpAction);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u1DbdcIdx = %d\x1b[m\n", rPpAlgCtrl.u1DbdcIdx);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4ThrX2_Value = %d\x1b[m\n", rPpAlgCtrl.u4ThrX2_Value);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4ThrX2_Shift = %d\x1b[m\n", rPpAlgCtrl.u4ThrX2_Shift);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4ThrX3_Value = %d\x1b[m\n", rPpAlgCtrl.u4ThrX3_Value);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4ThrX3_Shift = %d\x1b[m\n", rPpAlgCtrl.u4ThrX3_Shift);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4ThrX4_Value = %d\x1b[m\n", rPpAlgCtrl.u4ThrX4_Value);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4ThrX4_Shift = %d\x1b[m\n", rPpAlgCtrl.u4ThrX4_Shift);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4ThrX5_Value = %d\x1b[m\n", rPpAlgCtrl.u4ThrX5_Value);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4ThrX5_Shift = %d\x1b[m\n", rPpAlgCtrl.u4ThrX5_Shift);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4ThrX6_Value = %d\x1b[m\n", rPpAlgCtrl.u4ThrX6_Value);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4ThrX6_Shift = %d\x1b[m\n", rPpAlgCtrl.u4ThrX6_Shift);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4ThrX7_Value = %d\x1b[m\n", rPpAlgCtrl.u4ThrX7_Value);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4ThrX7_Shift = %d\x1b[m\n", rPpAlgCtrl.u4ThrX7_Shift);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4ThrX8_Value = %d\x1b[m\n", rPpAlgCtrl.u4ThrX8_Value);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4ThrX8_Shift = %d\x1b[m\n", rPpAlgCtrl.u4ThrX8_Shift);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m i4Recv1 = %d\x1b[m\n", i4Recv1);

		break;

	case PP_ALG_GET_STATISTICS:
		i4Recv1 = sscanf(arg, "%d", &u4Reset);

		if (i4Recv1 != 1) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
				"Format Error! Please enter in the following format\n"
					"iwpriv rax0 set ppalgctrl=[pAction]:[Reset]\n");
			return TRUE;
		}

		rPpAlgCtrl.u1DbdcIdx = u1DbdcIdx;
		rPpAlgCtrl.u1Reset = (UINT8)u4Reset;

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u4PpAction = %d\x1b[m\n", rPpAlgCtrl.u1PpAction);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u1DbdcIdx = %d\x1b[m\n", rPpAlgCtrl.u1DbdcIdx);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m u1Reset = %d\x1b[m\n", rPpAlgCtrl.u1Reset);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_NOTICE,
			"\x1b[32m i4Recv1 = %d\x1b[m\n", i4Recv1);

		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PP, DBG_LVL_ERROR,
			"\x1b[31m u4PpAction = %d is not supported !!\x1b[m\n", rPpAlgCtrl.u1PpAction);
		return TRUE;
	}

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		status = UniCmdPPAlgCtrl(pAd, &rPpAlgCtrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (status == NDIS_STATUS_SUCCESS)
		status = TRUE;
	else
		status = FALSE;

	return status;
}


/* for set/show function*/
NDIS_STATUS pp_cmd_cap_ctrl(IN PRTMP_ADAPTER pAd, IN P_PP_CMD_T pp_cmd_cap)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check pp_cmd_cap not null */
	if (!pp_cmd_cap) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(PP_CMD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_PP, DBG_LVL_INFO,
		"%s: cmd_sub_id = %d, mgmt_mode %d, pp_ctrl %x, pp_auto_mode %d, pp_bitmap %x\n",
		__func__,
		pp_cmd_cap->cmd_sub_id,
		pp_cmd_cap->pp_mgmt_mode,
		pp_cmd_cap->pp_ctrl,
		pp_cmd_cap->pp_auto_mode,
		pp_cmd_cap->pp_bitmap);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PP_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)pp_cmd_cap, sizeof(PP_CMD_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_PP, DBG_LVL_INFO, "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

#endif				/* CFG_SUPPORT_FALCON_PP */
