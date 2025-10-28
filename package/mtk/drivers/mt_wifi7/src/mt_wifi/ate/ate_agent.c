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
	ate_agent.c
*/

#include "rt_config.h"

#define MCAST_WCID_TO_REMOVE 0	/* Pat: TODO */

/*  CCK Mode */
static CHAR CCKRateTable[] = {0, 1, 2, 3, 8, 9, 10, 11, -1};

/*  OFDM Mode */
static CHAR OFDMRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, -1};

/*  HT Mixed Mode */
static CHAR HTMIXRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
				24, 25, 26, 27, 28, 29, 30, 31, 32, -1};

/* VHT Mode */
static CHAR VHTRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1};

/* HE SU Mode */
static CHAR HERateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 32, 33, -1};

/* HE ER Mode */
static CHAR HEERRateTable[] = {0, 1, 2, 16, 32, 33, 48, -1};

UINT_8  Addr1[6] = {0x00, 0x11, 0x11, 0x11, 0x11, 0x11};
UINT_8  Addr2[6] = {0x00, 0x22, 0x22, 0x22, 0x22, 0x22};
UINT_8  Addr3[6] = {0x00, 0x22, 0x22, 0x22, 0x22, 0x22};

#define QOS_NO_ACK 0x20

#if defined(TXBF_SUPPORT) && defined(MT_MAC)
UCHAR TemplateFrame[32] = {0x88, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0x00, 0xAA, 0xBB, 0x12, 0x34, 0x56,
				0x00, 0x11, 0x22, 0xAA, 0xBB, 0xCC, 0x00, 0x00,
				0x00, QOS_NO_ACK, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				};
#else
static UCHAR TemplateFrame[32] = {0x08, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
					0xFF, 0xFF, 0x00, 0xAA, 0xBB, 0x12, 0x34, 0x56,
					0x00,	0x11, 0x22, 0xAA, 0xBB, 0xCC, 0x00, 0x00,
					0x00, QOS_NO_ACK, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
					};
#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */

static UCHAR tx_mode_map[18+1][16] = {
	{"CCK"},
	{"OFDM"},
	{"HT-Mix"},
	{"HT-GreenField"},
	{"VHT"},
	{"PLR"},  // 5
	{"unknown"},
	{"unknown"},
	{"HE-SU"},// 8
	{"HE-ER"},
	{"HE-TB"},
	{"HE-MU"},
	{"VHT-MIMO"},
	{"EHT_MU_DL_SU"}, //13
	{"EHT_MU_UL_SU"},
	{"EHT_MU_DL_OFDMA"},
	{"EHT_TB_UL_OFDMA"},
};

VOID EEReadAll(PRTMP_ADAPTER pAd, UINT16 *Data, UINT32 size)
{
	UINT32 Offset = 0;
	UINT16 Value = 0;

	for (Offset = 0; Offset < (size >> 1);) {
		RT28xx_EEPROM_READ16(pAd, (Offset << 1), Value);
		Data[Offset] = Value;
		Offset++;
	}
}

INT32 SetATEQid(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	USHORT q_idx;

	q_idx = simple_strtol(Arg, 0, 10);
	TESTMODE_SET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), ac_idx, q_idx);
	MTWF_PRINT("%s: QID:%u\n", __func__, q_idx);
	return TRUE;
}

INT32 SetATETxSEnable(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UINT32 param = 0;

	MTWF_PRINT("%s: Parm = %s\n", __func__, Arg);
	param = simple_strtol(Arg, 0, 10);
	ATECtrl->txs_enable = param;

	return TRUE;
}


INT32 SetATEMPSStart(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = 0;
	UINT enable;

	MTWF_PRINT("%s: Parm = %s\n", __func__, arg);

	enable = simple_strtol(arg, 0, 10);

	if (enable)
		ret = ATEOp->MPSTxStart(pAd);
	else
		ret = ATEOp->MPSTxStop(pAd);

	if (!ret)
		return TRUE;
	else
		return FALSE;
}


static INT32 SetATEMPSParam(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg, UINT type)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Ret = 0;
	INT num_items = 0;
	CHAR *value;
	RTMP_STRING *tmp = arg;
	UINT32 *mps_setting = NULL;
	INT i;

	MTWF_PRINT("%s: Parm = %s\n", __func__, arg);

	value = rstrtok(tmp, ":");

	if (!value)
		goto err0;

	MTWF_PRINT("value:%s, arg:%s, tmp:%s\n", value, arg, tmp);
	num_items = simple_strtol(value, 0, 10);

	if (!num_items)
		goto err0;

	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32) * (num_items));

	if (Ret)
		goto err1;

	for (i = 0, value = rstrtok(NULL, ":"); value; value = rstrtok(NULL, ":")) {
		if (i == num_items)
			break;

		mps_setting[i++] = simple_strtol(value, 0, 10);
	}

	if (i != num_items)
		goto err2;

	ATEOp->MPSSetParm(pAd, type, num_items, mps_setting);

	if (mps_setting)
		os_free_mem(mps_setting);

	return TRUE;
err2:
	MTWF_PRINT("Number of items %d is not matched with number of params %d\n", num_items, i);

	if (mps_setting)
		os_free_mem(mps_setting);

err1:
	MTWF_PRINT("Mem allocate fail\n");
err0:
	MTWF_PRINT("[%u]Format: num_itmes:param1:param2:...\n", type);
	return FALSE;
}

INT32 SetATEMPSPhyMode(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_PHYMODE);
	return ret;
}

INT32 SetATEMPSRate(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_RATE);
	return ret;
}


INT32 SetATEMPSPath(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_PATH);
	return ret;
}

INT32 SetATEMPSPayloadLen(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_PAYLOAD_LEN);
	return ret;
}

INT32 SetATEMPSPktCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_TX_COUNT);
	return ret;
}

INT32 SetATEMPSPwr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_PWR_GAIN);
	return ret;
}

INT32 SetATEMPSNss(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_NSS);
	return ret;
}

INT32 SetATEMPSPktBw(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_PKT_BW);
	return ret;
}

INT32 SetATEDeqCnt(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#ifdef ATE_TXTHREAD
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT deq_cnt;

	deq_cnt = simple_strtol(Arg, 0, 10);
	ATECtrl->deq_cnt = deq_cnt;
	MTWF_PRINT("%s: deq_cnt:%d\n", __func__, deq_cnt);
#endif
	return TRUE;
}

INT32 SetATEDa(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UINT8 wcid = 1;
	UINT8 *addr = NULL;
	UINT8 tmp_addr[MAC_ADDR_LEN + 1];
	INT32 Octet;
	UINT32 rv;
	RTMP_STRING *mac_tok = NULL, *dash_ptr = NULL, *mac_str = NULL;
	UINT32 band_idx = TESTMODE_GET_BAND_IDX(pAd), func_data = 0;
	struct service_test *serv = NULL;
	struct test_wlan_info *winfo = NULL;

	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	serv = (struct service_test *)pAd->serv.serv_handle;
	winfo = serv->test_winfo;

	MTWF_PRINT("Da = %s\n", Arg);

	dash_ptr = strstr(Arg, "-");
	if (dash_ptr) {
		mac_str = dash_ptr+1;
		*dash_ptr = '\0';
		rv = kstrtoint(Arg, 10, &Octet);
		if (rv) {
			MTWF_PRINT("Format Error!\n");
			return FALSE;
		}
		wcid = Octet;
	} else
		mac_str = Arg;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(mac_str) != 17)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr1[wcid-1]);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr3[wcid-1]);
#endif /* CONFIG_STA_SUPPORT */

	if (addr == NULL) {
		MTWF_PRINT("addr is NULL\n");
		return FALSE;
	}

	for (Octet = 0, mac_tok = rstrtok(mac_str, ":"); mac_tok && Octet < SERV_MAC_ADDR_LEN
			; mac_tok = rstrtok(NULL, ":")) {
		/* sanity check */
		if ((strlen(mac_tok) != 2) || (!isxdigit((unsigned char)mac_tok[0])) || (!isxdigit((unsigned char)mac_tok[1])))
			return FALSE;

		AtoH(mac_tok, &addr[Octet++], 1);
	}

	/* sanity check */
	if (Octet != MAC_ADDR_LEN)
		return FALSE;

	sys_ad_move_mem(tmp_addr, addr, MAC_ADDR_LEN);
	net_ad_set(winfo, RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	func_data = 0;
	sys_ad_move_mem(&func_data, tmp_addr, 4);
	net_ad_set(winfo, RF_AT_FUNCID_SET_MAC_ADDRESS, func_data);
	func_data = 0;
	tmp_addr[6] = wcid - 1;
	sys_ad_move_mem(&func_data, tmp_addr + 4, 3);
	net_ad_set(winfo, (RF_AT_FUNCID_SET_MAC_ADDRESS | BIT(18)), func_data);

	MTWF_PRINT("%s: (DA = %02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(addr));

	return TRUE;
}

INT32 SetATESa(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UINT8 wcid = 1;
	RTMP_STRING *Value, *dash_ptr = NULL, *mac_str = NULL;
	INT32 Octet, sta_idx = 0;
	UINT32 rv;
	UCHAR *addr = NULL;
	UINT8 at_addr[MAC_ADDR_LEN + 1];
	UINT32 BandIdx = TESTMODE_GET_BAND_IDX(pAd), func_data = 0;
	struct service_test *serv = NULL;
	struct test_wlan_info *winfo = NULL;
	UCHAR *own_mac = NULL;

	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	serv = (struct service_test *)pAd->serv.serv_handle;
	winfo = serv->test_winfo;

	dash_ptr = strstr(Arg, "-");
	if (dash_ptr) {
		mac_str = dash_ptr+1;
		*dash_ptr = '\0';
		rv = kstrtoint(Arg, 10, &Octet);
		if (rv) {
			MTWF_PRINT("Format Error!\n");
			return FALSE;
		}
		wcid = Octet;
	} else
		mac_str = Arg;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(mac_str) != 17)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr3[0]);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr2[0]);
#endif /* CONFIG_STA_SUPPORT */

	if (addr == NULL) {
		MTWF_PRINT("addr is NULL\n");
		return FALSE;
	}

	for (Octet = 0, Value = rstrtok(mac_str, ":"); Value && Octet < SERV_MAC_ADDR_LEN;
			Value = rstrtok(NULL, ":")) {
		/* sanity check */
		if ((strlen(Value) != 2) || (!isxdigit(*Value)) || (!isxdigit(*(Value + 1))))
			return FALSE;

		AtoH(Value, &addr[Octet++], 1);
	}

	/* sanity check */
	if (Octet != MAC_ADDR_LEN)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		for (sta_idx = 1; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
			UCHAR *tmp_addr = NULL;

			tmp_addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr3[sta_idx]);
			os_move_mem(tmp_addr, addr, MAC_ADDR_LEN);
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		for (sta_idx = 1; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
			UCHAR *tmp_addr = NULL;

			tmp_addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr2[sta_idx]);
			os_move_mem(tmp_addr, addr, MAC_ADDR_LEN);
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	own_mac = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), own_mac);
	os_move_mem(own_mac, addr, MAC_ADDR_LEN);
	sys_ad_move_mem(at_addr, addr, MAC_ADDR_LEN);

	net_ad_set(winfo, RF_AT_FUNCID_SET_DBDC_BAND_IDX, BandIdx);
	sys_ad_move_mem(&func_data, at_addr, 4);
	net_ad_set(winfo, RF_AT_FUNCID_SET_TA, func_data);
	func_data = 0;
	at_addr[6] = wcid - 1;
	sys_ad_move_mem(&func_data, at_addr + 4, 3);
	net_ad_set(winfo, (RF_AT_FUNCID_SET_TA | BIT(18)), func_data);

	MTWF_PRINT("%s: (SA = %02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(addr));

	return TRUE;
}


INT32 SetATEBssid(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UINT8 wcid = 1;
	RTMP_STRING *Value, *dash_ptr = NULL, *mac_str = NULL;
	INT32 Octet, sta_idx = 0;
	UINT32 rv;
	UCHAR *addr = NULL;
	UINT8 at_addr[MAC_ADDR_LEN + 1];
	UINT32 band_idx = TESTMODE_GET_BAND_IDX(pAd), func_data = 0;
	struct service_test *serv = NULL;
	struct test_wlan_info *winfo = NULL;

	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	serv = (struct service_test *)pAd->serv.serv_handle;
	winfo = serv->test_winfo;

	dash_ptr = strstr(Arg, "-");
	if (dash_ptr) {
		mac_str = dash_ptr+1;
		*dash_ptr = '\0';
		rv = kstrtoint(Arg, 10, &Octet);
		if (rv) {
			MTWF_PRINT("Format Error!\n");
			return FALSE;
		}
		wcid = Octet;
	} else
		mac_str = Arg;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(mac_str) != 17)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr2[0]);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr1[0]);
#endif /* CONFIG_STA_SUPPORT */

	if (addr == NULL) {
		MTWF_PRINT("addr is NULL\n");
		return FALSE;
	}

	for (Octet = 0, Value = rstrtok(mac_str, ":"); Value && Octet < SERV_MAC_ADDR_LEN;
			Value = rstrtok(NULL, ":")) {
		/* sanity check */
		if ((strlen(Value) != 2) || (!isxdigit(*Value)) || (!isxdigit(*(Value + 1))))
			return FALSE;

		AtoH(Value, &addr[Octet++], 1);
	}

	/* sanity check */
	if (Octet != MAC_ADDR_LEN)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		for (sta_idx = 1; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
			UCHAR *tmp_addr = NULL;

			tmp_addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr2[sta_idx]);
			os_move_mem(tmp_addr, addr, MAC_ADDR_LEN);
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		for (sta_idx = 1; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
			UCHAR *tmp_addr = NULL;

			tmp_addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr1[sta_idx]);
			os_move_mem(tmp_addr, addr, MAC_ADDR_LEN);
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	sys_ad_move_mem(at_addr, addr, MAC_ADDR_LEN);

	net_ad_set(winfo, RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);
	sys_ad_move_mem(&func_data, at_addr, 4);
	net_ad_set(winfo, RF_AT_FUNCID_SET_BSSID, func_data);
	func_data = 0;
	at_addr[6] = wcid - 1;
	sys_ad_move_mem(&func_data, at_addr + 4, 3);
	net_ad_set(winfo, (RF_AT_FUNCID_SET_BSSID | BIT(18)), func_data);


	MTWF_PRINT("%s: (BSSID = %02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(addr));


	return TRUE;
}

INT32 SetATERxUser(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR band_idx = 0;
	UINT16 user_idx = 0;
	UINT8 i;
	CHAR *value;
	struct _RTMP_ADAPTER *mac_ad = NULL;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			user_idx = simple_strtol(value, 0, 16);
			break;
		default:
			MTWF_PRINT("Number of parameters exceed expectation !!\n");
			return FALSE;
		}
	}

	mac_ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, band_idx);

	if (mac_ad) {
		MTWF_PRINT("%s(): band_idx: %d, user_idx: %d\n", __func__, band_idx, user_idx);

		TESTMODE_SET_PARAM(mac_ad, band_idx, user_idx, user_idx);
	}

	return TRUE;
}

