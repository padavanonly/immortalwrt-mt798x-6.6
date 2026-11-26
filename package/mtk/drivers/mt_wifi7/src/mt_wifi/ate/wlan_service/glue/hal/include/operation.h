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
	operation.h
*/
#ifndef __OPERATION_H__
#define __OPERATION_H__

#include "test_mac.h"
#include "net_adaption.h"

extern int8_t g_hqa_frame_ctrl;

/*****************************************************************************
 *	Enum value definition
 *****************************************************************************/
/* gen4m object id list */
enum op_wlan_oid {
	OP_WLAN_OID_SET_TEST_MODE_START = 0,
	OP_WLAN_OID_SET_TEST_MODE_ABORT = 1,
	OP_WLAN_OID_RFTEST_SET_AUTO_TEST = 2,
	OP_WLAN_OID_QUERY_RX_STATISTICS = 3,
	OP_WLAN_OID_SET_TEST_ICAP_MODE = 4,
	OP_WLAN_OID_RFTEST_QUERY_AUTO_TEST  = 5,
	OP_WLAN_OID_SET_MCR_WRITE = 6,
	OP_WLAN_OID_QUERY_MCR_READ = 7,
	OP_WLAN_OID_GET_RECAL_COUNT = 8,
	OP_WLAN_OID_GET_RECAL_CONTENT = 9,
	OP_WLAN_OID_GET_ANTSWAP_CAPBILITY = 10,
	OP_WLAN_OID_SET_TEST_ICAP_START = 11,
	OP_WLAN_OID_SET_TEST_ICAP_ABORT = 12,
	OP_WLAN_OID_SET_TEST_ICAP_STATUS = 13,
	OP_WLAN_OID_GET_TEST_ICAP_MAX_DATA_LEN = 14,
	OP_WLAN_OID_GET_TEST_ICAP_DATA = 15,
	OP_WLAN_OID_RESET_RECAL_COUNT = 16,
	OP_WLAN_OID_NUM
};

enum ENUM_ANT_NUM {
	ANT_WF0 = 0,
	ANT_WF1 = 1,
	MAX_ANT_NUM
};

enum ENUM_M_BAND_NUM {
	M_BAND_0 = 0,
	M_BAND_1 = 1,
	M_BAND_NUM
};

enum ENUM_PREK_GROUP_OP {
	PREK_GROUP_CLEAN = 0,
	PREK_GROUP_DUMP = 1,
	PREK_GROUP_DUMP_5G = 2,
	PREK_GROUP_DUMP_6G = 3,
	PREK_GROUP_PROC = 4,
	PREK_GROUP_OP_NUM
};

enum ENUM_PREK_DPD_OP {
	PREK_DPD_CLEAN = 0,
	PREK_DPD_DUMP_2G = 1,
	PREK_DPD_DUMP_5G = 2,
	PREK_DPD_DUMP_6G = 3,
	PREK_DPD_5G_PROC = 4,
	PREK_DPD_2G_PROC = 5,
	PREK_DPD_6G_PROC = 6,
	PREK_DPD_OP_NUM
};

/*****************************************************************************
 *	Enum value definition
 *****************************************************************************/
/* gen4m/logan Function ID List */
enum ENUM_RF_AT_FUNCID {
	RF_AT_FUNCID_VERSION = 0,
	RF_AT_FUNCID_COMMAND,
	RF_AT_FUNCID_POWER,
	RF_AT_FUNCID_RATE,
	RF_AT_FUNCID_PREAMBLE,
	RF_AT_FUNCID_ANTENNA,
	RF_AT_FUNCID_PKTLEN,
	RF_AT_FUNCID_PKTCNT,
	RF_AT_FUNCID_PKTINTERVAL,
	RF_AT_FUNCID_TEMP_COMPEN,
	RF_AT_FUNCID_TXOPLIMIT,
	RF_AT_FUNCID_ACKPOLICY,
	RF_AT_FUNCID_PKTCONTENT,
	RF_AT_FUNCID_RETRYLIMIT,
	RF_AT_FUNCID_QUEUE,
	RF_AT_FUNCID_BANDWIDTH,
	RF_AT_FUNCID_GI,
	RF_AT_FUNCID_STBC,
	RF_AT_FUNCID_CHNL_FREQ,
	RF_AT_FUNCID_RIFS,
	RF_AT_FUNCID_TRSW_TYPE,
	RF_AT_FUNCID_RF_SX_SHUTDOWN,
	RF_AT_FUNCID_PLL_SHUTDOWN,
	RF_AT_FUNCID_SLOW_CLK_MODE,
	RF_AT_FUNCID_ADC_CLK_MODE,
	RF_AT_FUNCID_MEASURE_MODE,
	RF_AT_FUNCID_VOLT_COMPEN,
	RF_AT_FUNCID_DPD_TX_GAIN,
	RF_AT_FUNCID_DPD_MODE,
	RF_AT_FUNCID_TSSI_MODE,
	RF_AT_FUNCID_TX_GAIN_CODE,
	RF_AT_FUNCID_TX_PWR_MODE,

