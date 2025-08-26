/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	operation_jedi.c
*/
#include "rt_config.h"
#include "operation.h"

#ifndef CONFIG_LOGAN_ARHT_SOC_NOT_USE
#define PLE_TOP_HOST_REPORT2_ADDR 0x820c0098
#define PLE_TOP_PLE_FUNC_CTRL_2_ADDR 0x820c0074
#endif

#define tm_Set(_id, _data) net_ad_set(winfos, _id, _data)
#define tm_Get(_id, _data, _rsp) net_ad_get(winfos, _id, _data, _rsp)

/*****************************************************************************
 *	Global Variable
 *****************************************************************************/
static u_int32 tm_ch_num_to_freq(u_int32 ch_num)
{
	u_int32 ch_in_mhz;

	if (ch_num >= 1 && ch_num <= 13)
		ch_in_mhz = 2412 + (ch_num - 1) * 5;
	else if (ch_num == 14)
		ch_in_mhz = 2484;
	else if (ch_num == 133)
		ch_in_mhz = 3665;	/* 802.11y */
	else if (ch_num == 137)
		ch_in_mhz = 3685;	/* 802.11y */
	else if ((ch_num >= 34 && ch_num <= 181)
		 || (ch_num == 16))
		ch_in_mhz = 5000 + ch_num * 5;
	else if (ch_num >= 182 && ch_num <= 196)
		ch_in_mhz = 4000 + ch_num * 5;
	else if (ch_num == 201)
		ch_in_mhz = 2730;
	else if (ch_num == 202)
		ch_in_mhz = 2498;
	else
		ch_in_mhz = 0;

	return 1000 * ch_in_mhz;
}

static u_int32 tm_bw_hqa_mapping_at(u_int32 bw)
{
	u_int32 bw_mapping = 0;

	/* BW Mapping in QA Tool
	 * 0: BW20
	 * 1: BW40
	 * 2: BW80
	 * 3: BW10
	 * 4: BW5
	 * 5: BW160C
	 * 6: BW160NC
	 * 12?: BW320
	 */
	/* BW Mapping in FW hal_cal_flow_rom.h
	 * 0: CDBW_20
	 * 1: CDBW_40
	 * 2: CDBW_80
	 * 3: CDBW_160
	 * 4: CDBW_320
	 * 5: CDBW_5
	 * 6: CDBW_10
	 * 7: CDBW_80P80
	 */

	u_int32 mapping_table[13] = {
		0, 1, 2, 6, 5, 3, 7, 0,
		0, 0, 0, 0, 4
	};

	if (bw < 13)
		bw_mapping = mapping_table[bw];


	return bw_mapping;
}


s_int32 mt_op_set_tr_mac(
	struct test_wlan_info *winfos,
	s_int32 op_type, boolean enable, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
#ifdef CONFIG_HW_HAL_OFFLOAD
#ifndef WIFI_UNIFIED_COMMAND
	struct _EXT_CMD_ATE_TEST_MODE_T param;
#else
	struct _UNI_CMD_TEST_TR_PARAM_CTRL_T param;
	uint64_t u8TestTrParamFeature;
	uint8_t ucParamNum = 1;
#endif /*WIFI_UNIFIED_COMMAND*/
#endif /*CONFIG_HW_HAL_OFFLOAD*/

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	sys_ad_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = 1;
#ifndef WIFI_UNIFIED_COMMAND
	param.ucAteIdx = EXT_ATE_SET_TRX;
	param.aucReserved[1] = INIT_CMD_SET_AND_WAIT_RETRY_RSP;
#else
	param.ucAction = ENUM_TR_PARAM_SET;
	u8TestTrParamFeature = UNI_ATE_TRX_FEATURE;
#endif
	param.Data.rAteSetTrx.ucType = op_type;
	param.Data.rAteSetTrx.ucEnable = enable;
	param.Data.rAteSetTrx.ucBand = band_idx;
	/*
	 * Make sure FW command configuration completed
	 * for store tx packet in PLE first
	 * Use aucReserved[1] for ucATEIdx extension feasibility
	 */

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"op_type=%d, enable=%u, band_idx=%u\n",
		op_type, enable, band_idx);
#ifndef WIFI_UNIFIED_COMMAND
	ret = MtCmdATETest(ad, &param);
#else
	ret = UniCmdATETest(ad, &param, u8TestTrParamFeature, ucParamNum, NULL);
#endif /*WIFI_UNIFIED_COMMAND*/

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_tx_stream(
	struct test_wlan_info *winfos,
	u_int32 stream_nums, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
#ifdef CONFIG_HW_HAL_OFFLOAD
#ifndef WIFI_UNIFIED_COMMAND
	struct _EXT_CMD_ATE_TEST_MODE_T param;
#else
	struct _UNI_CMD_TEST_TR_PARAM_CTRL_T param;
	uint64_t u8TestTrParamFeature;
	uint8_t ucParamNum = 1;
#endif /*WIFI_UNIFIED_COMMAND*/
#endif /*CONFIG_HW_HAL_OFFLOAD*/

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	sys_ad_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = 1;

#ifndef WIFI_UNIFIED_COMMAND
	param.ucAteIdx = EXT_ATE_SET_TX_STREAM;
#else
	param.ucAction = ENUM_TR_PARAM_SET;
	u8TestTrParamFeature = UNI_ATE_TX_STREAM_FEATURE;
#endif /*WIFI_UNIFIED_COMMAND*/

	param.Data.rAteSetTxStream.ucStreamNum = stream_nums;
	param.Data.rAteSetTxStream.ucBand = band_idx;
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"stream_nums=%u, band_idx=%u\n",
		stream_nums, band_idx);

#ifndef WIFI_UNIFIED_COMMAND
	ret =  MtCmdATETest(ad, &param);
#else
	ret = UniCmdATETest(ad, &param, u8TestTrParamFeature, ucParamNum, NULL);
#endif /*WIFI_UNIFIED_COMMAND*/

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_tx_path(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	tm_Set(RF_AT_FUNCID_SET_TX_PATH,
			(u_int32)configs->tx_ant);

	return ret;
}

s_int32 mt_op_set_rx_path(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 rx_ant = (u_int32)configs->rx_ant;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	rx_ant = ((rx_ant << 16) | (0 & BITS(0, 15)));
	tm_Set(RF_AT_FUNCID_SET_RX_PATH, rx_ant);

	return ret;
}

s_int32 mt_op_set_rx_filter(
	struct test_wlan_info *winfos,
	struct rx_filter_ctrl rx_filter)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
#ifdef CONFIG_HW_HAL_OFFLOAD
	MT_RX_FILTER_CTRL_T filter;
#ifndef WIFI_UNIFIED_COMMAND
	struct _EXT_CMD_ATE_TEST_MODE_T param;
#else
	struct _UNI_CMD_TEST_TR_PARAM_CTRL_T param;
	uint64_t u8TestTrParamFeature;
	uint8_t ucParamNum = 1;
#endif /*WIFI_UNIFIED_COMMAND*/
#endif /*CONFIG_HW_HAL_OFFLOAD*/

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	sys_ad_zero_mem(&filter, sizeof(MT_RX_FILTER_CTRL_T));
	sys_ad_move_mem(&filter, &rx_filter, sizeof(MT_RX_FILTER_CTRL_T));
	sys_ad_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = 1;

#ifndef WIFI_UNIFIED_COMMAND
	param.ucAteIdx = EXT_ATE_SET_RX_FILTER;
#else
	param.ucAction = ENUM_TR_PARAM_SET;
	u8TestTrParamFeature = UNI_ATE_RX_FILTER_FEATURE;
#endif
	param.Data.rAteSetRxFilter.ucBand = filter.u1BandIdx;

	if (filter.bPromiscuous)
		param.Data.rAteSetRxFilter.ucPromiscuousMode = 1;
	else {
		param.Data.rAteSetRxFilter.ucReportEn =
					(u_char)filter.bFrameReport;
		param.Data.rAteSetRxFilter.u4FilterMask =
					cpu2le32(filter.filterMask);
	}
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"band_idx=%u\n", filter.u1BandIdx);

#ifndef WIFI_UNIFIED_COMMAND
	ret =  MtCmdATETest(ad, &param);
#else
	ret = UniCmdATETest(ad, &param, u8TestTrParamFeature, ucParamNum, NULL);
#endif /*WIFI_UNIFIED_COMMAND*/

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_clean_persta_txq(
	struct test_wlan_info *winfos,
	boolean sta_pause_enable,
	void *virtual_wtbl,
	u_char omac_idx,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#ifdef CONFIG_HW_HAL_OFFLOAD
	RTMP_ADAPTER *ad = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;

#ifndef WIFI_UNIFIED_COMMAND
	struct _EXT_CMD_ATE_TEST_MODE_T param;
#else
	struct _UNI_CMD_TEST_TR_PARAM_CTRL_T param;
	uint64_t u8TestTrParamFeature;
	uint8_t ucParamNum = 1;
#endif /*WIFI_UNIFIED_COMMAND*/

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	entry = (struct _MAC_TABLE_ENTRY *)virtual_wtbl;
	sys_ad_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = 1;
#ifndef WIFI_UNIFIED_COMMAND
	param.ucAteIdx = EXT_ATE_SET_CLEAN_PERSTA_TXQUEUE;
#else
	param.ucAction = ENUM_TR_PARAM_SET;
	u8TestTrParamFeature = UNI_ATE_CLEAN_PERSTA_TXQUEUE_FEATURE;
#endif /*WIFI_UNIFIED_COMMAND*/

	param.Data.rAteSetCleanPerStaTxQueue.fgStaPauseEnable =
							sta_pause_enable;
	/* Give a same STA ID */
	param.Data.rAteSetCleanPerStaTxQueue.ucStaID = entry->wcid;
	param.Data.rAteSetCleanPerStaTxQueue.ucBand = band_idx;
	/* use omac index*/
	param.Data.rAteSetCleanPerStaTxQueue.aucReserved[0] = omac_idx;
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"wcid[%d], pause=%u, band_idx=%u, reserved[0]=%u\n",
		entry->wcid, sta_pause_enable, band_idx,
		param.Data.rAteSetCleanPerStaTxQueue.aucReserved[0]);

#ifndef WIFI_UNIFIED_COMMAND
	ret =  MtCmdATETest(ad, &param);
#else
	ret = UniCmdATETest(ad, &param, u8TestTrParamFeature, ucParamNum, NULL);
#endif /*WIFI_UNIFIED_COMMAND*/
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
#else
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"function does't support yet.\n");
#endif

	return ret;
}

s_int32 mt_op_set_cfg_on_off(
	struct test_wlan_info *winfos,
	u_int8 type, u_int8 enable, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	/* type pass through to FW */
	if (enable)
		ret = tm_Set(RF_AT_FUNCID_SET_CFG_ON, type);
	else
		ret = tm_Set(RF_AT_FUNCID_SET_CFG_OFF, type);

	return ret;
}

s_int32 mt_op_log_on_off(
	struct test_wlan_info *winfos,
	struct test_log_dump_cb *log_cb,
	u_int32 log_type,
	u_int32 log_ctrl,
	u_int32 log_size)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 data = 0;
	u_int32 rxv;
	s_int32 i, target_len = 0, max_dump_rxv_cnt = 500;
	RTMP_ADAPTER *pAd = NULL;
	struct _ATE_CTRL *ATECtrl = NULL;
	struct _ATE_LOG_DUMP_CB *log_cbb = NULL;

	log_size = 6000;
	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(pAd, winfos->net_dev);
	if (pAd == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ATECtrl = &pAd->ATECtrl;
	log_cbb = &ATECtrl->log_dump[ATE_LOG_RDD - 1];

	if (log_cb == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	switch (log_ctrl) {
	case TEST_LOG_ON:
		/* init log */
		log_cb->len = log_size;
		log_cb->first_en = TRUE;

		if (!log_cb->entry) {
			sys_ad_zero_mem(log_cb, sizeof(*log_cb));
			ret = sys_ad_alloc_mem(
				(PUCHAR *)&log_cb->entry,
				log_size * sizeof(struct test_log_dump_entry));

			if (ret) {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
					"allocated memory fail! size %u\n",
					log_size);
				return SERV_STATUS_HAL_OP_FAIL;
			}

			sys_ad_zero_mem(log_cb->entry,
				log_size * sizeof(struct test_log_dump_entry));

			SERV_OS_ALLOCATE_SPIN_LOCK(&log_cb->lock);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"init log cb size %u\n",
				log_size);
		}

		if (!log_cbb->entry) {
			sys_ad_zero_mem(log_cbb, sizeof(*log_cbb));
			ret = sys_ad_alloc_mem(
				(PUCHAR *)&log_cbb->entry,
				log_size * sizeof(struct _ATE_LOG_DUMP_ENTRY));

			if (ret) {
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
						"allocated memory fail! size %u\n",
						log_size);
					return SERV_STATUS_HAL_OP_FAIL;
				}

			sys_ad_zero_mem(log_cbb->entry,
					log_size * sizeof(struct _ATE_LOG_DUMP_ENTRY));

			log_cbb->first_en = TRUE;
			log_cbb->len = log_size;

			SERV_OS_ALLOCATE_SPIN_LOCK(&log_cbb->lock);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
					"init rdd log cb size %u\n",
					log_cbb->len);
		}

		break;

	case TEST_LOG_OFF:
		break;

	case TEST_LOG_DUMP:
		if (log_type == TEST_LOG_RXV) {
			ret = tm_Get(RF_AT_FUNCID_RESULT_INFO
				, RF_AT_FUNCID_RXV_DUMP, &data);
			if (ret == SERV_STATUS_SUCCESS) {
				target_len = data * 36;
				if (target_len >= (max_dump_rxv_cnt * 36))
					target_len = (max_dump_rxv_cnt * 36);
			}

			for (i = 0; i < target_len; i += 4)	{
				ret = tm_Get(RF_AT_FUNCID_RXV_DUMP,
					i, &data);

				if (ret == SERV_STATUS_SUCCESS) {
					rxv = data;
					if (i % 36 == 0) {
						/* Todo */
						/* TOOL_PRINTLOG... */
					}

					if (((i % 36) / 4) + 1 == 9) {
						/* Todo */
						/* TOOL_PRINTLOG... */
					}
				}

			}

			/* TOOL_PRINTLOG(RFTEST, ERROR, "[LOG DUMP END]\n"); */
		}

	default:
		goto err0;
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"log_type:0x%x, log_ctrl:0x%x, log_size:0x%x\n",
			 log_type, log_ctrl, log_size);

	return ret;

err0:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"log type %d not supported\n",
		log_type);
	return SERV_STATUS_HAL_OP_FAIL;
}


s_int32 mt_op_set_antenna_port(
	struct test_wlan_info *winfos,
	u_int8 rf_mode_mask, u_int8 rf_port_mask, u_int8 ant_port_mask)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#ifdef CONFIG_HW_HAL_OFFLOAD
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"rf_mode_mask=%u, rf_port_mask=%u, ant_port_mask=%u\n",
		rf_mode_mask, rf_port_mask, ant_port_mask);

	ret = MtCmdSetAntennaPort(
		ad, rf_mode_mask, rf_port_mask, ant_port_mask);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
#endif

	return ret;
}