INT32 SetATETxPower0(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_ext("ATETXPOW0", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}

INT32 SetATEChPowerOffset(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_ext("ATECHOFFSET", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}

INT32 SetATECvtPowerOffset(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_ext("ATECVTOFFSET", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}

INT32 SetATETxAntenna(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_ext("ATETXANT", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}


INT32 SetATERxAntenna(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_ext("ATERXANT", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}


INT32 SetATETxFreqOffset(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_dw("ATETXFREQOFFSET", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}

INT32 SetATETxLength(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	struct service_test *serv = NULL;
	struct test_wlan_info *winfo = NULL;
	struct tx_time_param *tx_time_param;
	UINT32 TxLength;

	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	serv = (struct service_test *)pAd->serv.serv_handle;
	winfo = serv->test_winfo;

	TxLength = simple_strtol(Arg, 0, 10);

	TESTMODE_SET_PARAM(pAd, control_band_idx, tx_len, TxLength);
	tx_time_param = (struct tx_time_param *)TESTMODE_GET_PADDR(pAd, control_band_idx, tx_time_param);
	tx_time_param->pkt_tx_time_en = 0;

	net_ad_set(winfo, RF_AT_FUNCID_SET_DBDC_BAND_IDX, control_band_idx);
	net_ad_set(winfo, RF_AT_FUNCID_PKTLEN, TxLength);
	net_ad_set(winfo, RF_AT_FUNCID_SET_TX_TIME, 0);

	MTWF_PRINT("TxLength = %d, control_band_idx=%d\n", TxLength, control_band_idx);

	return TRUE;
}


INT32 SetATETxCount(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT32 TxCount = 0;

	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	MTWF_PRINT("%s: TxCount = %s, control_band_idx = %d\n", __func__, Arg, control_band_idx);

	TxCount = simple_strtol(Arg, 0, 10);

	if (TxCount == 0)
		TxCount = 0xFFFFFFFF;

	TESTMODE_SET_PARAM(pAd, control_band_idx, ATE_TX_CNT, TxCount);
	return TRUE;
}


INT32 CheckMCSValid(PRTMP_ADAPTER pAd, UCHAR tx_mode, UCHAR Mcs)
{
	int Index, TblSize = 0;
	PCHAR pRateTab = NULL;

	switch (tx_mode) {
	case MODE_CCK:
		pRateTab = CCKRateTable;
		TblSize = ARRAY_SIZE(CCKRateTable);
		break;

	case MODE_OFDM:
		pRateTab = OFDMRateTable;
		TblSize = ARRAY_SIZE(OFDMRateTable);
		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		pRateTab = HTMIXRateTable;
		TblSize = ARRAY_SIZE(HTMIXRateTable);
		break;

	case MODE_VHT:
		pRateTab = VHTRateTable;
		TblSize = ARRAY_SIZE(VHTRateTable);
		break;

	case MODE_HE_SU:
		pRateTab = HERateTable;
		TblSize = ARRAY_SIZE(HERateTable);
		break;

	case MODE_HE_EXT_SU:
		pRateTab = HEERRateTable;
		TblSize = ARRAY_SIZE(HEERRateTable);
		break;

	default:
		MTWF_PRINT("Unrecognizable Tx Mode\n");
		return -1;
	}

	Index = 0;

	while (Index < TblSize && pRateTab[Index] != -1) {
		if (pRateTab[Index] == Mcs)
			return 0;

		Index++;
	}

	return -1;
}


INT32 SetATETxMcs(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_dw("ATETXMCS", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}


INT32 SetATETxNss(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_dw("ATETXNSS", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}


INT32 SetATETxLdpc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UCHAR Ldpc;

	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	MTWF_PRINT("%s: Ldpc = %s, control_band_idx = %d\n", __func__, Arg, control_band_idx);

	Ldpc = simple_strtol(Arg, 0, 10);

	if (Ldpc > 1) {
		MTWF_PRINT("Out of range (%d)\n", Ldpc);
		return FALSE;
	}

	TESTMODE_SET_PARAM(pAd, control_band_idx, ldpc, Ldpc);
	return TRUE;
}


INT32 SetATETxStbc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_dw("ATETXSTBC", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}


INT32 SetATETxMode(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_dw("ATETXMODE", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}


INT32 SetATETxGi(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}
	return (mt_agent_cli_set_dw("ATETXGI", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}


INT32 set_ate_max_pe(RTMP_ADAPTER *ad, RTMP_STRING *Arg)
{
	UCHAR max_pe = 0;

	if (VIRTUAL_IF_NUM(ad) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	MTWF_PRINT("%s: Max Packet Extension = %s, control_band_idx = %d\n", __func__,
		Arg, TESTMODE_GET_BAND_IDX(ad));

	max_pe = simple_strtol(Arg, 0, 10);

	if (max_pe > 2) {
		MTWF_PRINT("%d is invalid (0: 0us, 1:8 us, 2:16 us)\n", max_pe);
		return FALSE;
	}

	TESTMODE_SET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), max_pkt_ext, max_pe);
	return TRUE;
}

#if defined(DOT11_HE_AX)
INT32 set_ate_ru_info(RTMP_ADAPTER *ad, RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(ad) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_ext("ATERUINFO", &ad->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}

INT32 set_ate_ru_rx_aid(RTMP_ADAPTER *ad, RTMP_STRING *Arg)
{
	INT32 ret = TRUE, aid = 0;
	UINT32 rv;

	if (VIRTUAL_IF_NUM(ad) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	if (strlen(Arg) > 0) {
		rv = sscanf(Arg, "%d", &aid);
		if (rv != 1) {
			MTWF_PRINT("Format Error!\n");
			return FALSE;
		}
		MTWF_PRINT("%s: RX MU PPDU Aid = %s, control_band_idx = %d\n", __func__, Arg, TESTMODE_GET_BAND_IDX(ad));

		TESTMODE_SET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), mu_rx_aid, aid);
	} else {
		MTWF_PRINT("Invalid string\n");
	}

	return ret;
}

INT32 set_ate_tx_policy(RTMP_ADAPTER *ad, RTMP_STRING *Arg)
{
	INT32 ret = TRUE;
	UINT32 tx_mode = 0, tx_method = 0,  rv;

	if (VIRTUAL_IF_NUM(ad) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	if (strlen(Arg) > 0) {
		rv = sscanf(Arg, "%d:%d", &tx_mode, &tx_method);
		if (rv != 2) {
			MTWF_PRINT("Format Error!\n");
			return FALSE;
		}
		if (tx_mode >= TX_MODE_MAX)
			MTWF_PRINT("[band%d] Unknown TX mode(%d).\n", TESTMODE_GET_BAND_IDX(ad), tx_mode);
		else if (tx_method > 1)
			MTWF_PRINT("[band%d] Unknown TX policy(%d, 0:TXD ; 1:TXC).\n", TESTMODE_GET_BAND_IDX(ad), tx_method);
		else {
			TESTMODE_SET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_method[tx_mode], tx_method);
			MTWF_PRINT("%s: [band%d] Set %s as %s mode.\n", __func__, TESTMODE_GET_BAND_IDX(ad), tx_mode_map[tx_mode],
					(TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_method[tx_mode]) ? "TXC" : "TXD"));
		}
	} else {
		MTWF_PRINT("Invalid string\n");
	}

	return ret;
}
#endif /* DOT11_HE_AX */


INT32 set_ate_retry(RTMP_ADAPTER *ad, RTMP_STRING *Arg)
{
	INT32 ret = TRUE;
	UINT32 retry = 0, rv;

	if (VIRTUAL_IF_NUM(ad) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	if (strlen(Arg) > 0) {
		rv = sscanf(Arg, "%d", &retry);
		if (rv != 1) {
			MTWF_PRINT("Format Error!\n");
			return FALSE;
		}
		MTWF_PRINT("%s: TX PPDU Retry = %s, control_band_idx = %d\n", __func__, Arg, TESTMODE_GET_BAND_IDX(ad));

		TESTMODE_SET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), retry, retry);
	} else {
		MTWF_PRINT("Invalid string\n");
	}

	return ret;
}


INT32 SetATELoadE2p(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UINT32 Ret = 0;
	RTMP_STRING *Src = EEPROM_BIN_FILE_NAME;
	RTMP_OS_FD Srcf;
	INT32 Retval;
	USHORT *WriteEEPROM = NULL;
	INT32 FileLength = 0;
	LONG Value = 0;
	RTMP_OS_FS_INFO	OsFSInfo;

	if (kstrtol(Arg, 10, &Value))
		return NDIS_STATUS_FAILURE;

	MTWF_PRINT("===> %s (value=%ld)\n\n", __func__, Value);

	Ret = os_alloc_mem(pAd, (PUCHAR *)&WriteEEPROM, EEPROM_SIZE);	/* TODO verify */

	if (Ret == NDIS_STATUS_FAILURE)
		return Ret;

	if (Value > 0) {
		/* zero the e2p buffer */
		NdisZeroMemory((PUCHAR)WriteEEPROM, EEPROM_SIZE);
		RtmpOSFSInfoChange(&OsFSInfo, TRUE);

		do {
			/* open the bin file */
			Srcf = RtmpOSFileOpen(Src, O_RDONLY, 0);

			if (IS_FILE_OPEN_ERR(Srcf)) {
				MTWF_PRINT("Error opening file %s\n", Src);
				break;
			}

			/* read the firmware from the file *.bin */
			FileLength = RtmpOSFileRead(Srcf, (RTMP_STRING *)WriteEEPROM, EEPROM_SIZE);

			if (FileLength != EEPROM_SIZE) {
				MTWF_PRINT("error file length (=%d) in e2p.bin\n", FileLength);
				break;
			}

			/* write the content of .bin file to EEPROM */
#if defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT)

			if (IS_PCI_INF(pAd)) {
				UINT16 Index = 0;
				UINT16 Value = 0;
				INT32 E2pSize = 512;/* == 0x200 for PCI interface */
				UINT16 TempData = 0;

				for (Index = 0 ; Index < (E2pSize >> 1); Index++) {
					/* "value" is especially for some compilers... */
					TempData = le2cpu16(WriteEEPROM[Index]);
					Value = TempData;
					RT28xx_EEPROM_WRITE16(pAd, (Index << 1), Value);
				}
			}

#else
			/* rt_ee_write_all(pAd, WriteEEPROM); */
#endif /* defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT) */
			Ret = TRUE;
			break;
		} while (TRUE);

		/* close firmware file */
		if (IS_FILE_OPEN_ERR(Srcf))
			;
		else {
			Retval = RtmpOSFileClose(Srcf);

			if (Retval) {
				MTWF_PRINT("--> Error %d closing %s\n", -Retval, Src);
			}
		}

		/* restore */
		RtmpOSFSInfoChange(&OsFSInfo, FALSE);
	}

	os_free_mem(WriteEEPROM);
	MTWF_PRINT("<=== %s (Ret=%d)\n", __func__, Ret);
	return Ret;
}


#ifdef RTMP_EFUSE_SUPPORT
INT32 SetATELoadE2pFromBuf(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	BOOLEAN Ret = FALSE;
	UINT32 Value = (UINT32)simple_strtol(Arg, 0, 10);

	MTWF_PRINT("===> %s (Value=%d)\n\n", __func__, Value);

	if (Value > 0) {
#if defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT)

		if (IS_PCI_INF(pAd)) {
			UINT16 Index = 0;
			UINT16 Value = 0;
			INT32 E2PSize = 512;/* == 0x200 for PCI interface */
			UINT16 TempData = 0;
			struct physical_device *ph_dev = pAd->physical_dev;

			for (Index = 0; Index < (E2PSize >> 1); Index++) {
				/* "value" is especially for some compilers... */
				TempData = le2cpu16(ph_dev->EEPROMImage[Index]);
				Value = TempData;
				RT28xx_EEPROM_WRITE16(pAd, (Index << 1), Value);
			}
		}

#else
		/* rt_ee_write_all(pAd, pAd->EEPROMImage); */
#endif /* defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT) */
		Ret = TRUE;
	}

	MTWF_PRINT("<=== %s (Ret=%d)\n", __func__, Ret);
	return Ret;
}
#endif /* RTMP_EFUSE_SUPPORT */


INT32 SetATEReadE2p(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 ret;
	UINT16 *Buffer = NULL;
	UINT16 size = EEPROM_SIZE;
	int i;

#if defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT)
	if (PD_GET_E2P_ACCESS_MODE(pAd->physical_dev) == E2P_FLASH_MODE)
		size = get_dev_eeprom_size(pAd);
#endif

	ret = os_alloc_mem(pAd, (PUCHAR *)&Buffer, size);

	if (ret == NDIS_STATUS_FAILURE)
		return ret;

	EEReadAll(pAd, (UINT16 *)Buffer, size);

	for (i = 0; i < (size >> 1); i++) {
		MTWF_PRINT("%4.4x ", *Buffer);

		if (((i + 1) % 16) == 0)
			MTWF_PRINT("\n");

		Buffer++;
	}

	os_free_mem(Buffer);
	return TRUE;
}

INT32 SetATEIpg(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UINT32 ipg = 0;

	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	/* Sanity check for input parameter */
	if (Arg == NULL) {
		MTWF_PRINT("No parameters!!\n");
		goto err0;
	}

	MTWF_PRINT("IPG = %s\n", Arg);

	ipg = simple_strtol(Arg, 0, 10);

	TESTMODE_SET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), ipg_param.ipg, ipg);
	return TRUE;
err0:
	return FALSE;
}

INT32 SetATEPayload(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_ext("ATEPAYLOAD", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}

INT32 SetATEFixedPayload(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_dw("ATEFIXEDPAYLOAD", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}

INT32 SetATEPreamPunc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_ext("ATEPREAMPUNC", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}

INT32 SetATEShow(RTMP_ADAPTER *ad, RTMP_STRING *Arg)
{
	struct _ATE_CTRL *ATECtrl = &ad->ATECtrl;
	RTMP_STRING *Mode_String = NULL;
	RTMP_STRING *TxMode_String = NULL;
	UCHAR         control_band_idx = TESTMODE_GET_BAND_IDX(ad);
	UINT8         loop_index;
	INT           status = TRUE;
	CHAR          *value = 0;
	UCHAR         ExtendInfo = 0;
	INT ret = 0;

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Configure Input Parameter */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/

	/* sanity check for input parameter*/
	if (Arg == NULL) {
		MTWF_PRINT("No parameters!!\n");
		MTWF_PRINT("Please use parameter 0 for Summary INFO, 1 for Detail INFO!!\n");
		return FALSE;
	}

	/* Parsing input parameter */
	for (loop_index = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":"), loop_index++) {
		switch (loop_index) {
		case 0:
			ExtendInfo = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_PRINT("Set wrong parameters\n");
			break;
		}
		}
	}

	MTWF_PRINT("%s: ExtendInfo = %d\n", __func__, ExtendInfo);

	MTWF_PRINT("%s: control_band_idx = %d !!!!!\n", __func__, control_band_idx);

	/* initialize pointer to structure of parameters of Band1 */
	MTWF_PRINT("%s: band[%d] ATE Mode = 0x%x !!!!!\n", __func__, TESTMODE_GET_BAND_IDX(ad),
			 TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), op_mode));

	/* check the ATE mode */
	switch (TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), op_mode)) {
	case (fATE_IDLE | fTEST_IN_RFTEST):
		Mode_String = "ATESTART";
		break;

	case (fATE_EXIT):
		Mode_String = "ATESTOP";
		break;

	case ((fATE_TX_ENABLE | fATE_TXCONT_ENABLE | fATE_IDLE | fTEST_IN_RFTEST)):
		Mode_String = "TXCONT";
		break;

	case ((fATE_TX_ENABLE | fATE_TXCARR_ENABLE | fATE_IDLE | fTEST_IN_RFTEST)):
		Mode_String = "TXCARR";
		break;

	case ((fATE_TX_ENABLE | fATE_TXCARRSUPP_ENABLE | fATE_IDLE | fTEST_IN_RFTEST)):
		Mode_String = "TXCARS";
		break;

	case ((fATE_TX_ENABLE | fATE_IDLE | fTEST_IN_RFTEST)):
		Mode_String = "TXFRAME";
		break;

	case ((fATE_RX_ENABLE | fATE_IDLE | fTEST_IN_RFTEST)):
		Mode_String = "RXFRAME";
		break;
	case ((fATE_TX_ENABLE | fATE_RX_ENABLE | fATE_IDLE | fTEST_IN_RFTEST)):
		Mode_String = "TX/RXFRAME";
		break;

	default: {
		Mode_String = "Unknown ATE mode";
		MTWF_PRINT("ERROR! Unknown ATE mode(0x%x)!\n", TESTMODE_GET_PARAM(ad, TESTMODE_BAND0, op_mode));
		break;
		}
	}

	TxMode_String = "Unknown phy mode";
	if (TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_mode) < ARRAY_SIZE(tx_mode_map))
		TxMode_String = tx_mode_map[TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_mode)];

	/* Generic information */
	MTWF_PRINT("=============================================\n");
	MTWF_PRINT("\t\tBand %d Generic INFO\n", control_band_idx);
	MTWF_PRINT("=============================================\n");

	{
		UCHAR *tx_method = TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_method);
		UCHAR *addr1 = TESTMODE_GET_PADDR(ad, TESTMODE_GET_BAND_IDX(ad), addr1[0][0]);
		UCHAR *addr2 = TESTMODE_GET_PADDR(ad, TESTMODE_GET_BAND_IDX(ad), addr2[0][0]);
		UCHAR *addr3 = TESTMODE_GET_PADDR(ad, TESTMODE_GET_BAND_IDX(ad), addr3[0][0]);

		MTWF_PRINT("ATE Mode = %s\n", Mode_String);
		MTWF_PRINT("ATE Tx Methods:\n");
		for (loop_index = 0 ; loop_index < TX_MODE_MAX ; loop_index++) {
			UCHAR tx_method_str[10] = "TXD";

			if (strlen(tx_mode_map[loop_index])) {
				if (tx_method[loop_index])
					ret = snprintf(tx_method_str, sizeof(tx_method_str), "TXC");
				if (os_snprintf_error(sizeof(tx_method_str), ret))
					MTWF_PRINT("snprintf error!\n");
				MTWF_PRINT("\t%s = %s\n", tx_mode_map[loop_index], tx_method_str);
			}
		}
		MTWF_PRINT("TxAntennaSel = 0x%x\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_ant));
		MTWF_PRINT("RxAntennaSel = 0x%x\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), rx_ant));
		MTWF_PRINT("BBPCurrentBW = %u\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), bw));
		MTWF_PRINT("GI = %u\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), sgi));
		MTWF_PRINT("MCS = %u\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), mcs));
		MTWF_PRINT("TxMode = %s\n", TxMode_String);
		MTWF_PRINT("Addr1 = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(addr1));
		MTWF_PRINT("Addr2 = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(addr2));
		MTWF_PRINT("Addr3 = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(addr3));
		MTWF_PRINT("Channel = %u\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), channel));
#ifdef DOT11_VHT_AC
		MTWF_PRINT("Channel_2nd = %u\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), channel_2nd));

#endif /* DOT11_VHT_AC */
		MTWF_PRINT("Ch_Band = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ch_band));
		MTWF_PRINT("Control Channel = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ctrl_ch));
		MTWF_PRINT("TxLength = %u\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_len));
		MTWF_PRINT("Header Length = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), hdr_len));
		MTWF_PRINT("Payload Length = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), pl_len));
		MTWF_PRINT("IPG = %dus\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ipg_param.ipg));
		MTWF_PRINT("Duty Cycle = %d%%\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), duty_cycle));
		MTWF_PRINT("Pkt Tx Time = %dus\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_time_param.pkt_tx_time));
		MTWF_PRINT("Payload Pattern = 0x%02x\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), payload[0]));
		MTWF_PRINT("RFFreqOffset = %u\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), rf_freq_offset));
		MTWF_PRINT("SKB Allocate = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), is_alloc_skb));
		MTWF_PRINT("wdev_idx = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), wdev_idx));
		MTWF_PRINT("QID = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ac_idx));
		MTWF_PRINT("PriSel = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), pri_sel));
		MTWF_PRINT("Nss = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), nss));
		MTWF_PRINT("PerPktBW = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), per_pkt_bw));
		MTWF_PRINT("PrimaryBWSel = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ch_offset));
		MTWF_PRINT("STBC = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), stbc));
		MTWF_PRINT("LDPC = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ldpc));
		MTWF_PRINT("Preamble = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), preamble));
		MTWF_PRINT("FixedPayload = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), fixed_payload));
		MTWF_PRINT("Thermal Value = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), thermal_val));
#ifdef DOT11_EHT_BE
		MTWF_PRINT("Preamble = %u\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), preampunc));
#endif
	}

	if (ExtendInfo) {
		/* TX information */
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("\t\tTX INFO\n");
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("Sequence = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), seq));
		MTWF_PRINT("TxDoneCount = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ATE_TXDONE_CNT));
		MTWF_PRINT("TxedCount = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ATE_TXED_CNT));

		/* TX power information */
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("\t\tTx Power INFO\n");
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("TxPower0 = %d\n", ATECtrl->TxPower0);
		MTWF_PRINT("TxPower1 = %d\n", ATECtrl->TxPower1);
		MTWF_PRINT("TxPower2 = %d\n", ATECtrl->TxPower2);
		MTWF_PRINT("TxPower3 = %d\n", ATECtrl->TxPower3);
#ifdef ATE_TXTHREAD
		/* TX thread information */
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("\t\tATE TX Thread INFO\n");
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("Current_Init_Thread = %d\n", ATECtrl->current_init_thread);
		MTWF_PRINT("Dequeue Count = %d\n", ATECtrl->deq_cnt);
#endif /* ATE_TXTHREAD */
#ifdef TXBF_SUPPORT
		/* BF related information */
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("\t\tBF Band %d INFO\n", control_band_idx);
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("fgEBfEverEnabled = %d\n", TESTMODE_GET_PARAM(ad, control_band_idx, fgEBfEverEnabled));
		MTWF_PRINT("TXBF INFO Length = %d\n", ATECtrl->txbf_info_len);

		MTWF_PRINT("ETXBF = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ebf));
		MTWF_PRINT("ITXBF = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ibf));
#endif /* TXBF_SUPPORT */
		/* MU related information */
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("\t\tMU INFO\n");
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("MU Enable = %d\n", ATECtrl->mu_enable);
		MTWF_PRINT("MU Users = %d\n", ATECtrl->mu_usrs);
		MTWF_PRINT("wcid_ref = %d\n", ATECtrl->wcid_ref);

	}

	return TRUE;
}

INT32 ShowATERUInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_show("ATERUINFO", &pAd->serv) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}

INT32 ShowATETxDoneInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_show("ATETXDONE", &pAd->serv) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}

INT32 set_ate_pkt_tx_time(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	struct tx_time_param *tx_time_param;
	struct service_test *serv = NULL;
	struct test_wlan_info *winfo = NULL;
	INT32 Ret = 0;
	UINT32 pkt_tx_time = 0;

	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	serv = (struct service_test *)pAd->serv.serv_handle;
	winfo = serv->test_winfo;

	/* Sanity check for input parameter */
	if (Arg == NULL) {
		MTWF_PRINT("No parameters!!\n");
		goto err0;
	}

	tx_time_param = (struct tx_time_param *)TESTMODE_GET_PADDR(pAd, control_band_idx, tx_time_param);
	pkt_tx_time = simple_strtol(Arg, 0, 10);

	tx_time_param->pkt_tx_time = pkt_tx_time;
	tx_time_param->pkt_tx_time_en = 1;

	net_ad_set(winfo, RF_AT_FUNCID_SET_DBDC_BAND_IDX, control_band_idx);
	net_ad_set(winfo, RF_AT_FUNCID_PKTLEN, 0);
	net_ad_set(winfo, RF_AT_FUNCID_SET_TX_TIME, tx_time_param->pkt_tx_time);

	MTWF_PRINT("Pkt Tx time=%dus, control_band_idx=%d\n",
		tx_time_param->pkt_tx_time, control_band_idx);

	if (!Ret)
		return TRUE;
err0:
	return FALSE;
}

INT32 ShowATECfgOnOffProc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 type = 0, band_idx = TESTMODE_GET_BAND_IDX(pAd);
	u_int32 enable = 0;
	struct service_test *serv_test = NULL;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	if (!Arg) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"No parameters!!\n");
		return FALSE;
	}

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx=%u, error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	type = os_str_tol(Arg, 0, 10);
	if (type > 99) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"Type is out of range.\n");
		return FALSE;
	}

	if (pAd != NULL) {
		serv_test = net_ad_wrap_service(pAd);
		ret = mt_serv_get_cfg_on_off(serv_test, band_idx, type, &enable);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"Type=%u, Enable=%u\n", type, enable);
	} else
		ret = SERV_STATUS_SERV_TEST_INVALID_NULL_POINTER;

	return ret;
}

INT32 ShowATERxStat(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 ret = 0;
	UCHAR i = 0;
	UCHAR band_idx = TESTMODE_GET_BAND_IDX(pAd);
	struct test_rx_stat  *test_rx_st;
	struct _TEST_RX_STAT_ALL_INFO rx_all;
	RX_STAT_CNT *rx_stat_cnt;
	UINT8 max_path = max(MCS_NSS_CAP(pAd)->max_path[0], MCS_NSS_CAP(pAd)->max_path[1]);
	struct service_test *serv = (struct service_test *)pAd->serv.serv_handle;
	struct test_rx_stat_band_info *band_info;
	struct test_rx_stat_comm_info *comm_info;
	RTMP_STRING *msg = NULL;
	int max_len = 4096;
	int msg_len = 0;
	char *stdout_flag = "stdout";

	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR) * (max_len));
	if (msg == NULL)
		return FALSE;
	memset(msg, 0x00, max_len);

	msg_len += snprintf(msg + msg_len, max_len - msg_len, "%s: band_idx: %d\n", __func__, band_idx);
	if (os_snprintf_error(max_len - msg_len, msg_len))
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "Line%d: snprintf error!\n", __LINE__);

	NdisZeroMemory(&rx_all, sizeof(rx_all));
	ret = chip_get_rx_stat_all(pAd, band_idx, 0, &rx_all);

	rx_stat_cnt = &pAd->rx_stat_cnt;
	test_rx_st = serv->test_rx_statistic + band_idx;
	band_info = &test_rx_st->rx_st_band[band_idx];
	comm_info = &test_rx_st->rx_st_comm;

	msg_len += snprintf(msg + msg_len, max_len - msg_len, "rcpi: ");
	if (os_snprintf_error(max_len - msg_len, msg_len))
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "Line%d: snprintf error!\n", __LINE__);
	for (i = 0; i < max_path; i++) {
		msg_len += snprintf(msg + msg_len, max_len - msg_len, "%d ", rx_all.rx_path_info[i].rcpi);
		if (os_snprintf_error(max_len - msg_len, msg_len))
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "Line%d: snprintf error!\n", __LINE__);
	}

	msg_len += snprintf(msg + msg_len, max_len - msg_len, "\nrssi: ");
	if (os_snprintf_error(max_len - msg_len, msg_len))
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "Line%d: snprintf error!\n", __LINE__);
	for (i = 0; i < max_path; i++) {
		msg_len += snprintf(msg + msg_len, max_len - msg_len, "%d ", rx_all.rx_path_info[i].rssi);
		if (os_snprintf_error(max_len - msg_len, msg_len))
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "Line%d: snprintf error!\n", __LINE__);
	}

	msg_len += snprintf(msg + msg_len, max_len - msg_len, "\nfagc rssi ib: ");
	if (os_snprintf_error(max_len - msg_len, msg_len))
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "Line%d: snprintf error!\n", __LINE__);
	for (i = 0; i < max_path; i++) {
		msg_len += snprintf(msg + msg_len, max_len - msg_len, "%d ", rx_all.rx_path_info[i].fagc_ib_rssi);
		if (os_snprintf_error(max_len - msg_len, msg_len))
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "Line%d: snprintf error!\n", __LINE__);
	}

	msg_len += snprintf(msg + msg_len, max_len - msg_len, "\nfagc rssi wb: ");
	if (os_snprintf_error(max_len - msg_len, msg_len))
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "Line%d: snprintf error!\n", __LINE__);
	for (i = 0; i < max_path; i++) {
		msg_len += snprintf(msg + msg_len, max_len - msg_len, "%d ", rx_all.rx_path_info[i].fagc_wb_rssi);
		if (os_snprintf_error(max_len - msg_len, msg_len))
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "Line%d: snprintf error!\n", __LINE__);
	}

	msg_len += snprintf(msg + msg_len, max_len - msg_len, "\n");
	if (os_snprintf_error(max_len - msg_len, msg_len))
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "Line%d: snprintf error!\n", __LINE__);

	/* update cumulated rx stat count */
	rx_stat_cnt->all_mac_rx_mdrdy_cnt += rx_all.rx_band_info.mac_rx_mdrdy_cnt;
	rx_stat_cnt->all_mac_rx_fcs_err_cnt += rx_all.rx_band_info.mac_rx_fcs_err_cnt;
	rx_stat_cnt->all_mac_rx_len_mismatch += rx_all.rx_band_info.mac_rx_len_mismatch;
	rx_stat_cnt->all_mac_rx_fifo_full += rx_all.rx_comm_info.rx_fifo_full;
	rx_stat_cnt->all_mac_rx_ok_cnt += rx_all.rx_band_info.mac_rx_fcs_ok_cnt;
	rx_stat_cnt->all_mac_rx_u2m_mpdu_cnt += rx_all.rx_band_info_ext.mac_rx_u2m_mpdu_cnt;

	if (rx_stat_cnt->all_mac_rx_mdrdy_cnt == 0)
		rx_stat_cnt->all_per = 0;
	else
		rx_stat_cnt->all_per = (rx_stat_cnt->all_mac_rx_mdrdy_cnt - rx_stat_cnt->all_mac_rx_ok_cnt)
								* 100 / rx_stat_cnt->all_mac_rx_mdrdy_cnt;

	msg_len += snprintf(msg + msg_len, max_len - msg_len,
	"mac_rx_fcs_err_cnt: %d\n"
	"mac_rx_len_mismatch: %d\n"
	"mac_rx_fifo_full: %d\n"
	"mac_rx_mdrdy_cnt: %d\n"
	"phy_rx_pd_cck: %d\n"
	"phy_rx_pd_ofdm: %d\n"
	"phy_rx_sig_err_cck: %d\n"
	"phy_rx_sfd_err_cck: %d\n"
	"phy_rx_sig_err_ofdm: %d\n"
	"phy_rx_tag_err_ofdm: %d\n"
	"phy_rx_mdrdy_cnt_cck: %d\n"
	"phy_rx_mdrdy_cnt_ofdm: %d\n"
	"all_mac_rx_mdrdy_cnt: %d\n"
	"all_mac_rx_fcs_err_cnt: %d\n"
	"all_mac_rx_len_mismatch : %d\n"
	"all_mac_rx_fifo_full : %d\n"
	"all_mac_rx_ok_cnt : %d\n"
	"all_mac_rx_mpdu : %d\n"
	"all_per : %d\n",
	rx_all.rx_band_info.mac_rx_fcs_err_cnt, rx_all.rx_band_info.mac_rx_len_mismatch, rx_all.rx_comm_info.rx_fifo_full,
	rx_all.rx_band_info.mac_rx_mdrdy_cnt, rx_all.rx_band_info.phy_rx_pd_cck, rx_all.rx_band_info.phy_rx_pd_ofdm,
	rx_all.rx_band_info.phy_rx_sig_err_cck, rx_all.rx_band_info.phy_rx_sfd_err_cck, rx_all.rx_band_info.phy_rx_sig_err_ofdm,
	rx_all.rx_band_info.phy_rx_tag_err_ofdm, rx_all.rx_band_info.phy_rx_mdrdy_cnt_cck, rx_all.rx_band_info.phy_rx_mdrdy_cnt_ofdm,
	rx_stat_cnt->all_mac_rx_mdrdy_cnt, rx_stat_cnt->all_mac_rx_fcs_err_cnt, rx_stat_cnt->all_mac_rx_len_mismatch,
	rx_stat_cnt->all_mac_rx_fifo_full, rx_stat_cnt->all_mac_rx_ok_cnt, rx_stat_cnt->all_mac_rx_u2m_mpdu_cnt, rx_stat_cnt->all_per);
	if (os_snprintf_error(max_len - msg_len, msg_len))
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "Line%d: snprintf error!\n", __LINE__);

	if (Arg != NULL && !strcmp(Arg, stdout_flag)) {
		ret = snprintf(Arg, max_len, "%s", msg);
		if (os_snprintf_error(max_len, ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "Line%d: snprintf error!\n", __LINE__);
	} else
		MTWF_PRINT("%s", msg);

	if (msg)
		os_free_mem(msg);

	return TRUE;

}

INT32 ShowATERxSwCount(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = TESTMODE_GET_BAND_IDX(pAd);
	u_int32 rxed_slt_pkt_cnt = 0;
	struct service_test *serv_test = NULL;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx=%u, error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	if (pAd != NULL) {
		serv_test = net_ad_wrap_service(pAd);
		ret = mt_serv_get_rx_sw_count(serv_test, band_idx, &rxed_slt_pkt_cnt);
		MTWF_PRINT("rxed_slt_pkt_cnt=%u\n", rxed_slt_pkt_cnt);
	} else
		ret = SERV_STATUS_SERV_TEST_INVALID_NULL_POINTER;

	return ret;
}


INT32 set_ate_rx_stat_reset(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR i = 0;
	UCHAR band_idx;
	RX_STAT_CNT *rx_stat_cnt;
	RX_STATISTIC_RXV *rx_stat_rxv;
	struct service_test *serv = NULL;
	struct test_wlan_info *winfo = NULL;
	struct _TEST_RX_STAT_ALL_INFO rx_all;

	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	serv = (struct service_test *)pAd->serv.serv_handle;
	winfo = serv->test_winfo;

	/* parameter parsing */
	band_idx = TESTMODE_GET_BAND_IDX(pAd);

	MTWF_PRINT("%s: band_idx: %d\n", __func__, band_idx);

	rx_stat_cnt = &pAd->rx_stat_cnt;
	rx_stat_rxv = &pAd->rx_stat_rxv;

	net_ad_set(winfo, RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);
	net_ad_set(winfo, RF_AT_FUNCID_RESETTXRXCOUNTER, 0);
	chip_get_rx_stat_all(pAd, band_idx, 0, &rx_all);

	/*reset rcpi/rssi */
	for (i = 0; i < 4; i++) {
		rx_stat_rxv->RCPI[i] = 0;
		rx_stat_rxv->RSSI[i] = -110;
		rx_stat_rxv->FAGC_RSSI_IB[i] = -110;
		rx_stat_rxv->FAGC_RSSI_WB[i] = -110;
	}

	/* reset cumulated rx stat count */
	rx_stat_cnt->all_mac_rx_mdrdy_cnt = 0;
	rx_stat_cnt->all_mac_rx_fcs_err_cnt = 0;
	rx_stat_cnt->all_mac_rx_len_mismatch = 0;
	rx_stat_cnt->all_mac_rx_fifo_full = 0;
	rx_stat_cnt->all_mac_rx_ok_cnt = 0;
	rx_stat_cnt->all_mac_rx_u2m_mpdu_cnt = 0;
	rx_stat_cnt->all_per = 0;

	return TRUE;

}

#if defined(TXBF_SUPPORT) && defined(MT_MAC)
INT SetATEApplyStaToMacTblEntry(RTMP_ADAPTER *pAd)
{
	P_MANUAL_CONN pManual_cfg = &pAd->AteManualConnInfo;
	UINT16 WCID = pManual_cfg->wtbl_idx;
	PMAC_TABLE_ENTRY pEntry = entry_get(pAd, WCID);

	/* Currently, for MU-MIMO, we only care the VHT/HT Cap Info and VHT MCS set */
	os_move_mem(&pEntry->vht_cap_ie.vht_cap, &pManual_cfg->vht_cap_info, sizeof(pEntry->vht_cap_ie.vht_cap));
	os_move_mem(&pEntry->HTCapability.HtCapInfo, &pManual_cfg->ht_cap_info, sizeof(pEntry->HTCapability.HtCapInfo));
	os_move_mem(&pEntry->vht_cap_ie.mcs_set, &pManual_cfg->vht_mcs_set, sizeof(pEntry->vht_cap_ie.mcs_set));
	return TRUE;
}


INT SetATEApplyStaToAsic(RTMP_ADAPTER *pAd)
{
	P_MANUAL_CONN manual_cfg = &pAd->AteManualConnInfo;
	UINT16 WCID = manual_cfg->wtbl_idx;
	UCHAR *pAddr = &manual_cfg->peer_mac[0];
	MT_WCID_TABLE_INFO_T WtblInfo;
	/* MAC_TABLE_ENTRY *mac_entry = NULL; */
	struct rtmp_mac_ctrl *wtbl_ctrl = &pAd->mac_ctrl;

	if (wtbl_ctrl->wtbl_entry_cnt[0] > 0)
		WCID = (wtbl_ctrl->wtbl_entry_cnt[0] > WCID ? WCID : MCAST_WCID_TO_REMOVE);
	else {
		MTWF_PRINT("PSE not init yet!\n");
		return FALSE;
	}

	os_zero_mem(&WtblInfo, sizeof(MT_WCID_TABLE_INFO_T));
	WtblInfo.Wcid = WCID;
	os_move_mem(&WtblInfo.Addr[0], &pAddr[0], 6);

	if (WCID == MCAST_WCID_TO_REMOVE || WCID == hc_get_chip_wtbl_max_num(pAd)) {
		WtblInfo.MacAddrIdx = 0xe;
		WtblInfo.WcidType = MT_WCID_TYPE_BMCAST;
		WtblInfo.CipherSuit = WTBL_CIPHER_NONE;
	} else {
		/* if (!mac_entry) { */
		/* ("%s(): mac_entry is NULL!\n", __FUNCTION__)); */
		/* return; */
		/* } */
		if (pAd->AteManualConnInfo.peer_op_type == OPMODE_AP)
			WtblInfo.WcidType = MT_WCID_TYPE_AP;
		else
			WtblInfo.WcidType = MT_WCID_TYPE_CLI;

		WtblInfo.MacAddrIdx = manual_cfg->ownmac_idx; /* mac_entry->wdev->OmacIdx; */
		/* WtblInfo.Aid = manual_cfg->wtbl_idx; //mac_entry->Aid; */
		WtblInfo.CipherSuit = WTBL_CIPHER_NONE;
		/* if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_WMM_CAPABLE)) */
		WtblInfo.SupportQoS = TRUE;

		if (WMODE_CAP_N(manual_cfg->peer_phy_mode)) {
			WtblInfo.SupportHT = TRUE;
			/* if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_RDG_CAPABLE)) */
			{
				WtblInfo.SupportRDG = TRUE;
			}
			WtblInfo.SmpsMode = 0; /* mac_entry->MmpsMode ; */
			WtblInfo.MpduDensity = 0; /* mac_entry->MpduDensity; */
			WtblInfo.MaxRAmpduFactor = 3; /* mac_entry->MaxRAmpduFactor; */
#ifdef DOT11_VHT_AC

			if (WMODE_CAP_AC(manual_cfg->peer_phy_mode))
				WtblInfo.SupportVHT = TRUE;

#endif /* DOT11_VHT_AC */
		}
	}

	WtblInfo.Aid     = manual_cfg->aid;
	WtblInfo.PfmuId  = manual_cfg->pfmuId;
	WtblInfo.spe_idx = manual_cfg->spe_idx;
	/*
	 *	WtblInfo.rca2    = manual_cfg->rca2;
	 *	WtblInfo.rv      = manual_cfg->rv;
	 */
	MTWF_PRINT("Update WTBL table WCID=%d, Addr=%02x:%02x:%02x:%02x:%02x:%02x, \
		WtblInfo.MacAddrIdx=%d",
		WCID, PRINT_MAC(pAddr), WtblInfo.MacAddrIdx);
	AsicUpdateRxWCIDTableDetail(pAd, WtblInfo);
#ifdef MANUAL_MU

	if (WMODE_CAP_N(manual_cfg->peer_phy_mode)) {
		INT tid;

		for (tid = 0; tid < 4; tid++) {
			AsicUpdateBASession(pAd,
				WtblInfo.Wcid,
				0, 0, 64, TRUE, BA_SESSION_ORI, FALSE);
		}
	}

	asic_dump_wtbl_info(pAd, WtblInfo.Wcid);
#endif /* MANUAL_MU */
	return TRUE;
}


static INT ATEMacStr2Hex(RTMP_STRING *arg, UINT8 *mac)
{
	INT i;
	RTMP_STRING *token, sepValue[] = ":";

	if (arg == NULL)
		return FALSE;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17*/
	if (strlen(arg) < 17)
		return FALSE;

	for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++) {
		if (i > 6)
			break;

		if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
			return FALSE;

		AtoH(token, (&mac[i]), 1);
	}

	if (i != 6)
		return FALSE;

	MTWF_PRINT("\n%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return TRUE;
}


INT ATEManualParsingParam(RTMP_ADAPTER *pAd, RTMP_STRING *type, RTMP_STRING *val)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 mac[MAC_ADDR_LEN] = {0};
	INT op_type = 0;
	INT wtbl_idx = 1;
	INT own_mac_idx = 0;
	INT phy_mode = 0;
	INT bw = BW_20;
	INT nss = 1;
	INT maxrate_mode = MODE_CCK;
	INT maxrate_mcs = 0;
	INT pfmuId = 0, speIdx = 24;
	INT aid = 0;
	UINT8 rca2 = 0, rv = 0;
	UINT8 fgIsSuBFee = 0;
	UINT8 fgIsMuBFee = 0;
	UINT8 fgIsSGIFor20 = 0;
	UINT8 fgIsSGIFor40 = 0;
	UINT8 fgIsSGIFor80 = 0;
	UINT8 fgIsSGIFor160 = 0;
	UINT8 bFeeNsts = 0;
	UINT8 mcsSupport = 0;

	if ((!type) || (!val))
		return FALSE;

	if (!wdev)
		return FALSE;

	/* mac:xx:xx:xx:xx:xx:xx */
	if (strcmp("mac", type) == 0) {
		if (ATEMacStr2Hex(val, &mac[0]) == FALSE) {
			NdisZeroMemory(&mac[0], MAC_ADDR_LEN);
			MTWF_PRINT("Invalid MAC address(%s), use default\n", (val == NULL ? "" : val));
		} else {
			MTWF_PRINT("Invalid MAC address(%s), use default\n", (val == NULL ? "" : val));
		}

		NdisMoveMemory(&pAd->AteManualConnInfo.peer_mac[0], mac, MAC_ADDR_LEN);
		MTWF_PRINT("MAC=%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}

	/* type:ap/sta */
	if (strcmp("type", type) == 0) {
		if (strcmp(val, "ap") == 0)
			op_type = OPMODE_AP;
		else if (strcmp(val, "sta") == 0)
			op_type = OPMODE_STA;
		else {
			MTWF_PRINT("Invalid type(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.peer_op_type = op_type;
		MTWF_PRINT("TYPE=%d\n", op_type);
	}

	/* wtbl:1~127 */
	if (strcmp("wtbl", type) == 0) {
		if (strlen(val)) {
			wtbl_idx = simple_strtol(val, 0, 10);

			if (wtbl_idx <= 0 || wtbl_idx > 127) {
				MTWF_PRINT("Invalid wtbl idx(%s), use default\n", (val == NULL ? "" : val));
				wtbl_idx = 1;
			}
		} else {
			MTWF_PRINT("Invalid wtbl idx(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.wtbl_idx = wtbl_idx;
		MTWF_PRINT("WTBL_IDX=%d\n", wtbl_idx);
	}

	/* ownmac:0~4, 0x10~0x1f */
	if (strcmp("ownmac", type)  == 0) {
		if (strlen(val)) {
			own_mac_idx = simple_strtol(val, 0, 10);

			if (!((own_mac_idx >= 0 && own_mac_idx <= 4) || (own_mac_idx >= 0x10 && own_mac_idx <= 0x1f))) {
				MTWF_PRINT("Invalid OwnMac idx(%s), use default\n", (val == NULL ? "" : val));
				own_mac_idx = 1;
			}
		} else {
			MTWF_PRINT("Invalid wtbl idx(%s), use default\n", (val == NULL ? "" : val));
		}
		pAd->AteManualConnInfo.ownmac_idx = own_mac_idx;

		MTWF_PRINT("OWN_MAC_IDX=%d\n", own_mac_idx);

	}

	/* pfmuId: */
	if (strcmp("pfmuId", type)  == 0) {
		if (strlen(val)) {
			pfmuId = simple_strtol(val, 0, 10);

			if (!(pfmuId >= 0x00 || pfmuId <= 0x3f)) {
				MTWF_PRINT("Invalid PFMU idx(%s), use default\n", (val == NULL ? "" : val));
				pfmuId = 0;
			}
		} else {
			MTWF_PRINT("Invalid PFMU idx(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.pfmuId = pfmuId;
		MTWF_PRINT("PFMU_IDX=%d\n", pfmuId);
	}

	/* aid: */
	if (strcmp("aid", type) == 0) {
		if (strlen(val)) {
			aid = simple_strtol(val, 0, 10);

			if (!(aid >= 0x00 || aid <= 2007)) {
				MTWF_PRINT("Invalid aid(%s), use default\n", (val == NULL ? "" : val));
				aid = 0;
			}
		} else {
			MTWF_PRINT("Invalid aid(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.aid = aid;
		MTWF_PRINT("AID =%d\n", aid);
	}

	/* spe-idx: */
	if (strcmp("speIdx", type)  == 0) {
		if (strlen(val)) {
			speIdx = simple_strtol(val, 0, 10);

			if (!(speIdx >= 0 || speIdx <= 30)) {
				MTWF_PRINT("Invalid SPE idx(%s), use default\n", (val == NULL ? "" : val));
				speIdx = 24;
			}
		} else {
			MTWF_PRINT("Invalid SPE idx(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.spe_idx = speIdx;
		MTWF_PRINT("SPE_IDX=%d\n", speIdx);
	}

	if (strcmp("mubfee", type) == 0) {
		if (strlen(val)) {
			fgIsMuBFee = simple_strtol(val, 0, 10);

			if (!(fgIsMuBFee == 0 || fgIsMuBFee == 1)) {
				MTWF_PRINT("Invalid mubfee(%s), use default\n", (val == NULL ? "" : val));
				fgIsMuBFee = 0;
			}
		} else {
			MTWF_PRINT("Invalid mubfee(%s), use default\n", (val == NULL ? "" : val));
		}

		if (fgIsMuBFee)
			pAd->AteManualConnInfo.vht_cap_info.bfee_cap_mu = fgIsMuBFee;

		MTWF_PRINT("mubfee =%d\n", fgIsMuBFee);
	}

	if (strcmp("sgi160", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor160 = simple_strtol(val, 0, 10);

			if (!(fgIsSGIFor160 == 0 || fgIsSGIFor160 == 1)) {
				MTWF_PRINT("Invalid sgi160(%s), use default\n", (val == NULL ? "" : val));
				fgIsSGIFor160 = 0;
			}
		} else {
			MTWF_PRINT("Invalid sgi160(%s), use default\n", (val == NULL ? "" : val));
		}

		if (fgIsSGIFor160)
			pAd->AteManualConnInfo.vht_cap_info.sgi_160M = fgIsSGIFor160;

		MTWF_PRINT("sgi160 =%d\n", fgIsSGIFor160);
	}

	if (strcmp("sgi80", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor80 = simple_strtol(val, 0, 10);

			if (!(fgIsSGIFor80 == 0 || fgIsSGIFor80 == 1)) {
				MTWF_PRINT("Invalid sgi80(%s), use default\n", (val == NULL ? "" : val));
				fgIsSGIFor80 = 0;
			}
		} else {
			MTWF_PRINT("Invalid sgi80(%s), use default\n", (val == NULL ? "" : val));
		}

		if (fgIsSGIFor80)
			pAd->AteManualConnInfo.vht_cap_info.sgi_80M = fgIsSGIFor80;

		MTWF_PRINT("sgi80 =%d\n", fgIsSGIFor80);
	}

	if (strcmp("sgi40", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor40 = simple_strtol(val, 0, 10);

			if (!(fgIsSGIFor40 == 0 || fgIsSGIFor40 == 1)) {
				MTWF_PRINT("Invalid sgi40(%s), use default\n", (val == NULL ? "" : val));
				fgIsSGIFor40 = 0;
			}
		} else {
			MTWF_PRINT("Invalid sgi40(%s), use default\n", (val == NULL ? "" : val));
		}

		if (fgIsSGIFor40)
			pAd->AteManualConnInfo.ht_cap_info.ShortGIfor40 = fgIsSGIFor40;

		MTWF_PRINT("sgi40 =%d\n", fgIsSGIFor40);
	}

	if (strcmp("sgi20", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor20 = simple_strtol(val, 0, 10);

			if (!(fgIsSGIFor20 == 0 || fgIsSGIFor20 == 1)) {
				MTWF_PRINT("Invalid sgi20(%s), use default\n", (val == NULL ? "" : val));
				fgIsSGIFor20 = 0;
			}
		} else {
			MTWF_PRINT("Invalid sgi20(%s), use default\n", (val == NULL ? "" : val));
		}

		if (fgIsSGIFor20)
			pAd->AteManualConnInfo.ht_cap_info.ShortGIfor20 = fgIsSGIFor20;

		MTWF_PRINT("sgi20 =%d\n", fgIsSGIFor20);
	}

	if (strcmp("rxmcsnss1", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);

			if (mcsSupport > 3) {
				MTWF_PRINT("Invalid rxmcsnss1(%s), use default\n", (val == NULL ? "" : val));
				mcsSupport = 3;
			}
		} else {
			MTWF_PRINT("Invalid rxmcsnss1(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.vht_mcs_set.rx_mcs_map.mcs_ss1 = mcsSupport;
		MTWF_PRINT("rxmcsnss1 =%d\n", mcsSupport);
	}

	if (strcmp("rxmcsnss2", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);

			if (mcsSupport > 3) {
				MTWF_PRINT("Invalid rxmcsnss2(%s), use default\n", (val == NULL ? "" : val));
				mcsSupport = 3;
			}
		} else {
			MTWF_PRINT("Invalid rxmcsnss2(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.vht_mcs_set.rx_mcs_map.mcs_ss2 = mcsSupport;
		MTWF_PRINT("rxmcsnss2 =%d\n", mcsSupport);
	}

	if (strcmp("rxmcsnss3", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);

			if (mcsSupport > 3) {
				MTWF_PRINT("Invalid rxmcsnss3(%s), use default\n", (val == NULL ? "" : val));
				mcsSupport = 3;
			}
		} else {
			MTWF_PRINT("Invalid rxmcsnss3(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.vht_mcs_set.rx_mcs_map.mcs_ss3 = mcsSupport;
		MTWF_PRINT("rxMcsNSS3 =%d\n", mcsSupport);
	}

	if (strcmp("rxmcsnss4", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);

			if (mcsSupport > 3) {
				MTWF_PRINT("Invalid rxmcsnss4(%s), use default\n", (val == NULL ? "" : val));
				mcsSupport = 3;
			}
		} else {
			MTWF_PRINT("Invalid rxmcsnss4(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.vht_mcs_set.rx_mcs_map.mcs_ss4 = mcsSupport;
		MTWF_PRINT("rxmcsnss4 =%d\n", mcsSupport);
	}

	if (strcmp("subfee", type) == 0) {
		if (strlen(val)) {
			fgIsSuBFee = simple_strtol(val, 0, 10);

			if (!(fgIsSuBFee == 0 || fgIsSuBFee == 1)) {
				MTWF_PRINT("Invalid subfee(%s), use default\n", (val == NULL ? "" : val));
				fgIsSuBFee = 0;
			}
		} else {
			MTWF_PRINT("Invalid subfee(%s), use default\n", (val == NULL ? "" : val));
		}

		if (fgIsSuBFee)
			pAd->AteManualConnInfo.vht_cap_info.bfee_cap_su = fgIsSuBFee;

		MTWF_PRINT("subfee =%d\n", fgIsSuBFee);
	}

	if (strcmp("bfeensts", type) == 0) {
		if (strlen(val)) {
			bFeeNsts = simple_strtol(val, 0, 10);

			if (bFeeNsts > 4) {
				MTWF_PRINT("Invalid bfeensts(%s), use default\n", (val == NULL ? "" : val));
				bFeeNsts = 4;
			}
		} else {
			MTWF_PRINT("Invalid bfeensts(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.vht_cap_info.bfee_sts_cap = bFeeNsts;
		MTWF_PRINT("bfeensts =%d\n", bFeeNsts);
	}

	/* mode:a/bg/n/ac */
	if (strcmp("mode", type) == 0) {
		RTMP_STRING *tok;

		tok = val;

		while (strlen(tok)) {
			if (*tok == 'b') {
				phy_mode |= WMODE_B;
				tok++;
			} else if (*tok == 'g') {
				if ((*(tok + 1) == 'n') && (strlen(tok) >= 2)) {
					phy_mode |= WMODE_GN;
					tok += 2;
				} else {
					phy_mode |= WMODE_G;
					tok += 1;
				}
			} else if (*tok == 'a') {
				if ((*(tok + 1) == 'n') && (strlen(tok) >= 2)) {
					phy_mode |= WMODE_AN;
					tok += 2;
				} else if ((*(tok + 1) == 'c') && (strlen(tok) >= 2)) {
					phy_mode |= WMODE_AC;
					tok += 2;
				} else {
					phy_mode |= WMODE_A;
					tok += 1;
				}
			} else {
				MTWF_PRINT("Invalid phy_mode %c\n", *tok);
				tok++;
			}
		}

		pAd->AteManualConnInfo.peer_phy_mode = phy_mode;
		MTWF_PRINT("phy_mode=%s, convert to PhyMode= 0x%x\n", (val == NULL ? "" : val), phy_mode);

	}

	/* bw:20/40/80/160 */
	if (strcmp("bw", type) == 0) {
		if (strlen(val)) {
			bw = simple_strtol(val, 0, 10);

			switch (bw) {
			case 20:
				bw = BW_20;
				break;

			case 40:
				bw = BW_40;
				break;

			case 80:
				bw = BW_80;
				break;

			case 160:
				bw = BW_160;
				break;

			default:
				bw = BW_20;
				break;
			}
		} else {
			MTWF_PRINT("Invalid BW string(%s), use default!\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.peer_bw = bw;
		MTWF_PRINT("BW=%d\n", bw);
	}

	if (strcmp("nss", type) == 0) {
		if (strlen(val)) {
			UINT8 ucTxPath = pAd->Antenna.field.TxPath;

			nss = simple_strtol(val, 0, 10);

			if (nss > ucTxPath) {
				MTWF_PRINT("Invalid NSS string(%s), use default!\n", (val == NULL ? "" : val));
				nss = 1;
			}
		} else {
			MTWF_PRINT("Invalid NSS setting, use default!\n");
		}

		pAd->AteManualConnInfo.peer_nss = nss;
		MTWF_PRINT("NSS=%d\n", nss);
	}

	/* rca2 = 0/1 */
	if (strcmp("rca2", type) == 0) {
		if (strlen(val)) {
			rca2 = simple_strtol(val, 0, 10);

			if (rca2 > 1) {
				MTWF_PRINT("Invalid RCA2 string(%s), use default!\n", (val == NULL ? "" : val));
				rca2 = 0;
			}
		} else {
			MTWF_PRINT("Invalid RCA2 string(%s), use default!\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.rca2 = rca2;
		MTWF_PRINT("RCA2=%d\n", rca2);
	}

	/* rv = 0/1 */
	if (strcmp("rv", type) == 0) {
		if (strlen(val)) {
			rv = simple_strtol(val, 0, 10);

			if (rv > 1) {
				MTWF_PRINT("Invalid RV string(%s), use default!\n", (val == NULL ? "" : val));
				rv = 0;
			}
		} else {
			MTWF_PRINT("Invalid RV setting, use default!\n");
		}

		pAd->AteManualConnInfo.rv = rv;
		MTWF_PRINT("RV=%d\n", rv);
	}

	/* maxrate:cck/ofdm/htmix/htgf/vht/_0~32 */
	if (strcmp("maxrate", type) == 0) {
		RTMP_STRING *tok;

		if (strlen(val)) {
			tok = rtstrchr(val, '_');

			if (tok && strlen(tok) > 1) {
				*tok = 0;
				tok++;
			} else {
				MTWF_PRINT("Invalid maxmcs setting(%s), use default!\n", (val == NULL ? "" : val));
				goto maxrate_final;
			}
		} else {
			MTWF_PRINT("Invalid maxmcs setting(%s), use default!\n", (val == NULL ? "" : val));
			goto maxrate_final;
		}

		if (strlen(tok)) {
			maxrate_mcs = simple_strtol(tok, 0, 10);
			MTWF_PRINT("input MCS string(%s) =%d\n", tok, maxrate_mcs);

		}

		if (strcmp(val, "cck") == 0) {
			maxrate_mode = MODE_CCK;

			if (maxrate_mcs > 4)
				maxrate_mcs = 3;
		} else if (strcmp(val, "ofdm") == 0) {
			maxrate_mode = MODE_OFDM;

			if (maxrate_mcs > 7)
				maxrate_mcs = 7;
		} else if (strcmp(val, "htmix") == 0) {
			maxrate_mode = MODE_HTMIX;

			if (maxrate_mcs > 32)
				maxrate_mcs = 32;
		} else if (strcmp(val, "htgf") == 0) {
			maxrate_mode = MODE_HTGREENFIELD;

			if (maxrate_mcs > 32)
				maxrate_mcs = 32;
		} else if (strcmp(val, "vht") == 0) {
			maxrate_mode = MODE_VHT;

			if (maxrate_mcs > 9)
				maxrate_mcs = 9;
		} else {
			MTWF_PRINT("Invalid RateMode string(%s), use default!\n", val);

			maxrate_mode = MODE_CCK;
			maxrate_mcs = 0;
		}

maxrate_final:
		pAd->AteManualConnInfo.peer_maxrate_mode = maxrate_mode;
		pAd->AteManualConnInfo.peer_maxrate_mcs = maxrate_mcs;
		MTWF_PRINT("MAXRATE=>MODE=%d,MCS=%d\n", maxrate_mode, maxrate_mcs);
	}

	return TRUE;
}


/*
 *	Assoc Parameters:
 *		mac:xx:xx:xx:xx:xx:xx-type:ap/sta-mode:a/b/g/gn/an/ac-bw:20/40/80/160-nss:1/2/3/4-pfmuId:xx-aid:xx-maxrate:
 *
 *	@jeffrey: For MU-MIMO, we need to configure the HT/VHP cap info to emulate different STAs (# of STA >= 2)  which
 *		  supports different Tx and Rx dimension for early algorithm verification
 */
INT SetATEAssocProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	char sep_type = '-', sep_val = ':';
	RTMP_STRING *tok, *param_str, *param_type, *param_val;
	INT stat;
	char ucNsts;
	UINT_32 rate[8];
	RA_PHY_CFG_T TxPhyCfg;
	RTMP_STRING rate_str[64];
	struct wifi_dev *wdev;
	INT ret = 0;

	NdisZeroMemory(&pAd->AteManualConnInfo, sizeof(MANUAL_CONN));
	tok = arg;

	while (tok) {
		if (strlen(tok)) {
			param_str = tok;
			tok = rtstrchr(tok, sep_type);

			if (tok) {
				*tok = 0;
				tok++;
			}

			MTWF_PRINT("param_str=%s\n", param_str);


			if (strlen(param_str)) {
				param_type = param_str;
				param_val = rtstrchr(param_str, sep_val);

				if (param_val) {
					*param_val = 0;
					param_val++;
				}

				if (strlen(param_type) && param_val && strlen(param_val)) {
					stat = ATEManualParsingParam(pAd, param_type, param_val);

					if (stat == FALSE)
						goto err_dump_usage;
				}
			}
		} else
			break;
	}

	MTWF_PRINT("User manual configured peer STA info:\n");
	MTWF_PRINT("\tMAC=>0x%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pAd->AteManualConnInfo.peer_mac));
	MTWF_PRINT("\tBAND=>%d\n", pAd->AteManualConnInfo.peer_band);
	MTWF_PRINT("\tOwnMacIdx=>%d\n", pAd->AteManualConnInfo.ownmac_idx);
	MTWF_PRINT("\tWTBL_Idx=>%d\n", pAd->AteManualConnInfo.wtbl_idx);
	MTWF_PRINT("\tOperationType=>%d\n", pAd->AteManualConnInfo.peer_op_type);
	MTWF_PRINT("\tPhyMode=>%d\n", pAd->AteManualConnInfo.peer_phy_mode);
	MTWF_PRINT("\tBandWidth=>%d\n", pAd->AteManualConnInfo.peer_bw);
	MTWF_PRINT("\tNSS=>%d\n", pAd->AteManualConnInfo.peer_nss);
	MTWF_PRINT("\tPfmuId=>%d\n", pAd->AteManualConnInfo.pfmuId);
	MTWF_PRINT("\tAid=>%d\n", pAd->AteManualConnInfo.aid);
	MTWF_PRINT("\tSpe_idx=>%d\n", pAd->AteManualConnInfo.spe_idx);
	MTWF_PRINT("\tMaxRate_Mode=>%d\n", pAd->AteManualConnInfo.peer_maxrate_mode);
	MTWF_PRINT("\tMaxRate_MCS=>%d\n", pAd->AteManualConnInfo.peer_maxrate_mcs);
	MTWF_PRINT("Now apply it to hardware!\n");

	/* This applied the manual config info into the mac table entry, including the HT/VHT cap, VHT MCS set */
	SetATEApplyStaToMacTblEntry(pAd);
	/* Fixed rate configuration */
	NdisZeroMemory(&rate_str[0], sizeof(rate_str));
	ret = snprintf(rate_str, sizeof(rate_str), "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d",
			pAd->AteManualConnInfo.wtbl_idx,
			pAd->AteManualConnInfo.peer_maxrate_mode,
			pAd->AteManualConnInfo.peer_bw,
			pAd->AteManualConnInfo.peer_maxrate_mcs,
			pAd->AteManualConnInfo.peer_nss,
			0, 0, 0, 0, 0);
	if (os_snprintf_error(sizeof(rate_str), ret))
		MTWF_PRINT("snprintf error!\n");
	MTWF_PRINT("\tSet fixed RateInfo string as %s\n", rate_str);
	/* Set_Fixed_Rate_Proc(pAd, rate_str); */
	ucNsts = asic_get_nsts_by_mcs(pAd, pAd->AteManualConnInfo.peer_maxrate_mode,
							 pAd->AteManualConnInfo.peer_maxrate_mcs,
							 FALSE,
							 pAd->AteManualConnInfo.peer_nss);
	rate[0] = asic_tx_rate_to_tmi_rate(pAd, pAd->AteManualConnInfo.peer_maxrate_mode,
								  pAd->AteManualConnInfo.peer_maxrate_mcs,
								  ucNsts,
								  FALSE,
								  0);
	rate[0] &= 0xfff;
	rate[1] = rate[2] = rate[3] = rate[4] = rate[5] = rate[6] = rate[7] = rate[0];
	os_zero_mem(&TxPhyCfg, sizeof(TxPhyCfg));
	TxPhyCfg.BW      = pAd->AteManualConnInfo.peer_bw;
	TxPhyCfg.ShortGI = FALSE;
	/* TxPhyCfg.ldpc  = HT_LDPC | VHT_LDPC; */
	TxPhyCfg.ldpc    = 0;
	AsicTxCapAndRateTableUpdate(pAd,
								  pAd->AteManualConnInfo.wtbl_idx,
								  &TxPhyCfg,
								  rate,
								  FALSE);
	/* WTBL configuration */
	SetATEApplyStaToAsic(pAd);
	wdev = wdev_search_by_omac_idx(pAd, pAd->AteManualConnInfo.ownmac_idx);
	if (wdev == NULL) {
		MTWF_PRINT("can't find wdev!!\n");
		return FALSE;
	}
	/* dump WTBL again */
	/* dump_wtbl_info(pAd, pAd->AteManualConnInfo.wtbl_idx); */
	return TRUE;
err_dump_usage:
	MTWF_PRINT("Parameter Usage:\n");
	MTWF_PRINT("\tiwpriv ra0 set assoc=[mac:hh:hh:hh:hh:hh:hh]-[wtbl:dd] \
				-[ownmac:dd]-[type:xx]-[mode:mmm]-[bw:dd]-[nss:ss]-[maxrate:kkk_dd]\n");
	MTWF_PRINT("\t\tmac: peer's mac address in hex format\n");
	MTWF_PRINT("\t\t\tExample=> mac:00:0c:43:12:34:56\n");
	MTWF_PRINT("\t\twtbl: the WTBL entry index peer will occupied, in range 1~127\n");
	MTWF_PRINT("\t\t\tExample=> wtbl:1\n");
	MTWF_PRINT("\t\townmac: the OwnMAC index we'll used to send frame to this peer, \
				in range 0~4 or 16~31\n");
	MTWF_PRINT("\t\t\tExample=> ownmac:0\n");
	MTWF_PRINT("\t\ttype: peer's operation type, is a ap or sta, \
				allow input: \"ap\" or \"sta\"\n");
	MTWF_PRINT("\t\t\tExample=> type:ap\n");
	MTWF_PRINT("\t\tmode: peer's phy operation mode, allow input: a/b/g/gn/an/ac\n");
	MTWF_PRINT("\t\t\tExample=> mode:aanac	to indicate peer can support A/AN/AC mode\n");
	MTWF_PRINT("\t\tbw: Peer's bandwidth capability, in range to 20/40/80/160\n");
	MTWF_PRINT("\t\t\tExample=> bw:40	indicate peer can support BW_40\n");
	MTWF_PRINT("\t\tnss: Peer's capability for Spatial stream which can tx/rx, \
				in range of 1~4 with restriction of Software/Hardware cap.\n");
	MTWF_PRINT("\t\t\tExample=> nss:2	indicate peer can support 2ss for both tx/rx\n");
	MTWF_PRINT("\t\tmaxrate: Peer's data rate capability for tx/rx, \
				separate as two parts and separate by '_' character\n");
	MTWF_PRINT("\t\t\t\t kkk: phy modulation mode, allow input:'cck', \
				'ofdm', 'htmix', 'htgf', 'vht'\n");
	MTWF_PRINT("\t\t\t\t dd:phy mcs rate, for CCK:0~3, OFDM:0~7, HT:0~32, VHT:0~9\n");
	MTWF_PRINT("\t\t\tExample=> maxrate:cck_1	indicate we only can transmit CCK \
				and MCS 1(2Mbps) or lower MCS to peer\n");
	MTWF_PRINT("\t\t\tExample=> maxrate:ofdm_3	indicate we only can transmit OFDM \
				and MCS 3(24Mbps) to peer\n");
	MTWF_PRINT("\t\t\tExample=> maxrate:htmix_3	indicate we only can transmit OFDM \
				and MCS 3(24Mbps) to peer\n");
	return FALSE;
}



INT SetATETxBfDutInitProc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	BOOLEAN     fgDbdc;
	RTMP_STRING cmdStr[24];
	ULONG       stTimeChk0, stTimeChk1;
	INT         sRet;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8 ucTxPath = pAd->Antenna.field.TxPath;

	fgDbdc = simple_strtol(Arg, 0, 10) & 1;

	NdisGetSystemUpTime(&stTimeChk0);

	/* Do ATESTART */
	SetATE(pAd, "ATESTART");

	/* set ATESA=00:22:22:22:22:22 */
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][1], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][2], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][3], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][4], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][5], 0x22);
	sRet = snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
		0x22, 0x22, 0x22, 0x22, 0x22);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	SetATESa(pAd, cmdStr);

	/* set ATEBSSID=00:22:22:22:22:22 */
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][1], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][2], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][3], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][4], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][5], 0x22);
	sRet = snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
		0x22, 0x22, 0x22, 0x22, 0x22);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	SetATEBssid(pAd, cmdStr);

//	Set_BssInfoUpdate(pAd, cmdStr);

	/* set ATEDA=00:11:11:11:11:11 */
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][1], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][2], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][3], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][4], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][5], 0x11);


	sRet = snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
		0x11, 0x11, 0x11, 0x11, 0x11);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	SetATEDa(pAd, cmdStr);

	/* set ATETXMODE=2 */
	SetATETxMode(pAd, "2");
	/* set ATETXBW=0 */
	SetATETxBw(pAd, "0");
	/* set ATETXGI=0 */
	SetATETxGi(pAd, "0");
	/* Set ATEIPG=999 */
	SetATEIpg(pAd, "999");
	/* Enable i/eBF */
	SetATETXBFProc(pAd, "3");

	switch (ucTxPath) {
	case TX_PATH_2:
		/* set ATETXANT=3 2T */
		SetATETxAntenna(pAd, "3");
		/* set ATERXANT=3  2R*/
		SetATERxAntenna(pAd, "3");

		/* set ATETXMCS=15 */
		SetATETxMcs(pAd, "15");
		break;

	case TX_PATH_3:
		/* set ATETXANT=7 3T */
		SetATETxAntenna(pAd, "7");
		/* set ATERXANT=7  3R*/
		SetATERxAntenna(pAd, "7");

		/* set ATETXMCS=23 */
		SetATETxMcs(pAd, "23");
		break;

	case TX_PATH_4:
	default:
		/* set ATETXANT=15 4T */
		SetATETxAntenna(pAd, "15");
		/* set ATERXANT=15  4R*/
		SetATERxAntenna(pAd, "15");

		/* set ATETXMCS=31 */
		SetATETxMcs(pAd, "31");
		break;
	case TX_PATH_5:
		/* set ATETXMODE=4 */
	    SetATETxMode(pAd, "4");
		/* set ATETXANT=15 4T */
		SetATETxAntenna(pAd, "31");
		/* set ATERXANT=15  4R*/
		SetATERxAntenna(pAd, "31");

		/* set ATETXMCS=7 */
		SetATETxMcs(pAd, "7");
		/* Set ATETxNss = 4 */
		SetATETxNss(pAd, "4");
		break;
	}

	/* SetATETxPower0(pAd, "14"); */
	TESTMODE_SET_PARAM(pAd, control_band_idx, fgEBfEverEnabled, FALSE);

	NdisGetSystemUpTime(&stTimeChk1);
	MTWF_PRINT("%s: Time consumption : %lu sec\n", __func__, (stTimeChk1 - stTimeChk0) * 1000 / OS_HZ);

	/* Enable Tx MAC HW before trigger sounding */
	/* MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, control_band_idx); */

	/* Init iBF phase calibration */
	if (ops->iBFPhaseCalInit)
		ops->iBFPhaseCalInit(pAd);

	return TRUE;
}


INT SetATETxBfGdInitProc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	LONG        i4PhyMode;
	RTMP_STRING cmdStr[80];
	ULONG       stTimeChk0, stTimeChk1;
	UCHAR       *addr1;
	UINT32      ret;
	INT         sRet = 0;

	if (kstrtol(Arg, 10, &i4PhyMode))
		return NDIS_STATUS_FAILURE;

	NdisGetSystemUpTime(&stTimeChk0);

	/* Do ATESTART */
	SetATE(pAd, "ATESTART");

	/* set ATESA=00:11:11:11:11:11 */
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][1], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][2], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][3], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][4], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][5], 0x11);

	sRet = snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
		0x11, 0x11, 0x11, 0x11, 0x11);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	SetATESa(pAd, cmdStr);


	/* set ATEBSSID=00:22:22:22:22:22 */
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][1], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][2], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][3], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][4], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][5], 0x11);
	sRet = snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
		0x22, 0x22, 0x22, 0x22, 0x22);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	SetATEBssid(pAd, cmdStr);


//	Set_BssInfoUpdate(pAd, cmdStr);

	/* set ATEDA=00:22:22:22:22:22 */
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][1], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][2], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][3], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][4], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][5], 0x11);


	sRet = snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
		0x22, 0x22, 0x22, 0x22, 0x22);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	SetATEDa(pAd, cmdStr);

	if (i4PhyMode == 4)
		/* set ATETXMODE=4 */
		SetATETxMode(pAd, "4");
	else
		/* set ATETXMODE=2 */
		SetATETxMode(pAd, "2");
	/* set ATETXMCS=0 */
	SetATETxMcs(pAd, "0");
	/* set ATETXBW=0 */
	SetATETxBw(pAd, "0");
	/* set ATETXGI=0 */
	SetATETxGi(pAd, "0");
	/* Set ATETXCNT=0 */
	SetATETxCount(pAd, "0");
	/* Set ATETXLEN=1024 */
	SetATETxLength(pAd, "1024");

	/* set ATETXANT=1 1T */
	SetATETxAntenna(pAd, "1");
	/* set ATERXANT=1  1R*/
	SetATERxAntenna(pAd, "1");

	/* Configure WTBL */
	/* iwpriv ra0 set ManualAssoc =mac:222222222222-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:1-pfmuId:0 */
	addr1 = TESTMODE_GET_PARAM(pAd, control_band_idx, addr1[0]);
	sRet = snprintf(cmdStr, sizeof(cmdStr),
		"mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:ap-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:1-pfmuId:0\n",
		 addr1[0], addr1[1], addr1[2], addr1[3], addr1[4], addr1[5]);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	MTWF_PRINT("%s\n", cmdStr);

	ret = MtCmdSetTestEngine(pAd, RF_AT_FUNCID_COMMAND, RF_AT_COMMAND_TX_COMMIT);
	if (ret) {
		MTWF_PRINT("%s, ret: 0x%x\n", __func__, ret);
		return FALSE;
	}

	NdisGetSystemUpTime(&stTimeChk1);
	MTWF_PRINT("%s: Time consumption : %lu sec\n", __func__, (stTimeChk1 - stTimeChk0) * 1000 / OS_HZ);
	return TRUE;
}