	/* Query command */
	RF_AT_FUNCID_TXED_COUNT = 32,
	RF_AT_FUNCID_TXOK_COUNT,
	RF_AT_FUNCID_RXOK_COUNT,
	RF_AT_FUNCID_RXERROR_COUNT,
	RF_AT_FUNCID_RESULT_INFO,
	RF_AT_FUNCID_TRX_IQ_RESULT,
	RF_AT_FUNCID_TSSI_RESULT,
	RF_AT_FUNCID_DPD_RESULT,
	RF_AT_FUNCID_RXV_DUMP,
	RF_AT_FUNCID_RX_PHY_STATIS,
	RF_AT_FUNCID_MEASURE_RESULT,
	RF_AT_FUNCID_TEMP_SENSOR,
	RF_AT_FUNCID_VOLT_SENSOR,
	RF_AT_FUNCID_READ_EFUSE,
	RF_AT_FUNCID_RX_RSSI,
	RF_AT_FUNCID_FW_INFO,
	RF_AT_FUNCID_DRV_INFO,
	RF_AT_FUNCID_PWR_DETECTOR,
	RF_AT_FUNCID_WBRSSI_IBSSI,

	/* Set command */
	RF_AT_FUNCID_SET_DPD_RESULT = 64,
	RF_AT_FUNCID_SET_CW_MODE,
	RF_AT_FUNCID_SET_JAPAN_CH14_FILTER,
	RF_AT_FUNCID_WRITE_EFUSE,
	RF_AT_FUNCID_SET_MAC_ADDRESS,
	RF_AT_FUNCID_SET_TA,
	RF_AT_FUNCID_SET_RX_MATCH_RULE,

	/* 80211AC & Jmode */
	RF_AT_FUNCID_SET_CBW = 71,
	RF_AT_FUNCID_SET_DBW,
	RF_AT_FUNCID_SET_PRIMARY_CH,
	RF_AT_FUNCID_SET_ENCODE_MODE,
	RF_AT_FUNCID_SET_J_MODE,

	/* ICAP command */
	RF_AT_FUNCID_SET_ICAP_CONTENT = 80,
	RF_AT_FUNCID_SET_ICAP_MODE,
	RF_AT_FUNCID_SET_ICAP_STARTCAP,
	RF_AT_FUNCID_SET_ICAP_SIZE = 83,
	RF_AT_FUNCID_SET_ICAP_TRIGGER_OFFSET,
	RF_AT_FUNCID_QUERY_ICAP_DUMP_FILE = 85,

	/* 2G 5G Band */
	RF_AT_FUNCID_SET_BAND = 90,

	/* Reset Counter */
	RF_AT_FUNCID_RESETTXRXCOUNTER = 91,

	/* FAGC RSSI Path */
	RF_AT_FUNCID_FAGC_RSSI_PATH = 92,

	/* Set RX Filter Packet Length */
	RF_AT_FUNCID_RX_FILTER_PKT_LEN = 93,

	/* Tone */
	RF_AT_FUNCID_SET_TONE_RF_GAIN = 96,
	RF_AT_FUNCID_SET_TONE_DIGITAL_GAIN = 97,
	RF_AT_FUNCID_SET_TONE_TYPE = 98,
	RF_AT_FUNCID_SET_TONE_DC_OFFSET = 99,
	RF_AT_FUNCID_SET_TONE_BW = 100,

	/* MT6632 Add */
	RF_AT_FUNCID_SET_MAC_HEADER = 101,
	RF_AT_FUNCID_SET_SEQ_CTRL = 102,
	RF_AT_FUNCID_SET_PAYLOAD = 103,
	RF_AT_FUNCID_SET_DBDC_BAND_IDX = 104,
	RF_AT_FUNCID_SET_BYPASS_CAL_STEP = 105,

	/* Set RX Path */
	RF_AT_FUNCID_SET_RX_PATH = 106,

	/* Set Frequency Offset */
	RF_AT_FUNCID_SET_FREQ_OFFSET = 107,

	/* Get Frequency Offset */
	RF_AT_FUNCID_GET_FREQ_OFFSET = 108,

	/* Set RXV Debug Index */
	RF_AT_FUNCID_SET_RXV_INDEX = 109,

	/* Set Test Mode DBDC Enable */
	RF_AT_FUNCID_SET_DBDC_ENABLE = 110,

	/* Get Test Mode DBDC Enable */
	RF_AT_FUNCID_GET_DBDC_ENABLE = 111,