s_int32 mt_op_set_slot_time(
	struct test_wlan_info *winfos,
	u_int8 slot_time, u_int8 sifs_time, u_int8 rifs_time,
	u_int16 eifs_time, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdATESetSlotTime(ad, slot_time, sifs_time,
				  rifs_time, eifs_time, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_power_drop_level(
	struct test_wlan_info *winfos,
	u_int8 pwr_drop_level, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#ifdef CONFIG_HW_HAL_OFFLOAD
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"pwr_drop_level=%u, band_idx=%u\n",
		pwr_drop_level, band_idx);

	ret = MtCmdATESetPowerDropLevel(ad, pwr_drop_level, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
#endif /* CONFIG_HW_HAL_OFFLOAD */

	return ret;
}

s_int32 mt_op_set_rx_filter_pkt_len(
	struct test_wlan_info *winfos,
	u_int8 enable, u_char band_idx, u_int32 rx_pkt_len)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 func_data;

	func_data = rx_pkt_len & BITS(0, 23);
	func_data |= (u_int32)(band_idx << 24);

	if (enable == 1)
		func_data |= BIT(30);
	else
		func_data &= ~BIT(30);

	ret = tm_Set(RF_AT_FUNCID_RX_FILTER_PKT_LEN, func_data);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_get_antswap_capability(
	struct test_wlan_info *winfos,
	u_int32 *antswap_support)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_set_antswap(
	struct test_wlan_info *winfos,
	u_int32 ant)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_set_eeprom_to_fw(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	MtCmdEfusBufferModeSet(ad, EEPROM_FLASH);

	return ret;
}

s_int32 mt_op_set_freq_offset(
	struct test_wlan_info *winfos,
	u_int32 freq_offset, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);
	ret = tm_Set(RF_AT_FUNCID_SET_FREQ_OFFSET, freq_offset);

	return ret;
}

s_int32 mt_op_set_freq_offset_c2(
	struct test_wlan_info *winfos,
	u_int32 freq_offset, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);
	ret = tm_Set(RF_AT_FUNCID_SET_FREQ_OFFSET_C2_SET, freq_offset);

	return ret;

}

s_int32 mt_op_set_phy_counter(
	struct test_wlan_info *winfos,
	s_int32 control, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdSetPhyCounter(ad, control, (u_int8) band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_rxv_index(
	struct test_wlan_info *winfos,
	u_int8 group_1, u_int8 group_2, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdSetRxvIndex(ad, group_1, group_2, (u_int8) band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_fagc_path(
	struct test_wlan_info *winfos,
	u_int8 path, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdSetFAGCPath(ad, path, (u_int8) band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_fw_mode(
	struct test_wlan_info *winfos, u_char fw_mode)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdATEModeCtrl(ad, fw_mode);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_rf_test_mode(
	struct test_wlan_info *winfos,
	u_int32 op_mode, u_int8 icap_len, u_int16 rsp_len)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "op_mode[%d]\n", op_mode);

	ret = MtCmdRfTestSwitchMode(ad, op_mode, icap_len, rsp_len);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_test_mode_start(
	struct test_wlan_info *winfos,
	struct test_backup_params *bak,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config = configs;
	u_char band_idx = 0, band_num = 0;
	u_int8 max_path;
	struct serv_chip_cap *cap = &winfos->chip_cap;
	struct serv_mcs_nss_caps *mcs_nss = &cap->mcs_nss;
	struct _net_testmode_chip_info testmode_chip_info;
	RTMP_ADAPTER *ad = NULL;
	struct mtk_mac_dev *mac_dev;
	struct mtk_mac_hw *hw;
	struct mtk_mac_cap_info *mac_cap_info;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;
	mac_dev = hc_get_mac_dev(ad);
	hw = &mac_dev->hw;
	mac_cap_info = &hw->cap_info;

#ifndef CONFIG_LOGAN_ARHT_SOC_NOT_USE
	MAC_IO_WRITE32(ad->hdev_ctrl, PLE_TOP_HOST_REPORT2_ADDR, 0x50707);
	MAC_IO_WRITE32(ad->hdev_ctrl, PLE_TOP_PLE_FUNC_CTRL_2_ADDR, 0x4);
#endif
	ad->CommonCfg.SKU_DUP_Patch_enable = FALSE;
	band_idx = hc_get_hw_band_idx(ad);

	ret = net_ad_get_chip_info(winfos, &testmode_chip_info);
	if (ret)
		return ret;

	band_num = testmode_chip_info.u1chip_band_num;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_num = %d, band_idx = %d\n", band_num, band_idx);
		test_config = configs;

	/*** Step1: Sanity check ***/
	/* Make sure mt_engine_init_band_info successfully when interface up */
	if (test_config->op_mode & OP_MODE_START) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_WARN,
			"test mode has already started, ");
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_WARN,
			"wdev_idx:%u, band_idx:%u\n",
			test_config->wdev_idx, band_idx);

		goto done;
	}

	/*** Step2: Remind firmware that enable test mode ***/
	ret = mt_op_set_fw_mode(winfos, 1);
	if (ret)
		goto err;

	max_path = max(GET_MAX_PATH(mcs_nss, band_idx, 0),
					GET_MAX_PATH(mcs_nss, band_idx, 1));
	test_config->tx_ant = BITS(0, (max_path-1));
	test_config->rx_ant = BITS(0, (max_path-1));

	/*** Step7: Stop normal function ***/
	ret = net_ad_exit_normal(winfos, bak);
	if (ret)
		goto err;

	test_config = configs;
	sys_ad_zero_mem(&test_config->stack,
			sizeof(test_config->stack));

	/*** Step8: Set op mode and init wdev/txpwr ***/
	/* call wdev_do_open must be behind ATE status set */
	/* call wdev_do_open must be after mt_adaption_stop_ap */
	test_config->op_mode = OP_MODE_START;
#if defined(NONEED)
	ret = net_ad_init_wdev(winfos, test_config, band_idx);
	if (ret)
		goto err;
#endif
	/* Init tx power feature */
	ret = net_ad_init_txpwr(winfos, test_config, band_idx);
	if (ret)
		goto err;

	if (winfos->dbdc_mode && !(winfos->chip_cap.is_dual_phy))
		test_config->stack.entry_limit = MAX_MULTI_TX_STA/2;
	else
		test_config->stack.entry_limit = MAX_MULTI_TX_STA;

	test_config->is_alloc_skb = 0;

	sys_ad_zero_mem(test_config->tx_method, sizeof(u_int8)*TEST_MODE_NUM);
	test_config->tx_method[TEST_MODE_HE_MU] = 1;
	test_config->tx_method[TEST_MODE_VHT_MIMO] = 1;
	test_config->tx_method[TEST_MODE_EHT_MU_DL_OFDMA] = 1;
	test_config->max_pkt_ext = 2;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
	"init test config, wdev_idx=%u,index = %d\n",
	test_config->wdev_idx, test_config->stack.index);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"enter test mode, wdev_idx=%u,index = %d\n",
		test_config->wdev_idx, configs->stack.index);

	/* notify FW go into rf test mode */
	mt_op_calibration_test_mode(winfos, fTEST_OPER_RFTEST_MODE);
	test_config->op_mode |= fTEST_IN_RFTEST;

#ifndef CONFIG_LOGAN_ARHT_SOC_NOT_USE
#ifdef WIFI_UNIFIED_COMMAND
	ret = uni_cmd_rro_rx_path(ad, mac_cap_info->rx_path_type);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
	"ret=0x%x, rx_path_type = %d\n", ret, mac_cap_info->rx_path_type);

	if (ret)
		goto err;
#endif /* WIFI_UNIFIED_COMMAND */
#endif

done:
	return ret;

err:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"enter test_mode_start fail, err=0x%8x\n", ret);

	return ret;
}

s_int32 mt_op_set_test_mode_abort(
	struct test_wlan_info *winfos,
	struct test_backup_params *bak,
	struct test_configuration *configs,
	struct test_operation *ops)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config = NULL;
	u_char band_idx;
	u_int16 rsp_len = 8;
	u_int8 icap_len = 0;
	struct _net_testmode_chip_info testmode_chip_info;
	RTMP_ADAPTER *ad = NULL;
	struct mtk_mac_dev *mac_dev;
	struct mtk_mac_hw *hw;
	struct mtk_mac_cap_info *mac_cap_info;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;
	mac_dev = hc_get_mac_dev(ad);
	hw = &mac_dev->hw;
	mac_cap_info = &hw->cap_info;

#ifndef CONFIG_LOGAN_ARHT_SOC_NOT_USE
	MAC_IO_WRITE32(ad->hdev_ctrl, PLE_TOP_HOST_REPORT2_ADDR, 0x707);
	MAC_IO_WRITE32(ad->hdev_ctrl, PLE_TOP_PLE_FUNC_CTRL_2_ADDR, 0x8004);
#endif
	band_idx = hc_get_hw_band_idx(ad);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "band_idx:%d\n",
		band_idx);

	ret = ops->op_get_chip_info(winfos, &testmode_chip_info);
	test_config = configs;

	if (ret)
		return ret;
	if (test_config == NULL)
		return SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	/* Rollback channel */
	if (band_idx == TEST_BAND0)
		test_config->channel = 1;
	else if (band_idx == TEST_BAND1)
		test_config->channel = 36;
	else if (band_idx == TEST_BAND2)
		test_config->channel = 36; //TODO : Bellwether band2 support

	/*** Step1: Sanity check ***/
	if ((test_config->op_mode & OP_MODE_STOP)
		|| !(test_config->op_mode & OP_MODE_START)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_WARN,
			"test mode has already stopped, ");
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_WARN,
			"wdev_idx=%u, band_idx=%u\n",
			test_config->wdev_idx, band_idx);

		test_config->op_mode = OP_MODE_STOP;

		goto done;
	}

	/*** Step3: Remind FW that switch mode ***/
	/*
	 * TODO: factor out here for rf test mode
	 * duplicated behavior reviewing
	 */
	if (test_config->op_mode & OP_MODE_FFT) {

		ret = ops->op_set_rf_test_mode(winfos,
					fTEST_OPER_NORMAL_MODE,
					icap_len,
					rsp_len);

		/* For FW to switch back to normal mode stable time */
		/* mdelay(2000); */
		if (ret)
			goto err;

		test_config->op_mode &= ~OP_MODE_FFT;
	}

	if (test_config->op_mode & fTEST_IN_RFTEST) {

		ret = ops->op_set_rf_test_mode(winfos,
					fTEST_OPER_NORMAL_MODE,
					icap_len,
					rsp_len);

		/* For FW to switch back to normal mode stable time */
		/* mdelay(2000); */
		if (ret)
			goto err;

		test_config->op_mode &= ~fTEST_IN_RFTEST;
	}

#if defined(NONEED)
	/*** Step6: Release wdev and set op mode ***/
	/* call wdev_do_close must be before mt_adaption_startup_ap */
	ret = net_ad_release_wdev(winfos, test_config, band_idx);
	if (ret)
		goto err;
#endif

	test_config->op_mode = OP_MODE_STOP;

	/*** Step7: Remind FW that disable test mode ***/
	ret = ops->op_set_fw_mode(winfos, 0);
	if (ret)
		goto err;

#if defined(NONEED)
	/*** Step8: Enable tmac/rmac/rxv ***/
	ret = ops->op_set_tr_mac(
		winfos, SERV_TEST_MAC_TXRX, TRUE, band_idx);
	if (ret)
		goto err;
#endif

	/*** Step9: resume normal function ***/
	ret = net_ad_enter_normal(winfos, bak);
	if (ret)
		goto err;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"leave test mode, wdev_idx=%u\n",
		test_config->wdev_idx);

#ifndef CONFIG_LOGAN_ARHT_SOC_NOT_USE
#ifdef WIFI_UNIFIED_COMMAND
	ret = uni_cmd_rro_rx_path(ad, mac_cap_info->rx_path_type);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
	"ret=0x%x, rx_path_type = %d\n", ret, mac_cap_info->rx_path_type);

	if (ret)
		goto err;
#endif /* WIFI_UNIFIED_COMMAND */
#endif

done:
	return ret;

err:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"leave test mode fail, err=0x%8x\n", ret);

	test_config->op_mode = OP_MODE_STOP;

	return ret;
}

s_int32 mt_op_backup_and_set_cr(
	struct test_wlan_info *winfos,
	struct test_bk_cr *bks,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = mt_test_mac_backup_and_set_cr(winfos, bks, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SET_MAC;

	return ret;
}

s_int32 mt_op_restore_cr(
	struct test_wlan_info *winfos,
	struct test_bk_cr *bks,
	u_char band_idx,
	u_char option)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = mt_test_mac_restore_cr(winfos, bks, band_idx, option);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SET_MAC;

	return ret;
}

s_int32 mt_op_set_ampdu_ba_limit(
	struct test_wlan_info *winfos,
	u_int8 wmm_idx,
	u_int8 agg_limit,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = mt_test_mac_set_ampdu_ba_limit(winfos, wmm_idx, agg_limit, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SET_MAC;

	return ret;
}

s_int32 mt_op_set_sta_pause_cr(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = mt_test_mac_set_sta_pause_cr(winfos);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SET_MAC;

	return ret;
}

s_int32 mt_op_set_ifs_cr(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = mt_test_mac_set_ifs_cr(winfos, configs, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SET_MAC;

	return ret;
}

s_int32 mt_op_write_mac_bbp_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_read_bulk_mac_bbp_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_read_bulk_rf_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_write_bulk_rf_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_read_bulk_eeprom(
	struct test_wlan_info *winfos,
	struct test_eeprom *eprms)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_start_tx(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char tx_pwr = 0;
	u_int32 aifs = configs->ipg_param.ipg;
	u_int32 pkt_cnt = configs->tx_stat.tx_cnt;
	struct test_ru_info *ru_sta = &configs->ru_info_list[0];
	u_int32 ant_idx = configs->pwr_param.ant_idx;

	if (ant_idx >= TEST_ANT_NUM) {
		ret = SERV_STATUS_ENGINE_INVALID_PARAM;
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
			"ant_idx:%u\n", ant_idx);
		goto err;
	}

	tx_pwr = configs->tx_pwr[ant_idx];

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	tm_Set(RF_AT_FUNCID_PKTCNT, pkt_cnt);

	/* QA tool pass through to FW */
	tm_Set(RF_AT_FUNCID_PREAMBLE, configs->tx_mode);

	if (configs->tx_mode == TEST_MODE_HE_TB ||
		configs->tx_mode == TEST_MODE_EHT_TB_UL_OFDMA) {
		/*do ru operation*/
		if (ru_sta->valid) {
			configs->dmnt_ru_idx = 0;

			/*Replace the mcs/nss/ldpc/mpdu_len setting*/
			configs->mcs = ru_sta->rate;
			configs->nss = ru_sta->nss;
			configs->ldpc = ru_sta->ldpc;
		}
	}
	tm_Set(RF_AT_FUNCID_SET_MAX_PE, configs->max_pkt_ext);

	tm_Set(RF_AT_FUNCID_RATE, configs->mcs);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"tx_pwr:%u, ant_idx:%u\n", tx_pwr, ant_idx);

	if (tx_pwr == 127) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"don't config tx_pwr:%u, ant_idx:%u\n",
			tx_pwr, ant_idx);
	} else
		tm_Set(RF_AT_FUNCID_POWER, tx_pwr);

	tm_Set(RF_AT_FUNCID_SET_TX_PATH, (u_int32)configs->tx_ant);
	tm_Set(RF_AT_FUNCID_STBC, (u_int32)configs->stbc);
	tm_Set(RF_AT_FUNCID_SET_ENCODE_MODE, (u_int32)configs->ldpc);
	tm_Set(RF_AT_FUNCID_SET_IBF_ENABLE, (u_int32)configs->ibf);
	tm_Set(RF_AT_FUNCID_SET_EBF_ENABLE, (u_int32)configs->ebf);
	tm_Set(RF_AT_FUNCID_PKTINTERVAL, aifs);
	tm_Set(RF_AT_FUNCID_GI, configs->sgi);
	tm_Set(RF_AT_FUNCID_SET_NSS, configs->nss);
	tm_Set(RF_AT_FUNCID_SET_HWTX_MODE, winfos->hw_tx_enable);
	tm_Set(RF_AT_FUNCID_SET_PUNCTURE, configs->preampunc);
	tm_Set(RF_AT_FUNCID_SET_AID_OFFSET, configs->wlan_id_offset);
	tm_Set(RF_AT_FUNCID_SET_CBW,
		tm_bw_hqa_mapping_at((u_int32)configs->bw));
	tm_Set(RF_AT_FUNCID_SET_DBW,
		tm_bw_hqa_mapping_at((u_int32)configs->per_pkt_bw));
	configs->op_mode |= OP_MODE_TXFRAME;
	tm_Set(RF_AT_FUNCID_COMMAND, RF_AT_COMMAND_STARTTX);