INT32 SetATETxPacketWithBf(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#ifndef CONFIG_WLAN_SERVICE
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
#else
	UCHAR control_band_idx = hc_get_hw_band_idx(pAd);
#endif
	INT32    i;
	UCHAR    ucWlanId, ucTxCnt, *value, ucBuf[4], cmdStr[32];
	BOOLEAN  fgBf;
	INT ret = 0;
#ifdef CONFIG_ATE
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test;
	serv_test = (struct service_test *)(pAd->serv.serv_handle);
#else
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */


	MTWF_PRINT("%s: control_band_idx = %d\n", __func__, control_band_idx);


	if (Arg == NULL)
		return FALSE;

	if (strlen(Arg) != 8)
		return FALSE;

	for (i = 0, value = rstrtok(Arg, ":"); value && i < ARRAY_SIZE(ucBuf); value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	fgBf     = ucBuf[0];
	ucWlanId = ucBuf[1];
	ucTxCnt  = ucBuf[2];

	/* Assign Wlan ID for fixed rate TxD */
#ifdef CONFIG_WLAN_SERVICE
	CONFIG_SET_PARAM(serv_test, wcid_ref,
			(u_int8)ucWlanId);
#else
	ATECtrl->wcid_ref = ucWlanId;
#endif

	/* At TxD, enable/disable BF Tx at DW6 bit28 */
	if (fgBf) {
		/* Stop Rx before ready to Tx */
		SetATE(pAd, "RXSTOP");

		/* Invalid iBF profile */
		TxBfProfileTagRead(pAd, 2, TRUE);
#if defined(DOT11_HE_AX)
		TxBfProfileTag_InValid(&pAd->pfmu_tags_info, FALSE);
#else
		TxBfProfileTag_InValid(&pAd->rPfmuTag1, FALSE);
#endif
		TxBfProfileTagWrite(pAd,
					&pAd->rPfmuTag1,
					&pAd->rPfmuTag2,
					2);
		/* ATECtrl->eTxBf = TRUE; */
		/* ATECtrl->iTxBf = TRUE; */
		TESTMODE_SET_PARAM(pAd, control_band_idx, ebf, TRUE);
		TESTMODE_SET_PARAM(pAd, control_band_idx, ibf, TRUE);
		TESTMODE_SET_PARAM(pAd, control_band_idx, fgEBfEverEnabled, TRUE);
		/* Stop Tx when the action of Tx packet is done */
		SetATE(pAd, "TXSTOP");
		/* Set the number of Tx packets */
		ret = snprintf(cmdStr, sizeof(cmdStr), "%d", ucTxCnt);
		if (os_snprintf_error(sizeof(cmdStr), ret))
			MTWF_PRINT("snprintf error!\n");
		SetATETxCount(pAd, cmdStr);

		/* Start packet Tx */
		SetATE(pAd, "TXFRAME");
	} else {
		if (TESTMODE_GET_PARAM(pAd, control_band_idx, fgEBfEverEnabled) == FALSE) {
			/* Stop Rx before ready to Tx */
			SetATE(pAd, "RXSTOP");

			/* ATECtrl->eTxBf = FALSE; */
			/* ATECtrl->iTxBf = FALSE; */
			TESTMODE_SET_PARAM(pAd, control_band_idx, ebf, FALSE);
			TESTMODE_SET_PARAM(pAd, control_band_idx, ibf, FALSE);
			/* Stop Tx when the action of Tx packet is done */
			SetATE(pAd, "TXSTOP");
			/* Set the number of Tx packets */
			ret = snprintf(cmdStr, sizeof(cmdStr), "%d", ucTxCnt);
			if (os_snprintf_error(sizeof(cmdStr), ret))
				MTWF_PRINT("snprintf error!\n");
			SetATETxCount(pAd, cmdStr);



			/* Start packet Tx */
			SetATE(pAd, "TXFRAME");
		} else {
			/* Invalid iBF profile */
			TxBfProfileTagRead(pAd, 2, TRUE);
#if defined(DOT11_HE_AX)
			TxBfProfileTag_InValid(&pAd->pfmu_tags_info, TRUE);
#else
			TxBfProfileTag_InValid(&pAd->rPfmuTag1, TRUE);
#endif
			TxBfProfileTagWrite(pAd,
						&pAd->rPfmuTag1,
						&pAd->rPfmuTag2,
						2);
			TESTMODE_SET_PARAM(pAd, control_band_idx, fgEBfEverEnabled, FALSE);
		}
	}

	return TRUE;
}


INT32 SetATETxBfChanProfileUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	/* struct _ATE_CTRL    *ATECtrl = &(pAd->ATECtrl); */
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	INT32       i;
	UCHAR       *value;
	RTMP_STRING cmdStr[80];
	CHAR	    value_T[12] = {0};
	UCHAR       strLen;
	BOOLEAN     fgFinalData;
	UINT16      u2Buf[13] = {0};
	UINT16      u2PfmuId,   u2Subcarr;
	INT16       i2Phi11,     i2Phi21,    i2Phi31,    i2Phi41;
	INT16       i2H11,       i2AngleH11, i2H21, i2AngleH21, i2H31, i2AngleH31, i2H41, i2AngleH41, i2H51 = 0, i2AngleH51 = 0;
	INT32       Ret = 0;
	UINT8       ucTxPath = pAd->Antenna.field.TxPath;
	INT         sRet = 0;

	if (Arg == NULL)
		return FALSE;

	if (strlen(Arg) != 43)
		return FALSE;

	if (!wdev)
		return FALSE;

	for (i = 0, value = rstrtok(Arg, ":"); value && i < ARRAY_SIZE(u2Buf);
			value = rstrtok(NULL, ":")) {
		if ((strlen(value) > 3) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		strLen = strlen(value);

		if (strLen & 1) {
			sRet = snprintf(value_T, sizeof(value_T) - strlen(value_T), "%s%s", "0", value);
			if (os_snprintf_error(sizeof(value_T), sRet)) {
				MTWF_PRINT("snprintf error!\n");
				return FALSE;
			}
			AtoH(value_T, (PCHAR)(&u2Buf[i]), 2);
			u2Buf[i] = be2cpu16(u2Buf[i]);
			i++;
		}
	}

	u2PfmuId   = u2Buf[0];
	u2Subcarr  = u2Buf[1];
	fgFinalData = u2Buf[2];
	i2H11      = (INT16)(u2Buf[3] << 3) >> 3;
	i2AngleH11 = (INT16)(u2Buf[4] << 3) >> 3;
	i2H21      = (INT16)(u2Buf[5] << 3) >> 3;
	i2AngleH21 = (INT16)(u2Buf[6] << 3) >> 3;
	i2H31      = (INT16)(u2Buf[7] << 3) >> 3;
	i2AngleH31 = (INT16)(u2Buf[8] << 3) >> 3;
	i2H41      = (INT16)(u2Buf[9] << 3) >> 3;
	i2AngleH41 = (INT16)(u2Buf[10] << 3) >> 3;
	i2H51      = (INT16)(u2Buf[11] << 3) >> 3;
	i2AngleH51 = (INT16)(u2Buf[12] << 3) >> 3;

	i2Phi11    = 0;
	i2Phi21    = 0;
	i2Phi31    = 0;
	i2Phi41    = 0;

	switch (ucTxPath) {
	case TX_PATH_2:
		i2Phi11 = i2AngleH21 - i2AngleH11;
		i2Phi21	  = 0;
		break;

	case TX_PATH_3:
		i2Phi11 = i2AngleH31 - i2AngleH11;
		i2Phi21 = i2AngleH31 - i2AngleH21;
		break;

	case TX_PATH_4:
	default:
		i2Phi11 = i2AngleH41 - i2AngleH11;
		i2Phi21 = i2AngleH41 - i2AngleH21;
		i2Phi31 = i2AngleH41 - i2AngleH31;
		break;

	case TX_PATH_5:
		i2Phi11 = i2AngleH51 - i2AngleH11;
		i2Phi21 = i2AngleH51 - i2AngleH21;
		i2Phi31 = i2AngleH51 - i2AngleH31;
		i2Phi41 = i2AngleH51 - i2AngleH41;
		break;
	}

	MTWF_PRINT("%s: i2AngleH11 = 0x%x, i2AngleH21 = 0x%x, i2AngleH31 = 0x%x",
		__func__, i2AngleH11, i2AngleH21, i2AngleH31);

	MTWF_PRINT("i2AngleH41 = 0x%x, i2AngleH51 = 0x%x\n",
		i2AngleH41, i2AngleH51);

	/* Update the tag to enable eBF profile */
	if (fgFinalData) {
		sRet = snprintf(cmdStr, sizeof(cmdStr), "%02x:01", u2PfmuId);
		if (os_snprintf_error(sizeof(cmdStr), sRet)) {
			MTWF_PRINT("snprintf error!\n");
			return FALSE;
		}
		Set_TxBfProfileTagRead(pAd, cmdStr);
		pAd->rPfmuTag1.rField.ucInvalidProf = TRUE;
		Set_TxBfProfileTagWrite(pAd, cmdStr);
	}

	/* Update the profile data per subcarrier */
	switch (ucTxPath) {
	case TX_PATH_3:
		sRet = snprintf(cmdStr, sizeof(cmdStr),
			"%02x:%03x:%03x:00:%03x:00:000:00:000:00:000:00:000:00:00:00:00:00",
			u2PfmuId, u2Subcarr,
			(UINT16)((UINT16)i2Phi11 & 0xFFF),
			(UINT16)((UINT16)i2Phi21 & 0xFFF));
		if (os_snprintf_error(sizeof(cmdStr), sRet)) {
			MTWF_PRINT("snprintf error!\n");
			return FALSE;
		}
		break;

	case TX_PATH_4:
	default:
		sRet = snprintf(cmdStr, sizeof(cmdStr),
			"%02x:%03x:%03x:00:%03x:00:%03x:00:000:00:000:00:000:00:00:00:00:00",
			 u2PfmuId, u2Subcarr,
			 (UINT16)((UINT16)i2Phi11 & 0xFFF),
			 (UINT16)((UINT16)i2Phi21 & 0xFFF),
			 (UINT16)((UINT16)i2Phi31 & 0xFFF));
		if (os_snprintf_error(sizeof(cmdStr), sRet)) {
			MTWF_PRINT("snprintf error!\n");
			return FALSE;
		}
		break;
	case TX_PATH_5:
		sRet = snprintf(cmdStr, sizeof(cmdStr),
			"%02x:%03x:%03x:00:%03x:00:%03x:00:%03x:00:000:00:000:00:00:00:00:00",
			 u2PfmuId, u2Subcarr,
			 (UINT16)((UINT16)i2Phi11 & 0xFFF),
			 (UINT16)((UINT16)i2Phi21 & 0xFFF),
			 (UINT16)((UINT16)i2Phi31 & 0xFFF),
			 (UINT16)((UINT16)i2Phi41 & 0xFFF));
		if (os_snprintf_error(sizeof(cmdStr), sRet)) {
			MTWF_PRINT("snprintf error!\n");
			return FALSE;
		}
		break;
	}

	MTWF_PRINT("%s", __func__);
	Ret = Set_TxBfProfileDataWrite(pAd, cmdStr);
	return Ret;
}



INT32 SetATETxBfProfileRead(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	/* struct _ATE_CTRL    *ATECtrl = &(pAd->ATECtrl); */
	INT32       i;
	UCHAR       ucPfmuId,    *value;
	RTMP_STRING cmdStr[32];
	CHAR	    value_T[12] = {0};
	UCHAR       strLen;
	UINT16      u2Buf[2] = {0};
	UINT16      u2SubCarrier;
	INT         sRet = 0;

	if (Arg == NULL)
		return FALSE;

	if (strlen(Arg) != 7)
		return FALSE;

	for (i = 0, value = rstrtok(Arg, ":"); value && i < ARRAY_SIZE(u2Buf);
			value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 3) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))) || (!isxdigit(*(value + 2))))
			return FALSE;  /*Invalid*/

		strLen = strlen(value);

		if (strLen & 1) {
			sRet = snprintf(value_T, sizeof(value_T) - strlen(value_T), "%s%s", "0", value);
			if (os_snprintf_error(sizeof(value_T), sRet))
				MTWF_PRINT("snprintf error!\n");
			AtoH(value_T, (PCHAR)(&u2Buf[i]), 2);
			u2Buf[i] = be2cpu16(u2Buf[i]);
			i++;
		}
	}

	ucPfmuId     = u2Buf[0];
	u2SubCarrier = u2Buf[1];
	sRet = snprintf(cmdStr, 11, "%.2x:01:%.2x:%.2x", ucPfmuId, (u2SubCarrier >> 8), (u2SubCarrier & 0xFF));
	if (os_snprintf_error(sizeof(cmdStr), sRet)) {
		MTWF_PRINT("snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileDataRead(pAd, cmdStr);
	return TRUE;
}


INT32 SetATETXBFProc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	UCHAR TxBfEn;
#ifdef MT_MAC
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	MTWF_PRINT("%s: control_band_idx = %d\n", __func__, control_band_idx);
#endif
	TxBfEn = simple_strtol(Arg, 0, 10);
#ifdef MT_MAC

	switch (TxBfEn) {
	case 0:
		/* no BF */
		TESTMODE_SET_PARAM(pAd, control_band_idx, ibf, FALSE);
		TESTMODE_SET_PARAM(pAd, control_band_idx, ebf, FALSE);
		break;

	case 1:
		/* ETxBF */
		TESTMODE_SET_PARAM(pAd, control_band_idx, ibf, FALSE);
		TESTMODE_SET_PARAM(pAd, control_band_idx, ebf, TRUE);
		break;

	case 2:
		/* ITxBF */
		TESTMODE_SET_PARAM(pAd, control_band_idx, ibf, TRUE);
		TESTMODE_SET_PARAM(pAd, control_band_idx, ebf, FALSE);
		break;

	case 3:
		/* Enable TXBF support */
		TESTMODE_SET_PARAM(pAd, control_band_idx, ibf, TRUE);
		TESTMODE_SET_PARAM(pAd, control_band_idx, ebf, TRUE);
		break;

	default:
		MTWF_PRINT("Set_ATE_TXBF_Proc: Invalid parameter %d\n", TxBfEn);
		Ret = TRUE;
		break;
	}

#endif

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATEIBfGdCal(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32       i;
	UCHAR       ucGroup,    ucGroup_L_M_H, *value, ucBuf[5] = {0};
	BOOLEAN     fgSX2;
	UCHAR       ucPhaseCal, ucPhaseVerifyLnaGainLevel, u1PhaseOutRptVersion = 0;
	INT32       Ret = 0;
	UINT8		u1BandIdx = TESTMODE_GET_BAND_IDX(pAd);

	if (Arg == NULL)
		return FALSE;

	if ((strlen(Arg) != 11) && (strlen(Arg) != 14))
		return FALSE;

	for (i = 0, value = rstrtok(Arg, ":"); value && i < ARRAY_SIZE(ucBuf);
			value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	MTWF_PRINT("%s: \n", __func__);
	ucGroup       = ucBuf[0];
	ucGroup_L_M_H = ucBuf[1];
	fgSX2         = ucBuf[2];
	ucPhaseCal    = ucBuf[3];
	u1PhaseOutRptVersion = ucBuf[4];
	ucPhaseVerifyLnaGainLevel = 0;
	Ret = CmdITxBfPhaseCal(pAd,
						   ucGroup,
						   ucGroup_L_M_H,
						   fgSX2,
						   ucPhaseCal,
						   ucPhaseVerifyLnaGainLevel,
						   u1BandIdx,
						   u1PhaseOutRptVersion);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATEIBfInstCal(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32       i;
	UCHAR       ucGroup,    ucGroup_L_M_H, *value, ucBuf[6] = {0};
	BOOLEAN     fgSX2;
	UCHAR       ucPhaseCal, ucPhaseLnaGainLevel, u1PhaseOutRptVersion = 0;
	INT32       Ret = 0;
	UINT8		u1BandIdx = TESTMODE_GET_BAND_IDX(pAd);

	if (Arg == NULL)
		return FALSE;

	if ((strlen(Arg) != 14) && (strlen(Arg) != 17))
		return FALSE;

	for (i = 0, value = rstrtok(Arg, ":"); value && i < ARRAY_SIZE(ucBuf);
			value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	MTWF_PRINT("%s: \n", __func__);
	ucGroup              = ucBuf[0];
	ucGroup_L_M_H        = ucBuf[1];
	fgSX2                = ucBuf[2];
	ucPhaseCal           = ucBuf[3];
	ucPhaseLnaGainLevel  = ucBuf[4];
	u1PhaseOutRptVersion = ucBuf[5];
	Ret = CmdITxBfPhaseCal(pAd,
						   ucGroup,
						   ucGroup_L_M_H,
						   fgSX2,
						   ucPhaseCal,
						   ucPhaseLnaGainLevel,
						   u1BandIdx,
						   u1PhaseOutRptVersion);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATETxBfLnaGain(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR       ucLnaGain;
	INT32       Ret = 0;
	UINT8		u1BandIdx = TESTMODE_GET_BAND_IDX(pAd);

	if (Arg == NULL)
		return FALSE;

	ucLnaGain = simple_strtol(Arg, 0, 10);
	MTWF_PRINT("%s: \n", __func__);
	Ret = CmdTxBfLnaGain(pAd, ucLnaGain, u1BandIdx);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATEIBfProfileUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#ifndef CONFIG_WLAN_SERVICE
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
#else
	UCHAR control_band_idx = hc_get_hw_band_idx(pAd);
#endif
	INT32 ret = 0, i;
	UINT16 u2Wcid;
	UCHAR BssIdx, phyMode, Nr, Nc, PfmuIdx, NdpNss, *value, ucBuf[3], aucPfmuMemRow[8] = {0}, aucPfmuMemCol[8] = {0};
	UCHAR ucTxAntConfig, *addr1 = NULL;
	RTMP_STRING cmdStr[80];
	RTMP_STRING cmdStr1[80];
	INT   sRet = 0;
#ifndef CONFIG_WLAN_SERVICE
	struct wifi_dev *wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, control_band_idx, wdev[0]);
#else
	struct wifi_dev *wdev = &pAd->ate_wdev;
#endif /* CONFIG_WLAN_SERVICE */

	if (Arg == NULL)
		return FALSE;

	if (strlen(Arg) != 8)
		return FALSE;

	addr1 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr1[0][0]);

	for (i = 0, value = rstrtok(Arg, ":"); value && i < ARRAY_SIZE(ucBuf); value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	PfmuIdx = ucBuf[0];

	MTWF_PRINT("%s: band[%d]'s TxAntennaSel = 0x%x\n",
		__func__, control_band_idx, TESTMODE_GET_PARAM(pAd, control_band_idx, tx_ant));
	ucTxAntConfig = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_ant);

	phyMode = 2;

	switch (ucTxAntConfig) {
	case 3:
		Nr = 1;
		break;
	case 7:
		Nr = 2;
		break;
	case 12:
		Nr = (IS_MT7915_FW_VER_E1(pAd)) ? 1 : 3;
		break;
	case 15:
		Nr = 3;
		break;
	case 31:
		Nr = 4;
		phyMode = 4;
		break;
	default:
		Nr = 3;
		break;
	}

	Nc = ucBuf[2];
	/* Configure iBF tag */
	/* PFMU ID */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", PfmuIdx);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	Set_TxBfProfileTag_PfmuIdx(pAd, cmdStr);
	/* ITxBf */
	Set_TxBfProfileTag_BfType(pAd, "0");
	/* BW20 */
	Set_TxBfProfileTag_DBW(pAd, "0");
	/* SU */
	Set_TxBfProfileTag_SuMu(pAd, "0");
	/* PFMU memory allocation */
	mt_WrapIBfCalGetIBfMemAlloc(pAd, aucPfmuMemRow, aucPfmuMemCol);
	sRet = snprintf(cmdStr, 24, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", aucPfmuMemCol[0], aucPfmuMemRow[0],
									aucPfmuMemCol[1], aucPfmuMemRow[1],
									aucPfmuMemCol[2], aucPfmuMemRow[2],
									aucPfmuMemCol[3], aucPfmuMemRow[3]);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	Set_TxBfProfileTag_Mem(pAd, cmdStr);
	/* Nr:Nc:Ng:LM:CB:HTCE */
	/* snprintf(cmdStr, 18, "%.2x:%.2x:00:01:00:00", Nr, Nc); */
	if (Nr == 4)
		sRet = snprintf(cmdStr, 18, "%.2x:%.2x:00:02:00:00", Nr, Nc);
	else
		sRet = snprintf(cmdStr, 18, "%.2x:%.2x:00:00:00:00", Nr, Nc);

	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	Set_TxBfProfileTag_Matrix(pAd, cmdStr);
	/* SNR */
	sRet = snprintf(cmdStr, 12, "00:00:00:00");
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	Set_TxBfProfileTag_SNR(pAd, cmdStr);
	/* SMART Antenna */
	Set_TxBfProfileTag_SmartAnt(pAd, "0");
	/* SE index */
	Set_TxBfProfileTag_SeIdx(pAd, "0");
	/* Rmsd */
	Set_TxBfProfileTag_RmsdThrd(pAd, "0");
	/* MCS threshold */
	sRet = snprintf(cmdStr, 18, "00:00:00:00:00:00");
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	Set_TxBfProfileTag_McsThrd(pAd, cmdStr);
	/* Time out disable */
	Set_TxBfProfileTag_TimeOut(pAd, "255");
	/* Desired BW20 */
	Set_TxBfProfileTag_DesiredBW(pAd, "0");
	/* Nr */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", Nr);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	Set_TxBfProfileTag_DesiredNr(pAd, cmdStr);
	/* Nc */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", Nc);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	Set_TxBfProfileTag_DesiredNc(pAd, cmdStr);
	/* Invalid the tag */
	Set_TxBfProfileTag_InValid(pAd, "1");
	/* Update PFMU tag */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", PfmuIdx);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	Set_TxBfProfileTagWrite(pAd, cmdStr);

	/* Configure the BF StaRec */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "01:%.2x:00:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
			wdev->BssIdx, PRINT_MAC(addr1));
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	ate_set_cmm_starec(pAd, cmdStr);

	switch (Nr) {
	case 1:
		NdpNss = 8;  /* MCS8, 2 streams */
		break;

	case 2:
		NdpNss = 16; /* MCS16, 3 streams */
		break;

	case 3:
		NdpNss = 24; /* MCS24, 4 streams */
		break;

	case 4:
		NdpNss = 32; /* MCS32, 5 streams */
		break;

	default:
		NdpNss = 24;
		break;
	}

	if (IS_MT7915(pAd))
		u2Wcid = (IS_MT7915_FW_VER_E1(pAd)) ? 4 : 1;
	else
		u2Wcid = 1;

	BssIdx = 0;
	if ((IS_MT7990(pAd)) ||
		(IS_MT7992(pAd)) ||
		(IS_MT7993(pAd))) {
		switch (control_band_idx) {
		case 0:
			u2Wcid = 1;
			break;
		case 1:
			u2Wcid = 2;
			BssIdx = 1;
			break;
		case 2:
			u2Wcid = 3;
			BssIdx = 2;
			break;
		}
	}

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_ERROR, "band ID = %d, Wlan ID = %d\n", control_band_idx, u2Wcid);

	sRet = snprintf(cmdStr1, sizeof(cmdStr1),
		"%.2x:%.2x:%.2x:00:00:00:%.2x:00:%.2x:%.2x:%.2x:00:00:00:00:",
		u2Wcid, BssIdx, PfmuIdx, NdpNss, phyMode, Nc, Nr);
	if (os_snprintf_error(sizeof(cmdStr1), sRet))
		MTWF_PRINT("snprintf error!\n");

	sRet = snprintf(cmdStr, sizeof(cmdStr),
		"%s%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
		cmdStr1,
		aucPfmuMemRow[0], aucPfmuMemCol[0],
		aucPfmuMemRow[1], aucPfmuMemCol[1],
		aucPfmuMemRow[2], aucPfmuMemCol[2],
		aucPfmuMemRow[3], aucPfmuMemCol[3]);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	Set_StaRecBfUpdate(pAd, cmdStr);

	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", u2Wcid);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	Set_StaRecBfRead(pAd, cmdStr);

	/* Configure WTBL */
	/* iwpriv ra0 set ManualAssoc =mac:222222222222-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:2-pfmuId:0 */
	sRet = snprintf(cmdStr1, sizeof(cmdStr1),
		"mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", PRINT_MAC(addr1));
	if (os_snprintf_error(sizeof(cmdStr1), sRet))
		MTWF_PRINT("snprintf error!\n");

	sRet = snprintf(cmdStr, sizeof(cmdStr),
		"%s-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:%d-pfmuId:%d\n",
		cmdStr1, (Nc + 1), PfmuIdx);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	if (IS_MT7990(pAd)) {
		ret = MtCmdSetTestEngine(pAd, RF_AT_FUNCID_COMMAND, RF_AT_COMMAND_TX_COMMIT);
		if (ret) {
			MTWF_PRINT("%s, ret: 0x%x\n", __func__, ret);
			return FALSE;
		}
	}

	return TRUE;
}


INT32 SetATEEBfProfileConfig(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#ifndef CONFIG_WLAN_SERVICE
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
#else
	UCHAR control_band_idx = hc_get_hw_band_idx(pAd);
#endif
	INT32 ret = 0, i;
	UINT16 u2Wcid;
	UCHAR BssIdx, phyMode, Nr, Nc, PfmuIdx, NdpaNss, NdpNss, *value, ucBuf[3], aucPfmuMemRow[8] = {0}, aucPfmuMemCol[8] = {0};
	UCHAR ucTxAntConfig, *addr1 = NULL;
	RTMP_STRING cmdStr[80];
	RTMP_STRING cmdStr1[80];
	INT   sRet = 0;
#ifndef CONFIG_WLAN_SERVICE
	struct wifi_dev *wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, control_band_idx, wdev[0]);
#else
	struct wifi_dev *wdev = &pAd->ate_wdev;
#endif /* CONFIG_WLAN_SERVICE */


	if (Arg == NULL)
		return FALSE;

	if (strlen(Arg) != 8)
		return FALSE;

	addr1 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr1[0][0]);
	for (i = 0, value = rstrtok(Arg, ":"); value && i < ARRAY_SIZE(ucBuf); value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	PfmuIdx = ucBuf[0];

	ucTxAntConfig = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_ant);

	phyMode = 2;

	switch (ucTxAntConfig) {
	case 3:
		Nr = 1;
		break;
	case 7:
		Nr = 2;
		break;
	case 12:
		Nr = (IS_MT7915_FW_VER_E1(pAd)) ? 1 : 3;
		break;
	case 15:
		Nr = 3;
		break;
	case 31:
		Nr = 4;
		phyMode = 4;
		break;
	default:
		Nr = 3;
		break;
	}

	Nc = ucBuf[2];
	/* Configure iBF tag */
	/* PFMU ID */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", PfmuIdx);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	Set_TxBfProfileTag_PfmuIdx(pAd, cmdStr);
	/* ETxBf */
	Set_TxBfProfileTag_BfType(pAd, "1");
	/* BW20 */
	Set_TxBfProfileTag_DBW(pAd, "0");
	/* SU */
	Set_TxBfProfileTag_SuMu(pAd, "0");
	/* PFMU memory allocation */
	mt_WrapIBfCalGetEBfMemAlloc(pAd, aucPfmuMemRow, aucPfmuMemCol);
	sRet = snprintf(cmdStr, 24, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", aucPfmuMemCol[0], aucPfmuMemRow[0],
									aucPfmuMemCol[1], aucPfmuMemRow[1],
									aucPfmuMemCol[2], aucPfmuMemRow[2],
									aucPfmuMemCol[3], aucPfmuMemRow[3]);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	Set_TxBfProfileTag_Mem(pAd, cmdStr);
	/* Nr:Nc:Ng:LM:CB:HTCE */
	if (Nr == 4)
		sRet = snprintf(cmdStr, 18, "%.2x:%.2x:00:02:00:00", Nr, Nc);
	else
		sRet = snprintf(cmdStr, 18, "%.2x:%.2x:00:01:00:00", Nr, Nc);

	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	Set_TxBfProfileTag_Matrix(pAd, cmdStr);
	/* SNR */
	sRet = snprintf(cmdStr, 12, "00:00:00:00");
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	Set_TxBfProfileTag_SNR(pAd, cmdStr);
	/* SMART Antenna */
	Set_TxBfProfileTag_SmartAnt(pAd, "0");
	/* SE index */
	Set_TxBfProfileTag_SeIdx(pAd, "0");
	/* Rmsd */
	Set_TxBfProfileTag_RmsdThrd(pAd, "0");
	/* MCS threshold */
	sRet = snprintf(cmdStr, 18, "00:00:00:00:00:00");
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	Set_TxBfProfileTag_McsThrd(pAd, cmdStr);
	/* Invalid the tag */
	Set_TxBfProfileTag_InValid(pAd, "1");
	/* Update PFMU tag */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", PfmuIdx);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	Set_TxBfProfileTagWrite(pAd, cmdStr);

	/* Configure the BF StaRec */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "01:%.2x:00:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
				wdev->BssIdx, PRINT_MAC(addr1));
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	ate_set_cmm_starec(pAd, cmdStr);

	NdpaNss = 0;

	switch (Nr) {
	case 1:
		NdpNss = 8;  /* MCS8, 2 streams */
		break;

	case 2:
		NdpNss = 16; /* MCS16, 3 streams */
		break;

	case 3:
		NdpNss = 24; /* MCS24, 4 streams */
		break;

	case 4:
		NdpNss = 0; /* MCS0, 5 streams */
		NdpaNss = 11; /* OFDM11 */
		break;

	default:
		NdpNss = 24;
		break;
	}

	if (IS_MT7915(pAd))
		u2Wcid = (IS_MT7915_FW_VER_E1(pAd)) ? 4 : 1;
	else
		u2Wcid = 1;

	BssIdx = 0;
	if ((IS_MT7990(pAd)) ||
		(IS_MT7992(pAd)) ||
		(IS_MT7993(pAd))) {
		switch (control_band_idx) {
		case 0:
			u2Wcid = 1;
			break;
		case 1:
			u2Wcid = 2;
			BssIdx = 1;
			break;
		case 2:
			u2Wcid = 3;
			BssIdx = 2;
			break;
		}
	}

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_ERROR, "band ID = %d, Wlan ID = %d\n", control_band_idx, u2Wcid);

	sRet = snprintf(cmdStr1, sizeof(cmdStr1),
		"%.2x:%.2x:%.2x:00:01:%.2x:%.2x:00:%.2x:%.2x:%.2x:00:00:00:00:",
		u2Wcid, BssIdx, PfmuIdx, NdpaNss, NdpNss, phyMode, Nc, Nr);
	if (os_snprintf_error(sizeof(cmdStr1), sRet))
		MTWF_PRINT("snprintf error!\n");

	sRet = snprintf(cmdStr, sizeof(cmdStr),
		"%s%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
		cmdStr1,
		aucPfmuMemRow[0], aucPfmuMemCol[0],
		aucPfmuMemRow[1], aucPfmuMemCol[1],
		aucPfmuMemRow[2], aucPfmuMemCol[2],
		aucPfmuMemRow[3], aucPfmuMemCol[3]);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	Set_StaRecBfUpdate(pAd, cmdStr);

	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", u2Wcid);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	Set_StaRecBfRead(pAd, cmdStr);

	/* Configure WTBL */
	/* iwpriv ra0 set ManualAssoc =mac:222222222222-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:2-pfmuId:0 */
	sRet = snprintf(cmdStr, sizeof(cmdStr),
			"mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:%d-pfmuId:%d\n",
			PRINT_MAC(addr1), (Nc + 1), PfmuIdx);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	if (IS_MT7990(pAd)) {
		ret = MtCmdSetTestEngine(pAd, RF_AT_FUNCID_COMMAND, RF_AT_COMMAND_TX_COMMIT);
		if (ret) {
			MTWF_PRINT("%s, ret: 0x%x\n", __func__, ret);
			return FALSE;
		}
	}

	return TRUE;
}