	/* Set ICAP Ring Capture */
	RF_AT_FUNCID_SET_ICAP_RING = 112,

	/* Set TX Path */
	RF_AT_FUNCID_SET_TX_PATH = 113,

	/* Set Nss */
	RF_AT_FUNCID_SET_NSS = 114,

	/* Set TX Antenna Mask */
	RF_AT_FUNCID_SET_ANTMASK = 115,

	/* TMR set command */
	RF_AT_FUNCID_SET_TMR_ROLE = 116,
	RF_AT_FUNCID_SET_TMR_MODULE = 117,
	RF_AT_FUNCID_SET_TMR_DBM = 118,
	RF_AT_FUNCID_SET_TMR_ITER = 119,

	/* Set ADC For IRR Feature */
	RF_AT_FUNCID_SET_ADC = 120,

	/* Set RX Gain For IRR Feature */
	RF_AT_FUNCID_SET_RX_GAIN = 121,

	/* Set TTG For IRR Feature */
	RF_AT_FUNCID_SET_TTG = 122,

	/* Set TTG ON/OFF For IRR Feature */
	RF_AT_FUNCID_TTG_ON_OFF = 123,

	/* Set TSSI for QA Tool Setting */
	RF_AT_FUNCID_SET_TSSI = 124,

	/* Set Recal Cal Step */
	RF_AT_FUNCID_SET_RECAL_CAL_STEP = 125,

	/* Set iBF/eBF enable */
	RF_AT_FUNCID_SET_IBF_ENABLE = 126,
	RF_AT_FUNCID_SET_EBF_ENABLE = 127,

	/* Set MPS Setting */
	RF_AT_FUNCID_SET_MPS_SIZE = 128,
	RF_AT_FUNCID_SET_MPS_SEQ_DATA = 129,
	RF_AT_FUNCID_SET_MPS_PAYLOAD_LEN = 130,
	RF_AT_FUNCID_SET_MPS_PKT_CNT = 131,
	RF_AT_FUNCID_SET_MPS_PWR_GAIN = 132,
	RF_AT_FUNCID_SET_MPS_NSS = 133,
	RF_AT_FUNCID_SET_MPS_PACKAGE_BW = 134,

	RF_AT_FUNCID_GET_CH_TX_PWR_OFFSET = 136,
	/* Antenna swap feature*/
	RF_AT_FUNCID_SET_ANT_SWP = 153,
	RF_AT_FUNCID_SET_RX_MU_AID = 157,
	RF_AT_FUNCID_SET_TX_HE_TB_TTRCR0 = 158,
	RF_AT_FUNCID_SET_TX_HE_TB_TTRCR1 = 159,
	RF_AT_FUNCID_SET_TX_HE_TB_TTRCR2 = 160,
	RF_AT_FUNCID_SET_TX_HE_TB_TTRCR3 = 161,
	RF_AT_FUNCID_SET_TX_HE_TB_TTRCR4 = 162,
	RF_AT_FUNCID_SET_TX_HE_TB_TTRCR5 = 163,
	RF_AT_FUNCID_SET_TX_HE_TB_TTRCR6 = 164,
	RF_AT_FUNCID_SET_SECURITY_MODE = 165,
	RF_AT_FUNCID_SET_LP_MODE = 166,

	/* Set HW TX enable */
	RF_AT_FUNCID_SET_HWTX_MODE = 167,

	/* 11 be */
	RF_AT_FUNCID_SET_PUNCTURE = 168,
	RF_AT_FUNCID_GET_CFG_ON_OFF = 169,

    /* Set FREQ OFFSET C2 */
	RF_AT_FUNCID_SET_FREQ_OFFSET_C2_SET = 171,
	RF_AT_FUNCID_SET_FREQ_OFFSET_C2_GET = 172,

	RF_AT_FUNCID_SET_CFG_ON = 176,
	RF_AT_FUNCID_SET_CFG_OFF = 177,
	/* Set MURU */
	RF_AT_FUNCID_SET_RU_INFO_NUM = 178,
	RF_AT_FUNCID_SET_RU_INFO_0 = 179, /* RUINFO Setting Table DW0 */
	RF_AT_FUNCID_SET_RU_INFO_1 = 180,
	RF_AT_FUNCID_SET_RU_INFO_2 = 181,
	RF_AT_FUNCID_GET_TB_INFO_A_FACTOR = 182,
	RF_AT_FUNCID_GET_TB_INFO_LDPC_STR_SYM = 183,
	RF_AT_FUNCID_GET_TB_INFO_PE_DISAMB = 184,
	RF_AT_FUNCID_GET_TB_INFO_TXPE = 185,
	RF_AT_FUNCID_GET_TB_INFO_L_SIG_LENGTH = 186,
	RF_AT_FUNCID_SET_CURR_STA_INDEX = 187,
	RF_AT_FUNCID_SET_STA_A1 = 188,
	RF_AT_FUNCID_SET_BSSID = 189,