err:

	return ret;
}

s_int32 mt_op_stop_tx(
	struct test_wlan_info *winfos,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);
	tm_Set(RF_AT_FUNCID_COMMAND, RF_AT_COMMAND_STOPTEST);

	return ret;
}

s_int32 mt_op_start_rx(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 func_data;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	if (configs->tx_mode == TEST_MODE_HE_MU ||
		(configs->tx_mode == TEST_MODE_EHT_MU_DL_SU) ||
		(configs->tx_mode == TEST_MODE_EHT_MU_UL_SU) ||
		configs->tx_mode == TEST_MODE_EHT_MU_DL_OFDMA) {
		if (configs->mu_rx_aid < 0x10000)
			ret = tm_Set(RF_AT_FUNCID_SET_RX_MU_AID,
					configs->mu_rx_aid);
	}

	tm_Set(RF_AT_FUNCID_PREAMBLE, configs->tx_mode);
	tm_Set(RF_AT_FUNCID_GI, configs->sgi);

	tm_Set(RF_AT_FUNCID_SET_MAX_PE, configs->max_pkt_ext);

	sys_ad_move_mem(&func_data, configs->own_mac, 4);
	tm_Set(RF_AT_FUNCID_SET_TA, func_data);

	func_data = 0;
	sys_ad_move_mem(&func_data, configs->own_mac + 4, 2);
	tm_Set((RF_AT_FUNCID_SET_TA | BIT(18)), func_data);

	configs->op_mode |= OP_MODE_RXFRAME;

	tm_Set(RF_AT_FUNCID_COMMAND, RF_AT_COMMAND_STARTRX);

	return ret;
}

s_int32 mt_op_stop_rx(
	struct test_wlan_info *winfos,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);
	tm_Set(RF_AT_FUNCID_COMMAND, RF_AT_COMMAND_STOPTEST);

	return ret;
}

s_int32 mt_op_set_channel(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char central_ch0 = configs->channel;
	u_char central_ch1 = configs->channel_2nd;
	u_char sys_bw = configs->bw;
	u_char per_pkt_bw = configs->per_pkt_bw;
	u_char ch_band = configs->ch_band;
	u_char pri_sel = configs->pri_sel;
	u_int32 indn_spec = configs->indication_specific;
	s_int32 SetFreq = 0;
	RTMP_ADAPTER *ad = NULL;
	MT_SWITCH_CHANNEL_CFG SwChCfg;
	struct _RTMP_CHIP_OP *chip_ops = NULL;

	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	chip_ops = hc_get_chip_ops(ad->hdev_ctrl);

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	memset(&SwChCfg, 0, sizeof(MT_SWITCH_CHANNEL_CFG));

	if ((central_ch0 >= 7 && central_ch0 <= 16) && ch_band == 1) {
		/*Ch7 - Ch12, 5G (5035-5060)  ch_band: 0:2.4G    1:5G */
		SetFreq = 1000 * (5000 + central_ch0 * 5);
	} else if (central_ch0 == 6 && ch_band == 1) {
		SetFreq = 1000 * 5032;
	} else if ((central_ch0 >= 1 && central_ch0 <= 233) && ch_band == 2) {
		/*ch_band: 2: 6G */
		if (central_ch0 == 2)
			SetFreq = 5935000;
		else
			SetFreq = 1000 * (5950 + central_ch0 * 5);
	} else {
		SetFreq = tm_ch_num_to_freq((u_int32)central_ch0);
	}

	if (sys_bw == 6) {
		SetFreq = tm_ch_num_to_freq((u_int32)central_ch1);
		tm_Set((RF_AT_FUNCID_CHNL_FREQ | BIT(16)), SetFreq);
	}

	tm_Set(RF_AT_FUNCID_SET_CBW,
			tm_bw_hqa_mapping_at((u_int32)sys_bw));

	/* For POR Cal Setting - 20160601 */
	if ((sys_bw == 6) && (per_pkt_bw == 6))
		tm_Set(RF_AT_FUNCID_SET_DBW, tm_bw_hqa_mapping_at(5));
	else {
		tm_Set(RF_AT_FUNCID_SET_DBW,
			tm_bw_hqa_mapping_at((u_int32)per_pkt_bw));
	}
	SwChCfg.CentralChannel = central_ch0;
	SwChCfg.Channel_Band = band_idx;
	if (per_pkt_bw == TEST_BW_160C)
		SwChCfg.Bw = BW_160;
	else if (per_pkt_bw == TEST_BW_320)
		SwChCfg.Bw = BW_320;
	else
		SwChCfg.Bw = tm_bw_hqa_mapping_at((u_int32)per_pkt_bw);

	tm_Set(RF_AT_FUNCID_SET_PRIMARY_CH, pri_sel);
	tm_Set(RF_AT_FUNCID_SET_BAND, ch_band);

	if (indn_spec & INDN_SPEC_POWER_CAL) {
		/* run power cal only, skip performance cal */
		tm_Set(RF_AT_FUNCID_SET_RECAL_CAL_STEP, INDN_SPEC_POWER_CAL);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"indn_spec power cal [0x%08x]\n", indn_spec);
	} else if (indn_spec & INDN_SPEC_RX_CAL_BYPASS_TXITEM) {
		/* by pass rx cal */
		tm_Set(RF_AT_FUNCID_SET_RECAL_CAL_STEP, INDN_SPEC_RX_CAL_BYPASS_TXITEM);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"indn_spec rx cal bypass tx item [0x%08x]\n", indn_spec);
	} else if (indn_spec & INDN_SPEC_FULL_CAL) {
		/* if not any indn set, do full cal */
		tm_Set(RF_AT_FUNCID_SET_RECAL_CAL_STEP, INDN_SPEC_FULL_CAL);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"indn_spec full cal [0x%08x]\n", indn_spec);
	}

	if (chip_ops->ChipCheckPrek)
		chip_ops->ChipCheckPrek(ad, SwChCfg);

	/* trigger switch channel calibration */
	tm_Set(RF_AT_FUNCID_CHNL_FREQ, SetFreq);
#ifdef SINGLE_SKU_V2
	MtPwrLimitTblChProc(ad, band_idx, ch_band,
		configs->ctrl_ch, central_ch0);
#endif

	return ret;
}

s_int32 mt_op_set_tx_content(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	struct serv_hdr_802_11 *phdr = NULL;
	u_int32 func_data;
	u_int16 fc_16;
	u_int32 fc, dur, seq;
	u_int32 gen_payload_rule = configs->fixed_payload;
	u_int32 pay_load = configs->payload[0];
	u_int32 tx_len = configs->tx_len, tx_time = configs->tx_time_param.pkt_tx_time;
	s_int32 ret = SERV_STATUS_SUCCESS;
	boolean enable = FALSE;
	u_int32 sta_cnt = 0, cnt1 = 0;
	struct test_ru_info *ru_info = configs->ru_info_list;

	phdr = (struct serv_hdr_802_11 *)&configs->template_frame[0];
	sys_ad_move_mem(&fc_16, &phdr->fc, sizeof(fc_16));
	fc = (u_int32)fc_16;
	dur = (u_int32)phdr->duration;
	seq = (u_int32)phdr->sequence;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);
	tm_Set(RF_AT_FUNCID_SET_MAC_HEADER, (fc | (dur << 16)));
	tm_Set(RF_AT_FUNCID_SET_SEQ_CTRL, seq);
	tm_Set(RF_AT_FUNCID_SET_PAYLOAD, ((gen_payload_rule << 16) | pay_load));

	enable = configs->tx_time_param.pkt_tx_time_en;
	if (enable) {
		tm_Set(RF_AT_FUNCID_PKTLEN, 0);
		tm_Set(RF_AT_FUNCID_SET_TX_TIME, tx_time);
	} else {
		tm_Set(RF_AT_FUNCID_PKTLEN, tx_len);
		tm_Set(RF_AT_FUNCID_SET_TX_TIME, 0);
	}

	sys_ad_move_mem(&func_data, configs->addr2[0], 4);
	tm_Set(RF_AT_FUNCID_SET_TA, func_data);
	func_data = 0;
	sys_ad_move_mem(&func_data, configs->addr2[0] + 4, 2);
	tm_Set((RF_AT_FUNCID_SET_TA | BIT(18)), func_data);

	sys_ad_move_mem(&func_data, configs->addr3[0], 4);
	tm_Set(RF_AT_FUNCID_SET_BSSID, func_data);
	func_data = 0;
	sys_ad_move_mem(&func_data, configs->addr3[0] + 4, 2);
	tm_Set((RF_AT_FUNCID_SET_BSSID | BIT(18)), func_data);

	sys_ad_move_mem(&func_data, configs->addr1[0], 4);
	tm_Set(RF_AT_FUNCID_SET_MAC_ADDRESS, func_data);
	func_data = 0;
	sys_ad_move_mem(&func_data, configs->addr1[0] + 4, 2);
	tm_Set((RF_AT_FUNCID_SET_MAC_ADDRESS | BIT(18)), func_data);

	for (sta_cnt = 0; sta_cnt < MAX_MULTI_TX_STA && ru_info[sta_cnt].valid; sta_cnt++)
		;

	for (cnt1 = 0; cnt1 < sta_cnt; cnt1++) {
		ret = tm_Set(RF_AT_FUNCID_SET_CURR_STA_INDEX, cnt1);
		if (ret)
			return ret;

		/* RU should have a Unicast address */
		if (!isMcastEtherAddr(configs->addr1[cnt1])) {
			func_data = 0;
			sys_ad_move_mem(&func_data, configs->addr1[cnt1], 4);
			tm_Set(RF_AT_FUNCID_SET_STA_A1, func_data);
			func_data = 0;
			sys_ad_move_mem(&func_data, configs->addr1[cnt1] + 4, 2);
			tm_Set(RF_AT_FUNCID_SET_STA_A1 | BIT(18), func_data);
		}
		/* Set NULL address. Let FW generate random address */
		else {
			func_data = 0;
			tm_Set(RF_AT_FUNCID_SET_STA_A1, func_data);
			tm_Set(RF_AT_FUNCID_SET_STA_A1 | BIT(18), func_data);
		}
	}

	return ret;
}

s_int32 mt_op_set_preamble(
	struct test_wlan_info *winfos,
	u_char mode)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	/* QA tool pass through to FW
	typedef enum
	{
		PREAMBLE_CCK = 0,
		PREAMBLE_OFDM,
		PREAMBLE_MIX_MODE,
		PREAMBLE_GREEN_FIELD,
		PREAMBLE_VHT,
		PREAMBLE_PLR_MODE,
		PREAMBLE_HE_SU = 8,
		PREAMBLE_HE_ER,
		PREAMBLE_HE_TRIG,
		PREAMBLE_HE_MU,
		PREAMBLE_EHT_MU_DL_SU = 13,
		PREAMBLE_EHT_MU_UL_SU,
		PREAMBLE_EHT_MU_DL_OFDMA,
		PREAMBLE_EHT_TB_UL_OFDMA
	} E_PREAMBLE_TYPE, *P_E_PREAMBLE_TYPE;
	*/

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"tx mode = %d\n",
				mode);

	tm_Set(RF_AT_FUNCID_PREAMBLE, mode);

	return ret;
}

s_int32 mt_op_set_rate(
	struct test_wlan_info *winfos,
	u_char mcs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"mcs = %d\n",
				mcs);

	ret = tm_Set(RF_AT_FUNCID_RATE, mcs);

	return ret;
}


s_int32 mt_op_set_system_bw(
	struct test_wlan_info *winfos,
	u_char sys_bw)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	tm_Set(RF_AT_FUNCID_SET_CBW,
		tm_bw_hqa_mapping_at((u_int32)sys_bw));

	return ret;
}

s_int32 mt_op_set_per_pkt_bw(
	struct test_wlan_info *winfos,
	u_char per_pkt_bw)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	tm_Set(RF_AT_FUNCID_SET_DBW,
			tm_bw_hqa_mapping_at((u_int32)per_pkt_bw));

	return ret;
}

s_int32 mt_op_reset_txrx_counter(
	struct test_wlan_info *winfos, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct _TEST_RX_STAT_ALL_INFO rx_all;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);
	tm_Set(RF_AT_FUNCID_RESETTXRXCOUNTER, 0);
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = chip_get_rx_stat_all(ad, band_idx, 0, &rx_all);

	return ret;
}

s_int32 mt_op_set_rx_vector_idx(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 group1,
	u_int32 group2)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 func_data = 0;

	func_data = group1;
	func_data |= (group2 << 8);
	func_data |= (u_int32)(band_idx << 16);

	ret = tm_Set(RF_AT_FUNCID_SET_RXV_INDEX, func_data);

	return ret;
}

s_int32 mt_op_set_fagc_rssi_path(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 fagc_path)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 func_data = 0;

	func_data = (u_int32)(band_idx << 16) | (fagc_path & BITS(0, 15));

	ret = tm_Set(RF_AT_FUNCID_FAGC_RSSI_PATH, func_data);

	return ret;
}