INT32 SetATEIBfPhaseComp(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32   Ret,         i;
	UCHAR   ucBW,        ucGroup, ucBand, ucDbdcBandIdx, *value, ucBuf[6];
	BOOLEAN fgRdFromE2p, fgDisComp, fgBw160Nc;

	if (Arg == NULL)
		return FALSE;

	if ((strlen(Arg) != 14) && (strlen(Arg) != 17))
		return FALSE;

	for (i = 0, value = rstrtok(Arg, ":"); value && i < ARRAY_SIZE(ucBuf); value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	ucBW          = ucBuf[0];
	ucDbdcBandIdx = ucBuf[1];
	ucGroup       = ucBuf[2];
	fgRdFromE2p   = ucBuf[3];
	fgDisComp     = ucBuf[4];
	fgBw160Nc     = (i == 6) ? ucBuf[4] : 0;
	ucBand = (ucGroup == 1) ? 1 : 0;
	Ret = CmdITxBfPhaseComp(pAd, ucBW, ucBand, ucDbdcBandIdx, ucGroup, fgRdFromE2p, fgDisComp);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATEIBfPhaseVerify(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32   Ret, i;
	UCHAR   ucGroup, ucGroup_L_M_H, ucDbdcBandIdx, ucPhaseCalType, ucBand, ucBw, *value;
	UCHAR   ucBuf[7] = {0};
	BOOLEAN fgRdFromE2p;
	UCHAR   ucPhaseVerifyLnaGainLevel, u1PhaseOutRptVersion = 0;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8	u1BandIdx = TESTMODE_GET_BAND_IDX(pAd);

	if (Arg == NULL)
		return FALSE;

	if ((strlen(Arg) != 17) && (strlen(Arg) != 20))
		return FALSE;

	for (i = 0, value = rstrtok(Arg, ":"); value && i < ARRAY_SIZE(ucBuf); value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	MTWF_PRINT("\n%s:\n", __func__);
	ucGroup        = ucBuf[0];
	ucGroup_L_M_H  = ucBuf[1];
	ucDbdcBandIdx  = ucBuf[2];
	ucPhaseCalType = ucBuf[3];
	ucPhaseVerifyLnaGainLevel = ucBuf[4];
	fgRdFromE2p    = ucBuf[5];
	u1PhaseOutRptVersion = ucBuf[6];
	ucBand = (ucGroup == 1) ? 1 : 0;
	ucBw = (ucGroup < 9) ? BW_20 : BW_160;
	Ret = CmdITxBfPhaseComp(pAd,
							ucBw,
							ucBand,
							ucDbdcBandIdx,
							ucGroup,
							fgRdFromE2p,
							FALSE);

	if (Ret) {
		/* Free memory allocated by iBF phase calibration */
		if (ops->iBFPhaseFreeMem)
			ops->iBFPhaseFreeMem(pAd);

		return FALSE;
	}

	Ret = CmdITxBfPhaseCal(pAd,
						   ucGroup,
						   ucGroup_L_M_H,
						   ucDbdcBandIdx,
						   ucPhaseCalType,
						   ucPhaseVerifyLnaGainLevel,
						   u1BandIdx,
						   u1PhaseOutRptVersion);

	if (Ret) {
		/* Free memory allocated by iBF phase calibration */
		if (ops->iBFPhaseFreeMem)
			ops->iBFPhaseFreeMem(pAd);

		return FALSE;
	}

	return TRUE;
}


INT32 SetATETxBfPhaseE2pUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32   i;
	UCHAR   ucGroup, ucUpdateAllType, *value, ucBuf[3];
	UCHAR fgSX2;
	struct _RTMP_ADAPTER *ad = NULL, *ad1 = NULL;
	struct _RTMP_CHIP_OP *ops;

	if (pAd == NULL || Arg == NULL || strlen(Arg) != 8)
		return FALSE;

	ops = hc_get_chip_ops(pAd->hdev_ctrl);

	for (i = 0, value = rstrtok(Arg, ":"); value && i < ARRAY_SIZE(ucBuf); value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	ucGroup         = ucBuf[0];
	fgSX2           = ucBuf[1];
	ucUpdateAllType = ucBuf[2];

	ops = hc_get_chip_ops(pAd->hdev_ctrl);

#ifdef CONFIG_WLAN_SERVICE
	ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, 0);
	ad1 = physical_device_get_mac_adapter_by_band(pAd->physical_dev, 1);
#else
	ad = pAd;
	ad1 = pAd;
#endif

	if ((ad == NULL) || (ad1 == NULL))
		return FALSE;

	/* Bit0   : BW160 ?
	   Bit1~3 : reserved
	   Bit4~5 : 0(Clean all), 1(Clean 2G iBF E2p only), 2(Clean 5G iBF E2p only)
	   Bit6~7 : reserved
	*/
	switch (fgSX2 >> 4)
	{
	case CLEAN_ALL:
		ad->u1IbfCalPhase2G5GE2pClean = 0; // Clean all
		ad1->u1IbfCalPhase2G5GE2pClean = 0;
		break;
	case CLEAN_2G:
		ad->u1IbfCalPhase2G5GE2pClean = 1; // Clean 2G
		ad1->u1IbfCalPhase2G5GE2pClean = 1;
		break;
	case CLEAN_5G:
		ad->u1IbfCalPhase2G5GE2pClean = 2; // Clean 5G
		ad1->u1IbfCalPhase2G5GE2pClean = 2;
		break;
	case CLEAN_6G:
		/* todo */
		break;
	default:
		ad->u1IbfCalPhase2G5GE2pClean = 0; // Clean all
		ad1->u1IbfCalPhase2G5GE2pClean = 0;
		break;
	}

	ops->iBFPhaseCalE2PUpdate(pAd, ucGroup, fgSX2, ucUpdateAllType);
	return TRUE;
}


INT32 SetATETxSoundingProc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32         Ret = 0;
	UCHAR         SoundingMode;
	struct _ATE_CTRL      *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_PRINT("%s: SoundingMode = %s\n", __func__, Arg);
	SoundingMode = simple_strtol(Arg, 0, 10);
	Ret = ATEOp->SetATETxSoundingProc(pAd, SoundingMode);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}

INT32 SetATEConTxETxBfInitProc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	UCHAR       control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT8       loop_index, u1Spe;
	INT         status = TRUE;
	CHAR        *value = 0;
	UINT8       TxMode = 0;
	UINT8       MCS = 0;
	UINT8       BW = 0;
	UINT8       BFBW = 0;
	UINT8       VhtNss = 0;
	UINT8       TRxStream = 0;
	UINT8       Power = 0;
	UINT8       Channel = 0;
	UINT8       Channel2 = 0;
	UINT8       Channl_band = 0;
	UINT16      TxPktLength = 0;
	UINT8       BF_Enable = 1;
	UINT8       Nr = 0;
	UINT8       LM = 0;
	UCHAR       OwnMacIdx = 0;
//	UCHAR       WlanIdx = 1;
	UCHAR       WlanIdx = control_band_idx + 1; /* Test Mode 2.0: Wlan Index=Band Index+1 */
	UCHAR       BssIdx = 0;
//	UCHAR       PfmuId = WlanIdx - 1;
	UCHAR       PfmuId = 0; /* Only for SUBF One STA, PFMU Index always be 0 */
	ULONG       stTimeChk0, stTimeChk1;
	RTMP_STRING cmdStr[80];
	UCHAR       *template, *addr1, *addr2, *addr3;
	INT         sRet = 0;
	long        tmp = 0;

	addr1 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr1[0][0]);
	addr2 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr2[0][0]);
	addr3 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr3[0][0]);

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Configure Input Parameter */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/

	/* sanity check for input parameter*/
	if (Arg == NULL) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(Arg) == 33) {
		MTWF_PRINT("Normal format!!\n");
	} else if (strlen(Arg) == 35) {
		MTWF_PRINT("Debug format!!\n");
	} else {
		MTWF_PRINT("Wrong parameter format!!\n");
		return FALSE;
	}

	/* Parsing input parameter */
	for (loop_index = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":"), loop_index++) {
		switch (loop_index) {
		case 0:
			sRet = kstrtol(value, 10, &tmp); /* 2-bit format */
			TxMode = (UINT8)tmp;
			break;

		case 1:
			sRet = kstrtol(value, 10, &tmp); /* 2-bit format */
			MCS = (UINT8)tmp;
			break;

		case 2:
			sRet = kstrtol(value, 10, &tmp); /* 2-bit format */
			BW = (UINT8)tmp;
			if (BW == 12)
				BFBW = 4; /* BW320 */
			else if (BW == 5)
				BFBW = 3; /* BW160 */
			else if (BW == 0 || BW == 1 || BW == 2 || BW == 6)
				BFBW = BW; /* BW 20, 40, 80, 8080 */
			else
				BFBW = BW + 1; /* BW 10, 5 */
			break;

		case 3:
			sRet = kstrtol(value, 10, &tmp); /* 2-bit format */
			VhtNss = (UINT8)tmp;
			break;

		case 4:
			sRet = kstrtol(value, 10, &tmp); /* 2-bit format */
			TRxStream = (UINT8)tmp;
			break;

		case 5:
			sRet = kstrtol(value, 10, &tmp); /* 2-bit format */
			Power = (UINT8)tmp;
			break;

		case 6:
			sRet = kstrtol(value, 10, &tmp); /* 3-bit format */
			Channel = (UINT8)tmp;
			break;

		case 7:
			sRet = kstrtol(value, 10, &tmp); /* 3-bit format */
			Channel2 = (UINT8)tmp;
			break;

		case 8:
			sRet = kstrtol(value, 10, &tmp); /* 1-bit format */
			Channl_band = (UINT8)tmp;
			break;

		case 9:
			sRet = kstrtol(value, 10, &tmp); /* 5-bit format */
			TxPktLength = (UINT16)tmp;
			break;

		case 10:
			sRet = kstrtol(value, 10, &tmp); /* 1-bit format */
			BF_Enable = (UINT8)tmp;
			break;

		default: {
			status = FALSE;
			MTWF_PRINT("%s: Set wrong parameters\n", __func__);
			break;
		}
		}
		if (sRet) {
			MTWF_PRINT("%s, ret: 0x%x\n", __func__, sRet);
			return FALSE;
		}
	}

	MTWF_PRINT("control_band = %d, BF_Enable = %d\n", control_band_idx, BF_Enable);

	MTWF_PRINT("TxMode = %d, MCS = %d, BW = %d, VhtNss = %d, TRxStream = %d\n",
		TxMode, MCS, BW, VhtNss, TRxStream);

	MTWF_PRINT("Power = %d, Channel = %d, Channel2 = %d, Channl_band = %d, TxPktLength = %d\n",
		Power, Channel, Channel2, Channl_band, TxPktLength);

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Load Preliminary Configuration */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
#ifdef CONFIG_AP_SUPPORT
	MTWF_PRINT("%s: control_band_idx = %d\n", __func__, control_band_idx);