	RF_AT_FUNCID_SET_TX_TIME = 190,
	RF_AT_FUNCID_SET_MAX_PE = 191,
	RF_AT_FUNCID_SET_TX_HE_TB_TTRCR7 = 192,
	RF_AT_FUNCID_SET_TX_HE_TB_TTRCR8 = 193,
	RF_AT_FUNCID_FREQ_MODE_SET = 194,
	RF_AT_FUNCID_FREQ_MODE_GET = 195,

	RF_AT_FUNCID_SET_RU_MAX_PE = 200,
	RF_AT_FUNCID_SET_AID_OFFSET = 204,

	RF_AT_FUNCID_GET_TEST_CHECKER_STATUS = 207,
	RF_AT_FUNCID_SET_RU_INFO_2_PRESET_LENGTH_EXT = 208,
	RF_AT_FUNCID_SET_TSSI_CH_OFFSET = 209,
	RF_AT_FUNCID_SET_CVT_OFFSET = 210,
	RF_AT_FUNCID_SET_TMR_TOAE_CAL_RESTORE = 225,
	RF_AT_FUNCID_GET_RX_SW_COUNT  = 226,

	RF_AT_FUNCID_END = 0xff
};

/* Command */
enum ENUM_RF_AT_COMMAND {
	RF_AT_COMMAND_STOPTEST = 0,
	RF_AT_COMMAND_STARTTX,
	RF_AT_COMMAND_STARTRX,
	RF_AT_COMMAND_RESET,
	RF_AT_COMMAND_OUTPUT_POWER,	/* Payload */
	/* Local freq is renamed to Local leakage */
	RF_AT_COMMAND_LO_LEAKAGE,
	/* OFDM (LTF/STF), CCK (PI,PI/2) */
	RF_AT_COMMAND_CARRIER_SUPPR,
	RF_AT_COMMAND_TRX_IQ_CAL,
	RF_AT_COMMAND_TSSI_CAL,
	RF_AT_COMMAND_DPD_CAL,
	RF_AT_COMMAND_CW,
	RF_AT_COMMAND_ICAP,
	RF_AT_COMMAND_RDD,
	RF_AT_COMMAND_CH_SWITCH_FOR_ICAP,
	RF_AT_COMMAND_RESET_DUMP_NAME,
	RF_AT_COMMAND_SINGLE_TONE,
	RF_AT_COMMAND_RDD_OFF,
	RF_AT_COMMAND_TX_COMMIT = 18,
	RF_AT_COMMAND_RX_COMMIT = 19,
	RF_AT_COMMAND_NUM
};

/*****************************************************************************
 *	Structure definition
 *****************************************************************************/
struct param_mtk_wifi_test_struct {
	u_int32 func_idx;
	u_int32 func_data;
};

struct param_custom_access_rx_stat {
	u_int32 seq_num;
	u_int32 total_num;
};

struct param_custom_mcr_rw_struct {
	u_int32 mcr_offset;
	u_int32 mcr_data;
};

struct test_txpwr_cfg {
	u_int32 ant_idx;
	u_int32 txpwr;
	u_int32 channel;
	u_int32 band_idx;
	u_int32 ch_band;
};

struct test_ch_cfg {
	u_int8 ctrl_ch;
	u_int8 ctrl_ch2;
	u_int8 central_ch;
	u_int8 bw;
	u_int8 tx_strm;
	u_int8 rx_path;
	boolean scan;
	boolean dfs_check;
	u_int8 band_idx;
	u_int8 ch_band;
	u_int32 out_band_freq;
};

/* Test rbist status for hqa command usage*/
struct GNU_PACKED hqa_rbist_cap_start {
	u_int32 trig;
	u_int32 ring_cap_en;
	u_int32 trig_event;
	u_int32 cap_node;
	u_int32 cap_len;
	u_int32 cap_stop_cycle;
	u_int32 mac_trig_event;
	u_int32 src_addr_lsb;
	u_int32 src_addr_msb;
	u_int32 band_idx;
	u_int32 bw;
	u_int32 en_bit_width; /* 0:32bit, 1:96bit, 2:128bit */
	u_int32 arch; /* 0:on-chip, 1:on-the-fly */
	u_int32 phy_idx;
	u_int32 emi_start_addr;
	u_int32 emi_end_addr;
	u_int32 emi_msb_addr;
	u_int32 cap_src;
	u_int32 resv[2];
};

struct test_struct_ext {
	u_int32 func_idx;
	union {
		u_int32 func_data;
		u_int32 cal_dump;
		struct hqa_rbist_cap_start icap_info;
	} data;
};