s_int32 mt_op_get_rx_stat_leg(
	struct test_wlan_info *winfos,
	struct test_rx_stat_leg *rx_stat)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL, *mac_ad = NULL;
	TESTMODE_STATISTIC_INFO st;
	RX_STATISTIC_RXV *rx_stat_rxv;
	u_char band_idx, band_num, user_idx;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	memset(&st, 0, sizeof(st));

	if (IS_TEST_DBDC(winfos)) { //TODO : Bellwether band2 support
		band_num = 2;
		mac_ad = physical_device_get_mac_adapter_by_band(ad->physical_dev, 0);
		if (mac_ad) {
			rx_stat_rxv = &mac_ad->rx_stat_rxv;
			rx_stat->fagc_ib_rssi[0] = rx_stat_rxv->FAGC_RSSI_IB[0];
			rx_stat->fagc_ib_rssi[1] = rx_stat_rxv->FAGC_RSSI_IB[1];
			rx_stat->fagc_wb_rssi[0] = rx_stat_rxv->FAGC_RSSI_WB[0];
			rx_stat->fagc_wb_rssi[1] = rx_stat_rxv->FAGC_RSSI_WB[1];
			rx_stat->rcpi0 = rx_stat_rxv->RCPI[0];
			rx_stat->rcpi1 = rx_stat_rxv->RCPI[1];
			rx_stat->rssi0 = rx_stat_rxv->RSSI[0];
			rx_stat->rssi1 = rx_stat_rxv->RSSI[1];
		}

		mac_ad = physical_device_get_mac_adapter_by_band(ad->physical_dev, 1);
		if (mac_ad) {
			rx_stat_rxv = &mac_ad->rx_stat_rxv;
			rx_stat->fagc_ib_rssi[2] = rx_stat_rxv->FAGC_RSSI_IB[0];
			rx_stat->fagc_ib_rssi[3] = rx_stat_rxv->FAGC_RSSI_IB[1];
			rx_stat->fagc_wb_rssi[2] = rx_stat_rxv->FAGC_RSSI_WB[0];
			rx_stat->fagc_wb_rssi[3] = rx_stat_rxv->FAGC_RSSI_WB[1];
			rx_stat->rcpi2 = rx_stat_rxv->RCPI[0];
			rx_stat->rcpi3 = rx_stat_rxv->RCPI[1];
			rx_stat->rssi2 = rx_stat_rxv->RSSI[0];
			rx_stat->rssi3 = rx_stat_rxv->RSSI[1];
		}
	} else {
		band_num = 1;
		rx_stat_rxv = &ad->rx_stat_rxv;
		rx_stat->fagc_ib_rssi[0] = rx_stat_rxv->FAGC_RSSI_IB[0];
		rx_stat->fagc_ib_rssi[1] = rx_stat_rxv->FAGC_RSSI_IB[1];
		rx_stat->fagc_ib_rssi[2] = rx_stat_rxv->FAGC_RSSI_IB[2];
		rx_stat->fagc_ib_rssi[3] = rx_stat_rxv->FAGC_RSSI_IB[3];
		rx_stat->fagc_wb_rssi[0] = rx_stat_rxv->FAGC_RSSI_WB[0];
		rx_stat->fagc_wb_rssi[1] = rx_stat_rxv->FAGC_RSSI_WB[1];
		rx_stat->fagc_wb_rssi[2] = rx_stat_rxv->FAGC_RSSI_WB[2];
		rx_stat->fagc_wb_rssi[3] = rx_stat_rxv->FAGC_RSSI_WB[3];
		rx_stat->rcpi0 = rx_stat_rxv->RCPI[0];
		rx_stat->rcpi1 = rx_stat_rxv->RCPI[1];
		rx_stat->rcpi2 = rx_stat_rxv->RCPI[2];
		rx_stat->rcpi3 = rx_stat_rxv->RCPI[3];
		rx_stat->rssi0 = rx_stat_rxv->RSSI[0];
		rx_stat->rssi1 = rx_stat_rxv->RSSI[1];
		rx_stat->rssi2 = rx_stat_rxv->RSSI[2];
		rx_stat->rssi3 = rx_stat_rxv->RSSI[3];
	}

	for (band_idx = 0; band_idx < band_num; band_idx++) {
		/* read statistic from firmware */
		chip_get_rx_stat(ad, band_idx, &st);

		/* Copy statistic info */
		switch (band_idx) {
		case 0:
			/* MAC COUNT */
			rx_stat->mac_rx_fcs_ok_cnt =
				st.mac_rx_fcs_ok_cnt;
			rx_stat->mac_rx_fcs_err_cnt =
				st.mac_rx_fcs_err_cnt;
			rx_stat->mac_rx_len_mismatch =
				st.mac_rx_len_mismatch;
			rx_stat->rx_fifo_full =
				st.mac_rx_fifo_full;
			rx_stat->mac_rx_mdrdy_cnt =
				st.mac_rx_mdrdy_cnt;

			/* PHY COUNT */
			rx_stat->phy_rx_pd_cck =
				st.phy_rx_pd_cck;
			rx_stat->phy_rx_pd_ofdm =
				st.phy_rx_pd_ofdm;
			rx_stat->phy_rx_sig_err_cck =
				st.phy_rx_sig_err_cck;
			rx_stat->phy_rx_sfd_err_cck =
				st.phy_rx_sfd_err_cck;
			rx_stat->phy_rx_sig_err_ofdm =
				st.phy_rx_sig_err_ofdm;
			rx_stat->phy_rx_tag_err_ofdm =
				st.phy_rx_tag_err_ofdm;
			rx_stat->phy_rx_mdrdy_cnt_cck =
				st.phy_rx_mdrdy_cnt_cck;
			rx_stat->phy_rx_mdrdy_cnt_ofdm =
				st.phy_rx_mdrdy_cnt_ofdm;
			rx_stat->phy_rx_fcs_err_cnt_cck =
				st.phy_rx_fcs_err_cnt_cck;
			rx_stat->phy_rx_fcs_err_cnt_ofdm =
				st.phy_rx_fcs_err_cnt_ofdm;
			break;

		case 1:
			/* MAC COUNT */
			rx_stat->mac_rx_fcs_err_cnt_band1 =
				st.mac_rx_fcs_err_cnt;
			rx_stat->mac_rx_len_mismatch_band1 =
				st.mac_rx_len_mismatch;
			rx_stat->rx_fifo_full_band1 =
				st.mac_rx_fifo_full;
			rx_stat->mac_rx_mdrdy_cnt_band1 =
				st.mac_rx_mdrdy_cnt;

			/* PHY COUNT */
			rx_stat->phy_rx_pd_cck_band1 =
				st.phy_rx_pd_cck;
			rx_stat->phy_rx_pd_ofdm_band1 =
				st.phy_rx_pd_ofdm;
			rx_stat->phy_rx_sig_err_cck_band1 =
				st.phy_rx_sig_err_cck;
			rx_stat->phy_rx_sfd_err_cck_band1 =
				st.phy_rx_sfd_err_cck;
			rx_stat->phy_rx_sig_err_ofdm_band1 =
				st.phy_rx_sig_err_ofdm;
			rx_stat->phy_rx_tag_err_ofdm_band1 =
				st.phy_rx_tag_err_ofdm;
			rx_stat->phy_rx_mdrdy_cnt_cck_band1 =
				st.phy_rx_mdrdy_cnt_cck;
			rx_stat->phy_rx_mdrdy_cnt_ofdm_band1 =
				st.phy_rx_mdrdy_cnt_ofdm;
			break;

		default:
			ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
			break;
		}
	}

	rx_stat->freq_offset_rx =
		rx_stat_rxv->FreqOffsetFromRx[0];

	for (user_idx = 0; user_idx < TEST_USER_NUM; user_idx++) {
		rx_stat->user_rx_freq_offset[user_idx] =
			rx_stat_rxv->FreqOffsetFromRx[user_idx];
		rx_stat->user_snr[user_idx] =
			(u_int32)rx_stat_rxv->SNR[user_idx];
		rx_stat->fcs_error_cnt[user_idx] =
			rx_stat_rxv->fcs_error_cnt[user_idx];
	}

	return ret;
}

s_int32 mt_op_get_rxv_dump_action(
	struct test_wlan_info *winfos,
	u_int32 action,
	u_int32 type_mask)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	if (action == TEST_RXV_DUMP_STOP)
		chip_rxv_dump_stop(ad);
	else if (action == TEST_RXV_DUMP_CLEAR_BUFFER)
		chip_rxv_dump_buf_clear(ad);
	else {
		chip_rxv_dump_buf_alloc(ad, type_mask);
		chip_rxv_dump_start(ad);
	}

	return ret;
}

s_int32 mt_op_get_rxv_dump_ring_attr(
	struct test_wlan_info *winfos,
	struct rxv_dump_ring_attr *attr)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RXV_DUMP_CTRL *ctrl = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ctrl = &ad->rxv_dump_ctrl;
	attr->type_mask = ctrl->type_mask;
	attr->valid_entry_num = ctrl->valid_entry_num;
	attr->ring_idx = ctrl->ring_idx;
	attr->dump_entry_total_num = ctrl->dump_entry_total_num;

	return ret;
}

s_int32 mt_op_get_rxv_content_len(
	struct test_wlan_info *winfos,
	u_int8 type_mask,
	u_int8 rxv_sta_cnt,
	u_int16 *len)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	chip_rxv_content_len(ad,
		type_mask, rxv_sta_cnt, len);

	return ret;
}

s_int32 mt_op_get_rxv_dump_rxv_content(
	struct test_wlan_info *winfos,
	u_int8 entry_idx,
	u_int32 *content_len,
	void *rxv_content)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	chip_rxv_dump_rxv_content_compose(ad,
		entry_idx, rxv_content, content_len);

	return ret;
}

s_int32 mt_op_dbdc_tx_tone(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 func_data;

	/*
	 * Select TX Antenna
	 * RF_Power: (1db) 0~15
	 * Digi_Power: (0.25db) -32~31
	 */
	func_data = (u_int32)configs->ant_idx << 16 | configs->rf_pwr;
	tm_Set(RF_AT_FUNCID_SET_TONE_RF_GAIN, func_data);
	func_data = (u_int32)configs->ant_idx << 16 | configs->digi_pwr;
	tm_Set(RF_AT_FUNCID_SET_TONE_DIGITAL_GAIN, func_data);

	/* DBDC Band Index : Band0, Band1 */
	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	if (configs->tx_tone_en) {
		/* Band : 2.4G/5G */
		tm_Set(RF_AT_FUNCID_SET_BAND,
			(u_int32)configs->ch_band);

		/* ToneType : Single or Two */
		tm_Set(RF_AT_FUNCID_SET_TONE_TYPE,
			(u_int32)configs->tone_type);

		/* ToneFreq: DC/5M/10M/20M/40M */
		tm_Set(RF_AT_FUNCID_SET_TONE_BW,
			configs->tone_freq);

		/* DC Offset I, DC Offset Q */
		func_data = (configs->dc_offset_Q << 16) |
			configs->dc_offset_I;
		tm_Set(RF_AT_FUNCID_SET_TONE_DC_OFFSET,
			func_data);

		/* Control TX Tone Start and Stop */
		tm_Set(RF_AT_FUNCID_COMMAND,
			RF_AT_COMMAND_SINGLE_TONE);
	} else {
		/* Control TX Tone Start and Stop */
		tm_Set(RF_AT_FUNCID_COMMAND, RF_AT_COMMAND_STOPTEST);
	}

	return ret;
}

s_int32 mt_op_dbdc_tx_tone_pwr(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_int8 ant_idx = 0;
	u_int32 digi_pwr = 0;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ant_idx = (u_int8) configs->ant_idx;
	digi_pwr = configs->digi_pwr;

	ret = MtCmdTxTonePower(ad, 0x12, digi_pwr, ant_idx, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_dbdc_continuous_tx(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 tx_mode = configs->tx_mode;
	u_int32 rate = configs->rate;

	if (configs->tx_tone_en) {
		tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX,
			(u_int32)band_idx);

		tm_Set(RF_AT_FUNCID_SET_PRIMARY_CH,
			configs->pri_sel);

		/* QA tool pass through to FW */
		ret = tm_Set(RF_AT_FUNCID_PREAMBLE, tx_mode);

		tm_Set(RF_AT_FUNCID_RATE, rate);

		ret = tm_Set(RF_AT_FUNCID_SET_CBW,
			tm_bw_hqa_mapping_at((u_int32)configs->bw));

		ret = tm_Set(RF_AT_FUNCID_SET_CW_MODE,
			configs->tx_fd_mode);

		if (ret != SERV_STATUS_SUCCESS)
			return SERV_STATUS_HAL_OP_FAIL;

		ret = tm_Set(RF_AT_FUNCID_SET_ANTMASK,
			(u_int32)configs->ant_mask);

		if (ret != SERV_STATUS_SUCCESS)
			return SERV_STATUS_HAL_OP_FAIL;

		ret = tm_Set(RF_AT_FUNCID_COMMAND, RF_AT_COMMAND_CW);
		if (ret != SERV_STATUS_SUCCESS)
			return SERV_STATUS_HAL_OP_FAIL;

	} else {
		ret = tm_Set(RF_AT_FUNCID_COMMAND,
			RF_AT_COMMAND_STOPTEST);

		if (ret != SERV_STATUS_SUCCESS)
			return SERV_STATUS_HAL_OP_FAIL;
	}
	return ret;
}

s_int32 mt_op_get_tx_info(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	struct test_configuration *test_configs_band,
	struct test_configuration *test_configs_band1)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 rsp = 0;

	ret = tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	ret = tm_Get(RF_AT_FUNCID_TXED_COUNT, 0, &rsp);
	if (ret == SERV_STATUS_SUCCESS)
		test_configs_band->tx_stat.tx_done_cnt = rsp;

	return ret;
}

s_int32 mt_op_get_rx_statistics_all(
	struct test_wlan_info *winfos,
	struct hqa_comm_rx_stat *hqa_rx_stat)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_calibration_test_mode(
	struct test_wlan_info *winfos,
	u_char mode)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_int8 icap_len = 0;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	if (mode == fTEST_OPER_RFTEST_MODE || mode == fTEST_OPER_ICAP_MODE)
		ret = MtCmdRfTestSwitchMode(ad, mode,
		icap_len, RF_TEST_DEFAULT_RESP_LEN);

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_icap_start(
	struct test_wlan_info *winfos,
	u_int8 *data)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;
#ifdef INTERNAL_CAPTURE_SUPPORT
	RBIST_CAP_START_T *prICapInfo = (RBIST_CAP_START_T *)data;
#endif/* INTERNAL_CAPTURE_SUPPORT */

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

#ifdef INTERNAL_CAPTURE_SUPPORT
	if (ops->ICapStart != NULL)
		ret = ops->ICapStart(ad, (u_int8 *)prICapInfo);
	else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"The function is not hooked !!\n");
	}
#endif/* INTERNAL_CAPTURE_SUPPORT */

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_get_icap_status(
	struct test_wlan_info *winfos,
	s_int32 *icap_stat)
{
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

#ifdef INTERNAL_CAPTURE_SUPPORT
	if (ops->ICapStatus != NULL)
		*icap_stat = ops->ICapStatus(ad);
	else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"The function is not hooked !!\n");
		return  SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
	}
#endif/* INTERNAL_CAPTURE_SUPPORT */

	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_get_icap_max_data_len(
	struct test_wlan_info *winfos,
	u_long *max_data_len)
{
	*max_data_len = (ICAP_EVENT_DATA_SAMPLE * sizeof(INT32));
	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_get_icap_data(
	struct test_wlan_info *winfos,
	s_int32 *icap_cnt,
	s_int32 *icap_data,
	u_int32 wf_num,
	u_int32 iq_type)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

#ifdef INTERNAL_CAPTURE_SUPPORT
	if (ops->ICapGetIQData != NULL)
		ret = ops->ICapGetIQData(ad
				, icap_data, icap_cnt, iq_type, wf_num);
	else if (ops->ICapCmdSolicitRawDataProc != NULL)
		ret = ops->ICapCmdSolicitRawDataProc(ad
				, icap_data, icap_cnt, iq_type, wf_num);
	else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"The function is not hooked !!\n");
	}
#endif/* INTERNAL_CAPTURE_SUPPORT */

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_do_cal_item(
	struct test_wlan_info *winfos,
	u_int32 item,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	tm_Set(RF_AT_FUNCID_SET_RECAL_CAL_STEP, item);

	return ret;
}

s_int32 mt_op_set_band_mode(
	struct test_wlan_info *winfos,
	struct test_band_state *band_state)
{
	s_int32 ret = SERV_STATUS_ENGINE_NOT_SUPPORTED;

	return ret;
}

s_int32 mt_op_get_chipid(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct _net_testmode_chip_info testmode_chip_info;

	ret = net_ad_get_chip_info(winfos, &testmode_chip_info);
	if (ret)
		return ret;

	if (testmode_chip_info.u4ChipId != 0xFFFF)
		winfos->chip_id = testmode_chip_info.u4ChipId;

	return ret;
}

s_int32 mt_op_get_sub_chipid(
	struct test_wlan_info *winfos,
	UINT32 *sub_chipid)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	if (FALSE == chip_get_sub_chipid(ad, sub_chipid))
		ret = SERV_STATUS_HAL_OP_FAIL;

	if (ret)
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"fail(0x%08x).\n", ret);

	return ret;
}

s_int32 mt_op_mps_start(
	struct test_wlan_info *winfos,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	tm_Set(RF_AT_FUNCID_COMMAND, RF_AT_COMMAND_STARTTX);

	return ret;
}

s_int32 mt_op_mps_set_nss(
	struct test_wlan_info *winfos,
	u_int32 len,
	struct test_mps_setting *mps_setting)
{
	u_int32 i;
	s_int32 ret = SERV_STATUS_SUCCESS;

	for (i = 0; i < len; i++) {
		tm_Set((RF_AT_FUNCID_SET_MPS_NSS | (i << 16)),
			mps_setting[i+1].nss);
	}

	return ret;
}

s_int32 mt_op_mps_set_per_packet_bw(
	struct test_wlan_info *winfos,
	u_int32 len,
	struct test_mps_setting *mps_setting)
{
	u_int32 i;
	s_int32 ret = SERV_STATUS_SUCCESS;

	for (i = 0; i < len; i++) {
		tm_Set((RF_AT_FUNCID_SET_MPS_PACKAGE_BW | (i << 16)),
			mps_setting[i+1].pkt_bw);
	}

	return ret;
}

s_int32 mt_op_mps_set_packet_count(
	struct test_wlan_info *winfos,
	u_int32 len,
	struct test_mps_setting *mps_setting)
{
	u_int32 i;
	s_int32 ret = SERV_STATUS_SUCCESS;