#endif /* CONFIG_AP_SUPPORT */

	/* obtain TemplateFrame */
	NdisMoveMemory(TESTMODE_GET_PADDR(pAd, control_band_idx, template_frame), TemplateFrame, 32);

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* DUT TxBf Initialization */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	NdisGetSystemUpTime(&stTimeChk0);
	/* Start ATE Mode */
	SetATE(pAd, "ATESTART");
	/* Enable ETxBF Capability */
	CmdTxBfHwEnableStatusUpdate(pAd, TRUE, FALSE);

	/* set ATEDA=00:11:11:11:11:11 */
	os_move_mem(addr1, Addr1, MAC_ADDR_LEN);
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", PRINT_MAC(addr1));
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	SetATEDa(pAd, cmdStr);

	/* set ATESA=00:22:22:22:22:22 */
	os_move_mem(addr2, Addr2, MAC_ADDR_LEN);
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
			PRINT_MAC(addr2));
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	SetATESa(pAd, cmdStr);

	/* set ATEBSSID=00:22:22:22:22:22 */
	os_move_mem(addr3, Addr3, MAC_ADDR_LEN);
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
			 PRINT_MAC(addr3));
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	SetATEBssid(pAd, cmdStr);

	/* Set ATE Tx Frame content */
	template = TESTMODE_GET_PARAM(pAd, control_band_idx, template_frame); /* structure type of TemplateFrame structure is HEADER_802_11 */
	NdisMoveMemory(template +  4, Addr1, MAC_ADDR_LEN);
	NdisMoveMemory(template + 10, Addr2, MAC_ADDR_LEN);
	NdisMoveMemory(template + 16, Addr3, MAC_ADDR_LEN);
	/* Set Tx mode */
	if (TxMode == 15) {
		sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", 13);
		if (os_snprintf_error(sizeof(cmdStr), sRet))
			MTWF_PRINT("snprintf error!\n");
	} else {
		sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", TxMode);
		if (os_snprintf_error(sizeof(cmdStr), sRet))
			MTWF_PRINT("snprintf error!\n");
	}
	SetATETxMode(pAd, cmdStr);  /* 0: CCK  1: OFDM  2: HT Mixe dmode 3: HT Green Mode   4: VHT mode  8: HE mode*/
	/* RtmpOsMsDelay(100); */
	/* Set Tx MCS */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", MCS);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	SetATETxMcs(pAd, cmdStr);
	/* RtmpOsMsDelay(100); */
	/* Set Tx BW */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d:%d", BW, BW);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	SetATETxBw(pAd, cmdStr);  /* 0: 20MHz  1: 40MHz  2: 80MHz  3: 10M  4: 5M  5: 160MHz(160C)  6: 160MHz (160NC) */
	/* RtmpOsMsDelay(100); */

	/* Set Tx VhtNss */
	if (TxMode == 4 || TxMode == 8) {
		sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", VhtNss);
		if (os_snprintf_error(sizeof(cmdStr), sRet))
			MTWF_PRINT("snprintf error!\n");
		SetATETxNss(pAd, cmdStr);
	}

	/* set ATETXGI=0 */
	SetATETxGi(pAd, "0");
	/* set ATETXLDPC=1 */
	SetATETxLdpc(pAd, "1");

	/* Set ATE Channel */
	TESTMODE_SET_PARAM(pAd, control_band_idx, channel, Channel);
	TESTMODE_SET_PARAM(pAd, control_band_idx, channel_2nd, Channel2);
	/* Set ATE Tx Power = 36 (unit is 0.5 dBm) */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", Power);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	SetATETxPower0(pAd, cmdStr);
	NdisGetSystemUpTime(&stTimeChk1);
	MTWF_PRINT("%s(): DUT Init Time consumption : %lu sec\n", __func__, (stTimeChk1 - stTimeChk0) * 1000 / OS_HZ);

	/* Device info Update */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
		OwnMacIdx, PRINT_MAC(addr2), control_band_idx);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	Set_DevInfoUpdate(pAd, cmdStr);
	/* STOP AUTO Sounding */
	Set_Stop_Sounding_Proc(pAd, "1");
	sRet = MtCmdSetTestEngine(pAd, RF_AT_FUNCID_COMMAND, RF_AT_COMMAND_TX_COMMIT);
	if (sRet) {
		MTWF_PRINT("%s, ret: 0x%x\n", __func__, sRet);
		return FALSE;
	}

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Continuous packet Tx Initializaton */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	if (BF_Enable) {
		/* Set ATE EBF Enable */
		/* need to before switch channel for TxStream config */
		/* since TxStream only can update correct when etxbf is enable for 3T and 2T */
		SetATEEBfTx(pAd, "1");
	}
	/* Set ATE Channel */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d:%d:0:%d", Channel, Channl_band, Channel2);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	SetATEChannel(pAd, cmdStr);
	RtmpOsMsDelay(1000);

	/* Set Tx Rx Ant */
	/* bitwise representration, ex: 0x3 means wifi[0] and wifi[1] ON */
	if (TRxStream == 5) {
		if (control_band_idx == DBDC_BAND2) {
			sRet = snprintf(cmdStr, sizeof(cmdStr), "1:26");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");
		} else if (control_band_idx == DBDC_BAND1) {
			sRet = snprintf(cmdStr, sizeof(cmdStr), "1:25");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");
		} else {
			sRet = snprintf(cmdStr, sizeof(cmdStr), "1:24");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");
		}
		SetATETxAntenna(pAd, cmdStr); /* 15 (0xF:  wifi[0], wifi[1], wifi[2], wifi[3] on) */
		SetATERxAntenna(pAd, "31");
	} else if (TRxStream == 4) {
		if (control_band_idx == DBDC_BAND2) {
			sRet = snprintf(cmdStr, sizeof(cmdStr), "1:26");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");
		} else if (control_band_idx == DBDC_BAND1) {
			sRet = snprintf(cmdStr, sizeof(cmdStr), "1:25");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");
		} else {
			sRet = snprintf(cmdStr, sizeof(cmdStr), "1:24");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");
		}
		SetATETxAntenna(pAd, cmdStr);  /* 15 (0xF:  wifi[0], wifi[1], wifi[2], wifi[3] on) */
		SetATERxAntenna(pAd, "15");
	} else if (TRxStream == 3) {
		if (control_band_idx == DBDC_BAND2) {
			sRet = snprintf(cmdStr, sizeof(cmdStr), "1:26");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");
		} else if (control_band_idx == DBDC_BAND1) {
			sRet = snprintf(cmdStr, sizeof(cmdStr), "1:25");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");
		} else {
			sRet = snprintf(cmdStr, sizeof(cmdStr), "1:24");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");
		}
		SetATETxAntenna(pAd, cmdStr); /* 7 (0x7:  wifi[0], wifi[1], wifi[2] on) */
		SetATERxAntenna(pAd, "7");    /* 7 (0x7:  wifi[0], wifi[1], wifi[2] on) */
	} else if (TRxStream == 2) {
		if (control_band_idx == DBDC_BAND2) {
			sRet = snprintf(cmdStr, sizeof(cmdStr), "1:26");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");
			SetATETxAntenna(pAd, cmdStr);	/* 12 (0xC:  wifi[2], wifi[3] on) */
			SetATERxAntenna(pAd, "12");
		} else if (control_band_idx == DBDC_BAND1) {
			sRet = snprintf(cmdStr, sizeof(cmdStr), "1:25");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");
			SetATETxAntenna(pAd, cmdStr);	/* 12 (0xC:  wifi[2], wifi[3] on) */
			SetATERxAntenna(pAd, "12");
		} else {
			sRet = snprintf(cmdStr, sizeof(cmdStr), "1:24");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");
			SetATETxAntenna(pAd, cmdStr);	 /* 3 (0x3:  wifi[0], wifi[1] on) */
			SetATERxAntenna(pAd, "3");
		}
	}

	/* Set ATE Tx packet Length (unit is byte)  */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", TxPktLength);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	SetATETxLength(pAd, cmdStr);
	/* Set ATE Tx packet number = 0 (Continuous packet Tx)  */
	SetATETxCount(pAd, "0");
	/* Set ATE Tx packet Length = 4 (unit is slot time)  */
	SetATEIpg(pAd, "4");
	/* Set Queue Priority = 1 (WMM_BK Queue)  */
	SetATEQid(pAd, "1");

	/* Set ATE Tx Dequeue size = 4 (allocate 4 packet after receiving 1 free count) (NOT Use Now!!!)*/
	/* ATE Start Continuos Packet Tx */
	/* SetATE(pAd, "TXFRAMESKB"); */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* EBF Profile Cnfiguration */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	if (BF_Enable) {
		/* set TxBfProfileTag_PFMU ID */
		snprintf(cmdStr, sizeof(cmdStr), "%d", PfmuId);
		Set_TxBfProfileTag_PfmuIdx(pAd, cmdStr);
		/* set TxBfProfileTag_Bf Type */
		Set_TxBfProfileTag_BfType(pAd, "1"); /* 0: iBF  1: eBF */
		/* set TxBfProfileTag_DBW */
		snprintf(cmdStr, sizeof(cmdStr), "%d", BFBW); /* 0:20 1:40 2:80 3:160 4:320 */
		Set_TxBfProfileTag_DBW(pAd, cmdStr);
		/* set TxBfProfileTag_SUMU */
		Set_TxBfProfileTag_SuMu(pAd, "0"); /* 0: SU  1: MU */
		/* PFMU memory allocation */
		snprintf(cmdStr, 24, "00:00:00:01:00:02:00:03");
		Set_TxBfProfileTag_Mem(pAd, cmdStr);

		/* set TxBfProfileTag_Matrix */
		if (IS_MT7915(pAd) && (TxMode == 4 || TxMode == 8) && ((BFBW == 3) || (BFBW == 6))) { /* 2 antenna for each 80MHz band in Harrier 160MHz DBDC mode */
			if (TRxStream == 4)
				Nr = 1;
			else
				MTWF_PRINT("%s: Invalid Configuration for BW160!! For BW160, TxStream number must be 4!!\n", __func__);
		} else {
			if (TRxStream == 5)
				Nr = 4;
			else if (TRxStream == 4)
				Nr = 3;
			else if (TRxStream == 3)
				Nr = 2;
			else if (TRxStream == 2)
				Nr = 1;
		}

		if (TxMode == 15)
			LM = 4;
		else if (TxMode == 8)
			LM = 3;
		else if (TxMode == 4)
			LM = 2;
		else if (TxMode == 2)
			LM = 1;

		snprintf(cmdStr, 18, "%.2x:00:00:%.2x:00:00", Nr, LM); /* Nr:Nc:Ng:LM:CB:HTCE */
		Set_TxBfProfileTag_Matrix(pAd, cmdStr);
		/* set TxBfProfileTag_SNR */
		snprintf(cmdStr, 12, "00:00:00:00");
		Set_TxBfProfileTag_SNR(pAd, cmdStr);
		/* set TxBfProfileTag_Smart Antenna */
		Set_TxBfProfileTag_SmartAnt(pAd, "0");
		/* set TxBfProfileTag_SE index */

		if (control_band_idx == DBDC_BAND2)
			u1Spe = 26;
		else if (control_band_idx == DBDC_BAND1)
			u1Spe = 25;
		else
			u1Spe = 24;

		sRet = snprintf(cmdStr, sizeof(cmdStr), "%u", u1Spe);
		if (os_snprintf_error(sizeof(cmdStr), sRet))
			MTWF_PRINT("snprintf error!\n");
		Set_TxBfProfileTag_SeIdx(pAd, cmdStr);
		MTWF_PRINT("%s: u1Spe: %u\n", __func__, u1Spe);

		/* set TxBfProfileTag_Rmsd */
		Set_TxBfProfileTag_RmsdThrd(pAd, "0");
		/* set TxBfProfileTag_MCS Threshold */
		sRet = snprintf(cmdStr, 18, "00:00:00:00:00:00");
		if (os_snprintf_error(sizeof(cmdStr), sRet))
			MTWF_PRINT("snprintf error!\n");
		Set_TxBfProfileTag_McsThrd(pAd, cmdStr);
		/* set TxBfProfileTag_Invalid Tag */
		Set_TxBfProfileTag_InValid(pAd, "1");
		/* Update PFMU Tag */
		snprintf(cmdStr, sizeof(cmdStr), "00");
		Set_TxBfProfileTagWrite(pAd, cmdStr);

		/* Station Record BF Info Update */
		if (TxMode == 15)
			snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:00:01:49:00:49:0F:00:%.2x:%.2x:00:00:00:00:00:01:00:02:00:03:00", WlanIdx, BssIdx, PfmuId, Nr, BFBW);
		else if (TxMode == 8)
			snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:00:01:49:00:49:08:00:%.2x:%.2x:00:00:00:00:00:01:00:02:00:03:00", WlanIdx, BssIdx, PfmuId, Nr, BFBW);
		else if (TxMode == 4)
			snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:00:01:09:00:09:04:00:%.2x:%.2x:00:00:00:00:00:01:00:02:00:03:00", WlanIdx, BssIdx, PfmuId, Nr, BFBW);
		else if (TxMode == 2) {
			if (TRxStream == 5)
				MTWF_PRINT("%s: HT mode do not support TRxStream = 5 eBF\n", __func__);
			else if (TRxStream == 4)
				snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:00:01:00:18:00:02:00:%.2x:%.2x:00:00:00:00:00:01:00:02:00:03:00", WlanIdx, BssIdx, PfmuId, Nr, BW);
			else if (TRxStream == 3)
				snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:00:01:00:10:00:02:00:%.2x:%.2x:00:00:00:00:00:01:00:02:00:03:00", WlanIdx, BssIdx, PfmuId, Nr, BW);
			else if (TRxStream == 2)
				snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:00:01:00:08:00:02:00:%.2x:%.2x:00:00:00:00:00:01:00:02:00:03:00", WlanIdx, BssIdx, PfmuId, Nr, BW);
		}
		Set_StaRecBfUpdate(pAd, cmdStr);

		/*-----------------------------------------------------------------------------*/
		/* EBF TxBf Apply */
		/*-----------------------------------------------------------------------------*/
		if (control_band_idx == DBDC_BAND0) {
			/* WTBL Update TxBf Apply */
			sRet = snprintf(cmdStr, sizeof(cmdStr), "01:01:00:00:00");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");

			Set_TxBfTxApply(pAd, cmdStr);
			/* TXCMD TxBf Apply */
			sRet = snprintf(cmdStr, sizeof(cmdStr), "01:01:01");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");

			Set_TxBfTxCmd(pAd, cmdStr);
			/* Read Station Bf Record */
			Set_StaRecBfRead(pAd, "1");
			/* Trigger one shot Sounding packet */
			sRet = snprintf(cmdStr, sizeof(cmdStr), "00:01:00:01:00:00:00");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");

			Set_Trigger_Sounding_Proc(pAd, cmdStr);
			/*---------------------------------------------------------------*/
			/* Periodical Sounding Trigger */
			/*---------------------------------------------------------------*/
			/* Trigger Periodical Sounding packet */
			sRet = snprintf(cmdStr, sizeof(cmdStr), "02:01:FF:01:00:00:00");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");

			Set_Trigger_Sounding_Proc(pAd, cmdStr);
		} else if (control_band_idx == DBDC_BAND1) {
			/* WTBL Update TxBf Apply */
			sRet = snprintf(cmdStr, sizeof(cmdStr), "02:01:00:00:00");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");

			Set_TxBfTxApply(pAd, cmdStr);
			/* TXCMD TxBf Apply */
			sRet = snprintf(cmdStr, sizeof(cmdStr), "01:01:01");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");

			Set_TxBfTxCmd(pAd, cmdStr);
			/* Read Station Bf Record */
			Set_StaRecBfRead(pAd, "2");
			/* Trigger one shot Sounding packet */
			sRet = snprintf(cmdStr, sizeof(cmdStr), "00:01:00:02:00:00:00");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");

			Set_Trigger_Sounding_Proc(pAd, cmdStr);
			/*---------------------------------------------------------------*/
			/* Periodical Sounding Trigger */
			/*---------------------------------------------------------------*/
			/* Trigger Periodical Sounding packet */
			sRet = snprintf(cmdStr, sizeof(cmdStr), "02:01:FF:02:00:00:00");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");

			Set_Trigger_Sounding_Proc(pAd, cmdStr);
		} else if (control_band_idx == DBDC_BAND2) {
			/* WTBL Update TxBf Apply */
			sRet = snprintf(cmdStr, sizeof(cmdStr), "03:01:00:00:00");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");

			Set_TxBfTxApply(pAd, cmdStr);
			/* TXCMD TxBf Apply */
			sRet = snprintf(cmdStr, sizeof(cmdStr), "01:01:01");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");

			Set_TxBfTxCmd(pAd, cmdStr);
			/* Read Station Bf Record */
			Set_StaRecBfRead(pAd, "3");
			/* Trigger one shot Sounding packet */
			sRet = snprintf(cmdStr, sizeof(cmdStr), "00:01:00:03:00:00:00");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");

			Set_Trigger_Sounding_Proc(pAd, cmdStr);
			/*---------------------------------------------------------------*/
			/* Periodical Sounding Trigger */
			/*---------------------------------------------------------------*/
			/* Trigger Periodical Sounding packet */
			sRet = snprintf(cmdStr, sizeof(cmdStr), "02:01:FF:03:00:00:00");
			if (os_snprintf_error(sizeof(cmdStr), sRet))
				MTWF_PRINT("snprintf error!\n");

			Set_Trigger_Sounding_Proc(pAd, cmdStr);
		} else {
			MTWF_PRINT("%s: Wrong control band idx=%d\n", __func__, control_band_idx);
		}
	}
	/* Enable MAC Rx */
	SetATE(pAd, "RXFRAME");
	return status;
}