/*****************************************************************************
 *	Function declaration
 *****************************************************************************/
s_int32 mt_op_set_tr_mac(
	struct test_wlan_info *winfos,
	s_int32 op_type, boolean enable, u_char band_idx);
s_int32 mt_op_set_tx_stream(
	struct test_wlan_info *winfos,
	u_int32 stream_nums, u_char band_idx);
s_int32 mt_op_set_tx_path(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs);
s_int32 mt_op_set_rx_path(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs);
s_int32 mt_op_set_rx_filter(
	struct test_wlan_info *winfos,
	struct rx_filter_ctrl rx_filter);
s_int32 mt_op_set_clean_persta_txq(
	struct test_wlan_info *winfos,
	boolean sta_pause_enable,
	void *virtual_wtbl,
	u_char omac_idx,
	u_char band_idx);
s_int32 mt_op_set_cfg_on_off(
	struct test_wlan_info *winfos,
	u_int8 type, u_int8 enable, u_char band_idx);
s_int32 mt_op_log_on_off(
	struct test_wlan_info *winfos,
	struct test_log_dump_cb *log_cb,
	u_int32 log_type,
	u_int32 log_ctrl,
	u_int32 log_size);
s_int32 mt_op_set_antenna_port(
	struct test_wlan_info *winfos,
	u_int8 rf_mode_mask, u_int8 rf_port_mask, u_int8 ant_port_mask);
s_int32 mt_op_set_slot_time(
	struct test_wlan_info *winfos,
	u_int8 slot_time, u_int8 sifs_time, u_int8 rifs_time,
	u_int16 eifs_time, u_char band_idx);
s_int32 mt_op_set_power_drop_level(
	struct test_wlan_info *winfos,
	u_int8 pwr_drop_level, u_char band_idx);
s_int32 mt_op_set_rx_filter_pkt_len(
	struct test_wlan_info *winfos,
	u_int8 enable, u_char band_idx, u_int32 rx_pkt_len);
s_int32 mt_op_get_antswap_capability(
	struct test_wlan_info *winfos,
	u_int32 *antswap_support);
s_int32 mt_op_set_antswap(
	struct test_wlan_info *winfos,
	u_int32 ant);
s_int32 mt_op_set_eeprom_to_fw(
	struct test_wlan_info *winfos);
s_int32 mt_op_set_freq_offset(
	struct test_wlan_info *winfos,
	u_int32 freq_offset, u_char band_idx);
s_int32 mt_op_set_freq_offset_c2(
	struct test_wlan_info *winfos,
	u_int32 freq_offset, u_char band_idx);
s_int32 mt_op_set_phy_counter(
	struct test_wlan_info *winfos,
	s_int32 control, u_char band_idx);
s_int32 mt_op_set_rxv_index(
	struct test_wlan_info *winfos,
	u_int8 group_1, u_int8 group_2, u_char band_idx);
s_int32 mt_op_set_fagc_path(
	struct test_wlan_info *winfos,
	u_int8 path, u_char band_idx);
s_int32 mt_op_set_fw_mode(
	struct test_wlan_info *winfos, u_char fw_mode);
s_int32 mt_op_set_rf_test_mode(
	struct test_wlan_info *winfos,
	u_int32 op_mode, u_int8 icap_len, u_int16 rsp_len);
s_int32 mt_op_start_tx(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs);
s_int32 mt_op_stop_tx(
	struct test_wlan_info *winfos,
	u_char band_idx);
s_int32 mt_op_start_rx(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs);
s_int32 mt_op_stop_rx(
	struct test_wlan_info *winfos,
	u_char band_idx);
s_int32 mt_op_set_channel(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs);
s_int32 mt_op_set_tx_content(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs);
s_int32 mt_op_set_preamble(
	struct test_wlan_info *winfos,
	u_char mode);
s_int32 mt_op_set_rate(
	struct test_wlan_info *winfos,
	u_char mcs);
s_int32 mt_op_set_system_bw(
	struct test_wlan_info *winfos,
	u_char sys_bw);
s_int32 mt_op_set_per_pkt_bw(
	struct test_wlan_info *winfos,
	u_char per_pkt_bw);
s_int32 mt_op_reset_txrx_counter(
	struct test_wlan_info *winfos,
	u_char band_idx);
s_int32 mt_op_set_rx_vector_idx(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 group1,
	u_int32 group2);
s_int32 mt_op_set_fagc_rssi_path(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 fagc_path);
s_int32 mt_op_get_rx_stat_leg(
	struct test_wlan_info *winfos,
	struct test_rx_stat_leg *rx_stat);
s_int32 mt_op_get_rxv_dump_action(
	struct test_wlan_info *winfos,
	u_int32 action,
	u_int32 type_mask);