	for (i = 0; i < len; i++) {
		tm_Set((RF_AT_FUNCID_SET_MPS_PKT_CNT | (i << 16)),
			mps_setting[i+1].pkt_cnt);
	}

	return ret;
}

s_int32 mt_op_mps_set_payload_length(
	struct test_wlan_info *winfos,
	u_int32 len,
	struct test_mps_setting *mps_setting)
{
	u_int32 i;
	s_int32 ret = SERV_STATUS_SUCCESS;

	for (i = 0; i < len; i++) {
		tm_Set((RF_AT_FUNCID_SET_MPS_PAYLOAD_LEN | (i << 16)),
			mps_setting[i+1].pkt_len);
	}

	return ret;
}

s_int32 mt_op_mps_set_power_gain(
	struct test_wlan_info *winfos,
	u_int32 len,
	struct test_mps_setting *mps_setting)
{
	u_int32 i;
	s_int32 ret = SERV_STATUS_SUCCESS;

	for (i = 0; i < len; i++) {
		tm_Set((RF_AT_FUNCID_SET_MPS_PWR_GAIN | (i << 16)),
			mps_setting[i+1].pwr);
	}

	return ret;
}

s_int32 mt_op_mps_set_seq_data(
	struct test_wlan_info *winfos,
	u_int32 len,
	struct test_mps_setting *mps_setting)
{
	u_int32 i;
	u_int32 *mps_set = NULL;
	u_int32 mode, mcs, tx_path;
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = sys_ad_alloc_mem((u_char **)&mps_set, sizeof(u_int32) * len);

	for (i = 0; i < len; i++) {
		mode = mps_setting[i+1].tx_mode;
		mcs = mps_setting[i+1].mcs;
		tx_path = mps_setting[i+1].tx_ant;

		mps_set[i] = (mcs) | (tx_path << 8) | (mode << 24);

	}

	tm_Set(RF_AT_FUNCID_SET_MPS_SIZE, len);

	for (i = 0; i < len; i++) {
		tm_Set((RF_AT_FUNCID_SET_MPS_SEQ_DATA | (i << 16)),
			mps_set[i]);
	}

	sys_ad_free_mem(mps_set);

	return ret;
}

s_int32 mt_op_get_tx_pwr(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx,
	u_char channel,
	u_char ant_idx,
	u_int32 *power)
{
	s_int32 ret = SERV_STATUS_SUCCESS, rsp = 0;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	ret = tm_Get(RF_AT_FUNCID_GET_CH_TX_PWR_OFFSET, 0, &rsp);

	if (ret == SERV_STATUS_SUCCESS)	{
		*power = rsp;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"pwr:%u!\n",
			*power);
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"fail!\n");
	}

	return ret;
}

s_int32 mt_op_set_tx_pwr(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx,
	struct test_txpwr_param *pwr_param)
{
	u_int32 Pwr = 0;
	s_int32 ret = SERV_STATUS_SUCCESS;

	/* sanity check for null pointer */
	if (!pwr_param)
		return SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	Pwr = pwr_param->power;
	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"tx_pwr:%u, ant_idx:%u\n", Pwr, pwr_param->ant_idx);

	if (Pwr == 127) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"verify stage, tx_pwr:%u\n", Pwr);
		return ret;
	}

	tm_Set(RF_AT_FUNCID_POWER, Pwr);

	return ret;
}

s_int32 mt_op_set_ch_pwr_offset(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx,
	struct test_txpwr_param *pwr_param)
{
	u_int32 ch_pwr_offset = 0;
	s_int32 ret = SERV_STATUS_SUCCESS;
	union pwr_w {
		struct {
			u_int16 antidx;
			u_int16 pwr;
		} w;
	} ch_pwr;

	/* sanity check for null pointer */
	if (!pwr_param)
		return SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	sys_ad_zero_mem((void *)&ch_pwr, sizeof(ch_pwr));
	ch_pwr.w.antidx = pwr_param->ant_idx;
	ch_pwr.w.pwr = pwr_param->ch_pwr_offset;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"ch_offset:%2x, ant_idx:%u\n", ch_pwr.w.pwr, pwr_param->ant_idx);

	sys_ad_move_mem((void *)&ch_pwr_offset, (void *)&ch_pwr, sizeof(ch_pwr_offset));
	tm_Set(RF_AT_FUNCID_SET_TSSI_CH_OFFSET, ch_pwr_offset);

	return ret;
}

s_int32 mt_op_set_cvt_pwr_offset(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx,
	struct test_txpwr_param *pwr_param)
{
	u_int32 cvt_offset = 0;
	s_int32 ret = SERV_STATUS_SUCCESS;
	union pwr_w {
		struct {
			u_int16 antidx;
			u_int16 pwr;
		} w;
	} cvt_pwr;

	/* sanity check for null pointer */
	if (!pwr_param)
		return SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	sys_ad_zero_mem((void *)&cvt_pwr, sizeof(cvt_pwr));
	cvt_pwr.w.antidx = pwr_param->ant_idx;
	cvt_pwr.w.pwr = pwr_param->cvt_pwr_offset;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"cvt_offset:%2x, ant_idx:%u\n", cvt_pwr.w.pwr, pwr_param->ant_idx);

	if (cvt_offset == 127) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"verify stage, cvt_offset:%d\n", cvt_pwr.w.pwr);
		return ret;
	}

	sys_ad_move_mem((void *)&cvt_offset, (void *)&cvt_pwr, sizeof(cvt_offset));
	tm_Set(RF_AT_FUNCID_SET_CVT_OFFSET, cvt_offset);

	return ret;
}


s_int32 mt_op_get_freq_offset(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 *freq_offset)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 rsp = 0;

	ret = tm_Get(RF_AT_FUNCID_GET_FREQ_OFFSET, 0, &rsp);
	if (ret == SERV_STATUS_SUCCESS)	{
		*freq_offset = rsp;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"get freq_offset:%u\n",
			*freq_offset);
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"fail!\n");
	}

	return ret;
}

s_int32 mt_op_get_freq_offset_c2(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 *freq_offset)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 rsp = 0;

	ret = tm_Get(RF_AT_FUNCID_SET_FREQ_OFFSET_C2_GET, 0, &rsp);
	if (ret == SERV_STATUS_SUCCESS)	{
		*freq_offset = rsp;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"freq_offset:%u!\n",
			*freq_offset);
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"fail!\n");
	}

	return ret;
}

s_int32 mt_op_get_cfg_on_off(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 type,
	u_int32 *result)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 rsp = 0;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	ret = tm_Get(RF_AT_FUNCID_GET_CFG_ON_OFF, type, &rsp);
	if (ret == SERV_STATUS_SUCCESS)	{
		*result = rsp;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"type:%u, result:%u\n",type, *result);
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"fail!\n");
	}

	return ret;
}

s_int32 mt_op_get_tx_tone_pwr(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 ant_idx,
	u_int32 *power)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdRfTestGetTxTonePower(ad, power, ant_idx, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_get_recal_cnt(
	struct test_wlan_info *winfos,
	u_int32 *recal_cnt,
	u_int32 *recal_dw_num)
{
	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_get_recal_content(
	struct test_wlan_info *winfos,
	u_int32 *content)
{
	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_get_rxv_cnt(
	struct test_wlan_info *winfos,
	u_int32 *rxv_cnt,
	u_int32 *rxv_dw_num)
{
	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_get_rxv_content(
	struct test_wlan_info *winfos,
	u_int32 dw_cnt,
	u_int32 *content)
{
	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_set_cal_bypass(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 cal_item)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdDoCalibration(ad, CALIBRATION_BYPASS, cal_item, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_dpd(
	struct test_wlan_info *winfos,
	u_int32 on_off,
	u_int32 wf_sel)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtAsicSetDPD(ad, on_off, wf_sel);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_tssi(
	struct test_wlan_info *winfos,
	u_int32 on_off,
	u_int32 wf_sel)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtAsicSetTSSI(ad, on_off, wf_sel);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_get_thermal_val(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_int8  sensorType,
	u_int8  band_idx,
	u_int8 *pu4SensorResult)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

#ifdef WIFI_UNIFIED_COMMAND

	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdGetThermalTempAdcResult(ad, sensorType,
		band_idx, pu4SensorResult);
	if (ret)
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"fail!\n");
#endif
	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_set_rdd_test(
	struct test_wlan_info *winfos,
	u_int32 rdd_idx,
	u_int32 rdd_sel,
	u_int32 enable)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"rdd_idx: %d, rdd_sel: %d, enable:%d\n",
		rdd_idx, rdd_sel, enable);

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdSetRDDTestExt(ad, rdd_idx, rdd_sel, enable);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_evt_rf_test_cb(
	struct test_wlan_info *winfos,
	struct test_log_dump_cb *test_log_dump,
	u_int32 en_log,
	u_int8 *data,
	u_int32 length)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = net_ad_rf_test_cb(winfos, test_log_dump, en_log, data, length);

	return ret;
}

s_int32 mt_op_set_off_ch_scan(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx,
	struct test_off_ch_param *param)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	EXT_CMD_OFF_CH_SCAN_CTRL_T ext_cmd_param;
	u_char ch = 0;
	u_char work_tx_strm_pth = 0, work_rx_strm_pth = 0, off_ch_idx = 0;
	u_char ch_ext_index = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * pChipCap;
#endif /* WIFI_UNIFIED_COMMAND */
	u_char ch_ext_above[] = {
	36, 44, 52, 60,
	100, 108, 116, 124,
	132, 140, 149, 157, 0
	};
	u_char ch_ext_below[] = {
	40, 48, 56, 64,
	104, 112, 120, 128,
	136, 144, 153, 161, 0
	};
	u_char prim_ch[off_ch_ch_idx_num] = {0, 0};
	u_char bw[off_ch_ch_idx_num] = {0, 0};
	u_char cen_ch[off_ch_ch_idx_num] = {0, 0};

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	work_tx_strm_pth = configs->tx_ant;
	work_rx_strm_pth = configs->rx_strm_pth;
	prim_ch[off_ch_wrk_ch_idx] = configs->channel;
	bw[off_ch_wrk_ch_idx] = configs->bw;
	prim_ch[off_ch_mntr_ch_idx] = param->mntr_ch;
	bw[off_ch_mntr_ch_idx] = param->mntr_bw;

	for (off_ch_idx = 0; off_ch_idx < off_ch_ch_idx_num; off_ch_idx++) {
		ch = prim_ch[off_ch_idx];

		/* Initialize index */
		ch_ext_index = 0;

		switch (bw[off_ch_idx]) {
		case SYS_BW_20:
			break;

		case SYS_BW_40:
			while (ch_ext_index < ARRAY_SIZE(ch_ext_above) &&
					ch_ext_above[ch_ext_index] != 0) {
				if (ch == ch_ext_above[ch_ext_index])
					ch = ch + 2;
				else if (ch == ch_ext_below[ch_ext_index])
					ch = ch - 2;

				ch_ext_index++;
			}
			break;

		case SYS_BW_80:
		case TEST_BW_160NC:
			ch = vht_cent_ch_freq(ch, VHT_BW_80, CMD_CH_BAND_5G);
			break;

		case SYS_BW_160:
			ch = vht_cent_ch_freq(ch, VHT_BW_160, CMD_CH_BAND_5G);
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"off_ch_idx %d, BW is invalid %d\n",
				off_ch_idx, bw[off_ch_idx]);
			ret = NDIS_STATUS_FAILURE;
			goto err0;
		}

		cen_ch[off_ch_idx] = ch;
	}

	/* Initialize */
	sys_ad_zero_mem(&ext_cmd_param, sizeof(ext_cmd_param));

	/* Fill in ext_cmd_param */
	ext_cmd_param.mntr_prim_ch = param->mntr_ch;
	ext_cmd_param.mntr_cntrl_ch = cen_ch[off_ch_mntr_ch_idx];
	ext_cmd_param.mntr_bw = bw[off_ch_mntr_ch_idx];
	ext_cmd_param.mntr_tx_strm_pth = param->mntr_tx_rx_pth;
	ext_cmd_param.mntr_rx_strm_pth = param->mntr_tx_rx_pth;

	ext_cmd_param.work_prim_ch = prim_ch[off_ch_wrk_ch_idx];
	ext_cmd_param.work_cntrl_ch = cen_ch[off_ch_wrk_ch_idx];
	ext_cmd_param.work_bw = bw[off_ch_wrk_ch_idx];
	ext_cmd_param.work_tx_strm_pth = work_tx_strm_pth;
	ext_cmd_param.work_rx_strm_pth = work_rx_strm_pth;

	ext_cmd_param.dbdc_idx = param->dbdc_idx;
	ext_cmd_param.scan_mode = param->scan_mode;
	ext_cmd_param.is_aband = param->is_aband;
	ext_cmd_param.off_ch_scn_type = off_ch_scan_simple_rx;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"mntr_ch:%d mntr_bw:%d mntr_central_ch:%d\n",
		ext_cmd_param.mntr_prim_ch,
		ext_cmd_param.mntr_bw, ext_cmd_param.mntr_cntrl_ch);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"work_prim_ch:%d work_bw:%d work_central_ch:%d\n",
		ext_cmd_param.work_prim_ch,
		ext_cmd_param.work_bw, ext_cmd_param.work_cntrl_ch);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"scan_mode:%d dbdc_idx:%d is_aband:%d\n",
		ext_cmd_param.scan_mode,
		ext_cmd_param.dbdc_idx, ext_cmd_param.is_aband);

#ifdef WIFI_UNIFIED_COMMAND
	pChipCap = hc_get_chip_cap(ad->hdev_ctrl);

	if (pChipCap->uni_cmd_support)
		ret = UniCmdOffChScan(ad, &ext_cmd_param);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		ret = mt_cmd_off_ch_scan(ad, &ext_cmd_param);

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;

err0:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"invalid parameters\n");
	return ret;
}

s_int32 mt_op_get_rdd_cnt(
	struct test_wlan_info *winfos,
	u_int32 *rdd_cnt,
	u_int32 *rdd_dw_num)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	RTMP_ADAPTER *pAd = NULL;
	struct _ATE_CTRL *ATECtrl = NULL;
	struct _ATE_LOG_DUMP_CB *log_cb = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(pAd, winfos->net_dev);
	if (pAd == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ATECtrl = &pAd->ATECtrl;
	log_cb = &ATECtrl->log_dump[ATE_LOG_RDD - 1];

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"\n");

	/* radar pulse number */
	*rdd_cnt = log_cb->idx;
	/* RDD buffer size */
	*rdd_dw_num = log_cb->len;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"radar pulse number is %d, RDD buffer size is %d\n",
		log_cb->idx, log_cb->len);

	return ret;
}