INT32 SetATEConTxETxBfGdProc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	UINT8       loop_index;
	INT         status = TRUE;
	CHAR        *value = 0;
	UINT32      TxMode = 0;
	UINT32      MCS = 0;
	UINT32      BW = 0;
	UINT32      Channel = 0;
	UINT8       Channel2 = 0;
	UINT8       Channl_band = 0;
	UINT32      CRvalue = 0;
	ULONG       stTimeChk0, stTimeChk1;
	RTMP_STRING cmdStr[80];
	UCHAR *addr1 = NULL, *addr2 = NULL, *addr3 = NULL;
	INT         sRet = 0;

	addr1 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr1[0][0]);
	addr2 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr2[0][0]);
	addr3 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr3[0][0]);

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Configure Input Parameter */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/

	/* sanity check for input parameter*/
	if (Arg == NULL) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(Arg) != 18) {
		MTWF_PRINT("Wrong parameter format!!\n");
		return FALSE;
	}

	/* Parsing input parameter */
	for (loop_index = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":"), loop_index++) {
		switch (loop_index) {
		case 0:
			TxMode = simple_strtol(value, 0, 10); /* 2-bit format */
			break;

		case 1:
			MCS = simple_strtol(value, 0, 10); /* 2-bit format */
			break;

		case 2:
			BW = simple_strtol(value, 0, 10); /* 2-bit format */
			break;

		case 3:
			Channel = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 4:
			Channel2 = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 5:
			Channl_band = simple_strtol(value, 0, 10); /* 1-bit format */
			break;

		default: {
			status = FALSE;
			MTWF_PRINT("Set wrong parameters\n");
			break;
		}
		}
	}

	MTWF_PRINT("TxMode = %d, MCS = %d, BW = %d, \
			Channel = %d, Channel2 = %d, Channl_band = %d\n",
			TxMode, MCS, BW, Channel, Channel2, Channl_band);
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* GOLDEN TxBf Initialization */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	NdisGetSystemUpTime(&stTimeChk0);

	/* Start ATE Mode */
	SetATE(pAd, "ATESTART");

	/* set ATEDA=00:22:22:22:22:22 */
	os_move_mem(addr1, Addr2, MAC_ADDR_LEN);
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", PRINT_MAC(addr1));
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	SetATEDa(pAd, cmdStr);

	/* set ATESA=00:11:11:11:11:11 */
	os_move_mem(addr2, Addr1, MAC_ADDR_LEN);

	/* set ATEBSSID=00:22:22:22:22:22 */
	os_move_mem(addr3, Addr3, MAC_ADDR_LEN);

	/* Set Tx mode */
	if (TxMode == 15) {
		sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", 13);
		if (os_snprintf_error(sizeof(cmdStr), sRet))
			MTWF_PRINT("snprintf error!\n");
	} else {
		sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", TxMode);
		if (os_snprintf_error(sizeof(cmdStr), sRet))
			MTWF_PRINT("snprintf error!\n");
	}
	/* 0:CCK 1:OFDM 2: HT Mixe dmode 3: HT Green Mode 4: VHT mode 8: HE mode*/
	SetATETxMode(pAd, cmdStr);
	/* Set Tx MCS */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d", MCS);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	SetATETxMcs(pAd, cmdStr);
	/* Set Tx BW */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d:%d", BW, BW);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	SetATETxBw(pAd, cmdStr);  /* 0: 20MHz  1: 40MHz  2: 80MHz  3: 160MHz(160C)  4: 5M  5: 10M  6: 160MHz (160NC) */
	/* set ATETXGI=0 */
	SetATETxGi(pAd, "0");

	if (IS_MT7986(pAd) ||
		IS_MT7916(pAd) ||
		IS_MT7981(pAd) ||
		IS_MT7990(pAd) ||
		IS_MT7992(pAd) ||
		IS_MT7993(pAd)) {
		SetATERxAntenna(pAd, "1");
	} else {
		if ((BW == 5) || (BW == 6)) {
			SetATETxAntenna(pAd, "5"); /* for BW160C, BW160NC */
			SetATERxAntenna(pAd, "5");
		} else {
			SetATETxAntenna(pAd, "1"); /* for BW20, BW40, BW80 */
			SetATERxAntenna(pAd, "1");
		}
	}

	NdisGetSystemUpTime(&stTimeChk1);
	MTWF_PRINT("%s: SetATETxBfGdInitProc Time consumption : %lu sec\n",
		__func__, (stTimeChk1 - stTimeChk0) * 1000 / OS_HZ);
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Turn On BBP CR for Rx */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* iwpriv ra0 mac 82070280=00008001 */
	PHY_IO_WRITE32(pAd->hdev_ctrl, 0x10280, 0x00008001);
	/* check  */
	PHY_IO_READ32(pAd->hdev_ctrl, 0x10280, &CRvalue);
	MTWF_PRINT("%s: <0x82070280> = 0x%x\n", __func__, CRvalue);
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Sounding Mechanism TRx configuration */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Set Channel Info to ATE Control structure */
	TESTMODE_SET_PARAM(pAd, control_band_idx, channel, Channel);
	TESTMODE_SET_PARAM(pAd, control_band_idx, channel_2nd, Channel2);

	/* Set ATE Channel */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "%d:%d:0:%d", Channel, Channl_band, Channel2);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");
	SetATEChannel(pAd, cmdStr);

	RtmpOsMsDelay(1000);
	/* ATE Start Continuos Packet Rx */
	SetATE(pAd, "RXFRAME");
	/* ATE MAC TRx configuration, Enable Tx MAC HW before trigger sounding */
	/* ENUM_ATE_MAC_RX_RXV: MAC to PHY Rx Enable
	MtATESetMacTxRx(pAd, 6, 1, control_band_idx); */

	sRet = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
			PRINT_MAC(addr2));
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	SetATESa(pAd, cmdStr);

	sRet = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
			 PRINT_MAC(addr3));
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_PRINT("snprintf error!\n");

	SetATEBssid(pAd, cmdStr);

	return status;
}