s_int32 mt_op_get_rxv_dump_ring_attr(
	struct test_wlan_info *winfos,
	struct rxv_dump_ring_attr *attr);
s_int32 mt_op_get_rxv_content_len(
	struct test_wlan_info *winfos,
	u_int8 type_mask,
	u_int8 rxv_sta_cnt,
	u_int16 *len);
s_int32 mt_op_get_rxv_dump_rxv_content(
	struct test_wlan_info *winfos,
	u_int8 entry_idx,
	u_int32 *content_len,
	void *rxv_content);
s_int32 mt_op_dbdc_tx_tone(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs);
s_int32 mt_op_dbdc_tx_tone_pwr(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs);
s_int32 mt_op_dbdc_continuous_tx(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs);
s_int32 mt_op_get_tx_info(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	struct test_configuration *test_configs_band0,
	struct test_configuration *test_configs_band1);
s_int32 mt_op_get_rx_statistics_all(
	struct test_wlan_info *winfos,
	struct hqa_comm_rx_stat *hqa_rx_stat);
s_int32 mt_op_calibration_test_mode(
	struct test_wlan_info *winfos,
	u_char mode);
s_int32 mt_op_set_icap_start(
	struct test_wlan_info *winfos,
	u_int8 *data);
s_int32 mt_op_get_icap_status(
	struct test_wlan_info *winfos,
	s_int32 *icap_stat);
s_int32 mt_op_get_icap_max_data_len(
	struct test_wlan_info *winfos,
	u_long *max_data_len);
s_int32 mt_op_get_icap_data(
	struct test_wlan_info *winfos,
	s_int32 *icap_cnt,
	s_int32 *icap_data,
	u_int32 wf_num,
	u_int32 iq_type);
s_int32 mt_op_do_cal_item(
	struct test_wlan_info *winfos,
	u_int32 item,
	u_char band_idx);
s_int32 mt_op_set_band_mode(
	struct test_wlan_info *winfos,
	struct test_band_state *band_state);
s_int32 mt_op_get_chipid(
	struct test_wlan_info *winfos);
s_int32 mt_op_get_sub_chipid(
	struct test_wlan_info *winfos,
	u_int32 *sub_chipid);
s_int32 mt_op_mps_start(
	struct test_wlan_info *winfos,
	u_char band_idx);
s_int32 mt_op_mps_set_nss(
	struct test_wlan_info *winfos,
	u_int32 len,
	struct test_mps_setting *mps_setting);
s_int32 mt_op_mps_set_per_packet_bw(
	struct test_wlan_info *winfos,
	u_int32 len,
	struct test_mps_setting *mps_setting);
s_int32 mt_op_mps_set_packet_count(
	struct test_wlan_info *winfos,
	u_int32 len,
	struct test_mps_setting *mps_setting);

s_int32 mt_op_mps_set_payload_length(
	struct test_wlan_info *winfos,
	u_int32 len,
	struct test_mps_setting *mps_setting);
s_int32 mt_op_mps_set_power_gain(
	struct test_wlan_info *winfos,
	u_int32 len,
	struct test_mps_setting *mps_setting);
s_int32 mt_op_mps_set_seq_data(
	struct test_wlan_info *winfos,
	u_int32 len,
	struct test_mps_setting *mps_setting);
s_int32 mt_op_set_muru_manual(
	void *virtual_device,
	struct test_wlan_info *winfos,
	struct test_configuration *configs);
s_int32 mt_op_set_tam_arb(
	struct test_wlan_info *winfos,
	u_int8 arb_op_mode);
s_int32 mt_op_set_mu_count(
	struct test_wlan_info *winfos,
	void *virtual_device,
	u_int32 count);
s_int32 mt_op_trigger_mu_counting(
	struct test_wlan_info *winfos,
	void *virtual_device,
	boolean enable);
/* gen4m operation define */
s_int32 mt_op_set_test_mode_start(
	struct test_wlan_info *winfos,
	struct test_backup_params *bak,
	struct test_configuration *configs);
s_int32 mt_op_set_test_mode_abort(
	struct test_wlan_info *winfos,
	struct test_backup_params *bak,
	struct test_configuration *configs,
	struct test_operation *ops);

/* For test mac usage */
s_int32 mt_op_backup_and_set_cr(
	struct test_wlan_info *winfos,
	struct test_bk_cr *bks,
	u_char band_idx);
s_int32 mt_op_restore_cr(
	struct test_wlan_info *winfos,
	struct test_bk_cr *bks,
	u_char band_idx,
	u_char option);
s_int32 mt_op_set_ampdu_ba_limit(
	struct test_wlan_info *winfos,
	u_int8 wmm_idx,
	u_int8 agg_limit,
	u_char band_idx);
s_int32 mt_op_set_sta_pause_cr(
	struct test_wlan_info *winfos);