s_int32 mt_op_get_rdd_content(
	struct test_wlan_info *winfos,
	u_int32 *content,
	u_int32 *total_cnt)
{
	static u_int32 idx;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 end = 0, remindIdx = 0;
	RTMP_ADAPTER *pAd = NULL;
	struct _ATE_CTRL *ATECtrl = NULL;
	struct _ATE_LOG_DUMP_CB *log_cb = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(pAd, winfos->net_dev);
	if (pAd == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ATECtrl = &pAd->ATECtrl;
	log_cb = &ATECtrl->log_dump[ATE_LOG_RDD - 1];

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	if (log_cb == NULL) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"log_cb == NULL\n");
		return SERV_STATUS_HAL_OP_INVALID_PAD;
	}

	if (!log_cb->entry) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"log_cb->entry == NULL\n");
		return SERV_STATUS_HAL_OP_INVALID_PAD;
	}

	if (log_cb->len == 0) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"RDD buffer size is empty\n");
		return SERV_STATUS_HAL_OP_INVALID_PAD;
	}

	/* Prepare for RDD dump */
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"[RDD DUMP START][HQA_GetDumpRDD]\n");

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"radar pulse number is %d, RDD buffer size is %d\n",
		log_cb->idx, log_cb->len);

	if (log_cb->first_en == TRUE) {
		/*
		 * Reset idx - 1. HQA RDD dump (re-)enable
		 */
		idx = 0;
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "idx: %d\n", idx);
	/* If log_cb->idx greater than log_cb->len(RDDBufferSize),
	 * it will re-count from 0
	 */
	remindIdx = (idx > log_cb->idx) ?
		((log_cb->idx + log_cb->len) - idx) :
		(log_cb->idx - idx);

	end = (remindIdx > TEST_RDD_DUMP_SIZE) ?
		((idx + TEST_RDD_DUMP_SIZE) % (log_cb->len)) :
		((idx + remindIdx) % (log_cb->len));

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"remindIdx: %d, end: %d\n", remindIdx, end);

	do {
		idx = (idx % (log_cb->len));
		if (log_cb->entry[idx].un_dumped) {
			struct _ATE_RDD_LOG *result =
				&log_cb->entry[idx].log.rdd;
			/* 1 pulse: 64 bits */
			UINT32 *pulse =
				(UINT32 *)result->aucBuffer;

			/* To sperate the interrupts of radar signals */
			if (!result->byPass) {
				*content = result->u4Prefix;
				content++;
				*content = result->u4Count;
				content++;
				*total_cnt = *total_cnt + 2;
			}

			log_cb->entry[idx].un_dumped = FALSE;

			*content = pulse[0];
			content++;
			*content = pulse[1];
			content++;
			*total_cnt = *total_cnt + 2;
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"[RDD]0x%08x %08x\n", pulse[0], pulse[1]);
		}

		INC_RING_INDEX(idx, log_cb->len);
	} while (idx != end);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"[After RDD dumping] idx: %d, end: %d, total_cnt: %d\n",
		idx, end, *total_cnt);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"idx = %d, log_cb->idx = %d\n",
		idx, log_cb->idx);

	if (idx == log_cb->idx) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"[RDD DUMP END]\n");
		idx = 0;
		log_cb->idx = 0;
	}

	SERV_OS_SEM_LOCK(&log_cb->lock);
	log_cb->is_dumping = FALSE;
	SERV_OS_SEM_UNLOCK(&log_cb->lock);

	log_cb->first_en = FALSE;

	return ret;
}

s_int32 mt_op_set_muru_manual(
	void *virtual_device,
	struct test_wlan_info *winfos,
	struct test_configuration *configs)
{
#if defined(CONFIG_AP_SUPPORT) && defined(CFG_SUPPORT_FALCON_MURU)

	RTMP_ADAPTER *ad = NULL;
	u_int8 ru_seq = 0, wmm_idx = 0;
#ifndef	WIFI_UNIFIED_COMMAND
	CMD_MURU_MANCFG_INTERFACER MuruManCfg;
#else
	struct UNI_MURU_MANUAL_CONFIG_T MuruManCfg;
#endif /* WIFI_UNIFIED_COMMAND */

	struct _MAC_TABLE_ENTRY *mac_tbl_entry = NULL;
	struct test_tx_stack *stack = &configs->stack;
	struct test_ru_info *ru_info = &configs->ru_info_list[0];
	u_int8 spe_idx = 0;
	u_int32 ant_sel = configs->tx_ant;

	if (ant_sel & TEST_ANT_USER_DEF) {
		ant_sel &= ~TEST_ANT_USER_DEF;
		spe_idx = ant_sel;
	} else {
		net_ad_get_speidx(winfos, ant_sel, &spe_idx);
	}

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	mac_tbl_entry = (struct _MAC_TABLE_ENTRY *)stack->virtual_wtbl[0];

	sys_ad_zero_mem(&MuruManCfg, sizeof(MuruManCfg));

	MuruManCfg.rCfgCmm.u1PpduFmt = MURU_PPDU_HE_MU;
	MuruManCfg.u4ManCfgBmpCmm = MURU_FIXED_CMM_PPDU_FMT;
	net_ad_get_band_idx(virtual_device, &MuruManCfg.rCfgCmm.u1Band);
	MuruManCfg.u4ManCfgBmpCmm |= MURU_FIXED_CMM_BAND;
	net_ad_get_wmm_idx(virtual_device, &wmm_idx);
#ifndef	WIFI_UNIFIED_COMMAND
	MuruManCfg.rCfgCmm.u1WmmSet = wmm_idx;
#else
	MuruManCfg.rCfgCmm.u1Ac = wmm_idx;
#endif /* WIFI_UNIFIED_COMMAND */
	//MuruManCfg.u4ManCfgBmpCmm |= MURU_FIXED_CMM_WMM_SET;
	MuruManCfg.rCfgCmm.u1SpeIdx = spe_idx;
	MuruManCfg.u4ManCfgBmpCmm |= MURU_FIXED_CMM_SPE_IDX;
	if (configs->per_pkt_bw > TEST_BW_80)
		MuruManCfg.rCfgDl.u1Bw = 0x3;	/* 0x3 imply 80+80/160 */
	else
		MuruManCfg.rCfgDl.u1Bw = configs->per_pkt_bw;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_BW;
#ifndef	WIFI_UNIFIED_COMMAND
	MuruManCfg.rCfgDl.u1SigBMcs = (configs->mcs & 0xf);
#else
	MuruManCfg.rCfgDl.u1SigMcs = (configs->mcs & 0xf);
#endif
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_SIGB_MCS;
#ifndef WIFI_UNIFIED_COMMAND
	MuruManCfg.rCfgDl.u1SigBDcm = ((configs->mcs & BIT5) ? 0x1 : 0);
#else
	MuruManCfg.rCfgDl.u1SigDcm = ((configs->mcs & BIT5) ? 0x1 : 0);
#endif
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_SIGB_DCM;
	MuruManCfg.rCfgDl.u1TxMode = configs->stack.tx_info[0].tx_mode;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_TX_MODE;
	MuruManCfg.rCfgDl.u1UserCnt = stack->index;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_TOTAL_USER_CNT;
#ifndef WIFI_UNIFIED_COMMAND
	sys_ad_move_mem(&MuruManCfg.rCfgDl.au1RU,
					&configs->ru_alloc,
					sizeof(configs->ru_alloc));
#else
	sys_ad_move_mem(&MuruManCfg.rCfgDl.au2RU,
					&configs->ru_alloc,
					sizeof(configs->ru_alloc));
#endif
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_TONE_PLAN;
	MuruManCfg.rCfgDl.u1GI = configs->stack.tx_info[0].gi;
	MuruManCfg.rCfgDl.u1Ltf = configs->stack.tx_info[0].ltf;
	MuruManCfg.u4ManCfgBmpDl |= (MURU_FIXED_GI | MURU_FIXED_LTF);
	for (ru_seq = 0 ; ru_seq < MAX_MULTI_TX_STA ; ru_seq++) {
		u_int8 seg = 0, alloc = 0;
#ifndef WIFI_UNIFIED_COMMAND
		MURU_DL_USER_INFO *user_info = NULL;
#else
		struct UNI_MURU_DL_USER_INFO *user_info = NULL;
#endif
		struct phy_params *phy_param = NULL;

		if (ru_info[ru_seq].valid) {
			user_info = &MuruManCfg.rCfgDl.arUserInfoDl[ru_seq];
			seg = (ru_info[ru_seq].ru_index & 0x1);
			alloc = (ru_info[ru_seq].ru_index >> 1);
			phy_param = &mac_tbl_entry[ru_seq].phy_param;

			user_info->u2WlanIdx = mac_tbl_entry[ru_seq].wcid;
			user_info->u1RuAllocBn = seg;
			user_info->u1RuAllocIdx = alloc;
			user_info->u1Mcs = (phy_param->rate & 0xf);
			user_info->u2TxPwrAlpha =
					(ru_info[ru_seq].alpha & 0xffff);

			if (phy_param->rate & BIT5)	/* DCM required */
				user_info->u1Mcs |= BIT4;
			user_info->u1Nss = phy_param->vht_nss;
			user_info->u1Ldpc = phy_param->ldpc;

			if ((ru_info[ru_seq].ru_index >> 1) == 18) {
				u_int8 *au1C26 = NULL;

				au1C26 = MuruManCfg.rCfgDl.au1C26;
				au1C26[(ru_info[ru_seq].ru_index & 1)] = 1;
			}
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"Add user[%d] wcid:%d, ru index:%d,\n",
				ru_seq, user_info->u2WlanIdx,
				user_info->u1RuAllocIdx);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"\t\tsegment:%d, mcs:%d\n",
				user_info->u1RuAllocBn,
				user_info->u1Mcs);
#ifdef WIFI_UNIFIED_COMMAND
			/*UNI_MURU_DL_USER_INFO*/
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"user_info WlanIdx[%x] RuAllocBn:%x, RuAllocIdx:%d, x:%x\n",
				user_info->u2WlanIdx, user_info->u1RuAllocBn,
				user_info->u1RuAllocIdx, user_info->u1RuAllocIdx);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"user_info u1Ldpc[%x] u1Nss:%x, u1Mcs:%d, u1MuGroupIdx:%x\n",
				user_info->u1Ldpc, user_info->u1Nss,
				user_info->u1Mcs, user_info->u1MuGroupIdx);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"user_info VhtGid[%x] VhtUp:%x, HeStream:%d, HeSpatial:%x\n",
				user_info->u1VhtGid, user_info->u1VhtUp,
				user_info->u1HeStartStream, user_info->u1HeMuMimoSpatial);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"user_info TxPwrAlpha[%x] AckPolicy:%x, RuAlPs160:%d, x:%x\n",
				user_info->u2TxPwrAlpha, user_info->u1AckPolicy,
				user_info->u1RuAllocPs160, user_info->u1RuAllocPs160);
#endif
		}
	}
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_USER_WLAN_ID;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_USER_COD;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_USER_MCS;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_USER_NSS;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_USER_RU_ALLOC;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_USER_PWR_ALPHA;
#ifndef WIFI_UNIFIED_COMMAND
	wifi_test_muru_set_manual_config(ad, &MuruManCfg);
#else
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"u4ManCfgBmpCmm[%x] u4ManCfgBmpDl:%x, u4ManCfgBmpUl:%x,\n",
				MuruManCfg.u4ManCfgBmpCmm, MuruManCfg.u4ManCfgBmpDl,
				MuruManCfg.u4ManCfgBmpUl);

	/*rCfgCmm -- UNI_MURU_CMM_MANUAL_CONFIG*/
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"rCfgCmm u1PdaPol[%x] u1Band:%x, u1SpeIdx:%d, u1ProcType:%x\n",
				MuruManCfg.rCfgCmm.u1PdaPol, MuruManCfg.rCfgCmm.u1Band,
				MuruManCfg.rCfgCmm.u1SpeIdx, MuruManCfg.rCfgCmm.u1ProcType);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"rCfgCmm u2MloCtrl[%x] u1SchType:%x, u1PpduFmt:%d, u1Ac:%x\n",
				MuruManCfg.rCfgCmm.u2MloCtrl, MuruManCfg.rCfgCmm.u1SchType,
				MuruManCfg.rCfgCmm.u1PpduFmt, MuruManCfg.rCfgCmm.u1Ac);

	/*rCfgDl-- UNI_MURU_DL_MANUAL_CONFIG*/
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"rCfgDl u1UserCnt:%x, u1TxMode:%x, u1Bw:%x, u1GI:%x\n",
				MuruManCfg.rCfgDl.u1UserCnt, MuruManCfg.rCfgDl.u1TxMode,
				MuruManCfg.rCfgDl.u1Bw, MuruManCfg.rCfgDl.u1GI);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"rCfgDl u1Ltf:%x, u1SigMcs:%x, u1SigDcm:%x, u1SigCmprs:%x\n",
				MuruManCfg.rCfgDl.u1Ltf, MuruManCfg.rCfgDl.u1SigMcs,
				MuruManCfg.rCfgDl.u1SigDcm, MuruManCfg.rCfgDl.u1SigCmprs);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"rCfgDl au2RU0:%x,%x,%x,%x,%x,%x,%x,%x\n",
				MuruManCfg.rCfgDl.au2RU[0], MuruManCfg.rCfgDl.au2RU[1],
				MuruManCfg.rCfgDl.au2RU[2], MuruManCfg.rCfgDl.au2RU[3],
				MuruManCfg.rCfgDl.au2RU[4], MuruManCfg.rCfgDl.au2RU[5],
				MuruManCfg.rCfgDl.au2RU[6], MuruManCfg.rCfgDl.au2RU[7]);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"rCfgDl au2RU8:%x,%x,%x,%x,%x,%x,%x,%x\n",
				MuruManCfg.rCfgDl.au2RU[8], MuruManCfg.rCfgDl.au2RU[9],
				MuruManCfg.rCfgDl.au2RU[10], MuruManCfg.rCfgDl.au2RU[11],
				MuruManCfg.rCfgDl.au2RU[12], MuruManCfg.rCfgDl.au2RU[13],
				MuruManCfg.rCfgDl.au2RU[14], MuruManCfg.rCfgDl.au2RU[15]);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"rCfgDl au1C260:%x, au1C261:%x, u1AckPly:%x, u1TxPwr:%x\n",
				MuruManCfg.rCfgDl.au1C26[0], MuruManCfg.rCfgDl.au1C26[1],
				MuruManCfg.rCfgDl.u1AckPly, MuruManCfg.rCfgDl.u1TxPwr);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"rCfgDl u2MuPpduDur:%x, u1AgcDispOrder:%x, x:%x, x:%x\n",
				MuruManCfg.rCfgDl.u2MuPpduDur, MuruManCfg.rCfgDl.u1AgcDispOrder,
				MuruManCfg.rCfgDl.u1Reserved, MuruManCfg.rCfgDl.u1Reserved);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"rCfgDl AgcDispPol:%x, ADpRatio:%x, ADpMGF:%x, PrmbPunc:%x\n",
				MuruManCfg.rCfgDl.u1AgcDispPol, MuruManCfg.rCfgDl.u1AgcDispRatio,
				MuruManCfg.rCfgDl.u2AgcDispLinkMGF, MuruManCfg.rCfgDl.u2PrmblPuncBmp);

	wifi_test_muru_set_arb_op_mode(ad, 0x2);
	UniCmdMuruParameterSet(ad, (RTMP_STRING *)&MuruManCfg, UNI_CMD_MURU_MANUAL_CONFIG);
#endif
#endif	/* CFG_SUPPORT_FALCON_MURU */

	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_set_tam_arb(
	struct test_wlan_info *winfos,
	u_int8 arb_op_mode)
{
#if defined(CONFIG_AP_SUPPORT)
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	wifi_test_muru_set_arb_op_mode(ad, arb_op_mode);
#endif /* CFG_SUPPORT_FALCON_MURU */

	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_set_mu_count(
	struct test_wlan_info *winfos,
	void *virtual_device,
	u_int32 count)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#if defined(CONFIG_AP_SUPPORT)
	RTMP_ADAPTER *ad = NULL;

#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_MURU_SET_MU_TXPKT_CNT SetMuTxPktCnt;
#else
	CMD_MURU_SET_MU_TX_PKT_CNT SetMuTxPktCnt;
#endif
	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL) {
		ret = SERV_STATUS_HAL_OP_INVALID_PAD;
		goto err_out;
	}

	ret = net_ad_get_band_idx(virtual_device, &SetMuTxPktCnt.u1BandIdx);
	if (ret)
		goto err_out;
	/* u4MuTxPktCnt set to 0 as continuously */
	if (count != 0x8fffffff)
		SetMuTxPktCnt.u4MuTxPktCnt = count;
	else
		SetMuTxPktCnt.u4MuTxPktCnt = 0;

#ifdef WIFI_UNIFIED_COMMAND
	UniCmdMuruParameterSet(ad, (RTMP_STRING *)&SetMuTxPktCnt, UNI_CMD_MURU_SET_MU_TX_PKT_CNT);