INT32 SetATESpeIdx(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	UINT8	loop_index;
	CHAR	*value = 0;
	INT	status = TRUE;

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Configure Input Parameter */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/

	/* sanity check for input parameter*/
	if (Arg == NULL) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(Arg) != 1) {
		MTWF_PRINT("Wrong parameter format!!\n");
		return FALSE;
	}

	/* Parsing input parameter */
	for (loop_index = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":"), loop_index++) {
		switch (loop_index) {

		default: {
			status = FALSE;
			MTWF_PRINT("Set wrong parameters\n");
			break;
		}
		}
	}
	return status;
}


INT32 SetATEEBfTx(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UCHAR	control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT8	loop_index;
	CHAR	*value = 0;
	UINT32	eTxBf = 0;
	INT	status = TRUE;
	UCHAR	addr[6] = {0x00, 0x11, 0x11, 0x11, 0x11, 0x11};
	UCHAR	*pate_pkt;
	UCHAR	WlanIdx = 1;
	UCHAR	PfmuId = WlanIdx - 1;

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Configure Input Parameter */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/

	/* sanity check for input parameter*/
	if (Arg == NULL) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(Arg) != 1) {
		MTWF_PRINT("Wrong parameter format!!\n");
		return FALSE;
	}

	/* Parsing input parameter */
	for (loop_index = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":"), loop_index++) {
		switch (loop_index) {
		case 0:
			eTxBf = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_PRINT("Set wrong parameters\n");
			break;
		}
		}
	}

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* EBF Configuration */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	TESTMODE_SET_PARAM(pAd, control_band_idx, ebf, eTxBf);
	MTWF_PRINT("%s: band[%d] eTxBf = %d !!!!!\n",
		__func__, control_band_idx, TESTMODE_GET_PARAM(pAd, control_band_idx, ebf));
	ATECtrl->wcid_ref = WlanIdx; /* For Sportan certification, only Golden */
	NdisCopyMemory(ATECtrl->pfmu_info[PfmuId].addr, addr, MAC_ADDR_LEN);

	if (TESTMODE_GET_PARAM(pAd, control_band_idx, ebf))
		SetATESpeIdx(pAd, "1");
	else
		SetATESpeIdx(pAd, "0");

	pate_pkt = TESTMODE_GET_PARAM(pAd, control_band_idx, test_pkt);
	/* Generate new packet with new contents */
	/* offload to fw control
	MT_ATEComposePkt(pAd, pate_pkt, control_band_idx, 0); */
	return status;
}


INT32 SetATEEBFCE(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	UINT8	loop_index;
	CHAR	*value = 0;
	INT	status = TRUE;

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Configure Input Parameter */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/

	/* sanity check for input parameter*/
	if (Arg == NULL) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(Arg) != 1) {
		MTWF_PRINT("Wrong parameter format!!\n");
		return FALSE;
	}

	/* Parsing input parameter */
	for (loop_index = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":"), loop_index++) {
		switch (loop_index) {
		default: {
			status = FALSE;
			MTWF_PRINT("Set wrong parameters\n");
			break;
		}
		}
	}

	return status;
}