s_int32 mt_op_set_ifs_cr(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx);
s_int32 mt_op_write_mac_bbp_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs);
s_int32 mt_op_read_bulk_mac_bbp_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs);
s_int32 mt_op_read_bulk_rf_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs);
s_int32 mt_op_write_bulk_rf_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs);
s_int32 mt_op_read_bulk_eeprom(
	struct test_wlan_info *winfos,
	struct test_eeprom *eprms);

/* For test phy usage */
s_int32 mt_op_get_tx_pwr(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx,
	u_char channel,
	u_char ant_idx,
	u_int32 *power);
s_int32 mt_op_set_tx_pwr(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx,
	struct test_txpwr_param *pwr_param);
s_int32 mt_op_set_ch_pwr_offset(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx,
	struct test_txpwr_param *pwr_param);
s_int32 mt_op_set_cvt_pwr_offset(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx,
	struct test_txpwr_param *pwr_param);
s_int32 mt_op_set_freq_offset(
	struct test_wlan_info *winfos,
	u_int32 freq_offset, u_char band_idx);
s_int32 mt_op_get_freq_offset(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 *freq_offset);
s_int32 mt_op_get_freq_offset_c2(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 *freq_offset);
s_int32 mt_op_get_cfg_on_off(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 type,
	u_int32 *result);
s_int32 mt_op_get_tx_tone_pwr(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 ant_idx,
	u_int32 *power);
s_int32 mt_op_get_recal_cnt(
	struct test_wlan_info *winfos,
	u_int32 *recal_cnt,
	u_int32 *recal_dw_num);
s_int32 mt_op_get_recal_content(
	struct test_wlan_info *winfos,
	u_int32 *content);
s_int32 mt_op_get_rxv_cnt(
	struct test_wlan_info *winfos,
	u_int32 *rxv_cnt,
	u_int32 *rxv_dw_num);
s_int32 mt_op_get_rxv_content(
	struct test_wlan_info *winfos,
	u_int32 dw_cnt,
	u_int32 *content);
s_int32 mt_op_set_cal_bypass(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 cal_item);
s_int32 mt_op_set_dpd(
	struct test_wlan_info *winfos,
	u_int32 on_off,
	u_int32 wf_sel);
s_int32 mt_op_set_tssi(
	struct test_wlan_info *winfos,
	u_int32 on_off,
	u_int32 wf_sel);
s_int32 mt_op_get_thermal_val(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char sensorType,
	u_char band_idx,
	u_int8 *pu4SensorResult);
s_int32 mt_op_set_rdd_test(
	struct test_wlan_info *winfos,
	u_int32 rdd_idx,
	u_int32 rdd_sel,
	u_int32 enable);
s_int32 mt_op_set_off_ch_scan(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx,
	struct test_off_ch_param *off_ch_param);
s_int32 mt_op_get_rdd_cnt(
	struct test_wlan_info *winfos,
	u_int32 *rdd_cnt,
	u_int32 *rdd_dw_num);
s_int32 mt_op_get_rdd_content(
	struct test_wlan_info *winfos,
	u_int32 *content,
	u_int32 *total_cnt);
s_int32 mt_op_evt_rf_test_cb(
	struct test_wlan_info *winfos,
	struct test_log_dump_cb *test_log_dump,
	u_int32 en_log,
	u_int8 *data,
	u_int32 length);
s_int32 mt_op_get_hetb_info(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_ru_info *ru_info);
s_int32 mt_op_set_ru_aid(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int16 mu_rx_aid);
s_int32 mt_op_set_ru_info(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_ru_info *ru_info);
s_int32 mt_op_set_mutb_spe(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_char tx_mode,
	u_int8 spe_idx);
s_int32 mt_op_get_rx_stat_band(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int8 blk_idx,
	struct test_rx_stat_band_info *rx_st_band);
s_int32 mt_op_get_rx_stat_path(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int8 blk_idx,
	struct test_rx_stat_path_info *rx_st_path);
s_int32 mt_op_get_rx_stat_user(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int8 blk_idx,
	struct test_rx_stat_user_info *rx_st_user);
s_int32 mt_op_get_rx_stat_comm(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int8 blk_idx,
	struct test_rx_stat_comm_info *rx_st_comm);
s_int32 mt_op_set_rx_user_idx(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int16 user_idx);
s_int32 mt_op_get_wf_path_comb(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	boolean dbdc_mode_en,
	u_int8 *path,
	u_int8 *path_len);

s_int32 mt_op_set_test_mode_dnlk_clean(struct test_wlan_info *winfos);
s_int32 mt_op_set_test_mode_rxgaink(struct test_wlan_info *winfos, u_char band_idx);
s_int32 mt_op_set_frtbl_info(
	struct test_wlan_info *winfos,
	struct test_tx_info tx_info,
	u_int8 spe_idx);