#else
	if (set_muru_mu_tx_pkt_cnt(ad, &SetMuTxPktCnt) == FALSE) {
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
		goto err_out;
	}
#endif

err_out:
#endif /* CFG_SUPPORT_FALCON_MURU */
	return ret;
}

s_int32 mt_op_trigger_mu_counting(
	struct test_wlan_info *winfos,
	void *virtual_device,
	boolean enable)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#if defined(CONFIG_AP_SUPPORT)
	RTMP_ADAPTER *ad = NULL;

#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_MURU_SET_MU_TXPKT_CNT SetMuTxPktCnt;
#else
	CMD_MURU_SET_MU_TX_PKT_CNT SetMuTxPktCnt;
#endif
	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL) {
		ret = SERV_STATUS_HAL_OP_INVALID_PAD;
		goto err_out;
	}

	sys_ad_zero_mem(&SetMuTxPktCnt, sizeof(SetMuTxPktCnt));

	ret = net_ad_get_band_idx(virtual_device, &SetMuTxPktCnt.u1BandIdx);
	if (ret)
		goto err_out;

	SetMuTxPktCnt.u1MuTxEn = enable;

#ifdef WIFI_UNIFIED_COMMAND
	UniCmdMuruParameterSet(ad, (RTMP_STRING *)&SetMuTxPktCnt, UNI_CMD_MURU_SET_MU_TX_PKT_EN);
#else
	if (set_muru_mu_tx_pkt_en(ad, &SetMuTxPktCnt) == FALSE) {
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
		goto err_out;
	}
#endif

err_out:
#endif /* CFG_SUPPORT_FALCON_MURU */
	return ret;
}

static struct _TEST_RX_STAT_ALL_INFO test_rx_stat_all[TEST_BAND_NUM];

s_int32 mt_op_get_rx_stat_band(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int8 blk_idx,
	struct test_rx_stat_band_info *rx_st_band)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	P_TEST_RX_STAT_BAND_INFO st = &test_rx_stat_all[band_idx].rx_band_info;
	struct _TEST_RX_STAT_BAND_INFO_EXT1 *p_band_info_ext = &test_rx_stat_all[band_idx].rx_band_info_ext;
	struct _TEST_RX_STAT_COMM_INFO *p_comm_info = &test_rx_stat_all[band_idx].rx_comm_info;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = chip_get_rx_stat_all(ad, band_idx, blk_idx, &test_rx_stat_all[band_idx]);

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	rx_st_band->mac_rx_fcs_err_cnt = st->mac_rx_fcs_err_cnt;
	rx_st_band->mac_rx_mdrdy_cnt = st->mac_rx_mdrdy_cnt;
	rx_st_band->mac_rx_len_mismatch = st->mac_rx_len_mismatch;
	rx_st_band->mac_rx_fcs_ok_cnt = st->mac_rx_fcs_ok_cnt;
	rx_st_band->phy_rx_fcs_err_cnt_cck = st->phy_rx_fcs_err_cnt_cck;
	rx_st_band->phy_rx_fcs_err_cnt_ofdm = st->phy_rx_fcs_err_cnt_ofdm;
	rx_st_band->phy_rx_pd_cck = st->phy_rx_pd_cck;
	rx_st_band->phy_rx_pd_ofdm = st->phy_rx_pd_ofdm;
	rx_st_band->phy_rx_sig_err_cck = st->phy_rx_sig_err_cck;
	rx_st_band->phy_rx_sfd_err_cck = st->phy_rx_sfd_err_cck;
	rx_st_band->phy_rx_sig_err_ofdm = st->phy_rx_sig_err_ofdm;
	rx_st_band->phy_rx_tag_err_ofdm = st->phy_rx_tag_err_ofdm;
	rx_st_band->phy_rx_mdrdy_cnt_cck = st->phy_rx_mdrdy_cnt_cck;
	rx_st_band->phy_rx_mdrdy_cnt_ofdm = st->phy_rx_mdrdy_cnt_ofdm;
	rx_st_band->aci_hit_Low = p_comm_info->aci_hit_low;
	rx_st_band->aci_hit_High = p_comm_info->aci_hit_high;
	rx_st_band->mac_rx_u2m_mpdu_count = p_band_info_ext->mac_rx_u2m_mpdu_cnt;

	return ret;
}

s_int32 mt_op_get_rx_stat_path(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int8 blk_idx,
	struct test_rx_stat_path_info *rx_st_path)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	P_TEST_RX_STAT_PATH_INFO st = NULL;

	if (band_idx >= TEST_BAND_NUM)
		return SERV_STATUS_HAL_MAC_INVALID_BANDIDX;

	if (blk_idx < ARRAY_SIZE(test_rx_stat_all[band_idx].rx_path_info))
		st = &test_rx_stat_all[band_idx].rx_path_info[blk_idx];
	else {
		ret = SERV_STATUS_HAL_OP_FAIL;
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"fail ret:%d\n", ret);
		return ret;
	}

	rx_st_path->rcpi = st->rcpi;
	rx_st_path->rssi = st->rssi;
	rx_st_path->fagc_ib_rssi = st->fagc_ib_rssi;
	rx_st_path->fagc_wb_rssi = st->fagc_wb_rssi;
	rx_st_path->inst_ib_rssi = st->inst_ib_rssi;
	rx_st_path->inst_wb_rssi = st->inst_wb_rssi;
	rx_st_path->fagc_rssi_adc = st->fagc_rssi_adc;

	return ret;
}

s_int32 mt_op_get_rx_stat_user(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int8 blk_idx,
	struct test_rx_stat_user_info *rx_st_user)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	P_TEST_RX_STAT_USER_INFO st = &test_rx_stat_all[band_idx].rx_user_info[blk_idx];

	rx_st_user->freq_offset_from_rx = st->freq_offset_from_rx;
	rx_st_user->snr = st->snr;
	rx_st_user->fcs_error_cnt = st->fcs_error_cnt;

	return ret;
}

s_int32 mt_op_get_rx_stat_comm(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int8 blk_idx,
	struct test_rx_stat_comm_info *rx_st_comm)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	P_TEST_RX_STAT_COMM_INFO st = &test_rx_stat_all[band_idx].rx_comm_info;

	rx_st_comm->rx_fifo_full = st->rx_fifo_full;
	rx_st_comm->aci_hit_low = st->aci_hit_low;
	rx_st_comm->aci_hit_high = st->aci_hit_high;
	rx_st_comm->mu_pkt_count = st->mu_pkt_count;
	rx_st_comm->sig_mcs = st->sig_mcs;
	rx_st_comm->sinr = st->sinr;
	rx_st_comm->driver_rx_count = st->driver_rx_count;
	rx_st_comm->ne_var_db_all_user = st->ne_var_db_all_user;

	return ret;
}

s_int32 mt_op_set_rx_user_idx(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int16 user_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = mt_cmd_set_rx_stat_user_idx(ad, band_idx, user_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_get_wf_path_comb(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	boolean dbdc_mode_en,
	u_int8 *path,
	u_int8 *path_len)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = chip_get_wf_path_comb(ad, band_idx, dbdc_mode_en, path, path_len);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_get_hetb_info(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_ru_info *ru_info)
{
	s_int32 ret = SERV_STATUS_SUCCESS, rsp = 0, id = 0;

	ret = tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);
	if (ret) {
		id = RF_AT_FUNCID_SET_DBDC_BAND_IDX;
		goto err;
	}

	ret = tm_Get(RF_AT_FUNCID_GET_TB_INFO_A_FACTOR, 0, &rsp);
	if (ret) {
		id = RF_AT_FUNCID_GET_TB_INFO_A_FACTOR;
		goto err;
	}
	ru_info->afactor_init = rsp;

	rsp = 0;
	ret = tm_Get(RF_AT_FUNCID_GET_TB_INFO_LDPC_STR_SYM, 0, &rsp);
	if (ret) {
		id = RF_AT_FUNCID_GET_TB_INFO_LDPC_STR_SYM;
		goto err;
	}
	ru_info->ldpc_extr_sym = rsp;

	rsp = 0;
	ret = tm_Get(RF_AT_FUNCID_GET_TB_INFO_PE_DISAMB, 0, &rsp);
	if (ret) {
		id = RF_AT_FUNCID_GET_TB_INFO_PE_DISAMB;
		goto err;
	}
	ru_info->pe_disamb = rsp;

	rsp = 0;
	ret = tm_Get(RF_AT_FUNCID_GET_TB_INFO_TXPE, 0, &rsp);
	if (ret) {
		id = RF_AT_FUNCID_GET_TB_INFO_TXPE;
		goto err;
	}
	ru_info->t_pe = rsp;

	rsp = 0;
	ret = tm_Get(RF_AT_FUNCID_GET_TB_INFO_L_SIG_LENGTH, 0, &rsp);
	if (ret) {
		id = RF_AT_FUNCID_GET_TB_INFO_L_SIG_LENGTH;
		goto err;
	}
	ru_info->l_len = rsp;

err:
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"%u fail ret:%d\n", id, ret);
		return ret;
	}

	return ret;
}

s_int32 mt_op_set_ru_aid(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int16 mu_rx_aid)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#ifdef CONFIG_HW_HAL_OFFLOAD
	RTMP_ADAPTER *ad = NULL;
#ifndef WIFI_UNIFIED_COMMAND
	struct _EXT_CMD_ATE_TEST_MODE_T param;
#else
	struct _UNI_CMD_TEST_TR_PARAM_CTRL_T param;
	uint64_t u8TestTrParamFeature;
	uint8_t ucParamNum = 1;
#endif /*WIFI_UNIFIED_COMMAND*/
	u_int8 testmode_en = 1;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	sys_ad_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = testmode_en;

#ifndef WIFI_UNIFIED_COMMAND
	param.ucAteIdx = ENUM_ATE_SET_MU_RX_AID;
	param.aucReserved[1] = INIT_CMD_SET_AND_WAIT_RETRY_RSP;
#else
	param.ucAction = ENUM_TR_PARAM_SET;
	u8TestTrParamFeature = UNI_ATE_MU_RX_AID_FEATURE;
#endif /*WIFI_UNIFIED_COMMAND*/

	param.Data.set_mu_rx_aid.band_idx = band_idx;
	param.Data.set_mu_rx_aid.aid = cpu2le16(mu_rx_aid);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"Set to decode MU accodring to AID:%d\n",
		param.Data.set_mu_rx_aid.aid);
#ifndef WIFI_UNIFIED_COMMAND
	ret = MtCmdATETest(ad, &param);
#else
	ret = UniCmdATETest(ad, &param, u8TestTrParamFeature, ucParamNum, NULL);
#endif /*WIFI_UNIFIED_COMMAND*/

	if (ret != 0)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
#endif /*CONFIG_HW_HAL_OFFLOAD*/

	return ret;
}

s_int32 mt_op_set_ru_info(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_ru_info *ru_info)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_int32 sta_cnt = 0, cnt1 = 0, data = 0, id = 0, func_data;
	struct service_test *serv_test;
	union ru_dw {
		struct {
			u_int8 sta_id;
			u_int8 mcs;
			u_int16 rsv:3;
			u_int16 nss:3;
			u_int16 mu_nss:3;
			u_int16 start_ss:3;
			u_int16 b0:1;
			u_int16 ps160:1;
			u_int16 isLDPC:1;
			u_int16 valid:1;
		} dw0;
		struct {
			u_int16 ru_alloc:9;
			u_int16 ru_index:7;
			u_int16 alpha;
		} dw1;
		struct {
			u_int32 length:20;
			u_int32 mu_group_id:4;
			u_int32 vht_up:4;
			u_int32 vht_gid:4;
		} dw2;
	} dw;

	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_SERV_TEST_INVALID_NULL_POINTER;

	serv_test = (struct service_test *)ad->serv.serv_handle;

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	for (sta_cnt = 0; sta_cnt < MAX_MULTI_TX_STA && ru_info[sta_cnt].valid; sta_cnt++)
		;

	ret = tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);
	if (ret) {
		id = RF_AT_FUNCID_SET_DBDC_BAND_IDX;
		goto err;
	}

	ret = tm_Set(RF_AT_FUNCID_SET_RU_INFO_NUM, sta_cnt);
	if (ret) {
		id = RF_AT_FUNCID_SET_RU_INFO_NUM;
		goto err;
	}

	for (cnt1 = 0; cnt1 < sta_cnt; cnt1++) {
		ret = tm_Set(RF_AT_FUNCID_SET_CURR_STA_INDEX, cnt1);
		if (ret) {
			id = RF_AT_FUNCID_SET_CURR_STA_INDEX;
			goto err;
		}

		/* RU should have Unicast address */
		if (!isMcastEtherAddr(serv_test->test_config.addr1[cnt1])) {
			func_data = 0;
			sys_ad_move_mem(&func_data, serv_test->test_config.addr1[cnt1], 4);
			tm_Set(RF_AT_FUNCID_SET_STA_A1, func_data);
			func_data = 0;
			sys_ad_move_mem(&func_data, serv_test->test_config.addr1[cnt1] + 4, 2);
			tm_Set(RF_AT_FUNCID_SET_STA_A1 | BIT(18), func_data);
		}
		/* Set NULL address. Let FW generate random address */
		else {
			func_data = 0;
			tm_Set(RF_AT_FUNCID_SET_STA_A1, func_data);
			tm_Set(RF_AT_FUNCID_SET_STA_A1 | BIT(18), func_data);
		}

		memset(&dw, 0, sizeof(dw));
		memset(&data, 0, sizeof(data));
		dw.dw0.sta_id = ru_info[cnt1].aid;
		dw.dw0.mcs = ru_info[cnt1].rate;
		dw.dw0.nss = ru_info[cnt1].nss;
		dw.dw0.mu_nss = ru_info[cnt1].ru_mu_nss;
		dw.dw0.start_ss = ru_info[cnt1].start_sp_st;
		dw.dw0.b0 = ru_info[cnt1].b0;
		dw.dw0.ps160 = ru_info[cnt1].ps160;
		dw.dw0.isLDPC = ru_info[cnt1].ldpc;
		dw.dw0.valid = ru_info[cnt1].valid;
		memmove(&data, &dw, sizeof(data));
		ret = tm_Set(RF_AT_FUNCID_SET_RU_INFO_0, data);
		if (ret) {
			id = RF_AT_FUNCID_SET_RU_INFO_0;
			goto err;
		}

		memset(&dw, 0, sizeof(dw));
		memset(&data, 0, sizeof(data));
		dw.dw1.ru_alloc = ru_info[cnt1].allocation;
		dw.dw1.ru_index = ru_info[cnt1].ru_index;
		memmove(&data, &dw, sizeof(data));
		ret = tm_Set(RF_AT_FUNCID_SET_RU_INFO_1, (u_int32)data);
		if (ret) {
			id = RF_AT_FUNCID_SET_RU_INFO_1;
			goto err;
		}

		memset(&dw, 0, sizeof(dw));
		memset(&data, 0, sizeof(data));
		dw.dw2.length = ru_info[cnt1].mpdu_length;

		/* add mpdu_length */
		ret = tm_Set(RF_AT_FUNCID_SET_RU_INFO_2_PRESET_LENGTH_EXT, ru_info[cnt1].mpdu_length);
		if (ret) {
			id = RF_AT_FUNCID_SET_RU_INFO_2_PRESET_LENGTH_EXT;
			goto err;
		}

		memmove(&data, &dw, sizeof(data));
		ret = tm_Set(RF_AT_FUNCID_SET_RU_INFO_2, (u_int32)data);
		if (ret) {
			id = RF_AT_FUNCID_SET_RU_INFO_2;
			goto err;
		}
	}

err:
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"%u fail sta:%u ret:%d\n", id, cnt1, ret);
	}

	return ret;
}