INT32 SetATEEBFCEHelp(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT	status = TRUE;

	MTWF_PRINT("=============================================================================================\n");
	MTWF_PRINT("                            ATE ETxBF Certification Procedure Guide\n");
	MTWF_PRINT("=============================================================================================\n");
	MTWF_PRINT("For HT20 mode\n");
	MTWF_PRINT("\n");
	MTWF_PRINT(" 1)  iwpriv ra0 set ATEEBFCE=1\n");
	MTWF_PRINT(" 2)  iwpriv ra0 set ATEConTxETxBfGdProc=02:00:00:036:112:1 (Use in Golden Device)\n");
	MTWF_PRINT(" 3)  iwpriv ra0 set ATEConTxETxBfInitProc=02:00:00:01:04:18:036:112:1:04000\n");
	MTWF_PRINT(" 4)  iwpriv ra0 set ATETXEBF=1 (Tx packet apply BF On)\n");
	MTWF_PRINT(" 5)  iwpriv ra0 set ATE=TXFRAME\n");
	MTWF_PRINT(" 6)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)\n");
	MTWF_PRINT(" 7)  check IQxel waveform\n");
	MTWF_PRINT(" 8)  iwpriv ra0 set ATETXEBF=0 (Tx packet apply BF Off)\n");
	MTWF_PRINT(" 9)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)\n");
	MTWF_PRINT("10)  check Iqxel waveformn");
	MTWF_PRINT("---------------------------------------------------------------------------------------------\n");
	MTWF_PRINT("For HT40 mode\n");
	MTWF_PRINT("\n");
	MTWF_PRINT(" 1)  iwpriv ra0 set ATEEBFCE=1\n");
	MTWF_PRINT(" 2)  iwpriv ra0 set ATEConTxETxBfGdProc=02:00:01:036:112:1 (Use in Golden Device)\n");
	MTWF_PRINT(" 3)  iwpriv ra0 set ATEConTxETxBfInitProc=02:00:01:01:04:18:036:112:1:04000\n");
	MTWF_PRINT(" 4)  iwpriv ra0 set ATETXEBF=1 (Tx packet apply BF On)\n");
	MTWF_PRINT(" 5)  iwpriv ra0 set ATE=TXFRAMESKB\n");
	MTWF_PRINT(" 6)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)\n");
	MTWF_PRINT(" 7)  check IQxel waveform\n");
	MTWF_PRINT(" 8)  iwpriv ra0 set ATETXEBF=0 (Tx packet apply BF Off)\n");
	MTWF_PRINT(" 9)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)\n");
	MTWF_PRINT("10)  check Iqxel waveform\n");
	MTWF_PRINT("---------------------------------------------------------------------------------------------\n");
	MTWF_PRINT("For VHT80 mode\n");
	MTWF_PRINT("\n");
	MTWF_PRINT(" 1)  iwpriv ra0 set ATEEBFCE=1\n");
	MTWF_PRINT(" 2)  iwpriv ra0 set ATEConTxETxBfGdProc=04:00:02:036:112:1 (Use in Golden Device)\n");
	MTWF_PRINT(" 3)  iwpriv ra0 set ATEConTxETxBfInitProc=04:00:02:01:04:18:036:112:1:16000\n");
	MTWF_PRINT(" 4)  iwpriv ra0 set ATETXEBF=1 (Tx packet apply BF On)\n");
	MTWF_PRINT(" 5)  iwpriv ra0 set ATE=TXFRAMESKB\n");
	MTWF_PRINT(" 6)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)\n");
	MTWF_PRINT(" 7)  check IQxel waveform\n");
	MTWF_PRINT(" 8)  iwpriv ra0 set ATETXEBF=0 (Tx packet apply BF Off)\n");
	MTWF_PRINT(" 9)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)\n");
	MTWF_PRINT("10)  check Iqxel waveform\n");
	MTWF_PRINT("---------------------------------------------------------------------------------------------\n");
	MTWF_PRINT("For VHT160C mode\n");
	MTWF_PRINT("\n");
	MTWF_PRINT(" 1)  iwpriv ra0 set ATEEBFCE=1\n");
	MTWF_PRINT(" 2)  iwpriv ra0 set ATEConTxETxBfGdProc=04:00:06:036:112:1 (Use in Golden Device)\n");
	MTWF_PRINT(" 3)  iwpriv ra0 set ATEConTxETxBfInitProc=04:00:03:01:04:18:036:112:1:16000\n");
	MTWF_PRINT(" 4)  iwpriv ra0 set ATETXEBF=1 (Tx packet apply BF On)\n");
	MTWF_PRINT(" 5)  iwpriv ra0 set ATE=TXFRAMESKB\n");
	MTWF_PRINT(" 6)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)\n");
	MTWF_PRINT(" 7)  check IQxel waveform\n");
	MTWF_PRINT(" 8)  iwpriv ra0 set ATETXEBF=0 (Tx packet apply BF Off)\n");
	MTWF_PRINT(" 9)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)\n");
	MTWF_PRINT("10)  check Iqxel waveform\n");
	MTWF_PRINT("---------------------------------------------------------------------------------------------\n");
	MTWF_PRINT("For VHT160NC mode\n");
	MTWF_PRINT("\n");
	MTWF_PRINT(" 1)  iwpriv ra0 set ATEEBFCE=1\n");
	MTWF_PRINT(" 2)  iwpriv ra0 set ATEConTxETxBfGdProc=04:00:06:036:112:1 (Use in Golden Device)\n");
	MTWF_PRINT(" 3)  iwpriv ra0 set ATEConTxETxBfInitProc=04:00:06:01:04:18:036:112:1:16000\n");
	MTWF_PRINT(" 4)  iwpriv ra0 set ATETXEBF=1 (Tx packet apply BF On)\n");
	MTWF_PRINT(" 5)  iwpriv ra0 set ATE=TXFRAMESKB\n");
	MTWF_PRINT(" 6)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)\n");
	MTWF_PRINT(" 7)  check IQxel waveform\n");
	MTWF_PRINT(" 8)  iwpriv ra0 set ATETXEBF=0 (Tx packet apply BF Off)\n");
	MTWF_PRINT(" 9)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)\n");
	MTWF_PRINT("10)  check Iqxel waveform\n");
	MTWF_PRINT("---------------------------------------------------------------------------------------------\n");
	MTWF_PRINT("For DBDC Band1 HT20 mode\n");
	MTWF_PRINT("\n");
	MTWF_PRINT(" 1)  configure DBDC mode and Reboot system\n");
	MTWF_PRINT(" 2)  iwpriv ra1 set ATEEBFCE=1\n");
	MTWF_PRINT(" 3)  iwpriv ra1 set ATEConTxETxBfGdProc=02:00:00:36:112:1 (Use in Golden Device)\n");
	MTWF_PRINT(" 4)  iwpriv ra1 set ATEConTxETxBfInitProc=02:00:00:01:02:18:36:112:1:04000\n");
	MTWF_PRINT(" 5)  iwpriv ra1 set ATETXEBF=1 (Tx packet apply BF On)\n");
	MTWF_PRINT(" 6)  iwpriv ra1 set ATE=TXFRAMESKB\n");
	MTWF_PRINT(" 7)  iwpriv ra1 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)\n");
	MTWF_PRINT(" 8)  check IQxel waveform\n");
	MTWF_PRINT(" 9)  iwpriv ra1 set ATETXEBF=0 (Tx packet apply BF Off)\n");
	MTWF_PRINT("10)  iwpriv ra1 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)\n");
	MTWF_PRINT("11)  check Iqxel waveform\n");
	MTWF_PRINT("=============================================================================================\n");
	MTWF_PRINT("                           Method for Dynamical Control Tx Power\n");
	MTWF_PRINT("=============================================================================================\n");
	MTWF_PRINT(" 1)  Follow ETxBF Certification Procedure to enable TxBf packet at first\n");
	MTWF_PRINT(" 2)  Use command \"iwpriv ra0 set ATE=TXSTOP\" to stop Tx\n");
	MTWF_PRINT(" 3)  Use command \"iwpriv ra0 set ATETXPOW0=XX\" to configure Tx Power DAC value for OFDM 54M\n");
	MTWF_PRINT(" 4)  USe command \"ra0 set ATE=TXFRAMESKB\" to start continuous packet Tx\n");
	MTWF_PRINT("=============================================================================================\n");


	return status;
}


#endif /* TXBF_SUPPORT && MT_MAC */


INT32 SetATEHelp(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;

	MTWF_PRINT("ATE=ATESTART, ATESTOP, TXCONT, TXFRAME, RXFRAME\n");
	MTWF_PRINT("ATEDA\n");
	MTWF_PRINT("ATESA\n");
	MTWF_PRINT("ATEBSSID\n");
	MTWF_PRINT("ATECHANNEL, range:0~14\n");
	MTWF_PRINT("ATETXPOW0, set power level.\n");
	MTWF_PRINT("ATETXANT, set TX antenna in bitwise.\n");
	MTWF_PRINT("ATERXANT, set RX antenna in bitwise.\n");
	MTWF_PRINT("ATETXBW, set BandWidth, 0:20MHz, 1:40MHz, 2:80MHz, 5:160MHz, 12:320MHz\n");
	MTWF_PRINT("ATETXLEN, set Frame length, range 24~%d\n", (MAX_FRAME_SIZE - 34/* == 2312 */));
	MTWF_PRINT("ATETXCNT, set how many frame going to transmit.\n");
	MTWF_PRINT("ATETXMCS, set MCS, reference to rate table.\n");
	MTWF_PRINT("ATETXMODE, set Mode 0:CCK, 1:OFDM, 2:HT-Mix, 3:GreenField, 4:VHT, reference to rate table.\n");
	MTWF_PRINT("ATETXGI, set GI interval, 0:Long, 1:Short\n");
	MTWF_PRINT("ATERE2P, display all EEPROM content.\n");
	MTWF_PRINT("ATEIPG, set ATE Tx frame IPG.\n");
	MTWF_PRINT("ATEPAYLOAD, set ATE payload pattern for TxFrame.\n");
#ifdef DOT11_EHT_BE
	MTWF_PRINT("ATEPREAMPUNC, set preamble puncture.\n");
#endif
	MTWF_PRINT("ATESHOW, display all parameters of ATE.\n");
	MTWF_PRINT("ATEHELP, online help.\n");

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}

INT32 ATEInit(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	NdisZeroMemory(ATECtrl, sizeof(*ATECtrl));
	/* restore wdev */
	ATECtrl->op_mode  = ATE_STOP;
	ATECtrl->tx_cnt = 0xFFFFFFFF;
	ATECtrl->payload[0] = 0xAA;
	ATECtrl->fixed_payload = 1;
	ATECtrl->tx_len = 1058;/* 1058 : sync with QA */
	ATECtrl->bw = BW_20;
	ATECtrl->tx_mode = MODE_OFDM;
	ATECtrl->mcs = 7;
	ATECtrl->sgi = 0;/* LONG GI : 800 ns*/
	if (BOARD_IS_5G_ONLY(pAd))
		ATECtrl->channel = 36;
	else
		ATECtrl->channel = 1;
	ATECtrl->tx_ant = 1;
	ATECtrl->rx_ant = 0;
	ATECtrl->ac_idx = QID_AC_BE;
	ATECtrl->addr1[0][0] = 0x00;
	ATECtrl->addr1[0][1] = 0x11;
	ATECtrl->addr1[0][2] = 0x22;
	ATECtrl->addr1[0][3] = 0xAA;
	ATECtrl->addr1[0][4] = 0xBB;
	ATECtrl->addr1[0][5] = 0xCC;
	NdisMoveMemory(ATECtrl->addr2[0], ATECtrl->addr1[0], MAC_ADDR_LEN);
	NdisMoveMemory(ATECtrl->addr3[0], ATECtrl->addr1[0], MAC_ADDR_LEN);
	ATECtrl->bQAEnabled = FALSE;
	ATECtrl->bQATxStart = FALSE;
	ATECtrl->bQARxStart = FALSE;
	ATECtrl->duty_cycle = 0;
	ATECtrl->tx_time_param.pkt_tx_time_en = FALSE;
	ATECtrl->tx_time_param.pkt_tx_time = 0;
	ATECtrl->ipg_param.ipg = 0;
	ATECtrl->ipg_param.sig_ext = SIG_EXTENSION;
	ATECtrl->ipg_param.slot_time = DEFAULT_SLOT_TIME;
	ATECtrl->ipg_param.sifs_time = DEFAULT_SIFS_TIME;
	ATECtrl->ipg_param.ac_num = QID_AC_BE;
	ATECtrl->ipg_param.aifsn = MIN_AIFSN;
	ATECtrl->ipg_param.cw = MIN_CW;
	ATECtrl->ipg_param.txop = 0;
	ATECtrl->control_band_idx = 0;	/* Control band0 as default setting */
	/* Assign wdev_idx */
	ATECtrl->wdev_idx = 0;
	ATECtrl->wmm_idx = 0; /* Need to modify after j mode implement done */
#ifdef TXBF_SUPPORT
	ATECtrl->ebf = FALSE;
	ATECtrl->ibf = FALSE;
#endif
	os_move_mem(&ATECtrl->template_frame, TemplateFrame, sizeof(TemplateFrame));
#ifdef MT_MAC
	/* Ret = MT_ATEInit(pAd); */
#endif
	return Ret;
}

INT32 ATEExit(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;

#ifdef MT_MAC
	/* Ret = MT_ATEExit(pAd); */
#endif
	return Ret;
}

INT32 SetATE(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	UCHAR band_idx = 0;

#ifdef SPECIAL_11B_OBW_FEATURE
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	struct wifi_dev *wdev = &pMbss->wdev;
#endif

	MTWF_PRINT("%s: Arg = %s\n", __func__, Arg);
	band_idx = hc_get_hw_band_idx(pAd);

	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

#if defined(CONFIG_WLAN_SERVICE)
#ifdef CONFIG_LOGAN_ARHT_SOC_NOT_USE
	if (!strncmp(Arg, "RXFRAME", 7)) {
		set_ate_rx_stat_reset(pAd, Arg);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
			"%s to clean rx counter\n", Arg);
	}
#endif

	Ret = mt_agent_cli_act(Arg, &pAd->serv);

	if (Ret)
		goto err1;
#endif	/* CONFIG_WLAN_SERVICE */

#ifdef SPECIAL_11B_OBW_FEATURE
	if (wdev->channel >= 1 && wdev->channel <= 14) {
		MtCmdSetTxTdCck(pAd, TRUE);
	}
#endif

	if (!Ret)
		return TRUE;

err1:
	MTWF_PRINT("- RF-test stop fail, ret:%d\n", Ret);
	return FALSE;
}

INT32 SetATECfgOnOffProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	u_int8 type;
	u_int8 enable;
	CHAR *value;
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct service_test *serv_test = NULL;

	if (!arg) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) > 4) { /* Type:Enable, acceptable format 0:1*/
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"Acceptable format is Type:Enable. Ex: 0:1\n");
		return false;
	}

	value = rstrtok(arg, ":");
	if (value == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"Type is NULL.\n");
		return FALSE;
	}

	type = os_str_tol(value, 0, 10);
	if (type > 99) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"Type is out of range.\n");
		return FALSE;
	}

	value = rstrtok(NULL, ":");
	if (value == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR, "Enable is NULL.\n");
		return FALSE;
	}

	enable = os_str_tol(value, 0, 10);
	if (enable > 1) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"Enable should be 0 or 1.\n");
		return FALSE;
	}

	if (pAd != NULL)
		serv_test = net_ad_wrap_service(pAd);
	else {
		ret = SERV_STATUS_SERV_TEST_INVALID_NULL_POINTER;
		return ret;
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, log_type, (u_char)type);
	CONFIG_SET_PARAM(serv_test, log_enable, (u_char)enable);

	ret = mt_serv_set_cfg_on_off(serv_test);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"type=%u, Enable=%u\n", type, enable);

	return ret;
}

INT32 SetATEChannel(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_ext("ATECHANNEL", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
}

INT32 SetATETxBw(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_PRINT("%s: No intf up in band\n", __func__);
		return -ENETDOWN;
	}

	return (mt_agent_cli_set_ext("ATETXBW", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
	return TRUE;
}

VOID rtmp_ate_init(RTMP_ADAPTER *pAd)
{

	if (ATEInit(pAd) != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("ATE initialization failed !\n");
		ATEExit(pAd);
		return;
	}
}

INT32 SetHQA(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{

	int8_t *hqa_param = NULL;
	struct hqa_frame_ctrl local_hqa;
	int32_t ret;

	MTWF_PRINT("%s: Arg:%s\n", __func__, Arg);

	hqa_param = (int8_t *)Arg;

#ifdef CONFIG_WLAN_SERVICE
	MTWF_PRINT("%s: hqa_param = %s\n", __func__, hqa_param);
	if (hqa_param) {
		local_hqa.type = 1;
		os_alloc_mem(NULL, (uint8_t **)&local_hqa.hqa_frame_eth, sizeof(struct hqa_frame));
		os_alloc_mem(NULL, (uint8_t **)&local_hqa.hqa_frame_string, SERV_IOCTLBUFF*sizeof(int8_t));

		if ((local_hqa.hqa_frame_eth) && (local_hqa.hqa_frame_string))  {
			memset(local_hqa.hqa_frame_string, 0, SERV_IOCTLBUFF*sizeof(int8_t));
			memcpy(local_hqa.hqa_frame_string, hqa_param, strlen(hqa_param));

			ret = mt_agent_hqa_cmd_handler(&pAd->serv, &local_hqa);

			MTWF_PRINT("%s: ret = %d\n", __func__, ret);
		}


		os_free_mem(local_hqa.hqa_frame_eth);
		os_free_mem(local_hqa.hqa_frame_string);
	}
#endif

	return TRUE;

}


#ifdef CONFIG_WLAN_SERVICE
INT ate_cmd_proc(
	RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
#define BUF_LEN	32

	INT ret = 0;
	UCHAR *param_ptr = NULL, param_idx = 0;
	CHAR buf[BUF_LEN], len = 0;
	UINT32 param[2] = {0, 0}; /* id, set data */
	UINT32 rsp_data = 0;

	if (arg == NULL) {
		MTWF_PRINT("arg is empty\n");
		return FALSE;
	}

	if (arg) {
		for (param_idx = 0, param_ptr = rstrtok(arg, "-"); param_ptr && param_idx < ARRAY_SIZE(param);
			param_ptr = rstrtok(NULL, "-"), param_idx++) {
			ret = sscanf(param_ptr, "%8x", &param[param_idx]);

			if (ret == 0) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR, " invalid format(%s), ignored!\n", param_ptr);

				goto err_out;
			}
		}
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO, "id = %d, data = 0x%08x\n", param[0], param[1]);
	}

	if (param_idx < 3) {
		if (param_idx == 1) {
			ret = MtCmdGetTestEngine(pAd, param[0], param[1], &rsp_data);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
					"id:%d value:%x\n", param[0], rsp_data);
		} else if (param_idx == 2) {
			ret = MtCmdSetTestEngine(pAd, param[0], param[1]);
		}
	} else
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR, " incorrect format(%s)\n", arg);

	os_zero_mem(buf, BUF_LEN);
	len = snprintf(buf, BUF_LEN, "%08x", rsp_data);

	MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO, "ret:%d, len:%d, buf:%s\n", ret, len, buf);
	if (len > 0) {
		wrq->u.data.length = len;
		if (copy_to_user(wrq->u.data.pointer, buf, wrq->u.data.length)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"copy_to_user() fail\n");
			ret = -EFAULT;
		}
	}

err_out:
	return TRUE;
}



static struct {
	RTMP_STRING *name;
	INT (*ATEngineCmd_proc)(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *pIoctlCmdStr);
} *PRTMP_PRIVATE_ATECMD_PROC, RTMP_PRIVATE_ATECMD_SUPPORT_PROC[] = {
	{"set",		ate_cmd_proc},
	{"get",		ate_cmd_proc},
	{NULL}
};

INT RTMPAPIoctlAutoTestEngineCmd(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *pIoctlCmdStr)
{
	RTMP_STRING *this_char, *value;
	INT Status = NDIS_STATUS_SUCCESS;
	UCHAR *tmp = NULL, *buf = NULL;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATTEST_ATE, DBG_LVL_INFO, "----------------->\n");

	if (pIoctlCmdStr->u.data.length < 1023)
		os_alloc_mem(NULL, (UCHAR **)&buf, pIoctlCmdStr->u.data.length + 1);

	if (!buf)
		return -ENOMEM;

	os_zero_mem(buf, pIoctlCmdStr->u.data.length + 1);

	if (copy_from_user(buf, pIoctlCmdStr->u.data.pointer, pIoctlCmdStr->u.data.length)) {
		os_free_mem(buf);
		return -EFAULT;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATTEST_ATE, DBG_LVL_INFO, "len:%d, str:%s\n", pIoctlCmdStr->u.data.length, buf);

	/* Play safe - take care of a situation in which user-space didn't NULL terminate */
	buf[pIoctlCmdStr->u.data.length] = 0;
	/* Use tmp to parse string, because strsep() would change it */
	tmp = buf;

	while ((this_char = strsep((char **)&tmp, "\0")) != NULL) {
		if (!*this_char)
			continue;

		value = strchr(this_char, '=');
		if (value != NULL)
			*value++ = 0;

		for (PRTMP_PRIVATE_ATECMD_PROC = RTMP_PRIVATE_ATECMD_SUPPORT_PROC;
			PRTMP_PRIVATE_ATECMD_PROC->name; PRTMP_PRIVATE_ATECMD_PROC++) {
			if (rtstrcasecmp(this_char, PRTMP_PRIVATE_ATECMD_PROC->name) == TRUE) {
				if (!PRTMP_PRIVATE_ATECMD_PROC->ATEngineCmd_proc(pAd, value, pIoctlCmdStr)) {
					/*FALSE:Set private failed then return Invalid argument */
					Status = -EINVAL;
				}
				break;  /*Exit for loop. */
			}
		}

		if (PRTMP_PRIVATE_ATECMD_PROC->name == NULL) {
			/*Not found argument */
			Status = -EINVAL;
			/* run default format parser "ate [id]-<data>" */
			MTWF_DBG(pAd, DBG_CAT_CFG, CATTEST_ATE, DBG_LVL_ERROR,
				"IOCTL::(iwpriv) Command not Support [%s=%s]\n", this_char, value);

			break;
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATTEST_ATE, DBG_LVL_INFO, "<-----------------\n");
	os_free_mem(buf);

	return Status;
}

#endif /* CONFIG_WLAN_SERVICE */