s_int32 mt_op_set_preamble_punc_cfg(
	struct test_wlan_info *winfos,
	u_char band_idx,
	boolean enable,
	u_int16 punc_bmp);

s_int32 mt_op_set_tmr(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_tmr_info *tmr_info);

s_int32 mt_op_set_txpwr_power_drop(
	struct test_wlan_info *winfos,
	s_int8 cPowerDropLevel,
	u_char band_idx);

s_int32 mt_op_set_txpwr_power_percentage(
	struct test_wlan_info *winfos,
	boolean enable,
	u_char band_idx);


#ifdef TXBF_SUPPORT
s_int32 mt_op_set_ibf_phase_cal_e2p_update(
	struct test_wlan_info *winfos,
	u_char group_idx,
	boolean fgSx2,
	u_char update_type);
s_int32 mt_op_set_txbf_lna_gain(
	struct test_wlan_info *winfos,
	u_char lna_gain);
s_int32 mt_op_set_ibf_phase_cal_init(
	struct test_wlan_info *winfos);
s_int32 mt_op_set_wite_txbf_pfmu_tag(
	struct test_wlan_info *winfos, u_char prf_idx);
s_int32 mt_op_set_txbf_pfmu_tag_invalid(
	struct test_wlan_info *winfos, boolean fg_invalid);
s_int32 mt_op_set_txbf_pfmu_tag_idx(
	struct test_wlan_info *winfos, u_char pfmu_idx);
s_int32 mt_op_set_txbf_pfmu_tag_bf_type(
	struct test_wlan_info *winfos, u_char bf_type);
s_int32 mt_op_set_txbf_pfmu_tag_dbw(
	struct test_wlan_info *winfos, u_char dbw);
s_int32 mt_op_set_txbf_pfmu_tag_sumu(
	struct test_wlan_info *winfos, u_char su_mu);
s_int32 mt_op_get_wrap_ibf_cal_ibf_mem_alloc(
	struct test_wlan_info *winfos,
	u_char *pfmu_mem_row,
	u_char *pfmu_mem_col);
s_int32 mt_op_get_wrap_ibf_cal_ebf_mem_alloc(
	struct test_wlan_info *winfos,
	u_char *pfmu_mem_row,
	u_char *pfmu_mem_col);
s_int32 mt_op_set_txbf_pfmu_tag_mem(
	struct test_wlan_info *winfos,
	u_char *pfmu_mem_row,
	u_char *pfmu_mem_col);
s_int32 mt_op_set_txbf_pfmu_tag_matrix(
	struct test_wlan_info *winfos,
	u_char nr,
	u_char nc,
	u_char ng,
	u_char lm,
	u_char cb,
	u_char he);
s_int32 mt_op_set_txbf_pfmu_tag_snr(
	struct test_wlan_info *winfos,
	u_char *snr_sts);
s_int32 mt_op_set_txbf_pfmu_tag_smart_ant(
	struct test_wlan_info *winfos,
	u_int32 smart_ant);
s_int32 mt_op_set_txbf_pfmu_tag_se_idx(
	struct test_wlan_info *winfos,
	u_char se_idx);
s_int32 mt_op_set_txbf_pfmu_tag_rmsd_thrd(
	struct test_wlan_info *winfos,
	u_char rmsd_thrd);
s_int32 mt_op_set_txbf_pfmu_tag_time_out(
	struct test_wlan_info *winfos,
	u_char time_out);
s_int32 mt_op_set_txbf_pfmu_tag_desired_bw(
	struct test_wlan_info *winfos,
	u_char desired_bw);
s_int32 mt_op_set_txbf_pfmu_tag_desired_nr(
	struct test_wlan_info *winfos,
	u_char desired_nr);
s_int32 mt_op_set_txbf_pfmu_tag_desired_nc(
	struct test_wlan_info *winfos,
	u_char desired_nc);
s_int32 mt_op_set_txbf_pfmu_data_write(
	struct test_wlan_info *winfos, u_int16 *angle_input);
s_int32 mt_op_set_manual_assoc(
	struct test_wlan_info *winfos, u_char *arg);

#endif

s_int32 mt_op_get_rx_ok(
	struct test_wlan_info *winfos,
	struct test_configuration *test_configs_band0);
s_int32 mt_op_set_security(
	struct test_wlan_info *winfos,
	struct test_configuration *configs);
s_int32 mt_op_set_LowPower_mode(
	struct test_wlan_info *winfos,
	u_int32 Control);

s_int32 mt_op_get_chip_info(
	struct test_wlan_info *winfos,
	struct _net_testmode_chip_info *ptestmode_chip_info);

s_int32 mt_op_get_rx_sw_count(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 *rxed_slt_pkt_cnt);

#endif /* __OPERATION_H__ */