s_int32 mt_op_set_mutb_spe(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_char tx_mode,
	u_int8 spe_idx)
{
	RTMP_ADAPTER *ad = NULL;
	struct _RTMP_CHIP_DBG *chip_dbg = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	chip_dbg = hc_get_chip_dbg(ad->hdev_ctrl);
	if (chip_dbg->chip_ctrl_spe)
		chip_dbg->chip_ctrl_spe(ad, band_idx, tx_mode, spe_idx);

	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_set_test_mode_dnlk_clean(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	/* Clear DNLK Image */
	ad->DnlCalOfst = 0;
	sys_ad_zero_mem(ad->TxDnlCal, DNL_CAL_SIZE);

	/* Clear TSSI Image */
	ad->TssiCal2GOfst = 0;
	sys_ad_zero_mem(ad->TssiCal2G, TSSI_CAL_2G_SIZE);
	ad->TssiCal5GOfst = 0;
	sys_ad_zero_mem(ad->TssiCal5G, TSSI_CAL_5G_SIZE);

	return ret;
}

s_int32 mt_op_set_test_mode_rxgaink(
	struct test_wlan_info *winfos,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	MT_SWITCH_CHANNEL_CFG ch_cfg;
	RTMP_ADAPTER *ad = NULL;
	RTMP_ADAPTER *ad_band0 = NULL;
	struct _ATE_CTRL *ATECtrl = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;
	if (IS_MT7990(ad) || IS_MT7992(ad) || IS_MT7993(ad))
		ad_band0 = net_ad_multi_inf_get_pAd_by_Band(winfos, 0);

	if (ad_band0 == NULL) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ad_band0 is null\n");
		return SERV_STATUS_HAL_OP_INVALID_PAD;
	}

	ATECtrl = &(ad->ATECtrl);
	ad_band0->RXGainCalOfst = 0; //Reset rxgain offset for event handler to map correct eeprom
	ad->RXGainCalOfst = 0;

	/* RX GAIN Calibration */
	sys_ad_zero_mem(&ch_cfg, sizeof(ch_cfg));

	ATECtrl->op_mode |= fATE_IN_RFTEST;

	/* Get Channel Info */

	/* Get Rx Gain Calibration result */
	MtCmdDoCalibration(ad, DO_RX_GAIN_CAL, (1<<24), band_idx);

	ATECtrl->op_mode &= ~fATE_IN_RFTEST;

	return ret;
}

s_int32 mt_op_set_frtbl_info(
	struct test_wlan_info *winfos,
	struct test_tx_info tx_info,
	u_int8 spe_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#ifndef WIFI_UNIFIED_COMMAND
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_WARN,
		"Not support!\n");
#else
	RTMP_ADAPTER *ad = NULL;
	struct _UNI_CMD_TEST_TR_PARAM_CTRL_T param;
	uint64_t u8TestTrParamFeature;
	uint8_t ucParamNum = 1;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	sys_ad_zero_mem(&param, sizeof(param));

	param.ucAction = ENUM_TR_PARAM_SET;
	param.ucAteTestModeEn = 1;
	u8TestTrParamFeature = UNI_ATE_FRTBL_CFG_FEATURE;

	//Use fr table idx 17
	//TODO : need config in servie_test or agent
	param.Data.rFrTblCfg.ucTblIdx = 17;
	param.Data.rFrTblCfg.rFrTblInfo.u2FixedRate = tx_info.fixed_rate;
	param.Data.rFrTblCfg.rFrTblInfo.ucFrAntIdx = 0;
	param.Data.rFrTblCfg.rFrTblInfo.ucFrSpeIdxSel = 0;
	param.Data.rFrTblCfg.rFrTblInfo.ucFrSpeIdx = spe_idx;
	param.Data.rFrTblCfg.rFrTblInfo.fgFrGI = tx_info.gi;
	param.Data.rFrTblCfg.rFrTblInfo.ucFrHeLtf = tx_info.ltf;
	param.Data.rFrTblCfg.rFrTblInfo.fgFrLDPC = tx_info.ldpc;
	param.Data.rFrTblCfg.rFrTblInfo.fgFrTBF = tx_info.ebf;
	param.Data.rFrTblCfg.rFrTblInfo.fgFrDynBw = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
		"u2FixedRate = %d!\n", param.Data.rFrTblCfg.rFrTblInfo.u2FixedRate);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
		"ucFrAntIdx = %d!\n", param.Data.rFrTblCfg.rFrTblInfo.ucFrAntIdx);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
		"ucFrSpeIdxSel = %d!\n", param.Data.rFrTblCfg.rFrTblInfo.ucFrSpeIdxSel);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
		"ucFrSpeIdx = %d!\n", param.Data.rFrTblCfg.rFrTblInfo.ucFrSpeIdx);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
		"fgFrGI = %d!\n", param.Data.rFrTblCfg.rFrTblInfo.fgFrGI);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
		"ucFrHeLtf = %d!\n", param.Data.rFrTblCfg.rFrTblInfo.ucFrHeLtf);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
		"fgFrLDPC = %d!\n", param.Data.rFrTblCfg.rFrTblInfo.fgFrLDPC);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
		"fgFrTBF = %d!\n", param.Data.rFrTblCfg.rFrTblInfo.fgFrTBF);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
		"fgFrDynBw = %d!\n", param.Data.rFrTblCfg.rFrTblInfo.fgFrDynBw);

	ret = UniCmdATETest(ad, &param, u8TestTrParamFeature, ucParamNum, NULL);
#endif
	if (ret != 0)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
	return ret;
}

s_int32 mt_op_set_preamble_punc_cfg(
	struct test_wlan_info *winfos,
	u_char band_idx,
	boolean enable,
	u_int16 punc_bmp)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#ifndef WIFI_UNIFIED_COMMAND
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_WARN,
		"Not support!\n");
#else
	struct _RTMP_ADAPTER *ad = NULL;
	struct _UNI_CMD_TEST_TR_PARAM_CTRL_T param;
	uint64_t u8TestTrParamFeature;
	uint8_t ucParamNum = 1;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	sys_ad_zero_mem(&param, sizeof(param));

	param.ucAction = ENUM_TR_PARAM_SET;
	param.ucAteTestModeEn = 1;
	u8TestTrParamFeature = UNI_ATE_PREAMBLE_PUNC_FEATURE;

	param.Data.rPreamblePuncCfg.ucDbdcIdx = band_idx;
	param.Data.rPreamblePuncCfg.fgEnable = enable;
	param.Data.rPreamblePuncCfg.u2PreamblePuncBitmap = punc_bmp;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"ucDbdcIdx = %d!\n", param.Data.rPreamblePuncCfg.ucDbdcIdx);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"fgEnable = %d!\n", param.Data.rPreamblePuncCfg.fgEnable);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"u2PreamblePuncBitmap = 0x%4X!\n", param.Data.rPreamblePuncCfg.u2PreamblePuncBitmap);

	ret = UniCmdATETest(ad, &param, u8TestTrParamFeature, ucParamNum, NULL);
#endif
	if (ret != 0)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
	return ret;
}

s_int32 mt_op_set_tmr(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_tmr_info *tmr_info)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 id = 0;

	ret = tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);
	if (ret) {
		id = RF_AT_FUNCID_SET_DBDC_BAND_IDX;
		goto err;
	}

	ret = tm_Set(RF_AT_FUNCID_SET_TMR_ROLE, tmr_info->setting);
	if (ret) {
		id = RF_AT_FUNCID_SET_TMR_ROLE;
		goto err;
	}

	ret = tm_Set(RF_AT_FUNCID_SET_TMR_MODULE, tmr_info->version);
	if (ret) {
		id = RF_AT_FUNCID_SET_TMR_MODULE;
		goto err;
	}

	ret = tm_Set(RF_AT_FUNCID_SET_TMR_DBM, tmr_info->through_hold);
	if (ret) {
		id = RF_AT_FUNCID_SET_TMR_DBM;
		goto err;
	}

	ret = tm_Set(RF_AT_FUNCID_SET_TMR_ITER, tmr_info->iter);
	if (ret) {
		id = RF_AT_FUNCID_SET_TMR_ITER;
		goto err;
	}

	ret = tm_Set(RF_AT_FUNCID_SET_TMR_TOAE_CAL_RESTORE, tmr_info->toae_cal);
	if (ret) {
		id = RF_AT_FUNCID_SET_TMR_TOAE_CAL_RESTORE;
		goto err;
	}

err:
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"at cmd id: %u fail, ret:%d\n", id, ret);
	}

	return ret;
}

s_int32 mt_op_set_txpwr_power_drop(
	struct test_wlan_info *winfos,
	s_int8 cPowerDropLevel,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"cPowerDropLevel: %d, band_idx: %d\n",
		cPowerDropLevel, band_idx);

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdTxPowerDropCtrl(ad, cPowerDropLevel, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_txpwr_power_percentage(
	struct test_wlan_info *winfos,
	boolean enable,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"Enable: %d, band_idx: %d\n",
		enable, band_idx);

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdTxPowerPercentCtrl(ad, enable, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}


#ifdef TXBF_SUPPORT
s_int32 mt_op_set_ibf_phase_cal_e2p_update(
	struct test_wlan_info *winfos,
	u_char group_idx,
	boolean fgSx2,
	u_char update_type)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->iBFPhaseCalE2PUpdate != NULL) {
		ad->fgCalibrationFail = FALSE; /* Enable EEPROM write */
						/* of calibrated phase */

		/* Bit0   : BW160 ? */
		/* Bit1~3 : reserved */
		/* Bit4~5 : 0(Clean all), 1(Clean 2G iBF E2p only),*/
				/*2(Clean 5G iBF E2p only)*/
		/* Bit6~7 : reserved */
		switch (fgSx2 >> 4) {
		case CLEAN_ALL:
			ad->u1IbfCalPhase2G5GE2pClean = 0; /* Clean all */
			break;
		case CLEAN_2G:
			ad->u1IbfCalPhase2G5GE2pClean = 1; /* Clean 2G */
			break;
		case CLEAN_5G:
			ad->u1IbfCalPhase2G5GE2pClean = 2; /* Clean 5G */
			break;
		default:
			ad->u1IbfCalPhase2G5GE2pClean = 0; /* Clean all */
			break;
		}

		ops->iBFPhaseCalE2PUpdate(ad, group_idx, fgSx2, update_type);
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"The function is not hooked !!\n");

		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;
	}

	return ret;
}

s_int32 mt_op_set_ibf_phase_cal_init(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->iBFPhaseCalInit)
		ops->iBFPhaseCalInit(ad);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}

s_int32 mt_op_set_wite_txbf_pfmu_tag(
	struct test_wlan_info *winfos, u_char prf_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->write_txbf_pfmu_tag)
		ops->write_txbf_pfmu_tag(ad->hdev_ctrl, prf_idx);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_invalid(
	struct test_wlan_info *winfos, boolean fg_invalid)
{
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG1_INVALID,
					fg_invalid);
	else
		return SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;
}


s_int32 mt_op_set_txbf_pfmu_tag_idx(
	struct test_wlan_info *winfos, u_char pfmu_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG1_PFMU_ID,
					pfmu_idx);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_bf_type(
	struct test_wlan_info *winfos, u_char bf_type)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG1_IEBF,
					bf_type);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_dbw(
	struct test_wlan_info *winfos, u_char dbw)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG1_DBW,
					dbw);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_sumu(
	struct test_wlan_info *winfos, u_char su_mu)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG1_SU_MU,
					su_mu);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_get_wrap_ibf_cal_ibf_mem_alloc(
	struct test_wlan_info *winfos,
	u_char *pfmu_mem_row,
	u_char *pfmu_mem_col)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->iBfCaliBfPfmuMemAlloc)
		ops->iBfCaliBfPfmuMemAlloc(ad, pfmu_mem_row, pfmu_mem_col);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_get_wrap_ibf_cal_ebf_mem_alloc(
	struct test_wlan_info *winfos,
	u_char *pfmu_mem_row,
	u_char *pfmu_mem_col)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->iBfCaleBfPfmuMemAlloc)
		ops->iBfCaleBfPfmuMemAlloc(ad, pfmu_mem_row, pfmu_mem_col);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_mem(
	struct test_wlan_info *winfos,
	u_char *pfmu_mem_row,
	u_char *pfmu_mem_col)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag) {
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_ROW0,
				pfmu_mem_row[0]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_ROW1,
				pfmu_mem_row[1]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_ROW2,
				pfmu_mem_row[2]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_ROW3,
				pfmu_mem_row[3]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_COL0,
				pfmu_mem_col[0]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_COL1,
				pfmu_mem_col[1]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_COL2,
				pfmu_mem_col[2]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_COL3,
				pfmu_mem_col[3]);
	} else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_matrix(
	struct test_wlan_info *winfos,
	u_char nr, u_char nc, u_char ng, u_char lm, u_char cb, u_char he)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag) {
		ops->set_txbf_pfmu_tag(ad->hdev_ctrl, TAG1_NR, nr);
		ops->set_txbf_pfmu_tag(ad->hdev_ctrl, TAG1_NC, nc);
		ops->set_txbf_pfmu_tag(ad->hdev_ctrl, TAG1_NG, ng);
		ops->set_txbf_pfmu_tag(ad->hdev_ctrl, TAG1_LM, lm);
		ops->set_txbf_pfmu_tag(ad->hdev_ctrl, TAG1_CODEBOOK, cb);
		ops->set_txbf_pfmu_tag(ad->hdev_ctrl, TAG1_HTC, he);
	} else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_snr(
	struct test_wlan_info *winfos,
	u_char *snr_sts)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag) {
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS0,
				snr_sts[0]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS1,
				snr_sts[1]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS2,
				snr_sts[2]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS3,
				snr_sts[3]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS4,
				snr_sts[4]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS5,
				snr_sts[5]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS6,
				snr_sts[6]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS7,
				snr_sts[7]);
	} else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_smart_ant(
	struct test_wlan_info *winfos,
	u_int32 smart_ant)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG2_SMART_ANT,
					smart_ant);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_se_idx(
	struct test_wlan_info *winfos,
	u_char se_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG2_SE_ID,
					se_idx);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_rmsd_thrd(
	struct test_wlan_info *winfos,
	u_char rmsd_thrd)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG2_RMSD_THRESHOLD,
					rmsd_thrd);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_time_out(
	struct test_wlan_info *winfos,
	u_char time_out)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG2_IBF_TIMEOUT,
					time_out);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_desired_bw(
	struct test_wlan_info *winfos,
	u_char desired_bw)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG2_IBF_DBW,
					desired_bw);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_desired_nr(
	struct test_wlan_info *winfos,
	u_char desired_nr)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG2_IBF_NROW,
					desired_nr);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_desired_nc(
	struct test_wlan_info *winfos,
	u_char desired_nc)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG2_IBF_NCOL,
					desired_nc);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_data_write(
	struct test_wlan_info *winfos,
	u_int16 *angle_input)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->write_txbf_profile_data)
		ret = ops->write_txbf_profile_data(ad, angle_input);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_manual_assoc(
	struct test_wlan_info *winfos,
	u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_manual_assoc)
		ops->set_manual_assoc(ad, arg);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}

#endif /* TXBF_SUPPORT */

s_int32 mt_op_get_chip_info(
	struct test_wlan_info *winfos,
	struct _net_testmode_chip_info *ptestmode_chip_info)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	ret = net_ad_get_chip_info(winfos, ptestmode_chip_info);
	if (ret)
		goto error;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"chip_band_num = %d\n",
		ptestmode_chip_info->u1chip_band_num);

	return ret;

error:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"get chip information fail, ");

	return SERV_STATUS_OSAL_NET_INVALID_PAD;
}

s_int32 mt_op_get_rx_sw_count(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 *rxed_slt_pkt_cnt)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 rsp = 0;

	tm_Set(RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);

	ret = tm_Get(RF_AT_FUNCID_GET_RX_SW_COUNT, 0, &rsp);
	if (ret == SERV_STATUS_SUCCESS)	{
		*rxed_slt_pkt_cnt = rsp;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"rxed_slt_pkt_cnt:%u\n",
			*rxed_slt_pkt_cnt);
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"fail!\n");
	}

	return ret;
}

