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
	agent.c
*/
#include "agent.h"
#include "rt_config.h"

int8_t g_hqa_frame_ctrl;

u_char *agnt_rstrtok;
u_char *agent_trtok(u_char *s, const u_char *ct)
{
	u_char *sbegin, *send;

	sbegin  = s ? s : agnt_rstrtok;

	if (!sbegin)
		return NULL;

	sbegin += strspn(sbegin, ct);

	if (*sbegin == '\0') {
		agnt_rstrtok = NULL;
		return NULL;
	}

	send = strpbrk(sbegin, ct);

	if (send && *send != '\0')
		*send++ = '\0';

	agnt_rstrtok = send;
	return sbegin;
}


/*****************************************************************************
 *	HQA DLL handler
 *****************************************************************************/
static s_int32 agent_cfg_find_next_token(struct agent_cfg_parse_state_s
				 *state)
{
	s_int8 *x = state->ptr;
	s_int8 *s;

	if (state->nexttoken) {
		s_int32 t = state->nexttoken;

		state->nexttoken = 0;
		return t;
	}

	for (;;) {
		switch (*x) {
		case 0:
			state->ptr = x;
			return AGENT_STATE_EOF;
		case '\n':
			x++;
			state->ptr = x;
			return AGENT_STATE_NEWLINE;
		case ' ':
		case ',':
		/*case ':':  should not including : , mac addr would be fail*/
		case '\t':
		case '\r':
			x++;
			continue;
		case '#':
			while (*x && (*x != '\n'))
				x++;
			if (*x == '\n') {
				state->ptr = x + 1;
				return AGENT_STATE_NEWLINE;
			}
			state->ptr = x;
			return AGENT_STATE_EOF;

		default:
			goto text;
		}
	}

textdone:
	state->ptr = x;
	*s = 0;
	return AGENT_STATE_TEXT;
text:
	state->text = s = x;
textresume:
	for (;;) {
		switch (*x) {
		case 0:
			goto textdone;
		case ' ':
		case ',':
		/* case ':': */
		case '\t':
		case '\r':
			x++;
			goto textdone;
		case '\n':
			state->nexttoken = AGENT_STATE_NEWLINE;
			x++;
			goto textdone;
		case '"':
			x++;
			for (;;) {
				switch (*x) {
				case 0:
					/* unterminated quoted thing */
					state->ptr = x;
					return AGENT_STATE_EOF;
				case '"':
					x++;
					goto textresume;
				default:
					*s++ = *x++;
				}
			}
			break;
		case '\\':
			x++;
			switch (*x) {
			case 0:
				goto textdone;
			case 'n':
				*s++ = '\n';
				break;
			case 'r':
				*s++ = '\r';
				break;
			case 't':
				*s++ = '\t';
				break;
			case '\\':
				*s++ = '\\';
				break;
			case '\r':
				/* \ <cr> <lf> -> line continuation */
				if (x[1] != '\n') {
					x++;
					continue;
				}
				fallthrough;
			case '\n':
				/* \ <lf> -> line continuation */
				x++;
				/* eat any extra whitespace */
				while ((*x == ' ') || (*x == '\t'))
					x++;
				continue;
			default:
				/* unknown escape -- just copy */
				*s++ = *x++;
			}
			continue;
		default:
			*s++ = *x++;
			state->textsize++;

		}
	}
	return AGENT_STATE_EOF;
}

static s_int32 set_param_and_shift_buf(
	boolean convert, u_int32 size, u_char *in, u_char **out)
{
	if (!(*out)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"*buf NULL pointer with size=%u\n", size);
		return SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	if (!in) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"out NULL pointer with size=%u\n", size);
		return SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	if (convert) {
		if (size == sizeof(u_int32)) {
			u_int32 *tmp = (u_int32 *) in;

			*tmp = SERV_OS_HTONL(*tmp);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
			"size=%u, val=%u\n", size, *tmp);
		} else if (size == sizeof(u_int16)) {
			u_int16 *tmp = (u_int16 *) in;

			*tmp = SERV_OS_HTONS(*tmp);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
			"size=%u, val=%u\n", size, *tmp);
		} else {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"size %u not supported\n", size);
			return SERV_STATUS_AGENT_NOT_SUPPORTED;
		}
	}

	sys_ad_move_mem(*out, in, size);
	*out = *out + size;

	return SERV_STATUS_SUCCESS;
}

static s_int32 get_param_and_shift_buf(
	boolean convert, u_int32 size, u_char *out,
	struct hqa_frame_ctrl *hqa_frame)
{
	struct agent_cfg_parse_state_s state;
	u_int16 ret = 0;
	u_int32 tmp_mac[SERV_MAC_ADDR_LEN] = { 0 };
	s_int8 *ptr_old;
	u_int16 i = 0;
	u_char input_cnt = 0;

	if (!(hqa_frame))  {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"*buf NULL pointer with size=%u\n", size);
		return SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	if (!out) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"out NULL pointer with size=%u\n", size);
		return SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	if (hqa_frame->type == 1) {
		state.ptr = hqa_frame->hqa_frame_string + hqa_frame->pars_idx;
		state.nexttoken = 0;
		state.maxsize = 0;
		state.textsize = 0;
		state.text = 0;

		ptr_old = state.ptr;
		switch (agent_cfg_find_next_token(&state)) {
		case AGENT_STATE_EOF:
			goto exit;
		case AGENT_STATE_NEWLINE:
			goto exit;
		case AGENT_STATE_TEXT:
			break;
		}

		if (size == SERV_MAC_ADDR_LEN) {
			input_cnt = sscanf(state.text,
					"%x:%x:%x:%x:%x:%x",
					&tmp_mac[0],
					&tmp_mac[1],
					&tmp_mac[2],
					&tmp_mac[3],
					&tmp_mac[4],
					&tmp_mac[5]);

			if (input_cnt != 6) {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
					"Invalid format, %s ignored!\n", state.text);
				return SERV_STATUS_AGENT_INVALID_PARAM;
			}

			for (i = 0; i < SERV_MAC_ADDR_LEN; i++)
				out[i] = (u_char)tmp_mac[i];

		} else if (size == sizeof(u_int32)) {
			ret = kstrtou32(state.text, 0,
			(u_int32 *)out);
		} else if (size == sizeof(u_int16)) {
			ret = kstrtou16(state.text, 0,
			(u_int16 *)out);
		} else if (size == sizeof(u_int8)) {
			ret = kstrtou8(state.text, 0,
			out);
		} else {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_WARN,
				"size %u not supported\n", size);

			return SERV_STATUS_AGENT_NOT_SUPPORTED;
		}
		hqa_frame->pars_idx = (u_int32)(state.ptr - ptr_old + hqa_frame->pars_idx);
	} else {
		sys_ad_move_mem(out, &(hqa_frame->hqa_frame_eth->data[hqa_frame->pars_idx]), size);
		hqa_frame->pars_idx += size;
	}

	if ((!convert) || (hqa_frame->type == 1)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
			"size=%u", size);
		return SERV_STATUS_SUCCESS;
	}

	if (size == sizeof(u_int32)) {
		u_int32 *tmp = (u_int32 *) out;

		*tmp = SERV_OS_NTOHL(*tmp);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
			"size=%u, val=%u\n", size, *tmp);

	} else if (size == sizeof(u_int16)) {
		u_int16 *tmp = (u_int16 *) out;

		*tmp = SERV_OS_NTOHS(*tmp);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
			"size=%u, val=%u\n", size, *tmp);

	} else if (size == sizeof(u_int8)) {
		u_int8 *tmp = (u_int8 *)out;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
			"size=%u, val=%u\n", size, *tmp);

	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_WARN,
			"size %u not supported\n", size);

		return SERV_STATUS_AGENT_NOT_SUPPORTED;
	}

exit:
	return SERV_STATUS_SUCCESS;
}

static s_int32 show_rx_statistics(
	u_int32 type_idx, u_int32 blk, struct test_rx_stat_u *st)
{
	char type[SERV_RX_STAT_TYPE_NUM][5] = {"band", "path", "user", "comm"};
	u_int32 all_per = 0;

	if (type_idx < SERV_RX_STAT_TYPE_NUM) {

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"%s %u:\n", type[type_idx], blk);

		switch (type_idx) {
		case SERV_RX_STAT_TYPE_BAND:
			if (st->u.rx_st_band.mac_rx_mdrdy_cnt == 0)
				all_per = 0;
			else
				all_per = (st->u.rx_st_band.mac_rx_mdrdy_cnt -
							st->u.rx_st_band.mac_rx_fcs_ok_cnt)
							* 100 / st->u.rx_st_band.mac_rx_mdrdy_cnt;

			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"\tmac_rx_fcs_err_cnt: %u\n"
				"\tmac_rx_mdrdy_cnt: %u\n"
				"\tmac_rx_len_mismatch: %u\n"
				"\tmac_rx_fcs_ok_cnt: %u\n"
				"\tphy_rx_fcs_err_cnt_cck: %u\n"
				"\tphy_rx_fcs_err_cnt_ofdm: %u\n"
				"\tphy_rx_pd_cck: %u\n"
				"\tphy_rx_pd_ofdm: %u\n"
				"\tphy_rx_sig_err_cck: %u\n"
				"\tphy_rx_sfd_err_cck: %u\n"
				"\tphy_rx_sig_err_ofdm: %u\n"
				"\tphy_rx_tag_err_ofdm: %u\n"
				"\tphy_rx_mdrdy_cnt_cck: %u\n"
				"\tphy_rx_mdrdy_cnt_ofdm: %u\n"
				"\tper: %u\n",
				st->u.rx_st_band.mac_rx_fcs_err_cnt,
				st->u.rx_st_band.mac_rx_mdrdy_cnt,
				st->u.rx_st_band.mac_rx_len_mismatch,
				st->u.rx_st_band.mac_rx_fcs_ok_cnt,
				st->u.rx_st_band.phy_rx_fcs_err_cnt_cck,
				st->u.rx_st_band.phy_rx_fcs_err_cnt_ofdm,
				st->u.rx_st_band.phy_rx_pd_cck,
				st->u.rx_st_band.phy_rx_pd_ofdm,
				st->u.rx_st_band.phy_rx_sig_err_cck,
				st->u.rx_st_band.phy_rx_sfd_err_cck,
				st->u.rx_st_band.phy_rx_sig_err_ofdm,
				st->u.rx_st_band.phy_rx_tag_err_ofdm,
				st->u.rx_st_band.phy_rx_mdrdy_cnt_cck,
				st->u.rx_st_band.phy_rx_mdrdy_cnt_ofdm,
				all_per);
			break;

		case SERV_RX_STAT_TYPE_PATH:
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"\trcpi: %d\n"
				"\trssi:%d\n"
				"\tfagc_ib_rssi: %d\n"
				"\tfagc_wb_rssi: %d\n"
				"\tinst_ib_rssi: %d\n"
				"\tinst_wb_rssi: %d\n"
				"\tfagc_rssi_adc: %d\n",
				st->u.rx_st_path.rcpi,
				st->u.rx_st_path.rssi,
				st->u.rx_st_path.fagc_ib_rssi,
				st->u.rx_st_path.fagc_wb_rssi,
				st->u.rx_st_path.inst_ib_rssi,
				st->u.rx_st_path.inst_wb_rssi,
				st->u.rx_st_path.fagc_rssi_adc);
			break;

		case SERV_RX_STAT_TYPE_USER:
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"\tfreq_offset_from_rx: %u\n"
				"\tsnr: %d\n"
				"\tfcs_error_cnt: %u\n",
				st->u.rx_st_user.freq_offset_from_rx,
				st->u.rx_st_user.snr,
				st->u.rx_st_user.fcs_error_cnt);
			break;

		case SERV_RX_STAT_TYPE_COMM:
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"\trx_fifo_full: %u\n"
				"\taci_hit_low: %u\n"
				"\taci_hit_high: %u\n"
				"\tmu_pkt_count: %u\n"
				"\tsig_mcs: %u\n"
				"\tsinr: %d\n"
				"\tdriver_rx_count: %u\n",
				st->u.rx_st_comm.rx_fifo_full,
				st->u.rx_st_comm.aci_hit_low,
				st->u.rx_st_comm.aci_hit_high,
				st->u.rx_st_comm.mu_pkt_count,
				st->u.rx_st_comm.sig_mcs,
				st->u.rx_st_comm.sinr,
				st->u.rx_st_comm.driver_rx_count);
			break;

		}
	}
	return SERV_STATUS_SUCCESS;
}

static s_int32 update_hqa_frame(
	struct hqa_frame_ctrl *hqa_frame_ctrl, s_int32 length, s_int32 status)
{
	if (hqa_frame_ctrl->type == 0) {
		hqa_frame_ctrl->hqa_frame_eth->length = SERV_OS_HTONS((length));
		status = SERV_OS_HTONS((status));
	} else {
		hqa_frame_ctrl->hqa_frame_eth->length = length;
	}
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data, &status, 2);
	return SERV_STATUS_SUCCESS;
}

static s_int32 legacy_function(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	/*
	 * Legacy function means older chips use only,
	 * but not support in following CONNAC2 projects.
	 */
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");
	update_hqa_frame(hqa_frame_ctrl, 2, SERV_STATUS_SUCCESS);
	return SERV_STATUS_AGENT_NOT_SUPPORTED;
}

static s_int32 todo_function(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_WARN, "\n");
	update_hqa_frame(hqa_frame_ctrl, 2, SERV_STATUS_SUCCESS);
	return SERV_STATUS_AGENT_NOT_SUPPORTED;
}

static s_int32 deprecated_function(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	/*
	 * Legacy function means older chips use only,
	 * but not support in following CONNAC2 projects.
	 */
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");
	update_hqa_frame(hqa_frame_ctrl, 2, SERV_STATUS_SUCCESS);
	return SERV_STATUS_AGENT_NOT_SUPPORTED;
}

static s_int32 hqa_open_adapter(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct _net_testmode_chip_info chip_info;
	struct service_test *serv = serv_test;
	u_int8 u1band_num = 0;
	u_int8 band_idx = 0;

	ret = mt_serv_get_chip_info(serv_test, &chip_info);
	u1band_num = chip_info.u1chip_band_num;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR, "u1band_num:%d, band_idx:%d\n",
		u1band_num, serv_test->ctrl_band_idx);

	for (band_idx = 0; band_idx < u1band_num; band_idx++) {
		serv = serv_test;
		ret = mt_serv_get_serv_test_by_band(&serv, band_idx);
		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"ret:%x band_idx:%d\n", ret, band_idx);

			/* qatool need success for this api handle */
			ret = SERV_STATUS_SUCCESS;
			continue;
		}

		ret = mt_serv_start(serv);
	}

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

s_int32 hqa_serv_start(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_frame_ctrl hqa_cmd;

	sys_ad_zero_mem(&hqa_cmd, sizeof(hqa_cmd));

	ret = sys_ad_alloc_mem((pu_char *)&(hqa_cmd.hqa_frame_eth),
			sizeof(struct hqa_frame));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");

		return ret;
	}
	sys_ad_zero_mem(hqa_cmd.hqa_frame_eth, sizeof(struct hqa_frame));

	ret = hqa_open_adapter(serv_test, &hqa_cmd);

	sys_ad_free_mem(hqa_cmd.hqa_frame_eth);

	return ret;
}

static s_int32 hqa_close_adapter(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct _net_testmode_chip_info chip_info;
	struct service_test *serv = serv_test;
	u_int8 u1band_num = 0;
	u_int8 band_idx = 0;

	ret = mt_serv_get_chip_info(serv_test, &chip_info);
	u1band_num = chip_info.u1chip_band_num;

	for (band_idx = 0; band_idx < u1band_num; band_idx++) {
		serv = serv_test;
		ret = mt_serv_get_serv_test_by_band(&serv, band_idx);
		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"ret:%x band_idx:%d\n", ret, band_idx);

			/* qatool need success for this api handle */
			ret = SERV_STATUS_SUCCESS;
			continue;
		}
		ret = mt_serv_stop(serv);
	}

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

s_int32 hqa_serv_stop(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_frame_ctrl hqa_cmd;

	sys_ad_zero_mem(&hqa_cmd, sizeof(hqa_cmd));

	ret = sys_ad_alloc_mem((pu_char *)&(hqa_cmd.hqa_frame_eth),
			sizeof(struct hqa_frame));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");

		return ret;
	}
	sys_ad_zero_mem(hqa_cmd.hqa_frame_eth, sizeof(struct hqa_frame));

	hqa_close_adapter(serv_test, &hqa_cmd);

	sys_ad_free_mem(hqa_cmd.hqa_frame_eth);

	return ret;
}

static s_int32 hqa_get_chip_info(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct _net_testmode_chip_info testmode_chip_info;
	u_int8 u1band_idx = 0, u1band_num = 0;
	u_int8 au1used_band[3] = {0};
	u_int8 u1used_band = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	ret = mt_serv_get_chip_info(serv_test, &testmode_chip_info);

	u1band_num = testmode_chip_info.u1chip_band_num;

	for (u1band_idx = 0; u1band_idx < u1band_num; u1band_idx++) {
		au1used_band[u1band_idx] = testmode_chip_info.au1used_band[u1band_idx];
		u1used_band = au1used_band[u1band_idx];
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
			"band %d is %s\n", u1band_idx, (u1used_band ? "used":"unused"));
	}

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_tx_path(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS, value = 0;
	u_char band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 tx_ant = 0;
	u_int16 length = 0;
	struct test_wlan_info *winfos = serv_test->test_winfo;
	struct serv_chip_cap *cap = &winfos->chip_cap;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	length = hqa_frame_ctrl->hqa_frame_eth->length;


	if (length >= 2) {
		/* new format with band index,
		 * and data length extedned to 8 bytes
		 */
		/* tx path in bitwise */
		get_param_and_shift_buf(TRUE, sizeof(value),
				(u_char *)&value, hqa_frame_ctrl);
		tx_ant = value;
		/* band index */
		get_param_and_shift_buf(TRUE, sizeof(value),
				(u_char *)&value, hqa_frame_ctrl);
		band_idx = value;

		if ((tx_ant & TEST_ANT_USER_DEF) == 0) {
			if (!cap->is_dual_phy) {
				if (band_idx && tx_ant > 0x3)
					tx_ant >>= 2;
			}
		} else
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"user_tx_path:0x%x, band:%d\n",
				tx_ant, band_idx);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"tx_path:0x%x, band:%d\n",
			tx_ant, band_idx);
	} else {
		/* legacy command format,
		 * data length is 2 bytes without band index
		 */
		get_param_and_shift_buf(TRUE, sizeof(tx_ant),
				(u_char *)&tx_ant, hqa_frame_ctrl);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"tx_path:0x%x\n", tx_ant);
	}

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx:%d,  tx_ant:%d error!!\n",
			band_idx, tx_ant);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, tx_ant, tx_ant);

	ret = mt_serv_set_tx_path(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_rx_path(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS, value = 0;
	u_char band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int16 rx_ant = 0;
	u_int16 length = 0;
	struct test_wlan_info *winfos = serv_test->test_winfo;
	struct serv_chip_cap *cap = &winfos->chip_cap;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");
	length = hqa_frame_ctrl->hqa_frame_eth->length;
	if (length >= 2) {
		/* new format with band index,
		 * and data length extedned to 8 bytes
		 */
		/* rx path in bitwise */
		get_param_and_shift_buf(TRUE, sizeof(value),
				(u_char *)&value, hqa_frame_ctrl);
		rx_ant = value;
		/* band index */
		get_param_and_shift_buf(TRUE, sizeof(value),
				(u_char *)&value, hqa_frame_ctrl);
		band_idx = value;

		if (!cap->is_dual_phy) {
			if ((band_idx == 1) && rx_ant > 0x3)
				rx_ant >>= 2;
		}

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"rx_path:%d, band:%d\n",
			rx_ant, band_idx);
	} else {
		/* legacy command format,
		 * data length is 2 bytes without band index
		 */
		get_param_and_shift_buf(TRUE, sizeof(rx_ant),
				(u_char *)&rx_ant, hqa_frame_ctrl);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"rx_path:%d\n", rx_ant);
	}

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx: %d error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}


	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, rx_ant, rx_ant);

	ret = mt_serv_set_rx_path(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_tx_power_ext(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 power = 0, band_idx = 0, channel = 0;
	u_int32 ch_band = 0, ant_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(power),
				(u_char *)&power, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(channel),
				(u_char *)&channel, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(ch_band),
				(u_char *)&ch_band, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(ant_idx),
				(u_char *)&ant_idx, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx: %d error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, pwr_param.ant_idx,
			(u_int32)ant_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.power,
			(u_int32)power);
	CONFIG_SET_PARAM(serv_test, pwr_param.channel,
			(u_int32)channel);
	CONFIG_SET_PARAM(serv_test, pwr_param.band_idx,
			(u_int32)band_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.ch_band,
			(u_int32)ch_band);

	ret = mt_serv_tx_power_operation(serv_test, SERV_TEST_TXPWR_SET_PWR);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"power=%u, band_idx=%u, channel=%u\n",
		power, band_idx, channel);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"ch_band=%u, ant_idx=%u\n",
		ch_band, ant_idx);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_get_test_engine(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 funcId = 0, data = 0, ret_data = 0;
	struct test_wlan_info *winfos = serv_test->test_winfo;

	get_param_and_shift_buf(TRUE, sizeof(funcId),
		(u_char *)&funcId, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(data),
		(u_char *)&data, hqa_frame_ctrl);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"funcId: %d, value: %d\n", funcId, data);

	ret = net_ad_get(winfos, funcId, data, &ret_data);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"funcId:0x%x, data:0x%x, ret_data: 0x%x\n",
		funcId, data, ret_data);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_test_engine(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 funcId = 0, data = 0;
	struct test_wlan_info *winfos = serv_test->test_winfo;

	get_param_and_shift_buf(TRUE, sizeof(funcId),
		(u_char *)&funcId, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(data),
		(u_char *)&data, hqa_frame_ctrl);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"funcId: %d, value: %d\n", funcId, data);

	net_ad_set(winfos, funcId, data);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"funcId:0x%x, data:0x%x\n", funcId, data);
	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static struct hqa_cmd_entry CMD_SET0[] = {
	/* cmd id start from 0x1000 */
	{0x0,	hqa_open_adapter, "OpenAdapter"},
	{0x1,	hqa_close_adapter, "CloseAdapter"},
	{0x2,	hqa_get_chip_info, "GetChipInfo"},
	{0x3,	legacy_function, "TODO"},
	{0x4,	legacy_function, "TODO"},
	{0x6,	legacy_function, "TODO"},
	{0x7,	legacy_function, "TODO"},
	{0x8,	legacy_function, "TODO"},
	{0xa,	legacy_function, "TODO"},
	{0xb,	hqa_set_tx_path, "SetTxPath"},
	{0xc,	hqa_set_rx_path, "SetRxPath"},
	{0xd,	legacy_function, "TODO"},
	{0xe,	legacy_function, "TODO"},
	{0xf,	legacy_function, "TODO"},
	{0x10,	legacy_function, "TODO"},
	{0x11,	hqa_set_tx_power_ext, "SetTxPwrExt"},
	{0x14,	legacy_function, "TODO"},
	{0x15,	todo_function, "TODO"},
	{0x18,	legacy_function, "TODO"},
};

static s_int32 hqa_set_preamble(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 tx_mode = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type: Mode (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(tx_mode),
				(u_char *)&tx_mode, hqa_frame_ctrl);

	/* Set parameters */
	/*
	 * 000: Legacy CCK
	 * 001: Legacy OFDM
	 * 010: HT Mixed mode
	 * 011: HT Green field mode
	 * 100: VHT mode
	 */
	CONFIG_SET_PARAM(serv_test, tx_mode, (u_char)tx_mode);

	ret = mt_serv_set_preamble(serv_test);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"phy_mode=%u\n", tx_mode);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_rate(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 mcs = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type: MCS (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(mcs),
				(u_char *)&mcs, hqa_frame_ctrl);

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, mcs, (u_char)mcs);

	ret = mt_serv_set_rate(serv_test);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"mcs=%u\n", mcs);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_nss(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 nss = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type: NSS (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(nss),
				(u_char *)&nss, hqa_frame_ctrl);

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, nss, (u_char)nss);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"nss=%u\n", nss);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_system_bw(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 bw = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type: BW (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(bw),
				(u_char *)&bw, hqa_frame_ctrl);

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, bw, (u_char)bw);

	ret = mt_serv_set_system_bw(serv_test);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"bw=%u\n", bw);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_per_pkt_bw(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 per_pkt_bw = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type: Per-Pkt BW (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(per_pkt_bw),
				(u_char *)&per_pkt_bw, hqa_frame_ctrl);

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, per_pkt_bw, (u_char)per_pkt_bw);

	ret = mt_serv_set_per_pkt_bw(serv_test);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"per_pkt_bw=%u\n", per_pkt_bw);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_freq_offset(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 freq_offset = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type: freq offset (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(freq_offset),
				(u_char *)&freq_offset, hqa_frame_ctrl);

	/* for backward compatible, old format is not bandinfo */
	if (hqa_frame_ctrl->hqa_frame_eth->length == 12) {
		get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
			"band_idx:%d\n", band_idx);
	}

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx: %d error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, rf_freq_offset,
			(u_int32)freq_offset);

	ret = mt_serv_set_freq_offset(serv_test, SERV_FREQ_C1, band_idx);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"freq offset=%u band_idx:%d\n",
		freq_offset, band_idx);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_freq_offset_c2(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 freq_offset = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type: freq offset (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(freq_offset),
				(u_char *)&freq_offset, hqa_frame_ctrl);

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, rf_freq_offset,
			(u_int32)freq_offset);

	ret = mt_serv_set_freq_offset(serv_test, SERV_FREQ_C2, band_idx);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"freq offset=%u\n", freq_offset);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_low_power(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 control = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(control),
				(u_char *)&control, hqa_frame_ctrl);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"control=%u\n", control);

	/*
	 * 0: enter low power mode
	 * 1: leave low power mode
	*/
	ret = mt_serv_set_low_power(serv_test, control);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_get_antswap_capability(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 antswap_support = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR, "\n");

	ret = mt_serv_get_antswap_capability(serv_test, &antswap_support);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"antswap_support = %x\n", antswap_support);

	antswap_support = SERV_OS_HTONL(antswap_support);
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &antswap_support,
		sizeof(antswap_support));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(antswap_support), ret);

	return ret;
}

static s_int32 hqa_set_antswap(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band, ant = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&band, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&ant, hqa_frame_ctrl);

	ret = mt_serv_set_antswap(serv_test, ant);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_eeprom_to_fw(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;


	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	ret = mt_serv_set_eeprom_to_fw(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static struct hqa_cmd_entry CMD_SET1[] = {
	/* cmd id start from 0x1100 */
	{0x0,	legacy_function, "TODO"},
	{0x1,	deprecated_function, "SetPreamble"},
	{0x2,	deprecated_function, "SetRate"},
	{0x3,	deprecated_function, "SetNss"},
	{0x4,	deprecated_function, "SetSysBW"},
	{0x5,	deprecated_function, "SetPerPktBW"},
	{0x6,	legacy_function, "TODO"},
	{0x7,	hqa_set_freq_offset, "SetFreqOffset"},
	{0x9,	legacy_function, "TODO"},
	{0xb,	hqa_low_power, "LowPwr"},
	{0xd,	hqa_get_antswap_capability, "GetAntswapCap"},
	{0xc,	hqa_set_eeprom_to_fw, "SetEepromToFw"},
	{0xe,	hqa_set_antswap, "SetAntswap"},
#ifdef SEC_EFUSE_FILE_SUPPORT
	{0xf,	hqa_set_sec_efuse_content, "SetSecEfuseContent"},
#endif
	{0x10,	hqa_set_freq_offset_c2, "SetFreqOffsetC2"},
};

static s_int32 hqa_reset_txrx_counter(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	ret = mt_serv_reset_txrx_counter(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_cal_bypass(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 item = 0, band_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(item),
				(u_char *)&item, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	ret = mt_serv_set_cal_bypass(serv_test, item);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"item=%u, band_idx=%u\n", item, band_idx);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_rx_vector_idx(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = 0, group1 = 0, group2 = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(group1),
				(u_char *)&group1, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(group2),
				(u_char *)&group2, hqa_frame_ctrl);

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}


	ret = mt_serv_set_rx_vector_idx(serv_test, group1, group2);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_fagc_rssi_path(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = 0, fagc_path = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(fagc_path),
				(u_char *)&fagc_path, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx: %d error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}


	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, fagc_path, (u_char)fagc_path);

	ret = mt_serv_set_fagc_rssi_path(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static struct hqa_cmd_entry CMD_SET2[] = {
	/* cmd id start from 0x1200 */
	{0x0,	hqa_reset_txrx_counter, "ResetTxRxCounter"},
//	{0x2,	hqa_get_rx_ok_data, "GetRxOkData"},
	{0x5,	legacy_function, "TODO"},
	{0x8,	hqa_cal_bypass, "CalBypass"},
	{0x9,	hqa_set_rx_vector_idx, "SetRxVecIdx"},
	{0xa,	hqa_set_fagc_rssi_path, "SetFagcRssiPath"}
};

static s_int32 hqa_mac_bbp_reg_read(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_register *test_regs = &serv_test->test_reg;
	u_int32 cr_val;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&test_regs->cr_addr, hqa_frame_ctrl);

	test_regs->cr_num = 1;

	/* Allocate cr_val memory */
	ret = sys_ad_alloc_mem((pu_char *)&test_regs->cr_val, sizeof(u_int32));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame_ctrl, 2, ret);

		return ret;
	}
	sys_ad_zero_mem(test_regs->cr_val, sizeof(u_int32));

	ret = mt_serv_reg_eprm_operation(serv_test, SERV_TEST_REG_MAC_READ);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"cr_addr=0x%08x, cr_val=0x%08x\n", test_regs->cr_addr, *test_regs->cr_val);

	cr_val = SERV_OS_HTONL(*test_regs->cr_val);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &cr_val, sizeof(cr_val));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(cr_val), ret);

	/* Free cr_val memory */
	sys_ad_free_mem(test_regs->cr_val);

	return ret;
}

static s_int32 hqa_mac_bbp_reg_write(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_register *test_regs = &serv_test->test_reg;
	u_int32 cr_val = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&test_regs->cr_addr, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&cr_val, hqa_frame_ctrl);

	/* Allocate cr_val memory */
	ret = sys_ad_alloc_mem((pu_char *)&test_regs->cr_val, sizeof(u_int32));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame_ctrl, 2, ret);

		return ret;
	}
	sys_ad_move_mem(test_regs->cr_val, &cr_val, sizeof(cr_val));

	ret = mt_serv_reg_eprm_operation(serv_test, SERV_TEST_REG_MAC_WRITE);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"cr_addr=0x%08x, cr_val=0x%08x\n", test_regs->cr_addr, *test_regs->cr_val);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	/* Free cr_val memory */
	sys_ad_free_mem(test_regs->cr_val);

	return ret;
}

static s_int32 hqa_mac_bbp_reg_bulk_read(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int16 cr_seq = 0;
	struct test_register *test_regs = &serv_test->test_reg;
	u_int32 cr_total_len, cr_value = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(test_regs->cr_addr),
				(u_char *)&test_regs->cr_addr, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(test_regs->cr_num),
				(u_char *)&test_regs->cr_num, hqa_frame_ctrl);

	/* Allocate cr_val memory */
	cr_total_len = test_regs->cr_num << 2;
	ret = sys_ad_alloc_mem((pu_char *)&test_regs->cr_val, cr_total_len);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame_ctrl, 2, ret);

		return ret;
	}
	sys_ad_zero_mem(test_regs->cr_val, cr_total_len);

	if (test_regs->cr_num > 371) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"requested len is longer, make it smaller\n");
		ret = SERV_STATUS_AGENT_INVALID_LEN;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame_ctrl, 2, ret);

		return ret;
	}

	ret = mt_serv_reg_eprm_operation(serv_test,
					SERV_TEST_REG_MAC_READ_BULK);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"cr_addr=0x%08x, cr_num(unit: 4bytes)=%d\n",
		test_regs->cr_addr, test_regs->cr_num);

	for (cr_seq = 0; cr_seq < test_regs->cr_num; cr_seq++) {
		cr_value = test_regs->cr_val[cr_seq];
		test_regs->cr_val[cr_seq] = SERV_OS_HTONL(cr_value);
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"cr_val = 0x%08x\n", *test_regs->cr_val);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, test_regs->cr_val, cr_total_len);
	update_hqa_frame(hqa_frame_ctrl, 2 + cr_total_len, ret);

	/* Free cr_val memory */
	sys_ad_free_mem(test_regs->cr_val);

	return ret;
}

static s_int32 hqa_rf_reg_bulk_read(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int16 cr_seq = 0;
	struct test_register *test_regs = &serv_test->test_reg;
	u_int32 cr_total_len, cr_value = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&test_regs->wf_sel, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&test_regs->cr_addr, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&test_regs->cr_num, hqa_frame_ctrl);

	if ((test_regs->cr_num == 0) || (test_regs->cr_num > 1000)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"requested len is longer, make cr_num smaller\n");
		ret = SERV_STATUS_AGENT_INVALID_LEN;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame_ctrl, 2, ret);

		return ret;
	}

	/* Allocate cr_val memory */
	cr_total_len = test_regs->cr_num << 2;

	if ((2 + (cr_total_len)) > sizeof(hqa_frame_ctrl->hqa_frame_eth->data)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"requested len is longer, make cr_num smaller\n");
		ret = SERV_STATUS_AGENT_INVALID_LEN;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame_ctrl, 2, ret);

		return ret;
	}

	ret = sys_ad_alloc_mem((pu_char *)&test_regs->cr_val, cr_total_len);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame_ctrl, 2, ret);

		return ret;
	}
	sys_ad_zero_mem(test_regs->cr_val, cr_total_len);

	ret = mt_serv_reg_eprm_operation(serv_test,
					SERV_TEST_REG_RF_READ_BULK);

	for (cr_seq = 0; cr_seq < test_regs->cr_num; cr_seq++) {
		cr_value = test_regs->cr_val[cr_seq];
		test_regs->cr_val[cr_seq] = SERV_OS_HTONL(cr_value);
	}
	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, test_regs->cr_val, cr_total_len);
	update_hqa_frame(hqa_frame_ctrl, 2 + cr_total_len, ret);

	/* Free cr_val memory */
	sys_ad_free_mem(test_regs->cr_val);

	return ret;
}

static s_int32 hqa_rf_reg_bulk_write(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_register *test_regs = &serv_test->test_reg;
	u_int32 idx, cr_total_len;
	u_int32 *dst;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&test_regs->wf_sel, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&test_regs->cr_addr, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&test_regs->cr_num, hqa_frame_ctrl);

	if (test_regs->cr_num == 0 || (test_regs->cr_num > 1000)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");
		ret = SERV_STATUS_AGENT_INVALID_PARAM;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame_ctrl, 2, ret);

		return ret;
	}

	/* Allocate cr_val memory */
	cr_total_len = test_regs->cr_num << 2;

	if ((2 + (cr_total_len)) > sizeof(hqa_frame_ctrl->hqa_frame_eth->data)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"requested len is longer, make it smaller\n");
		ret = SERV_STATUS_AGENT_INVALID_LEN;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame_ctrl, 2, ret);

		return ret;
	}
	ret = sys_ad_alloc_mem((pu_char *)&test_regs->cr_val, cr_total_len);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame_ctrl, 2, ret);

		return ret;
	}

	for (idx = 0; idx < test_regs->cr_num; idx++) {
		dst = test_regs->cr_val + idx;
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *) dst, hqa_frame_ctrl);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"cr_addr=0x%08x, cr_val=0x%08x\n",
			test_regs->cr_addr, *dst);
	}

	ret = mt_serv_reg_eprm_operation(serv_test,
					SERV_TEST_REG_RF_WRITE_BULK);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	/* Free cr_val memory */
	sys_ad_free_mem(test_regs->cr_val);

	return ret;
}

static s_int32 hqa_read_eeprom(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_int16 value;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(u_int16),
				(u_char *)&test_eprms->offset, hqa_frame_ctrl);

	/* Allocate value memory */
	ret = sys_ad_alloc_mem((pu_char *)&test_eprms->value, sizeof(u_int16));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate eeprom memory fail\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame_ctrl, 2, ret);

		return ret;
	}
	sys_ad_zero_mem(test_eprms->value, sizeof(u_int16));

	ret = mt_serv_reg_eprm_operation(serv_test, SERV_TEST_EEPROM_READ);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"offset=0x%04x, value=0x%04x\n",
		test_eprms->offset, *test_eprms->value);

	value = SERV_OS_HTONS(*test_eprms->value);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &value, sizeof(value));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(value), ret);

	/* Free value memory */
	sys_ad_free_mem(test_eprms->value);

	return ret;
}

static s_int32 hqa_write_eeprom(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_int16 value = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(u_int16),
				(u_char *)&test_eprms->offset, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int16),
				(u_char *)&value, hqa_frame_ctrl);

	/* Allocate value memory */
	ret = sys_ad_alloc_mem((pu_char *)&test_eprms->value, sizeof(u_int16));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate eeprom memory fail\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame_ctrl, 2, ret);

		return ret;
	}
	sys_ad_move_mem(test_eprms->value, &value, sizeof(value));

	ret = mt_serv_reg_eprm_operation(serv_test, SERV_TEST_EEPROM_WRITE);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"offset=0x%04x, value=0x%04x\n",
		test_eprms->offset, *test_eprms->value);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	/* Free value memory */
	sys_ad_free_mem(test_eprms->value);

	return ret;
}

static void memcpy_eeprom(u_char *dst, u_char *src, u_long len)
{
	u_long i;
	u_short *p_dst, *p_src;

	p_dst = (u_short *) dst;
	p_src = (u_short *) src;

	for (i = 0; i < (len >> 1); i++) {
		*p_dst = SERV_OS_NTOHS(*p_src);
		p_dst++;
		p_src++;
	}

	if ((len % 2) != 0) {
		sys_ad_move_mem(p_dst, p_src, (len % 2));
		*p_dst = SERV_OS_NTOHS(*p_dst);
	}
}

static s_int32 hqa_read_bulk_eeprom(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_int32 eeprom_size;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	eeprom_size = serv_test->test_winfo->chip_cap.efuse_size;
	if (test_eprms->offset + test_eprms->length <= eeprom_size) {
		/* Allocate value memory */
		ret = sys_ad_alloc_mem((pu_char *)&test_eprms->value,
				       test_eprms->length);
		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				 "allocate eeprom memory fail\n");
			ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

			/* Update hqa_frame with response: status (2 bytes) */
			update_hqa_frame(hqa_frame_ctrl, 2, ret);

			return ret;
		}

		ret = mt_serv_reg_eprm_operation(serv_test,
						SERV_TEST_EEPROM_READ_BULK);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"offset=0x%04x, length=%d\n",
			 test_eprms->offset, test_eprms->length);

		/* Update hqa_frame with response: status (2 bytes) */
		memcpy_eeprom(hqa_frame_ctrl->hqa_frame_eth->data + 2,
			      (u_char *)test_eprms->value, test_eprms->length);

	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"exceed eeprom size (offset=0x%04x, size=%d)\n",
			test_eprms->offset+test_eprms->length,
			eeprom_size);
		test_eprms->length = 0;
		ret = SERV_STATUS_AGENT_INVALID_LEN;
	}
	update_hqa_frame(hqa_frame_ctrl, 2 + test_eprms->length, ret);

	/* Free value memory */
	sys_ad_free_mem(test_eprms->value);

	return ret;
}

static s_int32 hqa_read_bulk_eeprom_v1(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_short w_offset = 0, w_length = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(u_short),
				(u_char *)&w_offset, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_short),
				(u_char *)&w_length, hqa_frame_ctrl);
	test_eprms->offset = w_offset;
	test_eprms->length = w_length;

	return hqa_read_bulk_eeprom(serv_test, hqa_frame_ctrl);
}

static s_int32 hqa_read_bulk_eeprom_v2(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_int dw_offset = 0, dw_length = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(u_int),
				(u_char *)&dw_offset, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int),
				(u_char *)&dw_length, hqa_frame_ctrl);
	test_eprms->offset = dw_offset;
	test_eprms->length = dw_length;

	return hqa_read_bulk_eeprom(serv_test, hqa_frame_ctrl);
}

static s_int32 hqa_write_bulk_eeprom(
	struct service_test *serv_test, u_char *data)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_int32 eeprom_size;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"offset: %u, length: %u\n",
			test_eprms->offset, test_eprms->length);

	eeprom_size = serv_test->test_winfo->chip_cap.efuse_size;
	if (test_eprms->offset + test_eprms->length <= eeprom_size) {
		/* Allocate value memory */
		ret = sys_ad_alloc_mem((pu_char *)&test_eprms->value,
				       test_eprms->length);
		if (ret)
			goto err_out;

		memcpy_eeprom((u_char *)test_eprms->value,
			      data, test_eprms->length);

		ret = mt_serv_reg_eprm_operation(serv_test,
						SERV_TEST_EEPROM_WRITE_BULK);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"offset=0x%04x, length=%d\n",
			 test_eprms->offset, test_eprms->length);
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"exceed eeprom size (offset=0x%04x, size=%d)\n",
			test_eprms->offset+test_eprms->length,
			eeprom_size);
		test_eprms->length = 0;
		ret = SERV_STATUS_AGENT_INVALID_LEN;
	}

err_out:
	if (test_eprms->value) {
		/* Free value memory */
		sys_ad_free_mem(test_eprms->value);
	}

	return ret;
}

static s_int32 hqa_write_bulk_eeprom_v1(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_short w_offset = 0, w_length = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(u_short),
				(u_char *)&w_offset, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_short),
				(u_char *)&w_length, hqa_frame_ctrl);
	test_eprms->offset = w_offset;
	test_eprms->length = w_length;
	ret = hqa_write_bulk_eeprom(serv_test,
		&hqa_frame_ctrl->hqa_frame_eth->data[sizeof(u_short)*2]);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_write_bulk_eeprom_v2(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_int dw_offset = 0, dw_length = 0;

	/* offset is 4-byte for v2 */
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(u_int),
				(u_char *)&dw_offset, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int),
				(u_char *)&dw_length, hqa_frame_ctrl);
	test_eprms->offset = dw_offset;
	test_eprms->length = dw_length;

	ret = hqa_write_bulk_eeprom(serv_test,
		&hqa_frame_ctrl->hqa_frame_eth->data[sizeof(u_int)*2]);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_check_efuse_mode(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 efuse_mode;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	if (WINFO_GET_PARAM(serv_test, use_efuse))
		efuse_mode = 1;
	else
		efuse_mode = 0;

	efuse_mode = SERV_OS_HTONL(efuse_mode);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"efuse_mode=%u\n", efuse_mode);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &efuse_mode, sizeof(efuse_mode));
	update_hqa_frame(hqa_frame_ctrl, 6, ret);

	return ret;
}

static s_int32 hqa_get_free_efuse_block(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 free_block, total_bank = 0, temp = 0;
	u_char	version = 0, die_index = 0;
	u_int8  format_ver = 0, i = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(u_char),
				(u_char *)&version, hqa_frame_ctrl);

	get_param_and_shift_buf(TRUE, sizeof(u_char),
				(u_char *)&die_index, hqa_frame_ctrl);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"version=%u die index=%u\n", version, die_index);

	EEPROM_SET_PARAM(serv_test, efuse_free_block, die_index);

	ret = mt_serv_reg_eprm_operation(serv_test,
					SERV_TEST_EEPROM_GET_FREE_EFUSE_BLOCK);

	free_block = EEPROM_GET_PARAM(serv_test, efuse_free_block);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"efuse_free_block=%u\n", free_block);

	free_block = SERV_OS_HTONL(free_block);

	format_ver = EEPROM_GET_PARAM(serv_test, free_block_format_ver);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"format_ver=%u\n", format_ver);
	if (format_ver == 2) {
		total_bank = EEPROM_GET_PARAM(serv_test, efuse_total_bank_num);
		temp = SERV_OS_HTONL(total_bank);
		/* Update hqa_frame with response: status (2 bytes) */
		sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2,
						&temp, sizeof(temp));
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"total_bank=%u\n", total_bank);

		for (i = 0; i < total_bank; i++) {
			temp = SERV_OS_HTONL(serv_test->test_eprm.free_block_v2[i]);
			sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data +
				2 + 4 + i * 3 * sizeof(u_int32), &temp, sizeof(temp));
			temp = SERV_OS_HTONL(serv_test->test_eprm.total_block_v2[i]);
			sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data +
				2 + 4 + (i * 3 + 1) * sizeof(u_int32), &temp, sizeof(temp));
			temp = SERV_OS_HTONL(serv_test->test_eprm.efuse_num_v2[i]);
			sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data +
				2 + 4 + (i * 3 + 2) * sizeof(u_int32), &temp, sizeof(temp));
		}
		/* Update hqa_frame with response: status (2 bytes) + bank count + n x 12*/
		update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(u_int32) + total_bank * (sizeof(u_int32) * 3), ret);
	} else {
		/* Update hqa_frame with response: status (2 bytes) */
		sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &free_block, sizeof(free_block));
		update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(free_block), ret);
	}

	return ret;
}

static s_int32 hqa_get_tx_power(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 power = 0, band_idx = 0, channel = 0;
	u_int32 ch_band = 0, ant_idx = 0, efuse_offset = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(channel),
				(u_char *)&channel, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(ch_band),
				(u_char *)&ch_band, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(ant_idx),
				(u_char *)&ant_idx, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx: %d error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}


	/* set parameters */
	CONFIG_SET_PARAM(serv_test, pwr_param.ant_idx,
			(u_int32)ant_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.channel,
			(u_int32)channel);
	CONFIG_SET_PARAM(serv_test, pwr_param.band_idx,
			(u_int32)band_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.ch_band,
			(u_int32)ch_band);

	ret = mt_serv_tx_power_operation(serv_test, SERV_TEST_TXPWR_GET_PWR);

	power = CONFIG_GET_PARAM(serv_test, pwr_param.power);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"power=%u, band_idx=%u, channel=%u\n",
		power, band_idx, channel);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"ch_band=%u, ant_idx=%u\n",
		ch_band, ant_idx);

	/* update hqa_frame with response: status (2 bytes) */
	efuse_offset = SERV_OS_HTONL(efuse_offset);
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &efuse_offset,
			sizeof(efuse_offset));
	power = SERV_OS_HTONL(power);
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2 + 4, &power, sizeof(power));
	update_hqa_frame(hqa_frame_ctrl,
		2 + sizeof(power) + sizeof(efuse_offset), ret);

	return ret;
}

static s_int32 hqa_set_cfg_on_off(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 type = 0, enable = 0, band_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(type),
				(u_char *)&type, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(enable),
				(u_char *)&enable, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%u, error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}


	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, log_type, (u_char)type);
	CONFIG_SET_PARAM(serv_test, log_enable, (u_char)enable);

	ret = mt_serv_set_cfg_on_off(serv_test);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"type=%u, enable=%u, band_idx=%u\n",
		type, enable, band_idx);

	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_get_freq_offset(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 freq_offset = 0, band_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"get freq offset, band_idx: %d\n", band_idx);

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
					(u_char *)&band_idx, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx=%d, error\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	ret = mt_serv_get_freq_offset(serv_test, &freq_offset, band_idx);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"freq offset: %d\n", freq_offset);

	/* update hqa_frame with response: status (2 bytes) */
	freq_offset = SERV_OS_HTONL(freq_offset);
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &freq_offset, sizeof(freq_offset));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(freq_offset), ret);

	return ret;
}

static s_int32 hqa_get_freq_offset_c2(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 freq_offset = 0, band_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"get freq offset, band_idx: %d\n", band_idx);

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
					(u_char *)&band_idx, hqa_frame_ctrl);


	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx=%d, error\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	ret = mt_serv_get_freq_offset(serv_test, &freq_offset, band_idx);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"freq offset: %d\n", freq_offset);

	/* update hqa_frame with response: status (2 bytes) */
	freq_offset = SERV_OS_HTONL(freq_offset);
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &freq_offset, sizeof(freq_offset));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(freq_offset), ret);

	return ret;
}

static s_int32 hqa_dbdc_tx_tone(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_tx_tone param;

	memset(&param, 0, sizeof(struct hqa_tx_tone));
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(param.band_idx),
				(u_char *)&param.band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.tx_tone_en),
				(u_char *)&param.tx_tone_en, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.ant_idx),
				(u_char *)&param.ant_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.tone_type),
				(u_char *)&param.tone_type, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.tone_freq),
				(u_char *)&param.tone_freq, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.dc_offset_I),
				(u_char *)&param.dc_offset_I, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.dc_offset_Q),
				(u_char *)&param.dc_offset_Q, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.band),
				(u_char *)&param.band, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.rf_pwr),
				(u_char *)&param.rf_pwr, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.digi_pwr),
				(u_char *)&param.digi_pwr, hqa_frame_ctrl);

	if (param.band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx=%d, error\n", param.band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, param.band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, tx_tone_en,
			(u_int32)param.tx_tone_en);
	CONFIG_SET_PARAM(serv_test, ant_idx,
			(u_int32)param.ant_idx);
	CONFIG_SET_PARAM(serv_test, tone_type,
			(u_int32)param.tone_type);
	CONFIG_SET_PARAM(serv_test, tone_freq,
			(u_int32)param.tone_freq);
	CONFIG_SET_PARAM(serv_test, dc_offset_I,
			(u_int32)param.dc_offset_I);
	CONFIG_SET_PARAM(serv_test, dc_offset_Q,
			(u_int32)param.dc_offset_Q);
	CONFIG_SET_PARAM(serv_test, ch_band,
			(u_char)param.band);
	CONFIG_SET_PARAM(serv_test, rf_pwr,
			(u_int32)param.rf_pwr);
	CONFIG_SET_PARAM(serv_test, digi_pwr,
			(u_int32)param.digi_pwr);

	ret = mt_serv_dbdc_tx_tone(serv_test);
	if (ret != SERV_STATUS_SUCCESS)
		goto error;

	ret = mt_serv_dbdc_tx_tone_pwr(serv_test);

error:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"band_idx=%u, tx_tone_en=%u, ant_idx=0x%x\n",
		param.band_idx, param.tx_tone_en, param.ant_idx);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"tone_type=%u, tone_freq=%u, dc_offset_I=0x%x\n",
		param.tone_type, param.tone_freq,
		param.dc_offset_I);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"dc_offset_Q=%u, band=%u, rf_pwr=0x%x, digi_pwr=0x%x\n",
		param.dc_offset_Q, param.band, param.rf_pwr,
		param.digi_pwr);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_dbdc_continuous_tx(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_continuous_tx param;

	memset(&param, 0, sizeof(struct hqa_continuous_tx));
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(param.band_idx),
				(u_char *) &param.band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.tx_tone_en),
				(u_char *) &param.tx_tone_en, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.ant_mask),
				(u_char *) &param.ant_mask, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.tx_mode),
				(u_char *) &param.tx_mode, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.bw),
				(u_char *) &param.bw, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.pri_ch),
				(u_char *) &param.pri_ch, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.rate),
				(u_char *) &param.rate, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.central_ch),
				(u_char *) &param.central_ch, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.tx_fd_mode),
				(u_char *) &param.tx_fd_mode, hqa_frame_ctrl);

	if (param.band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx=%u, error!!\n", param.band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, param.band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, tx_tone_en,
			(u_int32)param.tx_tone_en);
	CONFIG_SET_PARAM(serv_test, ant_mask,
			(u_int32)param.ant_mask);
	CONFIG_SET_PARAM(serv_test, tx_mode,
			(u_char)param.tx_mode);
	CONFIG_SET_PARAM(serv_test, bw,
			(u_char)param.bw);
	CONFIG_SET_PARAM(serv_test, ctrl_ch,
			(u_char)param.pri_ch);
	CONFIG_SET_PARAM(serv_test, rate,
			(u_int32)param.rate);
	CONFIG_SET_PARAM(serv_test, channel,
			(u_char)param.central_ch);
	CONFIG_SET_PARAM(serv_test, tx_fd_mode,
			(u_int32)param.tx_fd_mode);

	ret = mt_serv_dbdc_continuous_tx(serv_test);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"param band_idx=%u, tx_tone_en=%u, ant_mask=0x%x\n",
		param.band_idx, param.tx_tone_en, param.ant_mask);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"param phy_mode=%u, bw=%u, pri_ch=%u\n",
		param.tx_mode, param.bw, param.pri_ch);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"param rate=%u, central_ch=%u, tx_fd_mode=%u\n",
		param.rate, param.central_ch, param.tx_fd_mode);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_rx_filter_pkt_len(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = 0, enable = 0, rx_pkt_len = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(enable),
				(u_char *)&enable, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(rx_pkt_len),
				(u_char *)&rx_pkt_len, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx=%u, error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, rx_filter_en,
			(u_int8)enable);
	CONFIG_SET_PARAM(serv_test, rx_filter_pkt_len,
			(u_int32)rx_pkt_len);

	ret = mt_serv_set_rx_filter_pkt_len(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_get_tx_info(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 tx_cnt = 0;
	u_int32 tx_cnt_size = 0;
	struct _net_testmode_chip_info testmode_chip_info;
	struct service_test *serv = serv_test;
	u_char band_num = 0, i = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	mt_serv_get_tx_info(serv_test);

	mt_serv_get_chip_info(serv_test, &testmode_chip_info);

	band_num = testmode_chip_info.u1chip_band_num;

	for (i = 0; i < band_num; i++) {
		tx_cnt = 0;
		serv = serv_test;
		if (mt_serv_get_serv_test_by_band(&serv, i) == SERV_STATUS_SUCCESS)
			tx_cnt =  CONFIG_GET_PARAM(serv, tx_stat.tx_done_cnt);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"tx_cnt = %u, band_idx = %d, band_num = %d\n",
			tx_cnt, i, band_num);

		tx_cnt = SERV_OS_HTONL(tx_cnt);

		sys_ad_move_mem((hqa_frame_ctrl->hqa_frame_eth->data + 2 + tx_cnt_size),
			&tx_cnt, sizeof(tx_cnt));

		tx_cnt_size += sizeof(tx_cnt);
	}

	update_hqa_frame(hqa_frame_ctrl,
		2 + tx_cnt_size, ret);

	return ret;
}

static s_int32 hqa_get_cfg_on_off(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 type = 0, band_idx = 0;
	u_int32 result = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(type),
				(u_char *)&type, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx=%u, error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);

		/* qatool need success for this api handle */
		ret = SERV_STATUS_SUCCESS;
		goto err;
	}
	ret = mt_serv_get_cfg_on_off(serv_test, band_idx, type, &result);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"type=%u, result=%u\n", type, result);

	/* Update hqa_frame with response: status (2 bytes) */
	result = SERV_OS_HTONL(result);

err:
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &result, sizeof(result));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(result), ret);

	return ret;
}

static s_int32 hqa_ca53_reg_read(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_register *test_regs = &serv_test->test_reg;
	u_long cr_val;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Use u_long here to get 4bytes firmly */
	get_param_and_shift_buf(TRUE, sizeof(u_long),
				(u_char *)&test_regs->cr_addr, hqa_frame_ctrl);

	/* Allocate cr_val memory */
	ret = sys_ad_alloc_mem((pu_char *)&test_regs->cr_val, sizeof(u_long));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame_ctrl, 2, ret);

		return ret;
	}
	sys_ad_zero_mem(test_regs->cr_val, sizeof(u_long));

	ret = mt_serv_reg_eprm_operation(serv_test, SERV_TEST_REG_CA53_READ);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"cr_addr=0x%08x, cr_val=0x%08x\n",
		test_regs->cr_addr, *test_regs->cr_val);

	cr_val = SERV_OS_HTONL(*test_regs->cr_val);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &cr_val, sizeof(cr_val));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(cr_val), ret);

	/* Free cr_val memory */
	sys_ad_free_mem(test_regs->cr_val);

	return ret;
}

static s_int32 hqa_ca53_reg_write(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_register *test_regs = &serv_test->test_reg;
	u_int32 cr_val = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Use u_long here to get 4bytes firmly */
	get_param_and_shift_buf(TRUE, sizeof(u_long),
				(u_char *)&test_regs->cr_addr, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&cr_val, hqa_frame_ctrl);

	/* Allocate cr_val memory */
	ret = sys_ad_alloc_mem((pu_char *)&test_regs->cr_val, sizeof(u_int32));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame_ctrl, 2, ret);

		return ret;
	}
	sys_ad_move_mem(test_regs->cr_val, &cr_val, sizeof(cr_val));

	ret = mt_serv_reg_eprm_operation(serv_test, SERV_TEST_REG_CA53_WRITE);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"cr_addr=0x%08x, cr_val=0x%08x\n",
		test_regs->cr_addr, *test_regs->cr_val);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	/* Free cr_val memory */
	sys_ad_free_mem(test_regs->cr_val);

	return ret;
}

static s_int32 hqa_get_tx_tone_pwr(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 power = 0, ant_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* request format type */
	get_param_and_shift_buf(TRUE, sizeof(ant_idx),
				(u_char *)&ant_idx, hqa_frame_ctrl);

	ret = mt_serv_get_tx_tone_pwr(serv_test, ant_idx, &power);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"ant_idx: %d, power: %d\n",
		ant_idx, power);

	/* update hqa_frame with response: status (2 bytes) */
	power = SERV_OS_HTONL(power);
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &power, sizeof(power));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(power), ret);

	return ret;
}

static struct hqa_cmd_entry CMD_SET3[] = {
	/* cmd id start from 0x1300 */
	{0x0,	hqa_mac_bbp_reg_read, "MacBbpRegRead"},
	{0x1,	hqa_mac_bbp_reg_write, "MacBbpRegWrite"},
	{0x2,	hqa_mac_bbp_reg_bulk_read, "MacBbpRegBulkRead"},
	{0x3,	hqa_rf_reg_bulk_read, "RfRegBulkRead"},
	{0x4,	hqa_rf_reg_bulk_write, "RfRegBulkWrite"},
	{0x5,	hqa_read_eeprom, "ReadEeprom"},
	{0x6,	hqa_write_eeprom, "WriteEeprom"},
	{0x7,	hqa_read_bulk_eeprom_v1, "ReadBulkEepromV1"},
	{0x8,	hqa_write_bulk_eeprom_v1, "WriteBulkEepromV1"},
	{0x9,	hqa_check_efuse_mode, "ChkEfuseMode"},
	{0xa,	hqa_get_free_efuse_block, "GetFreeEfuseBlock"},
	{0xd,	hqa_get_tx_power, "GetTxPwr"},
	{0xe,	hqa_set_cfg_on_off, "SetCfgOnOff"},
	{0xf,	hqa_get_freq_offset, "GetFreqOffset"},
	{0x10,	hqa_dbdc_tx_tone, "DBDCTxTone"},
	{0x11,	hqa_dbdc_continuous_tx, "DBDCContinuousTx"},
	{0x12,	hqa_set_rx_filter_pkt_len, "SetRxFilterPktLen"},
	{0x13,	hqa_get_tx_info, "GetTxInfo"},
	{0x14,	hqa_get_cfg_on_off, "GetCfgOnOff"},
	{0x15,	legacy_function, "TODO"},
//	{0x16,	hqa_set_buffer_bin, "SetBufferBin"},
	{0x17,	legacy_function, "TODO"},
	{0x18,	hqa_ca53_reg_read, "ca53RegRead"},
	{0x19,	hqa_ca53_reg_write, "ca53RegWrite"},
	{0x1a,	hqa_get_tx_tone_pwr, "GetTxTonePwr"},
	{0x1b,	hqa_read_bulk_eeprom_v2, "ReadBulkEepromV2"},
	{0x1c,	hqa_write_bulk_eeprom_v2, "WriteBulkEepromV2"},
	{0x1f,	hqa_get_freq_offset_c2, "GetFreqOffsetC2"},
};

static s_int32 hqa_get_thermal_val(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct service_test *serv = serv_test;
	u_int32 band_idx = 0;
	u_int32 sensorType = 0;
	u_int32 adc = 0, temp = 0, sensorCount = 0, reSenCnt = 0;
	u_int32 length = 0;
	u_int8 ddie_num = 0, index = 0;
	u_int8 *pu4SensorResult = NULL;
	struct get_temp_adc *temp_adc = NULL;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(sensorType),
				(u_char *)&sensorType, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	ddie_num = serv_test->test_winfo->chip_cap.ddie_lvts_num;

	ret = sys_ad_alloc_mem((pu_char *)&pu4SensorResult,
			sizeof(u_int8) + (sizeof(struct get_temp_adc) * ddie_num));

	if (ret != SERV_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate memory failed. ret:%d\n", ret);
		goto err0;
	}

	ret = mt_serv_get_thermal_val(serv, sensorType, band_idx, pu4SensorResult);

	if (ret != SERV_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"Fail to get thermal. ret:%d\n", ret);
		goto err1;
	}

	sensorCount = *pu4SensorResult;
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"Sensor:%u\n", sensorCount);

	if (ddie_num > 0 && sensorCount > 0)
		temp_adc = (struct get_temp_adc *)(pu4SensorResult + 1);


	reSenCnt = SERV_OS_HTONL(sensorCount);
	length += 2;
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + length, &reSenCnt, sizeof(sensorCount));
	length += sizeof(reSenCnt);

	if (sensorCount > ddie_num) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"sensorCount:%d, ret:%d\n", sensorCount, ret);
		goto err1;
	}

	for (index = 0 ; index < sensorCount && index < ddie_num; index++, temp_adc++) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"index:%u adc: %x temp:%x\n", index, temp_adc->adc, temp_adc->temp);

		/* update hqa_frame with response: status (2 bytes) */
		adc = temp_adc->adc;
		temp = temp_adc->temp;
		adc = SERV_OS_HTONL(adc);
		temp = SERV_OS_HTONL(temp);
		sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + length, &adc, sizeof(adc));
		length += sizeof(adc);
		sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + length, &temp, sizeof(temp));
		length += sizeof(temp);
	}

err1:
	if (pu4SensorResult != NULL)
		sys_ad_free_mem(pu4SensorResult);
err0:
	update_hqa_frame(hqa_frame_ctrl, length, ret);
	return ret;
}

static struct hqa_cmd_entry CMD_SET4[] = {
	/* cmd id start from 0x1400 */
	{0x1,	hqa_get_thermal_val, "GetThermalVal"}
};

static s_int32 hqa_get_fw_info(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct serv_fw_info *fw_info = NULL;
	u_char op_mode = 0;
	u_int8 loop, month = 0;
	u_char date[8], time[6];
	u_char *kernel_info = NULL;
	u_char *month_array[12] = {
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec",
	};

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	fw_info = WINFO_GET_PADDR(serv_test, wm_fw_info);
	sys_ad_zero_mem(&date[0], sizeof(date));
	sys_ad_zero_mem(&time[0], sizeof(time));

	/* Get information from kernel */
	for (loop = 0; loop < 12; loop++) {
		kernel_info = strstr(utsname()->version, month_array[loop]);

		if (kernel_info)
			break;
	}

	/* SW package build time */
	sys_ad_move_mem(&time[0], kernel_info + 7, 2);
	sys_ad_move_mem(&time[2], kernel_info + 10, 2);
	sys_ad_move_mem(&time[4], kernel_info + 13, 2);

	/* SW package build date */
	sys_ad_move_mem(&date[0], kernel_info + 20, 4);
	sys_ad_move_mem(&date[6], kernel_info + 4, 2);

	for (loop = 0; loop < 12; loop++) {
		if (sys_ad_cmp_mem(month_array[loop], kernel_info, 3) == 0) {
			month = loop + 1;
			break;
		}
	}

	date[4] = month / 10 % 10 + '0';
	date[5] = month % 10 + '0';

	/* Update hqa_frame with response: status (2 bytes) */
	/* The 1 byte for op_mode doesn't use anymore but still keep it */
	sys_ad_move_mem((hqa_frame_ctrl->hqa_frame_eth->data + 2), &op_mode, sizeof(op_mode));
	sys_ad_move_mem((hqa_frame_ctrl->hqa_frame_eth->data + 2 + sizeof(op_mode)),
			&date, sizeof(date));
	sys_ad_move_mem((hqa_frame_ctrl->hqa_frame_eth->data + 2 + sizeof(op_mode) + sizeof(date)),
			&time, sizeof(time));
	sys_ad_move_mem((hqa_frame_ctrl->hqa_frame_eth->data + 2 + sizeof(op_mode)
			+ sizeof(date) + sizeof(time)),
			&fw_info->ram_built_date[0],
			sizeof(fw_info->ram_built_date));
	update_hqa_frame(hqa_frame_ctrl, (2+1+8+6+15), ret);

	return ret;
}

static s_int32 hqa_set_stbc(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 stbc = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type: Enable (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(stbc),
				(u_char *)&stbc, hqa_frame_ctrl);

	if (hqa_frame_ctrl->hqa_frame_eth->length == 8) {
		get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
			"band_idx:%d\n", band_idx);
	}

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx=%u, error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, stbc, (u_char)stbc);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"stbc enable=%u\n", stbc);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_short_gi(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 sgi = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type: Enable (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(sgi),
				(u_char *)&sgi, hqa_frame_ctrl);

	if (hqa_frame_ctrl->hqa_frame_eth->length == 8) {
		get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
			"band_idx:%d\n", band_idx);
	}

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx=%u, error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, sgi, (u_char)sgi);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"sgi enable=%u\n", sgi);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_tx_pe(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	struct service_test *serv = serv_test;
	u_int32 max_pkt_ext = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type: Enable (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(max_pkt_ext),
				(u_char *)&max_pkt_ext, hqa_frame_ctrl);
	if (hqa_frame_ctrl->hqa_frame_eth->length > 4) {
		get_param_and_shift_buf(TRUE, sizeof(band_idx),
					(u_char *)&band_idx, hqa_frame_ctrl);
	}

	ret = mt_serv_get_serv_test_by_band(&serv, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv, max_pkt_ext, (u_char)max_pkt_ext);


	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"Tx pe=%u length:%u\n", max_pkt_ext, hqa_frame_ctrl->hqa_frame_eth->length);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}


static s_int32 hqa_set_fixed_payload(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 fixed_payload = 0, pay_load = 0;
	u_int32 band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	struct test_wlan_info *winfo = serv_test->test_winfo;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type: Enable (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(fixed_payload),
				(u_char *)&fixed_payload, hqa_frame_ctrl);

	if (fixed_payload == 0) {
		/* Set parameters for random */
		fixed_payload = 2;
	} else {
		fixed_payload = 1;
	}
	CONFIG_SET_PARAM(serv_test, fixed_payload,
			(u_char)fixed_payload);

	ret = kstrtou32(CONFIG_GET_PARAM(serv_test, payload), 16, &pay_load);
	if (ret != SERV_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"Covert fail!!\n");
		return SERV_STATUS_AGENT_FAIL;
	}

	net_ad_set(winfo, RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);
	net_ad_set(winfo, RF_AT_FUNCID_SET_PAYLOAD, ((fixed_payload << 16) | pay_load));


	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"fixed_payload =%u\n", fixed_payload);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_dpd(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 on_off = 0, wf_sel = 0;
	u_int32 band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(on_off),
				(u_char *)&on_off, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(wf_sel),
				(u_char *)&wf_sel, hqa_frame_ctrl);

	if (hqa_frame_ctrl->hqa_frame_eth->length == 12) {
		get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
			"band_idx:%d\n", band_idx);
	}

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx=%u, error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	ret = mt_serv_set_dpd(serv_test, on_off, wf_sel);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"on_off: %d, wf_sel: %d\n", on_off, wf_sel);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_tssi_onoff(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 on_off = 0, wf_sel = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(on_off),
				(u_char *)&on_off, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(wf_sel),
				(u_char *)&wf_sel, hqa_frame_ctrl);

	ret = mt_serv_set_tssi(serv_test, on_off, wf_sel);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"on_off: %d\n", on_off);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_get_rx_statistics_all(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	u_int32 buf_size = 0;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 type_mask = 0, band_idx = 0, type_num = 0, length;
	u_int32 blk_idx = 0, type_idx = 0, buf = 0;
	u_int32 dw_idx = 0, dw_cnt = 0;
	u_int32 *ptr2 = NULL;
	struct test_rx_stat_u *rx_stat = NULL;

	boolean dbdc_mode = FALSE;
	struct serv_chip_cap *chip_cap = NULL;
	u_int8 path[TEST_ANT_NUM] = {0};
	u_int8 path_len = 0, need_show = 0;
	u_int8 *ptr = NULL;
	struct hqa_rx_stat_resp_format st_form[SERV_RX_STAT_TYPE_NUM] = {
	 {SERV_RX_STAT_TYPE_BAND, 0, 0, 0,
		 sizeof(struct hqa_rx_stat_band_info)},
	 {SERV_RX_STAT_TYPE_PATH, 0, 0, 0,
		 sizeof(struct hqa_rx_stat_path_info)},
	 {SERV_RX_STAT_TYPE_USER, 0, 0, 0,
		 sizeof(struct hqa_rx_stat_user_info)},
	 {SERV_RX_STAT_TYPE_COMM, 0, 0, 0,
		 sizeof(struct hqa_rx_stat_comm_info)}
	};

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(type_mask),
				(u_char *)&type_mask, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"invalid band index=%u, error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* check wifi path combination for specific band */
	chip_cap = &serv_test->test_winfo->chip_cap;

	if (band_idx >= 3) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"invalid band index(%d).\n", band_idx);
		ret = SERV_STATUS_AGENT_INVALID_BANDIDX;
		goto error2;
	}

	/* show relative information */
	if (type_mask & BIT(8))
		need_show = 1;

	path_len = chip_cap->mcs_nss.max_path[1];
	ret = mt_serv_get_wf_path_comb(serv_test,
			band_idx,
			dbdc_mode,
			path,
			&path_len);

	if (ret != SERV_STATUS_SUCCESS)
		goto error2;

	/* TODO: check user combination for specific band  */
	/* host driver not support related utility */
	/* if need this function, need host driver support */


	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"path_len:%x, path:%x\n", path_len, path[band_idx]);


	/* update item mask for each type */
	st_form[SERV_RX_STAT_TYPE_BAND].item_mask = BIT(band_idx);
	for (blk_idx = 0; blk_idx < path_len; blk_idx++)
		st_form[SERV_RX_STAT_TYPE_PATH].item_mask |= BIT(path[blk_idx]);
	for (blk_idx = 0; blk_idx < TEST_USER_NUM; blk_idx++)
		st_form[SERV_RX_STAT_TYPE_USER].item_mask |= BIT(blk_idx);
	st_form[SERV_RX_STAT_TYPE_COMM].item_mask = BIT(0);

	/* update block count for each type */
	for (type_idx = SERV_RX_STAT_TYPE_BAND;
		type_idx < SERV_RX_STAT_TYPE_NUM; type_idx++) {
		for (blk_idx = 0; blk_idx < 32; blk_idx++) {
			if (st_form[type_idx].item_mask & BIT(blk_idx))
				st_form[type_idx].blk_cnt++;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"path.item_mask:%x, blk_cnt:%d\n",
			st_form[SERV_RX_STAT_TYPE_PATH].item_mask,
			st_form[SERV_RX_STAT_TYPE_PATH].blk_cnt);

	ptr = hqa_frame_ctrl->hqa_frame_eth->data + 2 + sizeof(type_num);

	/* allocate dynamic memory for rx stat info */
	ret = sys_ad_alloc_mem((pu_char *)&rx_stat,
			sizeof(struct test_rx_stat_u));
	if (ret != SERV_STATUS_SUCCESS)
		goto error1;

	for (type_idx = SERV_RX_STAT_TYPE_BAND;
			type_idx < SERV_RX_STAT_TYPE_NUM; type_idx++) {
		if (type_mask & BIT(type_idx)) {
			type_num++;
			length = st_form[type_idx].blk_cnt *
				st_form[type_idx].blk_size;

			/* fill in type */
			buf = SERV_OS_HTONL(st_form[type_idx].type);
			sys_ad_move_mem(ptr, &buf, sizeof(buf));
			ptr += sizeof(st_form[type_idx].type);
			buf_size += sizeof(st_form[type_idx].type);

			/* fill in version */
			buf = SERV_OS_HTONL(st_form[type_idx].version);
			sys_ad_move_mem(ptr, &buf, sizeof(buf));
			ptr += sizeof(st_form[type_idx].version);
			buf_size += sizeof(st_form[type_idx].version);

			/* fill in item mask */
			buf = SERV_OS_HTONL(st_form[type_idx].item_mask);
			sys_ad_move_mem(ptr, &buf, sizeof(buf));
			ptr += sizeof(st_form[type_idx].item_mask);
			buf_size += sizeof(st_form[type_idx].item_mask);

			/* fill in length */
			buf = SERV_OS_HTONL(length);
			sys_ad_move_mem(ptr, &buf, sizeof(buf));
			ptr += sizeof(length);
			buf_size += sizeof(length);

			for (blk_idx = 0; blk_idx < 32; blk_idx++) {
				if (st_form[type_idx].item_mask
						& BIT(blk_idx)) {
					/* service handle for rx stat info */
					mt_serv_get_rx_stat(serv_test,
						band_idx,
						blk_idx,
						type_idx,
						rx_stat);

					if (need_show)
						show_rx_statistics(type_idx, blk_idx, rx_stat);

					ptr2 = (u_int32 *) rx_stat;
					dw_cnt = st_form[type_idx].blk_size
						>> 2;
					for (dw_idx = 0; dw_idx < dw_cnt;
							dw_idx++, ptr2++,
							ptr += 4) {
						/* endian transform */
						buf = SERV_OS_HTONL(*ptr2);
						/* fill in block content */
						sys_ad_move_mem(ptr, &buf,
								sizeof(buf));
					}

					buf_size += st_form[type_idx].blk_size;
				}
			}
		}
	}

	/* free allocated memory */
	sys_ad_free_mem(rx_stat);

	/* fill in type num */
	ptr = hqa_frame_ctrl->hqa_frame_eth->data + 2;
	buf = SERV_OS_HTONL(type_num);
	sys_ad_move_mem(ptr, &buf, sizeof(buf));
	buf_size += sizeof(type_num);
	update_hqa_frame(hqa_frame_ctrl, (2 + buf_size), ret);

	return ret;

error1:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"memory allocation fail for rx stat.\n");
	update_hqa_frame(hqa_frame_ctrl, 2, ret);
	return ret;

error2:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"invalid band index for non-dbdc mode.\n");
	update_hqa_frame(hqa_frame_ctrl, 2, ret);
	return ret;
}

static s_int32 hqa_get_chip_capability(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_capability *chip_cap = NULL;

	os_alloc_mem(NULL, (PUCHAR *)&chip_cap, sizeof(struct test_capability));
	if (chip_cap == NULL)
		return SERV_STATUS_AGENT_NOT_SUPPORTED;

	os_zero_mem(chip_cap, sizeof(struct test_capability));
	ret = mt_serv_get_chip_capability(serv_test, chip_cap);
	if (ret)
		ret = SERV_STATUS_AGENT_NOT_SUPPORTED;

	/* Get parameters */
	chip_cap->version = SERV_OS_HTONL(chip_cap->version);
	chip_cap->tag_num = SERV_OS_HTONL(chip_cap->tag_num);

	chip_cap->ph_cap.tag = SERV_OS_HTONL(chip_cap->ph_cap.tag);
	chip_cap->ph_cap.tag_len = SERV_OS_HTONL(chip_cap->ph_cap.tag_len);
	chip_cap->ph_cap.protocol = SERV_OS_HTONL(chip_cap->ph_cap.protocol);
	chip_cap->ph_cap.ant_num = SERV_OS_HTONL(chip_cap->ph_cap.ant_num);
	chip_cap->ph_cap.dbdc = SERV_OS_HTONL(chip_cap->ph_cap.dbdc);
	chip_cap->ph_cap.coding = SERV_OS_HTONL(chip_cap->ph_cap.coding);
	chip_cap->ph_cap.channel_band = SERV_OS_HTONL(chip_cap->ph_cap.channel_band);
	chip_cap->ph_cap.bandwidth = SERV_OS_HTONL(chip_cap->ph_cap.bandwidth);
	chip_cap->ph_cap.channel_band_dbdc = SERV_OS_HTONL(chip_cap->ph_cap.channel_band_dbdc);
	chip_cap->ph_cap.channel_band_dbdc_ext = SERV_OS_HTONL(chip_cap->ph_cap.channel_band_dbdc_ext);
	chip_cap->ph_cap.phy_adie_index = SERV_OS_HTONL(chip_cap->ph_cap.phy_adie_index);
	chip_cap->ph_cap.band_0_1_wf_path_num = SERV_OS_HTONL(chip_cap->ph_cap.band_0_1_wf_path_num);
	chip_cap->ph_cap.band_2_3_wf_path_num = SERV_OS_HTONL(chip_cap->ph_cap.band_2_3_wf_path_num);
	chip_cap->ph_cap.band_bandwidth = SERV_OS_HTONL(chip_cap->ph_cap.band_bandwidth);

	chip_cap->ext_cap.tag = SERV_OS_HTONL(chip_cap->ext_cap.tag);
	chip_cap->ext_cap.tag_len = SERV_OS_HTONL(chip_cap->ext_cap.tag_len);
	chip_cap->ext_cap.feature1 = SERV_OS_HTONL(chip_cap->ext_cap.feature1);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"ant_num: %d\n", chip_cap->ph_cap.ant_num);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem((hqa_frame_ctrl->hqa_frame_eth->data + 2), chip_cap, sizeof(struct test_capability));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(struct test_capability), ret);

	if (chip_cap)
		os_free_mem(chip_cap);

	return ret;
}

static s_int32 hqa_get_rf_type_capability(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 tx_ant = 0, rx_ant = 0;
	u_int32 band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);


	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
			(u_char *)&band_idx, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx=%u, error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	ret = mt_serv_get_rf_type_capability(serv_test, band_idx, &tx_ant, &rx_ant);
	if (ret) {
		ret = SERV_STATUS_AGENT_NOT_SUPPORTED;
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"tx_ant: %d, rx_ant: %d\n", tx_ant, rx_ant);

	/* Get parameters */
	tx_ant = SERV_OS_HTONL(tx_ant);
	rx_ant = SERV_OS_HTONL(rx_ant);

	sys_ad_move_mem((hqa_frame_ctrl->hqa_frame_eth->data + 2),
			&tx_ant, sizeof(tx_ant));
	sys_ad_move_mem((hqa_frame_ctrl->hqa_frame_eth->data + 2 + sizeof(tx_ant)),
			&rx_ant, sizeof(rx_ant));
	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl,
			2 + sizeof(tx_ant) + sizeof(rx_ant), ret);

	return ret;
}

static s_int32 hqa_get_rxv_dump(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 version = 0, action = 0;
	u_int32 rxv_cnt = 0, rxv_content_len = 0, type_mask = 0;
	/* u_int32 rxv_cnt_max = 100; */
	u_int32 rxv_content_len_max = 1000;
	u_int32 buf = 0;
	u_int8 *ptr = NULL, *ptr3 = NULL;
	u_int32 *ptr2 = NULL;
	u_int32 *ptr2_temp = NULL;
	u_int32 buf_size = 0;
	u_int32 mem_buf_len = 0;
	u_int16 len = 0, dw_cnt = 0, dw_idx = 0;
	u_int8 rxv_type = 0, entry_idx = 0;
	s_int8 idx = 0;
	struct rxv_dump_ring_attr ring_attr;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* request format type */
	get_param_and_shift_buf(TRUE, sizeof(version),
				(u_char *)&version, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(action),
				(u_char *)&action, hqa_frame_ctrl);

	switch (action) {
	case TEST_RXV_DUMP_START:
		get_param_and_shift_buf(TRUE, sizeof(type_mask),
			(u_char *)&type_mask, hqa_frame_ctrl);
		break;

	case TEST_RXV_DUMP_STOP:
	case TEST_RXV_DUMP_REPORT:
		break;

	case TEST_RXV_DUMP_CLEAR_BUFFER:
	default:
		goto error4;
	}

	if ((action == TEST_RXV_DUMP_START) || (action == TEST_RXV_DUMP_STOP)) {
		ret = mt_serv_rxv_dump_action(serv_test, action, type_mask);
		if (ret != SERV_STATUS_SUCCESS)
			goto error5;
	} else if (action == TEST_RXV_DUMP_REPORT) {
		ptr = hqa_frame_ctrl->hqa_frame_eth->data + 2;
		buf = SERV_OS_HTONL(version);
		sys_ad_move_mem(ptr, &buf, sizeof(version));
		buf_size += sizeof(version);
		ptr += sizeof(version);

		/* request rxv dump ctrl info */
		ret = mt_serv_get_rxv_dump_ring_attr(serv_test, &ring_attr);
		if (ret != SERV_STATUS_SUCCESS)
			goto error6;

		/* compute buffer memory size needed */
		for (rxv_type = 0; rxv_type < TEST_RXV_CONTENT_NUM;
			rxv_type++) {
			if (ring_attr.type_mask & BIT(rxv_type)) {
				mt_serv_get_rxv_content_len(serv_test,
					rxv_type, 1, &len);
				mem_buf_len += len;
			}
		}

		/* allocate dynamic memory for rx stat info */
		ret = sys_ad_alloc_mem((pu_char *)&ptr2, mem_buf_len);
		if (ret != SERV_STATUS_SUCCESS)
			goto error1;

		sys_ad_zero_mem(ptr2, mem_buf_len);
		ptr2_temp = ptr2;

		/* keep pointer for further update value */
		ptr3 = ptr;
		buf_size += sizeof(rxv_cnt);
		ptr += sizeof(rxv_cnt);

		buf_size += sizeof(rxv_content_len);
		ptr += sizeof(rxv_content_len);

		for (idx = ring_attr.ring_idx - ring_attr.valid_entry_num;
			idx < ring_attr.ring_idx; idx++) {

			entry_idx = idx;
			if (idx < 0)
				entry_idx += ring_attr.dump_entry_total_num;

			ret = mt_serv_get_rxv_dump_content(serv_test,
				entry_idx, ptr2, &len);
			if (ret != SERV_STATUS_SUCCESS)
				goto error2;

			/* check buffer length validity */
			if (rxv_content_len > rxv_content_len_max)
				goto error3;

			dw_cnt = len >> 2;
			for (dw_idx = 0; dw_idx < dw_cnt;
				dw_idx++, ptr2++, ptr += 4) {
				/* endian transform */
				buf = SERV_OS_HTONL(*ptr2);
				/* fill in block content */
				sys_ad_move_mem(ptr, &buf, sizeof(buf));
			}

			/* increment rxv count */
			rxv_cnt++;

			/* rxv content length */
			rxv_content_len += len;

			/* increment used buffer size */
			buf_size += len;

		}

		ptr2 = ptr2_temp;

		/* free allocated memory */
		sys_ad_free_mem(ptr2);
		ptr2 = NULL;

		/* restore pointer address for update parameter */
		ptr = ptr3;
		buf = SERV_OS_HTONL(rxv_cnt);
		sys_ad_move_mem(ptr, &buf, sizeof(rxv_cnt));
		ptr += sizeof(rxv_cnt);

		buf = SERV_OS_HTONL(rxv_content_len);
		sys_ad_move_mem(ptr, &buf, sizeof(rxv_content_len));
		ptr += sizeof(rxv_content_len);

		/* clear rxv content buffer */
		mt_serv_rxv_dump_action(serv_test,
			TEST_RXV_DUMP_CLEAR_BUFFER, type_mask);
	}

	update_hqa_frame(hqa_frame_ctrl, (2 + buf_size), ret);
	return ret;

error1:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"memory allocation fail for rxv content.\n");
	update_hqa_frame(hqa_frame_ctrl, 2, ret);
	return ret;

error2:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"query rxv content fail.\n");
	/* free allocated memory */
	sys_ad_free_mem(ptr2);
	update_hqa_frame(hqa_frame_ctrl, 2, ret);
	return ret;

error3:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"rxv content length is larger than max buffer size.\n");
	/* free allocated memory */
	sys_ad_free_mem(ptr2);
	update_hqa_frame(hqa_frame_ctrl, 2, ret);
	return SERV_STATUS_AGENT_NOT_SUPPORTED;

error4:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"invalid action type.\n");
	update_hqa_frame(hqa_frame_ctrl, 2, ret);
	return SERV_STATUS_AGENT_NOT_SUPPORTED;

error5:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"rxv dump action fail for action(%d), type_mask(%d).\n", action, type_mask);
	update_hqa_frame(hqa_frame_ctrl, 2, ret);
	return SERV_STATUS_AGENT_FAIL;

error6:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"rxv dump ring attr fail.\n");
	update_hqa_frame(hqa_frame_ctrl, 2, ret);
	return SERV_STATUS_AGENT_FAIL;
}

static s_int32 hqa_calibration_test_mode(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config;
	u_int32 mode = 0, resp_len = 2, bypass = 0;

	get_param_and_shift_buf(TRUE, sizeof(mode),
				(u_char *)&mode, hqa_frame_ctrl);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR, "mode[%d]\n", mode);

	/* Set parameters */
	test_config = &serv_test->test_config;

	if (test_config) {
		if (mode == fTEST_OPER_NORMAL_MODE) {
			bypass = 1;

			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"\033[1;33m normal mode DONT support, CurrMode=%d, band_idx=%d\033[0m\n"
			, test_config->op_mode, serv_test->ctrl_band_idx);
		} else if (mode == fTEST_OPER_RFTEST_MODE) {
			test_config->op_mode |= fTEST_IN_RFTEST;
		} else if (mode == fTEST_OPER_ICAP_MODE) {
			test_config->op_mode |= fTEST_IN_RFTEST;
		} else {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"Mode = %d error!!!\n", mode);
		}
	} else
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"Mode = test_config is null!!!\n");


	if (bypass)
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"\033[1;33m Mode=%d, bypass=%d, band_idx=%d !!!\033[0m\n",
			mode, bypass, serv_test->ctrl_band_idx);
	else
		ret = mt_serv_calibration_test_mode(serv_test, mode);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, resp_len, ret);

	return ret;
}

static s_int32 hqa_do_cal_item(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 item = 0, band_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(item),
				(u_char *)&item, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx=%u, error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	serv_test->ctrl_band_idx = (u_char)band_idx;

	ret = mt_serv_do_cal_item(serv_test, item);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"item: %d, band_idx: %d\n", item, band_idx);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_tmr_setting(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_tmr_info tmr_info;
	struct test_tmr_info *ptmr = &tmr_info;

	sys_ad_zero_mem(&tmr_info, sizeof(struct test_tmr_info));

	get_param_and_shift_buf(TRUE, sizeof(ptmr->setting),
				(u_char *)&ptmr->setting, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(ptmr->version),
				(u_char *)&ptmr->version, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(ptmr->through_hold),
				(u_char *)&ptmr->through_hold, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(ptmr->iter),
				(u_char *)&ptmr->iter, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(ptmr->toae_cal),
				(u_char *)&ptmr->toae_cal, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(ptmr->band_idx),
				(u_char *)&ptmr->band_idx, hqa_frame_ctrl);

	ret = mt_serv_get_serv_test_by_band(&serv_test, ptmr->band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	sys_ad_move_mem(&serv_test->test_tmr, ptmr, sizeof(struct test_tmr_info));

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"bn_idx=%d, setting=%d, ver=%d, tput_hold=%d, iter=%d, toae_cal=%d\n",
		ptmr->band_idx, ptmr->setting, ptmr->version,
		ptmr->through_hold, ptmr->iter, ptmr->toae_cal);

	ret = mt_serv_set_tmr(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_get_chipid(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 chip_id;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	ret = mt_serv_get_chipid(serv_test);

	chip_id = WINFO_GET_PARAM(serv_test, chip_id);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"chip_id=0x%x\n", chip_id);

	chip_id = SERV_OS_HTONL(chip_id);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &chip_id, sizeof(chip_id));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(chip_id), ret);

	return ret;
}

static s_int32 hqa_get_sub_chipid(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 sub_chip_id;

	ret = mt_serv_get_sub_chipid(serv_test, &sub_chip_id);

	sub_chip_id = SERV_OS_HTONL(sub_chip_id);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR, "0x%08x\n", sub_chip_id);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &sub_chip_id, sizeof(sub_chip_id));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(sub_chip_id), ret);

	return ret;
}

static s_int32 hqa_mps_set_seq_data(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config = NULL;
	struct test_mps_cb *mps_cb = NULL;
	struct test_mps_setting *mps_setting = NULL;
	u_int32 *param = NULL;
	u_int32 band_idx = 0;
	u_int32 idx, value = 0;
	u_int16 len;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	len = hqa_frame_ctrl->hqa_frame_eth->length / sizeof(u_int32) - 1;
	if ((len > TEST_MPS_ITEM_LEN) || (len == 0)) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err;
	}

	ret = sys_ad_alloc_mem((u_char **)&param, sizeof(u_int32) * len);
	if (ret) {
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	for (idx = 0; idx < len; idx++) {
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, hqa_frame_ctrl);
		param[idx] = value;
	}

	/* Set parameters */
	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx error(%d)!!!\n", band_idx);
		ret = SERV_STATUS_AGENT_INVALID_BANDIDX;
		goto err;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);

		if (param)
			sys_ad_free_mem(param);

		return ret;
	}

	test_config = &serv_test->test_config;

	if (!test_config) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"test_config NULL Pointer!!!\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	mps_cb = &test_config->mps_cb;

	if (test_config->op_mode & fTEST_MPS) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err;
	}

	if (!mps_cb->mps_setting && !mps_cb->mps_cnt) {
		mps_cb->mps_cnt = len;
		ret = sys_ad_alloc_mem((u_char **)&mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
		if (ret) {
			ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
			goto err;
		}
		sys_ad_zero_mem(mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
	}

	if (!mps_cb->mps_setting) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"mps_setting NULL Pointer!!!\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	mps_cb->mps_cnt = len;
	mps_setting = mps_cb->mps_setting;

	for (idx = 0; idx < len; idx++) {
		mps_setting[idx+1].tx_mode = (param[idx] & 0x0F000000) >> 24;
		mps_setting[idx+1].tx_ant = (param[idx] & 0x00FFFF00) >> 8;
		mps_setting[idx+1].mcs = (param[idx] & 0x000000FF);
	}

	ret = mt_serv_mps_set_seq_data(serv_test);

err:
	if (test_config && mps_cb)
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"band_idx=%d, len=%d, op_mode=0x%x, mps_cnt=%d\n",
			band_idx, len,
			test_config->op_mode, mps_cb->mps_cnt);
	else
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%d, len=%d\n",
			band_idx, len);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	if (param)
		sys_ad_free_mem(param);

	return ret;
}

static s_int32 hqa_mps_set_payload_length(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config = NULL;
	struct test_mps_cb *mps_cb = NULL;
	struct test_mps_setting *mps_setting = NULL;
	u_int32 *param = NULL;
	u_int32 band_idx = 0;
	u_int32 idx, value = 0;
	u_int16 len;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	len = hqa_frame_ctrl->hqa_frame_eth->length / sizeof(u_int32) - 1;
	if ((len > TEST_MPS_ITEM_LEN) || (len == 0)) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err;
	}

	ret = sys_ad_alloc_mem((u_char **)&param, sizeof(u_int32) * len);
	if (ret) {
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	for (idx = 0; idx < len; idx++) {
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, hqa_frame_ctrl);
		param[idx] = value;
	}

	/* Set parameters */
	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx error(%d)!!!\n", band_idx);
		ret = SERV_STATUS_AGENT_INVALID_BANDIDX;
		goto err;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);

		if (param)
			sys_ad_free_mem(param);

		return ret;
	}

	test_config = &serv_test->test_config;

	if (!test_config) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"test_config NULL Pointer!!!\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	mps_cb = &test_config->mps_cb;

	if (test_config->op_mode & fTEST_MPS) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err;
	}

	if (!mps_cb->mps_setting && !mps_cb->mps_cnt) {
		mps_cb->mps_cnt = len;
		ret = sys_ad_alloc_mem((u_char **)&mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
		if (ret) {
			ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
			goto err;
		}
		sys_ad_zero_mem(mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
	}

	if (!mps_cb->mps_setting) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"mps_setting NULL Pointer!!!\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	mps_setting = mps_cb->mps_setting;

	for (idx = 0; idx < len; idx++) {
		if (param[idx] > TEST_MAX_PKT_LEN)
			param[idx] = TEST_MAX_PKT_LEN;
		else if (param[idx] < TEST_MIN_PKT_LEN)
			param[idx] = TEST_MIN_PKT_LEN;

		mps_setting[idx+1].pkt_len = param[idx];
	}

	ret = mt_serv_mps_set_payload_length(serv_test);

err:
	if (test_config && mps_cb)
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"band_idx=%d, len=%d, op_mode=0x%x, mps_cnt=%d\n",
			band_idx, len,
			test_config->op_mode, mps_cb->mps_cnt);
	else
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%d, len=%d\n",
			band_idx, len);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	if (param)
		sys_ad_free_mem(param);

	return ret;
}

static s_int32 hqa_mps_set_packet_count(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config = NULL;
	struct test_mps_cb *mps_cb = NULL;
	struct test_mps_setting *mps_setting = NULL;
	u_int32 *param = NULL;
	u_int32 band_idx = 0, idx = 0, value = 0;
	u_int16 len;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	len = hqa_frame_ctrl->hqa_frame_eth->length / sizeof(u_int32) - 1;
	if ((len > TEST_MPS_ITEM_LEN) || (len == 0)) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err;
	}

	ret = sys_ad_alloc_mem((u_char **)&param, sizeof(u_int32) * len);
	if (ret) {
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	for (idx = 0; idx < len; idx++) {
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, hqa_frame_ctrl);
		param[idx] = value;
	}

	/* Set parameters */
	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx error(%d)!!!\n", band_idx);
		ret = SERV_STATUS_AGENT_INVALID_BANDIDX;
		goto err;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);

		if (param)
			sys_ad_free_mem(param);

		return ret;
	}

	test_config = &serv_test->test_config;

	if (!test_config) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"test_config NULL Pointer!!!\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	mps_cb = &test_config->mps_cb;

	if (test_config->op_mode & fTEST_MPS) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err;
	}

	if (!mps_cb->mps_setting && !mps_cb->mps_cnt) {
		mps_cb->mps_cnt = len;
		ret = sys_ad_alloc_mem((u_char **)&mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
		if (ret) {
			ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
			goto err;
		}
		sys_ad_zero_mem(mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
	}

	if (!mps_cb->mps_setting) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"mps_setting NULL Pointer!!!\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	mps_setting = mps_cb->mps_setting;

	for (idx = 0; idx < len; idx++)
		mps_setting[idx+1].pkt_cnt = param[idx];

	ret = mt_serv_mps_set_packet_count(serv_test);

err:
	if (test_config && mps_cb)
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"band_idx=%d, len=%d, op_mode=0x%x, mps_cnt=%d\n",
			band_idx, len,
			test_config->op_mode, mps_cb->mps_cnt);
	else
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%d, len=%d\n",
			band_idx, len);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	if (param)
		sys_ad_free_mem(param);

	return ret;
}

static s_int32 hqa_mps_set_power_gain(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config = NULL;
	struct test_mps_cb *mps_cb = NULL;
	struct test_mps_setting *mps_setting = NULL;
	u_int32 *param = NULL;
	u_int32 band_idx = 0;
	u_int32 idx, value = 0;
	u_int16 len;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	len = hqa_frame_ctrl->hqa_frame_eth->length / sizeof(u_int32) - 1;
	if ((len > TEST_MPS_ITEM_LEN) || (len == 0)) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err;
	}

	ret = sys_ad_alloc_mem((u_char **)&param, sizeof(u_int32) * len);
	if (ret) {
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	for (idx = 0; idx < len; idx++) {
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, hqa_frame_ctrl);
		param[idx] = value;
	}

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx error(%d)!!!\n", band_idx);
		ret = SERV_STATUS_AGENT_INVALID_BANDIDX;
		goto err;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);

		if (param)
			sys_ad_free_mem(param);

		return ret;
	}

	/* Set parameters */
	test_config = &serv_test->test_config;

	if (!test_config) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"test_config NULL Pointer!!!\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	mps_cb = &test_config->mps_cb;

	if (test_config->op_mode & fTEST_MPS) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err;
	}

	if (!mps_cb->mps_setting && !mps_cb->mps_cnt) {
		mps_cb->mps_cnt = len;
		ret = sys_ad_alloc_mem((u_char **)&mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
		if (ret) {
			ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
			goto err;
		}
		sys_ad_zero_mem(mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
	}

	if (!mps_cb->mps_setting) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"mps_setting NULL Pointer!!!\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	mps_setting = mps_cb->mps_setting;

	for (idx = 0; idx < len; idx++)
		mps_setting[idx+1].pwr = param[idx];

	ret = mt_serv_mps_set_power_gain(serv_test);

err:
	if (test_config && mps_cb)
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"band_idx=%d, len=%d, op_mode=0x%x, mps_cnt=%d\n",
			band_idx, len,
			test_config->op_mode, mps_cb->mps_cnt);
	else
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%d, len=%d\n",
			band_idx, len);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	if (param)
		sys_ad_free_mem(param);

	return ret;
}

static s_int32 hqa_mps_start(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	/* Set parameters */
	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%d error !\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	ret = mt_serv_mps_operation(serv_test, SERV_TEST_MPS_START_TX);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_mps_stop(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%d error !\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);


	ret = mt_serv_mps_operation(serv_test, SERV_TEST_MPS_STOP_TX);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_check_efuse_mode_type(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 e2p_cur_mode = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	e2p_cur_mode = (u_int32)WINFO_GET_PARAM(serv_test, e2p_cur_mode);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"e2p_cur_mode=0x%x\n", e2p_cur_mode);

	e2p_cur_mode = SERV_OS_HTONL(e2p_cur_mode);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &e2p_cur_mode,
			sizeof(e2p_cur_mode));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(e2p_cur_mode), ret);

	return ret;
}

static s_int32 hqa_check_efuse_nativemode_type(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 e2p_access_mode = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	e2p_access_mode = (u_int32)WINFO_GET_PARAM(serv_test, e2p_access_mode);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"e2p_access_mode=0x%x\n", e2p_access_mode);

	e2p_access_mode = SERV_OS_HTONL(e2p_access_mode);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &e2p_access_mode,
			sizeof(e2p_access_mode));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(e2p_access_mode), ret);

	return ret;
}

static s_int32 hqa_set_band_mode(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	/* struct test_band_state *band_state; */
	u_int32 band_mode = 0, band_type = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_mode),
				(u_char *)&band_mode, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(band_type),
				(u_char *)&band_type, hqa_frame_ctrl);

	/* Set parameters */
	/* band_state = SERV_GET_PADDR(serv_test, test_bstat); */
	BSTATE_SET_PARAM(serv_test, band_mode, band_mode);
	BSTATE_SET_PARAM(serv_test, band_type, band_type);

	ret = mt_serv_set_band_mode(serv_test);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"band_mode=%u, band_type=%u\n",
		band_mode, band_type);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_get_band_mode(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_band_state *band_state;
	u_int32 band_type = 0, band_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%d error !\n", band_idx);
		goto err;
	}

	band_state = SERV_GET_PADDR(serv_test, test_bstat);
	ret = mt_serv_get_band_mode(serv_test, (u_int8)band_idx);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"band_type=%u\n", band_state->band_type);

	band_type = SERV_OS_HTONL(band_state->band_type);

err:

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"band_type=%u\n", band_type);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &band_type, sizeof(band_type));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(band_type), ret);

	return ret;
}

static s_int32 hqa_rdd_start(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 rdd_num = 0;
	u_int32 rdd_sel = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(rdd_num),
				(u_char *)&rdd_num, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(rdd_sel),
				(u_char *)&rdd_sel, hqa_frame_ctrl);

	ret = mt_serv_set_rdd_on_off(serv_test, rdd_num, rdd_sel, TRUE);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"rdd_num: %d, rdd_sel: %d\n",
		rdd_num, rdd_sel);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_rdd_stop(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 rdd_num = 0;
	u_int32 rdd_sel = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(rdd_num),
				(u_char *)&rdd_num, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(rdd_sel),
				(u_char *)&rdd_sel, hqa_frame_ctrl);

	ret = mt_serv_set_rdd_on_off(serv_test, rdd_num, rdd_sel, FALSE);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"rdd_num: %d, rdd_sel: %d\n",
		rdd_num, rdd_sel);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_log_on_off(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = 0, log_type = 0, log_ctrl = 0, log_size = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(log_type),
				(u_char *)&log_type, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(log_ctrl),
				(u_char *)&log_ctrl, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(log_size),
				(u_char *)&log_size, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%d error !\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	ret = mt_serv_log_on_off(serv_test, log_type, log_ctrl, log_size);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_mps_set_nss(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config = NULL;
	struct test_mps_cb *mps_cb = NULL;
	struct test_mps_setting *mps_setting = NULL;
	u_int32 *param = NULL;
	u_int32 band_idx = 0;
	u_int32 idx, value = 0;
	u_int16 len;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *) &band_idx, hqa_frame_ctrl);

	len = hqa_frame_ctrl->hqa_frame_eth->length / sizeof(u_int32) - 1;
	if ((len > TEST_MPS_ITEM_LEN) || (len == 0)) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err;
	}

	ret = sys_ad_alloc_mem((u_char **)&param, sizeof(u_int32) * len);
	if (ret) {
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	for (idx = 0; idx < len; idx++) {
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *) &value, hqa_frame_ctrl);
		param[idx] = value;
	}

	/* Set parameters */
	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx: %d error!!\n", band_idx);
		goto err;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);

		if (param)
			sys_ad_free_mem(param);

		return ret;
	}

	test_config = &serv_test->test_config;

	if (!test_config) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"test_config NULL Pointer!!!\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	mps_cb = &test_config->mps_cb;

	if (test_config->op_mode & fTEST_MPS) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err;
	}

	if (!mps_cb->mps_setting && !mps_cb->mps_cnt) {
		mps_cb->mps_cnt = len;
		ret = sys_ad_alloc_mem((u_char **)&mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
		if (ret) {
			ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
			goto err;
		}
		sys_ad_zero_mem(mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
	}

	if (!mps_cb->mps_setting) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"mps_setting NULL Pointer!!!\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	mps_setting = mps_cb->mps_setting;

	for (idx = 0; idx < len; idx++)
		mps_setting[idx+1].nss = param[idx];

	ret = mt_serv_mps_set_nss(serv_test);

err:
	if (test_config && mps_cb)
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"band_idx=%d, len=%d, op_mode=0x%x, mps_cnt=%d\n",
			band_idx, len,
			test_config->op_mode, mps_cb->mps_cnt);
	else
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%d, len=%d\n",
			band_idx, len);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	if (param)
		sys_ad_free_mem(param);

	return ret;
}

static s_int32 hqa_mps_set_per_packet_bw(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config = NULL;
	struct test_mps_cb *mps_cb = NULL;
	struct test_mps_setting *mps_setting = NULL;
	u_int32 *param = NULL;
	u_int32 band_idx = 0;
	u_int32 idx, value = 0;
	u_int16 len;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	len = hqa_frame_ctrl->hqa_frame_eth->length / sizeof(u_int32) - 1;
	if ((len > TEST_MPS_ITEM_LEN) || (len == 0)) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err;
	}

	ret = sys_ad_alloc_mem((u_char **)&param, sizeof(u_int32) * len);
	if (ret) {
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	for (idx = 0; idx < len; idx++) {
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *) &value, hqa_frame_ctrl);

		switch (value) {
		case 0:
			value = TEST_BW_20;
			break;

		case 1:
			value = TEST_BW_40;
			break;

		case 2:
			value = TEST_BW_80;
			break;

		case 3:
			value = TEST_BW_10;
			break;

		case 4:
			value = TEST_BW_5;
			break;

		case 5:
			value = TEST_BW_160C;
			break;

		case 6:
			value = TEST_BW_160NC;
			break;

		default:
			value = TEST_BW_20;
			break;
		}

		param[idx] = value;
	}

	/* Set parameters */
	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx (%d) error!!!\n", band_idx);
		goto err;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);

		if (param)
			sys_ad_free_mem(param);

		return ret;
	}

	test_config = &serv_test->test_config;

	if (!test_config) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"test_config NULL Pointer!!!\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	mps_cb = &test_config->mps_cb;

	if (test_config->op_mode & fTEST_MPS) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err;
	}

	if (!mps_cb->mps_setting && !mps_cb->mps_cnt) {
		mps_cb->mps_cnt = len;
		ret = sys_ad_alloc_mem((u_char **)&mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
		if (ret) {
			ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
			goto err;
		}
		sys_ad_zero_mem(mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
	}

	if (!mps_cb->mps_setting) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"mps_setting NULL Pointer!!!\n");
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	mps_setting = mps_cb->mps_setting;

	for (idx = 0; idx < len; idx++)
		mps_setting[idx+1].pkt_bw = param[idx];

	ret = mt_serv_mps_set_per_packet_bw(serv_test);

err:
	if (test_config && mps_cb)
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"band_idx=%d, len=%d, op_mode=0x%x, mps_cnt=%d\n",
			band_idx, len,
			test_config->op_mode, mps_cb->mps_cnt);
	else
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%d, len=%d\n",
			band_idx, len);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	if (param)
		sys_ad_free_mem(param);

	return ret;
}

static s_int32 hqa_icap_ctrl(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	s_int32 icap_stat = 0;
	u_int32 wf_num = 0, iq_type = 0;
	u_int32 control = 0, resp_len = 2;
	u_int32 value = 0, i = 0;
	u_long max_data_len = 0;
	u_char src_addr[SERV_MAC_ADDR_LEN];
	s_int32 *icap_data = NULL;
	s_int32 *icap_data_cnt = NULL;
	struct hqa_rbist_cap_start icap_info;

	get_param_and_shift_buf(TRUE, sizeof(control),
				(u_char *)&control, hqa_frame_ctrl);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"control: %d\n", control);

	switch (control) {
	case 1:
		/* clear memory */
		sys_ad_zero_mem(&icap_info, sizeof(icap_info));

		get_param_and_shift_buf(TRUE, sizeof(icap_info.trig),
			(u_char *)&(icap_info.trig), hqa_frame_ctrl);

		get_param_and_shift_buf(TRUE, sizeof(icap_info.ring_cap_en),
			(u_char *)&(icap_info.ring_cap_en), hqa_frame_ctrl);

		get_param_and_shift_buf(TRUE, sizeof(icap_info.trig_event),
			(u_char *)&(icap_info.trig_event), hqa_frame_ctrl);

		get_param_and_shift_buf(TRUE, sizeof(icap_info.cap_node),
			(u_char *)&(icap_info.cap_node), hqa_frame_ctrl);

		get_param_and_shift_buf(TRUE, sizeof(icap_info.cap_len),
			(u_char *)&(icap_info.cap_len), hqa_frame_ctrl);

		get_param_and_shift_buf(TRUE, sizeof(icap_info.cap_stop_cycle),
			(u_char *)&(icap_info.cap_stop_cycle), hqa_frame_ctrl);

		get_param_and_shift_buf(TRUE, sizeof(icap_info.bw),
			(u_char *)&(icap_info.bw), hqa_frame_ctrl);

		get_param_and_shift_buf(TRUE, sizeof(icap_info.mac_trig_event),
			(u_char *)&(icap_info.mac_trig_event), hqa_frame_ctrl);

		get_param_and_shift_buf(FALSE, SERV_MAC_ADDR_LEN,
			(u_char *)src_addr, hqa_frame_ctrl);

		get_param_and_shift_buf(TRUE, sizeof(icap_info.band_idx),
			(u_char *)&(icap_info.band_idx), hqa_frame_ctrl);

		get_param_and_shift_buf(TRUE, sizeof(icap_info.phy_idx),
			(u_char *)&(icap_info.phy_idx), hqa_frame_ctrl);

		get_param_and_shift_buf(TRUE, sizeof(icap_info.cap_src),
			(u_char *)&(icap_info.cap_src), hqa_frame_ctrl);

		/* source address lsb */
		for (i = 0; i < 4; i++)
			icap_info.src_addr_lsb |= ((src_addr[i]) << (i << 3));

		/* source address msb */
		for (i = 0; i < 2; i++)
			icap_info.src_addr_msb |= ((src_addr[i+4]) << (i << 3));
		icap_info.src_addr_msb |= ((0x1) << 16);

		/* capture source select */
		icap_info.cap_src = 0; /* ICAP0_data (128bit) for WF PHY used */

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"trig = 0x%08x, ring_cap_en = 0x%08x\n"
		, icap_info.trig, icap_info.ring_cap_en);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"trig_event = 0x%08x, cap_node = 0x%08x\n"
		, icap_info.trig_event, icap_info.cap_node);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"cap_len = 0x%08x, cap_stop_cycle = 0x%08x\n"
		, icap_info.cap_len, icap_info.cap_stop_cycle);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"bw = 0x%08x, mac_trig_event = 0x%08x\n"
		, icap_info.bw, icap_info.mac_trig_event);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"src_addr_msb = 0x%08x, src_addr_lsb = 0x%08x\n"
		, icap_info.src_addr_msb, icap_info.src_addr_lsb);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"band_idx = 0x%08x, phy_idx = 0x%08x\n"
		, icap_info.band_idx, icap_info.phy_idx);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"cap_src = 0x%08x\n", icap_info.cap_src);

		if (icap_info.band_idx >= TEST_BAND_NUM) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"band_idx=%d error !\n", icap_info.band_idx);
			return SERV_STATUS_AGENT_INVALID_BANDIDX;
		}

		ret = mt_serv_get_serv_test_by_band(&serv_test, icap_info.band_idx);
		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"ret:%x\n", ret);
			return ret;
		}

		ret = mt_serv_set_icap_start(serv_test, &icap_info);
		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"mt_serv_set_icap_start is failed!!\n");
			goto error1;
		}
		break;

	case 2:
		ret = mt_serv_get_icap_status(serv_test, &icap_stat);
		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"mt_serv_get_icap_status is failed!!\n");
			goto error1;
		} else {
			ret = icap_stat;
			update_hqa_frame(hqa_frame_ctrl, resp_len, ret);
			return SERV_STATUS_SUCCESS;
		}
		break;

	case 3:
		get_param_and_shift_buf(TRUE, sizeof(wf_num),
			(u_char *)&(wf_num), hqa_frame_ctrl);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"wf_num: %d\n", wf_num);

		get_param_and_shift_buf(TRUE, sizeof(iq_type),
			(u_char *)&(iq_type), hqa_frame_ctrl);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"iq_type: %d\n", iq_type);

		ret = sys_ad_alloc_mem((u_char **)&icap_data_cnt
				, sizeof(s_int32));
		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"Not enough memory for dynamic allocating!!\n");
			goto error1;
		}
		sys_ad_zero_mem(icap_data_cnt, sizeof(s_int32));

		ret = mt_serv_get_icap_max_data_len(serv_test, &max_data_len);
		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"mt_serv_get_icap_max_data_len is failed!!\n");
			goto error1;
		}

		ret = sys_ad_alloc_mem((u_char **)&icap_data, max_data_len);
		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"Not enough memory for dynamic allocating!!\n");
			goto error1;
		}
		sys_ad_zero_mem(icap_data, max_data_len);

		ret = mt_serv_get_icap_data(serv_test,
				icap_data_cnt, icap_data, wf_num, iq_type);
		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"mt_serv_get_icap_data is not supported!!\n");
			goto error1;
		}

		if (g_hqa_frame_ctrl == 1) {
			sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + resp_len,
				&ret, sizeof(ret));
			resp_len += sizeof(ret);
		} else {
			value = SERV_OS_HTONL(control);
			sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + resp_len,
				&value, sizeof(value));
			resp_len += sizeof(value);
			value = SERV_OS_HTONL(wf_num);
			sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + resp_len,
				&value, sizeof(value));
			resp_len += sizeof(value);
			value = SERV_OS_HTONL(iq_type);
			sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + resp_len,
				&value, sizeof(value));
			resp_len += sizeof(value);
			value = SERV_OS_HTONL(*icap_data_cnt);
			sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + resp_len,
				&value, sizeof(value));
			resp_len += sizeof(value);

			for (i = 0; i < *icap_data_cnt; i++) {
				value = SERV_OS_HTONL(icap_data[i]);
				sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + resp_len,
					&value, sizeof(value));
				resp_len += sizeof(value);
			}
		}
		break;

	default:
		break;
	}

error1:
	if (ret)
		ret = SERV_STATUS_AGENT_NOT_SUPPORTED;

	update_hqa_frame(hqa_frame_ctrl, resp_len, ret);

	if (icap_data_cnt)
		sys_ad_free_mem(icap_data_cnt);

	if (icap_data)
		sys_ad_free_mem(icap_data);

	return ret;
}

static s_int32 hqa_get_dump_recal(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 resp_len = 6;
	u_int32 band_idx = 0;
	u_int32 recal_cnt = 0, recal_dw_num = 0;
	u_int32 *content = NULL;
	u_int32 value = 0, dw_cnt = 0, i = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%d error !\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	serv_test->ctrl_band_idx = (u_char)band_idx;

	mt_serv_get_recal_cnt(serv_test,
		&recal_cnt, &recal_dw_num);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"band_idx: %d, rxv_cnt: %d, rxv_dw_num: %d\n",
		band_idx, recal_cnt, recal_dw_num);

	dw_cnt = recal_cnt * recal_dw_num;

	ret = sys_ad_alloc_mem((u_char **)&content,
		sizeof(*content) * dw_cnt);
	if (ret != SERV_STATUS_SUCCESS)
		goto error1;

	ret = mt_serv_get_recal_content(serv_test, content);

	/* Update hqa_frame with response: status (2 bytes) */
	value = SERV_OS_HTONL(recal_cnt);
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2,
		&value, sizeof(value));

	for (i = 0; i < dw_cnt; i++, content++) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"content[%d]: 0x%x\n", i, *content);

		value = SERV_OS_HTONL(*content);
		sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + resp_len,
			&value, sizeof(value));

		resp_len += sizeof(value);
	}

	/* Free memory */
	sys_ad_free_mem(content);

	update_hqa_frame(hqa_frame_ctrl, resp_len, ret);

	return ret;


error1:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"dynamic memory allocate fail!!\n");
	if (content)
		sys_ad_free_mem(content);
	/* TODO: respond to application for error handle */
	update_hqa_frame(hqa_frame_ctrl, resp_len, ret);

	return ret;
}

static s_int32 hqa_get_dump_rxv_legacy(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 resp_len = 2;
	u_int32 band_idx = 0;
	u_int32 rxv_cnt = 0, rxv_dw_num = 0;
	u_int32 *content = NULL;
	u_int32 value = 0, dw_cnt = 0, i = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%d error !\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	serv_test->ctrl_band_idx = (u_char)band_idx;

	ret = mt_serv_get_rxv_cnt(serv_test, &rxv_cnt, &rxv_dw_num);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"band_idx: %d, rxv_cnt: %d, rxv_dw_num: %d\n",
		band_idx, rxv_cnt, rxv_dw_num);

	dw_cnt = rxv_cnt * rxv_dw_num;
	ret = sys_ad_alloc_mem((u_char **)&content,
		sizeof(*content) * dw_cnt);
	if (ret != SERV_STATUS_SUCCESS)
		goto error1;

	ret = mt_serv_get_rxv_content(serv_test, dw_cnt, content);

	/* Update hqa_frame with response: Count (2 bytes) */
	value = SERV_OS_HTONL(rxv_cnt);
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + resp_len,
		&value, sizeof(value));

	resp_len += sizeof(value);

	for (i = 0; i < dw_cnt; i += 4, content++) {
		value = SERV_OS_HTONL(*content);
		sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + resp_len,
			&value, sizeof(value));
		resp_len += sizeof(value);
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"resp_len = %d\n", resp_len);

	/* Free memory */
	sys_ad_free_mem(content);

	update_hqa_frame(hqa_frame_ctrl, resp_len, ret);

	return ret;
error1:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"dynamic memory allocate fail!!\n");
	if (content)
		sys_ad_free_mem(content);
	/* TODO: respond to application for error handle */
	update_hqa_frame(hqa_frame_ctrl, resp_len, ret);

	return ret;
}

static s_int32 hqa_get_dump_rdd(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 resp_len = 2;
	u_int32 band_idx = 0;
	u_int32 rdd_cnt = 0, rdd_dw_num = 0;
	u_int32 *content = NULL;
	u_int32 value = 0, i = 0, total_cnt = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%d error !\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	serv_test->ctrl_band_idx = (u_char)band_idx;

	mt_serv_get_rdd_cnt(serv_test, &rdd_cnt, &rdd_dw_num);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"band_idx: %d, pulse number: %d, rdd buffer size: %d\n",
		band_idx, rdd_cnt, rdd_dw_num);

	ret = sys_ad_alloc_mem((u_char **)&content,
		sizeof(*content) * rdd_dw_num);
	if (ret != SERV_STATUS_SUCCESS)
		goto error1;

	ret = mt_serv_get_rdd_content(serv_test, content, &total_cnt);

	if (total_cnt > 0) {
		/* Update hqa_frame with response: status (2 bytes) */
		/* Response format:
		 * cmd type + cmd ID + length + Sequence +
		 * data:
		 * status (2 bytes) +
		 * [count (4 bytes)] + value1 (4 bytes) + value2 (4 bytes)
		 */
		/* Count = Total number of 4 bytes RDD values divided by 2 */
		value = SERV_OS_HTONL(total_cnt/2);
		sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + resp_len,
			&value, sizeof(value));
		resp_len += sizeof(value);

		for (i = 0; i < total_cnt; i++, content++) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				"content[%d]: 0x%08x\n",
				i, *content);

			value = SERV_OS_HTONL(*content);
			sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + resp_len,
				&value, sizeof(value));
			resp_len += sizeof(value);
		}

	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"total_cnt %d\n", total_cnt);
		sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + resp_len,
			&total_cnt, sizeof(total_cnt));
		resp_len += sizeof(total_cnt);
	}

	/* Free memory */
	sys_ad_free_mem(content);

	update_hqa_frame(hqa_frame_ctrl, resp_len, ret);

	return ret;

error1:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"dynamic memory allocate fail!!\n");
	if (content)
		sys_ad_free_mem(content);
	/* TODO: respond to application for error handle */
	update_hqa_frame(hqa_frame_ctrl, resp_len, ret);

	return ret;
}

static s_int32 hqa_get_hetb_info(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 resp_len = 2;
	u_int32 band_idx = serv_test->ctrl_band_idx;
	u_int32 afactor = 0, ldpc_str_sym = 0, pe_disamb = 0;
	u_int32 tx_pe = 0, l_sig_len = 0, value = 0;
	u_int8 dmnt_ru_idx = 0;
	struct test_ru_info *ru_info = NULL;

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%d error !\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	dmnt_ru_idx = CONFIG_GET_PARAM(serv_test, dmnt_ru_idx);
	ru_info = CONFIG_GET_PADDR(serv_test,
					 ru_info_list[dmnt_ru_idx]);

	serv_test->test_op->op_get_hetb_info(
		serv_test->test_winfo,
		band_idx,
		ru_info);

	afactor = ru_info->afactor_init;
	ldpc_str_sym = ru_info->ldpc_extr_sym;
	pe_disamb = ru_info->pe_disamb;
	tx_pe = ru_info->t_pe;
	l_sig_len = ru_info->l_len;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"band_idx:0x%x, dmnt_ru_idx:0x%x afactor:0x%x\n",
			band_idx, dmnt_ru_idx, afactor);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"ldpc_str_sym:0x%x, pe_disamb:0x%x\n",
			ldpc_str_sym, pe_disamb);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"tx_pe:0x%x,l_sig_len:0x%x\n",
			tx_pe, l_sig_len);

	/* The response array should be a-factor,
	 *				    ldpc extra symbol,
	 *				    PE disambiguilty,
	 *				    TX PE,
	 *				    L-SIG length
	 */
	value = SERV_OS_HTONL(afactor);
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &value, sizeof(value));
	value = SERV_OS_HTONL(ldpc_str_sym);
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2 + sizeof(value), &value,
				sizeof(value));
	value = SERV_OS_HTONL(pe_disamb);
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2 + sizeof(value)*2,
				&value, sizeof(value));
	value = SERV_OS_HTONL(tx_pe);
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2 + sizeof(value)*3, &value,
				sizeof(value));
	value = SERV_OS_HTONL(l_sig_len);
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2 + sizeof(value)*4, &value,
				sizeof(value));
	resp_len += 5*sizeof(u_int32);

	update_hqa_frame(hqa_frame_ctrl, resp_len, ret);

	return ret;
}

static s_int32 hqa_translate_ru_allocation(
	u_int32 user_ru_allocation,
	u_int32 *allocation)
{
	u_int8 i = 0;

	*allocation = 0;
	for (i = 0 ; i < sizeof(u_int32)*2 ; i++) {
		*allocation |= ((user_ru_allocation & 0x1) << i);
		user_ru_allocation >>= 4;
	}

	return SERV_STATUS_SUCCESS;
}

#define SEG_STA_CNT		16
static s_int32 hqa_set_ru_info(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 resp_len = 2;
	u_int32 band_idx = (u_int32)(serv_test->ctrl_band_idx);
	u_int32 len = 0, seg_sta_cnt[2] = {0}, sta_seq = 0, value = 0;
	u_char param_cnt = 0, param_loop = 0;
	struct test_ru_allocatoin *ru_allocation = NULL;
	struct test_ru_info *ru_info = NULL;

	len = hqa_frame_ctrl->hqa_frame_eth->length;
	get_param_and_shift_buf(TRUE,
				   sizeof(u_int32),
				   (u_char *)&band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE,
				   sizeof(u_int32),
				   (u_char *)&seg_sta_cnt[0], hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE,
				   sizeof(u_int32),
				   (u_char *)&seg_sta_cnt[1], hqa_frame_ctrl);

	if ((seg_sta_cnt[0] > SEG_STA_CNT)
		|| (seg_sta_cnt[1] > SEG_STA_CNT)) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	len -= sizeof(u_int32)*3;		/* array length */
	if (seg_sta_cnt[0]+seg_sta_cnt[1] == 0) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err_out;
	}

	len /= (seg_sta_cnt[0]+seg_sta_cnt[1]);	/* per ru length */
	param_cnt = len/sizeof(u_int32);	/* param count */
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		 "Band:%d [ru_segment 0]:%d, [ru_segment 1]:%d\n",
		 band_idx, seg_sta_cnt[0], seg_sta_cnt[1]);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		 "\t\tparameters count:%d\n", param_cnt);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx:%d error\n", band_idx);
		ret = SERV_STATUS_AGENT_INVALID_BANDIDX;
		goto err_out;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	ru_allocation = CONFIG_GET_PADDR(serv_test, ru_alloc);
	ru_info = CONFIG_GET_PADDR(serv_test, ru_info_list[0]);
	sys_ad_zero_mem(ru_info, sizeof(struct test_ru_info)*MAX_MULTI_TX_STA);
	sys_ad_set_mem(ru_allocation, sizeof(struct test_ru_allocatoin), (u_char)UNUSED_RU);

	/* for maximum bw 80+80/160, 2 segments only */
	for (sta_seq = 0;
	     sta_seq < seg_sta_cnt[0]+seg_sta_cnt[1];
	     sta_seq++) {
		param_loop = param_cnt;

		if (sta_seq < seg_sta_cnt[0])
			ru_info[sta_seq].b0 = 0;
		else
			ru_info[sta_seq].b0 = 1;

		ru_info[sta_seq].ps160 = 0;

		ru_info[sta_seq].valid = TRUE;
		/* ru caterogy */
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   (u_char *)&value, hqa_frame_ctrl);
		param_loop--;
		/* ru allocation */
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   (u_char *)&value, hqa_frame_ctrl);
		param_loop--;
		hqa_translate_ru_allocation(value,
				    &ru_info[sta_seq].allocation);
		/* aid */
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   (u_char *)&value, hqa_frame_ctrl);
		param_loop--;
		ru_info[sta_seq].aid = value;
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   (u_char *)&value, hqa_frame_ctrl);
		param_loop--;
		ru_info[sta_seq].ru_index = value;
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   (u_char *)&value, hqa_frame_ctrl);
		param_loop--;
		ru_info[sta_seq].rate = value;
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   (u_char *)&value, hqa_frame_ctrl);
		param_loop--;
		ru_info[sta_seq].ldpc = value;
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   (u_char *)&value, hqa_frame_ctrl);
		param_loop--;
		ru_info[sta_seq].nss = value;
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   (u_char *)&value, hqa_frame_ctrl);
		param_loop--;
		ru_info[sta_seq].start_sp_st = value-1;
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   (u_char *)&value, hqa_frame_ctrl);
		param_loop--;
		ru_info[sta_seq].mpdu_length = value;


		if (param_loop) {
			get_param_and_shift_buf(TRUE,
						   sizeof(u_int32),
						   (u_char *)&value, hqa_frame_ctrl);
			param_loop--;
			ru_info[sta_seq].alpha = value;
		}

		if (param_loop) {
			get_param_and_shift_buf(TRUE,
						   sizeof(u_int32),
						   (u_char *)&value, hqa_frame_ctrl);
			param_loop--;
			ru_info[sta_seq].ru_mu_nss = value;
		}

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			 "ru_segment[%d][0x%x]: ru_idx:%d\n",
			 ru_info[sta_seq].b0,
			 ru_info[sta_seq].allocation,
			 ru_info[sta_seq].ru_index);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			 "\t\t\t\trate:%x, ldpc:%d\n",
			 ru_info[sta_seq].rate,
			 ru_info[sta_seq].ldpc);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			 "\t\t\t\tnss:%d, mimo nss:%d\n",
			 ru_info[sta_seq].nss,
			 ru_info[sta_seq].ru_mu_nss);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			 "\t\t\t\t start spatial stream:%d,\n",
			 ru_info[sta_seq].start_sp_st);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			 "\t\t\t\tmpdu length=%d, alpha:%d\n",
			 ru_info[sta_seq].mpdu_length,
			 ru_info[sta_seq].alpha);
	}

	ret = serv_test->test_op->op_set_ru_info(
		serv_test->test_winfo,
		band_idx,
		ru_info);

err_out:
	update_hqa_frame(hqa_frame_ctrl, resp_len, ret);

	return ret;
}

static s_int32 hqa_set_ru_info_v2(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
#define SEG_CNT 4

	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 resp_len = 2;
	u_int32 band_idx = (u_int32)(serv_test->ctrl_band_idx);
	u_int32 len = 0, seg_sta_cnt[SEG_CNT] = {0}, sta_seq = 0, value = 0, u4SegCount = 0;
	u_char param_cnt = 0, segment_idx = 0, param_loop = 0;
	struct test_ru_allocatoin *ru_allocation = NULL;
	struct test_ru_info *ru_info = NULL;
	u_int8 cnt1, cnt2;

	len = hqa_frame_ctrl->hqa_frame_eth->length;

	/*band idx*/
	get_param_and_shift_buf(TRUE,
				   sizeof(u_int32),
				   (u_char *)&band_idx, hqa_frame_ctrl);

	/*seg count*/
	get_param_and_shift_buf(TRUE,
				   sizeof(u_int32),
				   (u_char *)&u4SegCount, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx:%d error\n", band_idx);
		ret = SERV_STATUS_AGENT_INVALID_BANDIDX;
		return ret;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	param_cnt = sizeof(struct test_ru_info_host) / sizeof(u_int32);	/* param count */
	ru_allocation = CONFIG_GET_PADDR(serv_test, ru_alloc);
	ru_info = CONFIG_GET_PADDR(serv_test, ru_info_list[0]);
	sys_ad_zero_mem(ru_info, sizeof(struct test_ru_info)*MAX_MULTI_TX_STA);

	for (cnt1 = 0; cnt1 < SUB20_NUM; ++cnt1)
		ru_allocation->sub20[cnt1] = UNUSED_RU;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		 "\t\tBandidx:%d, SegCount:%d, Len:%d, ParamCount:%d\n",
			band_idx, u4SegCount, len, param_cnt);

	if (u4SegCount > SEG_CNT) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	/* for maximum bw 80+80/160, 2 segments only */
	for (cnt1 = 0; cnt1 < u4SegCount ; cnt1++) {
		get_param_and_shift_buf(TRUE,
				   sizeof(u_int32),
				   (u_char *)&seg_sta_cnt[cnt1], hqa_frame_ctrl);

		segment_idx = cnt1;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		 "_segment(%d), sta_count:%d\n",
		 cnt1, seg_sta_cnt[cnt1]);

		if ((seg_sta_cnt[0] > SEG_STA_CNT) || (seg_sta_cnt[1] > SEG_STA_CNT)
			|| (seg_sta_cnt[2] > SEG_STA_CNT) || (seg_sta_cnt[3] > SEG_STA_CNT)) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		for (cnt2 = 0; cnt2 < seg_sta_cnt[cnt1]; cnt2++) {
			param_loop = param_cnt;

			ru_info[sta_seq].valid = TRUE;
			/* ru caterogy */
			get_param_and_shift_buf(TRUE,
						   sizeof(u_int32),
						   (u_char *)&value, hqa_frame_ctrl);
			param_loop--;
			/* ru allocation */
			get_param_and_shift_buf(TRUE,
						   sizeof(u_int32),
						   (u_char *)&value, hqa_frame_ctrl);
			param_loop--;
			ru_info[sta_seq].allocation = value;
			/* aid, STA_ID */
			get_param_and_shift_buf(TRUE,
						   sizeof(u_int32),
						   (u_char *)&value, hqa_frame_ctrl);
			param_loop--;
			ru_info[sta_seq].aid = value;

			/* RU index */
			get_param_and_shift_buf(TRUE,
						   sizeof(u_int32),
						   (u_char *)&value, hqa_frame_ctrl);
			param_loop--;
			ru_info[sta_seq].ru_index = value;

			/* MCS */
			get_param_and_shift_buf(TRUE,
						   sizeof(u_int32),
						   (u_char *)&value, hqa_frame_ctrl);
			param_loop--;
			ru_info[sta_seq].rate = value;

			/* LDPC */
			get_param_and_shift_buf(TRUE,
						   sizeof(u_int32),
						   (u_char *)&value, hqa_frame_ctrl);
			param_loop--;
			ru_info[sta_seq].ldpc = value;

			/* nss */
			get_param_and_shift_buf(TRUE,
						   sizeof(u_int32),
						   (u_char *)&value, hqa_frame_ctrl);
			param_loop--;
			ru_info[sta_seq].nss = value;

			/* start spatial stream */
			get_param_and_shift_buf(TRUE,
						   sizeof(u_int32),
						   (u_char *)&value, hqa_frame_ctrl);
			param_loop--;
			ru_info[sta_seq].start_sp_st = value-1;


			/* MPDU length */
			get_param_and_shift_buf(TRUE,
						   sizeof(u_int32),
						   (u_char *)&value, hqa_frame_ctrl);
			param_loop--;
			ru_info[sta_seq].mpdu_length = value;

			/* alpha , power? */
			if (param_loop) {
				get_param_and_shift_buf(TRUE,
							   sizeof(u_int32),
							   (u_char *)&value, hqa_frame_ctrl);
				param_loop--;
				ru_info[sta_seq].alpha = value;
			}

			/* MU NSS */
			if (param_loop) {
				get_param_and_shift_buf(TRUE,
							   sizeof(u_int32),
							   (u_char *)&value, hqa_frame_ctrl);
				param_loop--;
				ru_info[sta_seq].ru_mu_nss = value;
			}

			/* ps 80 */
			if (cnt1 % 2 == 0)
				ru_info[sta_seq].b0 = 0;
			else
				ru_info[sta_seq].b0 = 1;

			/* handle ps160 */
			if (cnt1 < 2)	// segment0 , segment1
				ru_info[sta_seq].ps160 = 0;
			else	// segment2 , segment3
				ru_info[sta_seq].ps160 = 1;

			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				 "segment_idx[%d]allocation[0x%x]: ru_idx:%d\n",
				 segment_idx,
				 ru_info[sta_seq].allocation,
				 ru_info[sta_seq].ru_index >> 1);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				 "\t\t\t\trate:%x, ldpc:%d\n",
				 ru_info[sta_seq].rate,
				 ru_info[sta_seq].ldpc);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				 "\t\t\t\tnss:%d, mimo nss:%d\n",
				 ru_info[sta_seq].nss,
				 ru_info[sta_seq].ru_mu_nss);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				 "\t\t\t\t start spatial stream:%d,\n",
				 ru_info[sta_seq].start_sp_st);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
				 "\t\t\t\tmpdu length=%d, alpha:%d,ps160:%d\n\n",
				 ru_info[sta_seq].mpdu_length,
				 ru_info[sta_seq].alpha,
				 ru_info[sta_seq].ps160);

			/* Sta RU info, done */
			sta_seq++;

		}
	}

	ret = serv_test->test_op->op_set_ru_info(
		serv_test->test_winfo,
		band_idx,
		ru_info);

err_out:

	update_hqa_frame(hqa_frame_ctrl, resp_len, ret);

	return ret;
}


static s_int32 hqa_w_cali(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 resp_len = 2;
	u_char *data = hqa_frame_ctrl->hqa_frame_eth->data;
	u_int32 storage = 0;

	get_param_and_shift_buf(TRUE,
				   sizeof(u_int32),
				   (u_char *)&storage, hqa_frame_ctrl);

	mt_serv_store_cali(serv_test, storage, data+4);
	update_hqa_frame(hqa_frame_ctrl, resp_len, ret);

	return ret;
}

static s_int32 hqa_set_txpwr_drop(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = 0, cPowerDropLevel = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(cPowerDropLevel),
				(u_char *)&cPowerDropLevel, hqa_frame_ctrl);


	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"cPowerDropLevel=%d, band_idx=%d\n", cPowerDropLevel, band_idx);

	ret = mt_serv_set_txpwr_drop(serv_test, (s_int8)(cPowerDropLevel),
		(u_char)(band_idx));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_txpwr_percentage(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = 0, enable = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(enable),
				(u_char *)&enable, hqa_frame_ctrl);


	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"Enable=%d, band_idx=%d\n", enable, band_idx);

	ret = mt_serv_set_txpwr_percentage(serv_test, (boolean)(enable),
		(u_char)(band_idx));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}


static struct hqa_cmd_entry CMD_SET5[] = {
	/* cmd id start from 0x1500 */
	{0x0,	hqa_get_fw_info, "GetFwInfo"},
	{0x1,	legacy_function, "TODO"},
	{0x2,	hqa_set_stbc, "SetStbc"},
	{0x3,	hqa_set_short_gi, "SetShortGi"},
	{0x4,	hqa_set_dpd, "SetDpd"},
	{0x5,	hqa_set_tssi_onoff, "SetTssiOnOff"},
	{0x6,	legacy_function, "TODO"},
	{0x7,	legacy_function, "TODO"},
	{0x8,	legacy_function, "TODO"},
	{0x9,	hqa_calibration_test_mode, "CalibrationTestMode"},
	{0xa,	hqa_do_cal_item, "DoCalItem"},
	{0xf,	hqa_tmr_setting, "TmrSetting"},
	{0x10,	legacy_function, "TODO"},
	{0x11,	hqa_w_cali, "WCali"},
	{0x12,	legacy_function, "TODO"},
	{0x13,	legacy_function, "TODO"},
	{0x14,	hqa_get_chipid, "GetChipid"},
	{0x15,	hqa_mps_set_seq_data, "MpsSetSeqData"},
	{0x16,	hqa_mps_set_payload_length, "MpsSetPayloadLen"},
	{0x17,	hqa_mps_set_packet_count, "MpsSetPktCnt"},
	{0x18,	hqa_mps_set_power_gain, "MpsSetPwrGain"},
	{0x19,	hqa_mps_start, "MpsStart"},
	{0x1a,	hqa_mps_stop, "MpsStop"},
	{0x1b,	hqa_get_sub_chipid, "GetSubChipId"},
	{0x1c,	hqa_get_rx_statistics_all, "GetRXStatisticsAllNew"},
	{0x1d,	hqa_get_chip_capability, "GetTestCap"},
	{0x1e,	hqa_get_rf_type_capability, "GetRfTypeCap"},
	{0x21,	legacy_function, "TODO"},
	{0x22,	hqa_check_efuse_mode_type, "ChkEfuseModeType"},
	{0x23,	hqa_check_efuse_nativemode_type, "ChkEfuseNativemodeType"},
	{0x24,	legacy_function, "TODO"},
	{0x25,	legacy_function, "TODO"},
	{0x26,	legacy_function, "TODO"},
	{0x27,	legacy_function, "TODO"},
	{0x28,	legacy_function, "TODO"},
	{0x29,	legacy_function, "TODO"},
	{0x2c,	hqa_set_band_mode, "SetBandMode"},
	{0x2d,	hqa_get_band_mode, "GetBandMode"},
	{0x2e,	hqa_rdd_start, "RddStart"},
	{0x2f,	hqa_rdd_stop, "RddStop"},
	{0x31,	legacy_function, "TODO"},
	{0x32,	legacy_function, "TODO"},
	{0x33,	hqa_log_on_off, "LogOnOff"},
	{0x34,	legacy_function, "TODO"},
	{0x35,	legacy_function, "TODO"},
	{0x36,	hqa_mps_set_nss, "MpsSetNss"},
	{0x37,	hqa_mps_set_per_packet_bw, "MpsSetPerPktBw"},
	{0x80,	hqa_icap_ctrl, "CapWiFiSpectrum"},
	{0x81,	hqa_get_dump_recal, "GetDumpRecal"},
	{0x82,	hqa_get_dump_rxv_legacy, "GetDumpRxvLegacy"},
	{0x83,	hqa_get_dump_rdd, "GetDumpRdd"},
	{0x84,	hqa_get_rxv_dump, "GetRxvDump"},
	{0x90,	hqa_set_tx_pe, "SetTxPe"},
	{0x91,	hqa_get_hetb_info, "GetHetbInfo"},
	{0x94,	hqa_set_ru_info, "SetRuInfo"},
	{0x96,	hqa_set_ru_info_v2, "SetRuInfoV2"},
	{0x9e,	hqa_set_txpwr_percentage, "SetTxpwrPercentage"},
	{0x9f,  hqa_set_txpwr_drop, "SetTxpwrDrop"},
	{0xf0,  hqa_calibration_test_mode, "SetModeEng"},
	{0xf1,  hqa_get_test_engine, "GetTestEng"},
	{0xf2,  hqa_set_test_engine, "SetTestEng"}
};

static s_int32 hqa_set_channel_ext(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_set_ch param;

	memset(&param, 0, sizeof(struct hqa_set_ch));
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(param.ext_id),
				(u_char *)&param.ext_id, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.num_param),
				(u_char *)&param.num_param, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.band_idx),
				(u_char *)&param.band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.central_ch0),
				(u_char *)&param.central_ch0, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.central_ch1),
				(u_char *)&param.central_ch1, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.sys_bw),
				(u_char *)&param.sys_bw, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.perpkt_bw),
				(u_char *)&param.perpkt_bw, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.pri_sel),
				(u_char *)&param.pri_sel, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.reason),
				(u_char *)&param.reason, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.ch_band),
				(u_char *)&param.ch_band, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.out_band_freq),
				(u_char *)&param.out_band_freq, hqa_frame_ctrl);

	if (param.num_param >= 10 || hqa_frame_ctrl->hqa_frame_eth->length >= 48) {
		get_param_and_shift_buf(TRUE, sizeof(param.indication_specific),
				(u_char *)&param.indication_specific, hqa_frame_ctrl);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"indication_specific = 0x%08x\n", param.indication_specific);
	}

	if (param.band_idx >= TEST_BAND_NUM) {
		ret = SERV_STATUS_AGENT_INVALID_BANDIDX;
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx setting error!\n");
		goto err_out;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, param.band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, channel,
			(u_char)param.central_ch0);
	CONFIG_SET_PARAM(serv_test, channel_2nd,
			(u_char)param.central_ch1);
	CONFIG_SET_PARAM(serv_test, per_pkt_bw,
			(u_char)param.perpkt_bw);
	CONFIG_SET_PARAM(serv_test, bw,
			(u_char)param.sys_bw);
	CONFIG_SET_PARAM(serv_test, pri_sel,
			(u_char)param.pri_sel);
	CONFIG_SET_PARAM(serv_test, ch_band,
			(u_char)param.ch_band);
	CONFIG_SET_PARAM(serv_test, out_band_freq,
			(u_int32)param.out_band_freq);
	CONFIG_SET_PARAM(serv_test, indication_specific,
			(u_int32)param.indication_specific);

	ret = mt_serv_set_channel(serv_test);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"band_idx: %d, ch0: %d, ch1: %d, sys_bw: %d, ",
		param.band_idx, param.central_ch0,
		param.central_ch1, param.sys_bw);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"perpkt_bw: %d, pri_sel: %d, ch_band: %d\n",
		param.perpkt_bw, param.pri_sel, param.ch_band);

err_out:
	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, (u_char *) &param.ext_id, 4);
	update_hqa_frame(hqa_frame_ctrl, 6, ret);

	return ret;
}

static s_int32 hqa_set_txcontent_ext(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
#define CUSTOMIZED_MAC_ADDR 0x80

	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_tx_content param;
	struct serv_hdr_802_11 *phdr = NULL;
	u_char *payload, sta_seq = 0;
	boolean enable = FALSE;
	u_int32 i = 0;
	u_int32 UserIdx = 0;

	memset(&param, 0, sizeof(struct hqa_tx_content));
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(param.ext_id),
				(u_char *)&param.ext_id, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.num_param),
				(u_char *)&param.num_param, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.band_idx),
				(u_char *)&param.band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.fc),
				(u_char *)&param.fc, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.dur),
				(u_char *)&param.dur, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.seq),
				(u_char *)&param.seq, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.fixed_payload),
				(u_char *)&param.fixed_payload, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.txlen),
				(u_char *)&param.txlen, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.payload_len),
				(u_char *)&param.payload_len, hqa_frame_ctrl);
	get_param_and_shift_buf(FALSE, SERV_MAC_ADDR_LEN,
				param.addr1, hqa_frame_ctrl);
	get_param_and_shift_buf(FALSE, SERV_MAC_ADDR_LEN,
				param.addr2, hqa_frame_ctrl);
	get_param_and_shift_buf(FALSE, SERV_MAC_ADDR_LEN,
				param.addr3, hqa_frame_ctrl);

	/* 52 for the size before payload */
	if (param.payload_len > TEST_MAX_PATTERN_SIZE)
		param.payload_len = TEST_MAX_PATTERN_SIZE;

	if (param.band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"param.band_idx: %d error!\n", param.band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, param.band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* handle MAC address */
	if (param.fixed_payload & CUSTOMIZED_MAC_ADDR) {
		//RDV tool case , set specified multi-user mac addr given by iwpriv HQA cmd
		if (param.num_param >= 11)
			get_param_and_shift_buf(TRUE, sizeof(UserIdx),
				(u_char *)&UserIdx, hqa_frame_ctrl);

		if (UserIdx < MAX_MULTI_TX_STA) {
			CONFIG_SET_PADDR(serv_test, addr1[UserIdx], param.addr1,
					SERV_MAC_ADDR_LEN);
			CONFIG_SET_PADDR(serv_test, addr2[UserIdx], param.addr2,
					SERV_MAC_ADDR_LEN);
			CONFIG_SET_PADDR(serv_test, addr3[UserIdx], param.addr3,
					SERV_MAC_ADDR_LEN);
		}
	} else {
		//QA tool case , set auto-gen multi-user mac addr by QA tool MAC addr
		for (sta_seq = 0; sta_seq < MAX_MULTI_TX_STA ; sta_seq++) {
			param.addr1[5] +=
			sta_seq+(param.band_idx*(MAX_MULTI_TX_STA/TEST_BAND_NUM));
			CONFIG_SET_PADDR(serv_test, addr1[sta_seq], param.addr1,
					SERV_MAC_ADDR_LEN);
			CONFIG_SET_PADDR(serv_test, addr2[sta_seq], param.addr2,
					SERV_MAC_ADDR_LEN);
			CONFIG_SET_PADDR(serv_test, addr3[sta_seq], param.addr3,
					SERV_MAC_ADDR_LEN);
		}
	}
	param.fixed_payload &= ~CUSTOMIZED_MAC_ADDR;

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, dur,
			(u_short)param.dur);
	CONFIG_SET_PARAM(serv_test, seq,
			(u_short)param.seq);
	CONFIG_SET_PARAM(serv_test, pl_len,
			(u_int32)param.payload_len);
	CONFIG_SET_PARAM(serv_test, tx_len,
			(u_int32)param.txlen);
	CONFIG_SET_PARAM(serv_test, fixed_payload,
			(u_int32)param.fixed_payload);

	payload = CONFIG_GET_PADDR(serv_test, payload[0]);
	phdr = (struct serv_hdr_802_11 *)CONFIG_GET_PADDR(serv_test,
							template_frame);
	sys_ad_move_mem(&phdr->fc, &param.fc, sizeof(phdr->fc));
	phdr->duration = (u_int16)param.dur;
	phdr->sequence = (u_int16)param.seq;

	/* Error check for txlen and payload_len */
	/* the fixed_payload means Normal : 0,  repeat:1 , random:2*/
	/* random:2 dont follow this rule*/
	if ((param.txlen == 0) || (param.payload_len == 0)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"txlen/payload_len=%u/%u can't be 0!!\n",
			param.txlen, param.payload_len);
		return SERV_STATUS_AGENT_INVALID_LEN;
	}

	/* Packet tx time feature implementation */
	enable = CONFIG_GET_PARAM(serv_test, tx_time_param.pkt_tx_time_en);
	if (enable == TRUE) {
		CONFIG_SET_PARAM(serv_test, tx_time_param.pkt_tx_time,
				(u_int32)param.txlen);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"fc=0x%04x, dur=%u, seq=%u, plen=%u\n",
			param.fc, param.dur, param.seq,
			param.payload_len);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"pkt_tx_time=%u, genpkt=%u\n",
			param.txlen,
			param.fixed_payload);
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"fc=0x%04x, dur=%u, seq=%u, plen=%u\n",
			param.fc, param.dur, param.seq,
			param.payload_len);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"txlen=%u, genpkt=%u\n",
			param.txlen, param.fixed_payload);
	}

	for (i = 0; i < param.payload_len; i++)
		get_param_and_shift_buf(FALSE, sizeof(u_char), payload + i, hqa_frame_ctrl);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"useridx=%u, fixed_payload=%u\n",
			UserIdx, param.fixed_payload);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"addr1=%02x:%02x:%02x:%02x:%02x:%02x\n",
		param.addr1[0], param.addr1[1], param.addr1[2],
		param.addr1[3], param.addr1[4], param.addr1[5]);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"addr2=%02x:%02x:%02x:%02x:%02x:%02x\n",
		param.addr2[0], param.addr2[1], param.addr2[2],
		param.addr2[3], param.addr2[4], param.addr2[5]);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"addr3=%02x:%02x:%02x:%02x:%02x:%02x\n",
		param.addr3[0], param.addr3[1], param.addr3[2],
		param.addr3[3], param.addr3[4], param.addr3[5]);

	ret = mt_serv_set_tx_content(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, (u_char *) &param.ext_id,
			sizeof(param.ext_id));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(param.ext_id), ret);

	return ret;
}

static s_int32 hqa_start_tx_ext(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_tx param;
	u_char ant_idx = 0;

	memset(&param, 0, sizeof(struct hqa_tx));
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(param.ext_id),
				(u_char *)&param.ext_id, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.num_param),
				(u_char *)&param.num_param, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.band_idx),
				(u_char *)&param.band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.pkt_cnt),
				(u_char *)&param.pkt_cnt, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.tx_mode),
				(u_char *)&param.tx_mode, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.rate),
				(u_char *)&param.rate, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.pwr),
				(u_char *)&param.pwr, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.stbc),
				(u_char *)&param.stbc, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.ldpc),
				(u_char *)&param.ldpc, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.ibf),
				(u_char *)&param.ibf, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.ebf),
				(u_char *)&param.ebf, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.wlan_id),
				(u_char *)&param.wlan_id, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.aifs),
				(u_char *)&param.aifs, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.gi),
				(u_char *)&param.gi, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.tx_path),
				(u_char *)&param.tx_path, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.nss),
				(u_char *)&param.nss, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.hw_tx_enable),
				(u_char *)&param.hw_tx_enable, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.preampunc),
				(u_char *)&param.preampunc, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param.wlan_id_offset),
				(u_char *)&param.wlan_id_offset, hqa_frame_ctrl);

	if (!param.pkt_cnt)
		param.pkt_cnt = 0x8fffffff;

	if (param.band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx:%d error!\n", param.band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, param.band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}


	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, tx_stat.tx_cnt,
			(u_int32)param.pkt_cnt);
	CONFIG_SET_PARAM(serv_test, tx_mode,
			(u_char)param.tx_mode);
	CONFIG_SET_PARAM(serv_test, mcs,
			(u_char)param.rate);
	CONFIG_SET_PARAM(serv_test, stbc,
			(u_char)param.stbc);
	CONFIG_SET_PARAM(serv_test, ldpc,
			(u_char)param.ldpc);
	if (param.tx_path & TEST_ANT_USER_DEF)
		CONFIG_SET_PARAM(serv_test, tx_ant,
				(u_int32)param.tx_path);
	CONFIG_SET_PARAM(serv_test, nss,
			(u_char)param.nss);
	CONFIG_SET_PARAM(serv_test, sgi,
			(u_char)param.gi);
	CONFIG_SET_PARAM(serv_test, wcid_ref,
			(u_int8)param.wlan_id);
	CONFIG_SET_PARAM(serv_test, ipg_param.ipg,
			(u_int32)param.aifs);
	CONFIG_SET_PARAM(serv_test, ibf,
			(u_char)param.ibf);
	CONFIG_SET_PARAM(serv_test, ebf,
			(u_char)param.ebf);
	for (ant_idx = 0; ant_idx < TEST_ANT_NUM; ant_idx++)
		CONFIG_SET_PARAM(serv_test, tx_pwr[ant_idx],
			(u_int32)param.pwr);
	WINFO_SET_PARAM(serv_test, hw_tx_enable, param.hw_tx_enable);

	CONFIG_SET_PARAM(serv_test, preampunc,
			(u_int16)param.preampunc);
	CONFIG_SET_PARAM(serv_test, wlan_id_offset,
			(u_int32)param.wlan_id_offset);

	if ((param.hw_tx_enable & TX_CFG_SUBMIT_SKIP) == 0)
		ret = mt_serv_submit_tx(serv_test);
	else
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"skip mt_serv_submit_tx, hw_tx_enable=%u\n",
		param.hw_tx_enable);

	if (ret != SERV_STATUS_SUCCESS)
		goto err_out;

	if ((param.hw_tx_enable & TX_CFG_FRAME_SKIP) == 0)
		ret = mt_serv_start_tx(serv_test);
	else
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"skip mt_serv_start_tx, hw_tx_enable=%u\n",
		param.hw_tx_enable);

	param.hw_tx_enable &= ~(TX_CFG_FRAME_SKIP | TX_CFG_SUBMIT_SKIP);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"param.num=%u,band_idx=%u, pkt_cnt=0x%4x, phy=%u\n",
		param.num_param, param.band_idx, param.pkt_cnt, param.tx_mode);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"pwr=%u, mcs=%u, stbc=%u, ldpc=%u\n",
		param.pwr, param.rate, param.stbc, param.ldpc);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"ibf=%u, ebf=%u, wlan_id=%u, aifs=%u\n",
		param.ibf, param.ebf, param.wlan_id, param.aifs);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"gi=%u, nss=%u, hwtx=%u ,preampunc=%d\n",
		param.gi, param.nss, param.hw_tx_enable, param.preampunc);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"wlan_id_offset=%x\n",
		param.wlan_id_offset);

	if (param.tx_path & TEST_ANT_USER_DEF)
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"tx_path=0x%x\n",
			param.tx_path);

err_out:
	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, &param.ext_id,
			sizeof(param.ext_id));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(param.ext_id), ret);

	return ret;
}

static s_int32 hqa_start_rx_ext(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char own_mac[SERV_MAC_ADDR_LEN] = {0};
	u_int32 ext_id = 0, param_num = 0, band_idx = 0, rx_path = 0;
	u_int32 user_idx = 0, tx_mode = 0, ltf_gi = 0, mu_rx_aid = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	sys_ad_zero_mem(own_mac, SERV_MAC_ADDR_LEN);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&ext_id, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&param_num, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(FALSE, SERV_MAC_ADDR_LEN,
				(u_char *)own_mac, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&rx_path, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx:%d error\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&mu_rx_aid, hqa_frame_ctrl);
	if (param_num > 3) {
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&tx_mode, hqa_frame_ctrl);
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&ltf_gi, hqa_frame_ctrl);
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&user_idx, hqa_frame_ctrl);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"param num:%d\n", param_num);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"\ttx_mode:%d, ltf_gi:%d, user_idx:%d\n",
			tx_mode, ltf_gi, user_idx);
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* Set parameters */
	CONFIG_SET_PADDR(serv_test, own_mac, own_mac,
			SERV_MAC_ADDR_LEN);
	CONFIG_SET_PARAM(serv_test, tx_mode, tx_mode);
	CONFIG_SET_PARAM(serv_test, sgi, ltf_gi);
	CONFIG_SET_PARAM(serv_test, user_idx, (u_int16)user_idx);
	CONFIG_SET_PARAM(serv_test, mu_rx_aid, mu_rx_aid);

	ret = mt_serv_start_rx(serv_test);

	if (ret)
		return ret;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"param num=%u, band_idx=%u , mu_rx_aid=%d\n",
		param_num, band_idx, CONFIG_GET_PARAM(serv_test, mu_rx_aid));
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
		SERV_PRINT_MAC(own_mac));

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, (u_char *) &ext_id,
				sizeof(ext_id));

	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(ext_id), ret);

	return ret;
}

static s_int32 hqa_stop_tx_ext(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 ext_id = 0, param_num = 0, band_idx = 0;
	u_int32 txconfig = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(ext_id),
				(u_char *)&ext_id, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param_num),
				(u_char *)&param_num, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	if (param_num >= 2) {
		get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&txconfig, hqa_frame_ctrl);
	}

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx=%d error !\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);

	if ((txconfig & TX_CFG_FRAME_SKIP) == 0)
		ret = mt_serv_stop_tx(serv_test);
	else {
		ret = SERV_STATUS_SUCCESS;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"skip mt_serv_stop_tx, txconfig=%u\n",
		txconfig);
	}

	if (ret != SERV_STATUS_SUCCESS)
		goto err_out;

	if ((txconfig & TX_CFG_SUBMIT_SKIP) == 0)
		ret = mt_serv_revert_tx(serv_test);
	else {
		ret = SERV_STATUS_SUCCESS;

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"skip mt_serv_stop_tx, txconfig=%u\n",
		txconfig);
	}

	if (ret != SERV_STATUS_SUCCESS)
		goto err_out;

err_out:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"band_idx=%u\n", band_idx);
	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, (u_char *) &ext_id,
			sizeof(ext_id));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(ext_id), ret);

	return ret;
}

static s_int32 hqa_stop_rx_ext(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 ext_id = 0, param_num = 0, band_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(ext_id),
				(u_char *)&ext_id, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(param_num),
				(u_char *)&param_num, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);

	/* Set parameters */
	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx=%u error\n", band_idx);
		goto err_out;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);

	ret = mt_serv_stop_rx(serv_test);
	if (ret != SERV_STATUS_SUCCESS)
		goto err_out;

err_out:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"band_idx=%u\n", band_idx);
	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, (u_char *) &ext_id,
			sizeof(ext_id));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(ext_id), ret);

	return ret;
}


static s_int32 hqa_set_tx_time(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 ext_id = 0, is_tx_time = 0, band_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	get_param_and_shift_buf(TRUE, sizeof(ext_id),
				(u_char *)&ext_id, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(is_tx_time),
				(u_char *)&is_tx_time, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"band_idx=%u error!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}


	/* Set parameters */
	/* 0: use tx length, 1: use tx time */
	if (is_tx_time == 1)
		CONFIG_SET_PARAM(serv_test, tx_time_param.pkt_tx_time_en,
				TRUE);
	else {
		CONFIG_SET_PARAM(serv_test, tx_time_param.pkt_tx_time_en,
				FALSE);
		/* Reset to 0 when start tx everytime */
		CONFIG_SET_PARAM(serv_test, tx_time_param.pkt_tx_time,
				0);
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"band_idx=%u, is_tx_time=%u\n",
		band_idx, is_tx_time);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, (u_char *) &ext_id,
			sizeof(ext_id));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(ext_id), ret);

	return ret;
}

static s_int32 hqa_off_ch_scan(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 ext_id = 0, band_idx = 0, mntr_ch = 0;
	u_int32 is_aband = 0, mntr_bw = 0, mntr_tx_rx_pth = 0;
	u_int32 scan_mode = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Get parameters from command frame */
	/* Data sequences of command frame:
	*  ext_id (4 bytes) + dbdc_idx (4 bytes) + mntr_ch (4 bytes) +
	*  is_aband (4 bytes) + mntr_bw (4 bytes) +
	*  mntr_tx_rx_pth (4 bytes) + scan_mode (4 bytes)
	*/
	get_param_and_shift_buf(TRUE, sizeof(ext_id),
			(u_char *)&ext_id, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
			(u_char *)&band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(mntr_ch),
			(u_char *)&mntr_ch, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(is_aband),
			(u_char *)&is_aband, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(mntr_bw),
			(u_char *)&mntr_bw, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(mntr_tx_rx_pth),
			(u_char *)&mntr_tx_rx_pth, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(scan_mode),
			(u_char *)&scan_mode, hqa_frame_ctrl);

	switch (mntr_bw) {
	case HQA_BAND_WIDTH_20:
		mntr_bw = SYS_BW_20;
		break;

	case HQA_BAND_WIDTH_40:
		mntr_bw = SYS_BW_40;
		break;

	case HQA_BAND_WIDTH_80:
		mntr_bw = SYS_BW_80;
		break;

	case HQA_BAND_WIDTH_10:
		mntr_bw = TEST_BW_10;
		break;

	case HQA_BAND_WIDTH_5:
		mntr_bw = TEST_BW_5;
		break;

	case HQA_BAND_WIDTH_160:
		mntr_bw = SYS_BW_160;
		break;

	case HQA_BAND_WIDTH_8080:
		mntr_bw = TEST_BW_160NC;
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"bw 0x%x is out of range\n", mntr_bw);
		mntr_bw = TEST_BW_20;
		break;
	}

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"dbdc_idx %d error\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, off_ch_param.ext_id,
			(u_int32)ext_id);
	CONFIG_SET_PARAM(serv_test, off_ch_param.dbdc_idx,
			(u_int32)band_idx);
	CONFIG_SET_PARAM(serv_test, off_ch_param.mntr_ch,
			(u_int32)mntr_ch);
	CONFIG_SET_PARAM(serv_test, off_ch_param.is_aband,
			(u_int32)is_aband);
	CONFIG_SET_PARAM(serv_test, off_ch_param.mntr_bw,
			(u_int32)mntr_bw);
	CONFIG_SET_PARAM(serv_test, off_ch_param.mntr_tx_rx_pth,
			(u_int32)mntr_tx_rx_pth);
	CONFIG_SET_PARAM(serv_test, off_ch_param.scan_mode,
			(u_int32)scan_mode);

	ret = mt_serv_set_off_ch_scan(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, (u_char *) &ext_id,
			sizeof(ext_id));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(ext_id), ret);

	return ret;
}

static s_int32 hqa_testmode_at_cmd_get(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 funcId = 0, data = 0, ret_data = 0;
	u_int32 ext_id = 0;
	struct test_wlan_info *winfos = serv_test->test_winfo;

	get_param_and_shift_buf(TRUE, sizeof(ext_id),
			(u_char *)&ext_id, hqa_frame_ctrl);
	/* key */
	get_param_and_shift_buf(TRUE, sizeof(funcId),
		(u_char *)&funcId, hqa_frame_ctrl);
	/* subkey */
	get_param_and_shift_buf(TRUE, sizeof(data),
		(u_char *)&data, hqa_frame_ctrl);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"funcId: %d, value: %d\n", funcId, data);

	ret = net_ad_get(winfos, funcId, data, &ret_data);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"funcId:0x%x, data:0x%x, ret_data: 0x%x\n",
		funcId, data, ret_data);

	ret_data = SERV_OS_HTONL(ret_data);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, (u_char *) &ext_id,
			sizeof(ext_id));
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2 + sizeof(ext_id),
			(u_char *) &ret_data, sizeof(ret_data));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(ext_id) + sizeof(ret_data), ret);

	return ret;
}

static s_int32 hqa_testmode_at_cmd_set(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 funcId = 0, data = 0;
	u_int32 ext_id = 0;
	struct test_wlan_info *winfos = serv_test->test_winfo;

	get_param_and_shift_buf(TRUE, sizeof(ext_id),
			(u_char *)&ext_id, hqa_frame_ctrl);
	/* key */
	get_param_and_shift_buf(TRUE, sizeof(funcId),
		(u_char *)&funcId, hqa_frame_ctrl);
	/* value */
	get_param_and_shift_buf(TRUE, sizeof(data),
		(u_char *)&data, hqa_frame_ctrl);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"funcId: %d, value: %d\n", funcId, data);

	net_ad_set(winfos, funcId, data);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"funcId:0x%x, data:0x%x\n", funcId, data);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2, (u_char *) &ext_id,
			sizeof(ext_id));
	update_hqa_frame(hqa_frame_ctrl, 2 + sizeof(ext_id), ret);

	return ret;
}

static s_int32 hqa_set_ch_power_offset(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = 0, ant_idx = 0;
	u_int32 ch_pwr_offset = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(ant_idx),
				(u_char *)&ant_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(ch_pwr_offset),
				(u_char *)&ch_pwr_offset, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx: %d error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	if (ant_idx >= TEST_ANT_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ant_idx: %d error!!\n", ant_idx);
		return SERV_STATUS_AGENT_INVALID_PARAM;
	}

	if (!((ch_pwr_offset >= 0x80 && ch_pwr_offset <= 0x8f) ||
		(ch_pwr_offset >= 0xc0 && ch_pwr_offset <= 0xcf))) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"Channel offset: +-7.5dB, min: 0x8f/max: 0xcf, cur: %2x\n", ch_pwr_offset);
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err;
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, pwr_param.band_idx,
			(u_int32)band_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.ant_idx,
			(u_int32)ant_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.ch_pwr_offset,
			(u_int32)ch_pwr_offset);

	ret = mt_serv_tx_power_operation(serv_test, SERV_TEST_TXPWR_SET_CH_PWR_OFFSET);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"ch_pwr_offset=%2x, band_idx=%u, ant_idx:%u\n",
		ch_pwr_offset, band_idx, ant_idx);
err:
	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}

static s_int32 hqa_set_cvt_power_offset(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 band_idx = 0, ant_idx = 0;
	u_int32 cvt_pwr_offset = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				(u_char *)&band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(ant_idx),
				(u_char *)&ant_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(cvt_pwr_offset),
				(u_char *)&cvt_pwr_offset, hqa_frame_ctrl);

	if (band_idx >= TEST_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"band_idx: %d error!!\n", band_idx);
		return SERV_STATUS_AGENT_INVALID_BANDIDX;
	}

	if (ant_idx >= TEST_ANT_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ant_idx: %d error!!\n", ant_idx);
		return SERV_STATUS_AGENT_INVALID_PARAM;
	}

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	if (!((cvt_pwr_offset > 0x80 && cvt_pwr_offset <= 0xbc) ||
		(cvt_pwr_offset >= 0xc0 && cvt_pwr_offset <= 0xfc))) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"CVT: +-30dB, min: 0xbc/max: 0xfc, cur: %2x\n", cvt_pwr_offset);
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err;
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, pwr_param.band_idx,
			(u_int32)band_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.ant_idx,
			(u_int32)ant_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.cvt_pwr_offset,
			(u_int32)cvt_pwr_offset);

	ret = mt_serv_tx_power_operation(serv_test, SERV_TEST_TXPWR_SET_CVT_PWR_OFFSET);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"cvt_pwr_offset=%2x, band_idx=%u, ant_idx:%u\n",
		cvt_pwr_offset, band_idx, ant_idx);
err:
	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame_ctrl, 2, ret);

	return ret;
}


#ifdef TXBF_SUPPORT
static s_int32 hqa_get_ibf_status_ext(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  *txbf_info;
	u_int16 hqa_len;
	u_int32 oper_mode;
	u_int32 ext_id = 0;
	u_int32 status = 0;
	u_int32 band_idx = 0;

	get_param_and_shift_buf(TRUE, sizeof(ext_id), (u_char *)&ext_id, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(band_idx), (u_char *)&band_idx, hqa_frame_ctrl);

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	txbf_info = CONFIG_GET_PARAM(serv_test,
						txbf_info);

	/* Get calibration status */
	status = CONFIG_GET_PARAM(serv_test, iBFCalStatus);

	/* Update op mode */
	oper_mode = CONFIG_GET_PARAM(serv_test, op_mode);
	oper_mode &= ~fTEST_IN_BF;
	CONFIG_SET_PARAM(serv_test, op_mode, oper_mode);

	/* Free memory */
	if (txbf_info) {
		sys_ad_free_mem(txbf_info);
		txbf_info = NULL;
	}

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2,
			(u_char *) &ext_id, sizeof(ext_id));
	hqa_len = 2 + sizeof(ext_id);
	status = SERV_OS_HTONL(status);
	sys_ad_move_mem((hqa_frame_ctrl->hqa_frame_eth->data + hqa_len), &status, sizeof(status));
	hqa_len += sizeof(status);
	update_hqa_frame(hqa_frame_ctrl, hqa_len, ret);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
		"HQA iBF phase cal ID = %d and status = %d\n",
		ext_id, status);

	return ret;
}


static s_int32 hqa_set_ibf_value_ext(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32  ctrl_band_idx = 0;
	u_char  *txbf_info;
	u_int16 hqa_len;
	u_int32 oper_mode;
	u_int32 ext_id = 0;
	u_int32 action = 0;
	u_int32 arg[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	char    *cmd;
	INT sRet = 0;


	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
		"Band index = %d\n",
		ctrl_band_idx);

	txbf_info = NULL;

	ret = sys_ad_alloc_mem((u_char **)&cmd,
				sizeof(char) * (HQA_BF_STR_SIZE));
	if (ret != SERV_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"CMD can't be created!!\n");
		goto HQA_IBF_CMD_FAIL;
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
		"cmd ptr = 0x%p\n",
		&cmd);

	sys_ad_zero_mem(cmd, sizeof(char) * (HQA_BF_STR_SIZE));

	get_param_and_shift_buf(TRUE, sizeof(ext_id),
				(u_char *)&ext_id, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&action, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&arg[0], hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&arg[1], hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&arg[2], hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&arg[3], hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&arg[4], hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&arg[5], hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&arg[6], hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&arg[7], hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&ctrl_band_idx, hqa_frame_ctrl);

	ret = mt_serv_get_serv_test_by_band(&serv_test, ctrl_band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);

		if (cmd)
			sys_ad_free_mem(cmd);

		return ret;
	}

	txbf_info = CONFIG_GET_PARAM(serv_test,
							txbf_info);

	switch (action) {
	case HQA_TXBF_INIT:
		sRet = snprintf(cmd, HQA_BF_STR_SIZE, "%d", (u_char)arg[0]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, sRet))
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"snprintf error!\n");
		mt_agent_set_txbf_dut_init_proc(serv_test, cmd);
		break;

	case HQA_CHANNEL:
		if (arg[1] == 1)
			sRet = snprintf(cmd, HQA_BF_STR_SIZE, "%d:1", (u_char)arg[0]);
		else
			sRet = snprintf(cmd, HQA_BF_STR_SIZE, "%d", (u_char)arg[0]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, sRet))
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"snprintf error!\n");

		mt_agent_set_channel(serv_test, cmd);
		break;

	case HQA_TX_MCS:
		CONFIG_SET_PARAM(serv_test, mcs, (u_char)arg[0]);
		break;

	case HQA_TX_POW0:
		sRet = snprintf(cmd, HQA_BF_STR_SIZE, "%d", (u_char)arg[0]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, sRet))
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"snprintf error!\n");
		mt_agent_set_pwr(serv_test, cmd);
		break;

	case HQA_TX_ANT:
		sRet = snprintf(cmd, HQA_BF_STR_SIZE, "%d", (u_char)arg[0]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, sRet))
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"snprintf error!\n");
		mt_agent_set_txant(serv_test, cmd);
		break;

	case HQA_RX_FRAME:
		mt_serv_stop_tx(serv_test);
		mt_serv_start_rx(serv_test);
		break;

	case HQA_RX_ANT:
		sRet = snprintf(cmd, HQA_BF_STR_SIZE, "%d", (u_char)arg[0]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, sRet))
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"snprintf error!\n");
		mt_agent_set_rxant(serv_test, cmd);
		break;

	case HQA_TXBF_LNA_GAIN:
		sRet = snprintf(cmd, HQA_BF_STR_SIZE, "%d", (u_char)arg[0]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, sRet))
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"snprintf error!\n");
		mt_agent_set_txbf_lna_gain(serv_test, cmd);
		break;

	case HQA_IBF_PHASE_COMP:
		/* BW:DBDC idx:Group:Read from E2P:Dis compensation */
		sRet = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x:%02x",
				arg[0], arg[1], arg[2], arg[3], arg[4]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, sRet))
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"snprintf error!\n");
		ret = mt_agent_set_ibf_phase_comp(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				" ATE_IBF_PHASE_COMP is failed!!\n");
			goto HQA_IBF_CMD_FAIL;
		}

		break;

	case HQA_IBF_TX:
		arg[2] = 0; /* for test purpose */

		/* fgBf:WLAN idx:Txcnt */
		sRet = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x",
				arg[0], arg[1], arg[2], arg[3]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, sRet))
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"snprintf error!\n");
		ret = mt_agent_set_tx_pkt_with_ibf(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				" ATE_IBF_TX is failed!!\n");
			goto HQA_IBF_CMD_FAIL;
		}

		break;

	case HQA_IBF_PROF_UPDATE:
		/* Pfmu idx:Nr:Nc */
		sRet = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x", arg[0], arg[1], arg[2]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, sRet))
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"snprintf error!\n");
		ret = mt_agent_set_ibf_profile_update(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				" ATE_IBF_PROF_UPDATE is failed!!\n");
			goto HQA_IBF_CMD_FAIL;
		}

		sys_ad_zero_mem(cmd, sizeof(char) * (HQA_BF_STR_SIZE));

		/* Wlan Id:EBf:IBf:Mu:PhaseCalFlg */
		switch ((u_char)ctrl_band_idx) {
		case 0:
			sRet = snprintf(cmd, HQA_BF_STR_SIZE, "01:00:01:00:01");
			break;
		case 1:
			sRet = snprintf(cmd, HQA_BF_STR_SIZE, "02:00:01:00:01");
			break;
		case 2:
			sRet = snprintf(cmd, HQA_BF_STR_SIZE, "03:00:01:00:01");
			break;
		default:
			sRet = snprintf(cmd, HQA_BF_STR_SIZE, "01:00:01:00:01");
			break;
		}
		if (os_snprintf_error(HQA_BF_STR_SIZE, sRet))
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"snprintf error!\n");
		ret = mt_agent_set_txbf_tx_apply(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				" IBF flag setting in WTBL is failed!!\n");
			goto HQA_IBF_CMD_FAIL;
		}

		break;

	case HQA_EBF_PROF_UPDATE:
		/* Pfmu idx:Nr:Nc */
		sRet = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x", arg[0], arg[1], arg[2]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, sRet))
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"snprintf error!\n");
		ret = mt_agent_set_ebf_profile_update(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				" ATE_EBF_PROF_UPDATE is failed!!\n");
			goto HQA_IBF_CMD_FAIL;
		}

		break;

	case HQA_IBF_INST_CAL:
		arg[4] = 0; /* for test purpose */

		/* Update op mode */
		oper_mode = CONFIG_GET_PARAM(serv_test, op_mode);
		oper_mode |= fTEST_IN_BF;
		CONFIG_SET_PARAM(serv_test, op_mode, oper_mode);

		/* Group idx:Group_L_M_H:fgSX2:Calibration type:Lna level:Calibration Revision */
		sRet = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
				arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, sRet))
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"snprintf error!\n");
		ret = mt_agent_set_ibf_inst_cal(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				" ATE_IBF_GD_CAL is failed!!\n");
			goto HQA_IBF_CMD_FAIL;
		}

		break;

	case HQA_IBF_INST_VERIFY:
		oper_mode = CONFIG_GET_PARAM(serv_test, op_mode);
		oper_mode |= fTEST_IN_BF;
		CONFIG_SET_PARAM(serv_test, op_mode, oper_mode);

		arg[3] = 4; /* iBF phase verification with instrument */
		arg[4] = 1; /* Force LNA gain is middle gain */

		/* Group idx:Group_L_M_H:fgSX2:Calibration type:Lna level:Calibration revision */
		sRet = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
				arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, sRet))
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"snprintf error!\n");
		ret = mt_agent_set_ibf_inst_cal(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				" ATE_IBF_INST_VERIFY is failed!!\n");
			goto HQA_IBF_CMD_FAIL;
		}

		break;

	case HQA_TXBF_GD_INIT:
		break;

	case HQA_IBF_PHASE_E2P_UPDATE:
		/* Group idx:fgSX2:E2P update type */
		sRet = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x", arg[0], arg[1], arg[2]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, sRet))
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"snprintf error!\n");
		ret = mt_agent_set_ibf_phase_cal_e2p_update(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				" ATE_IBF_PHASE_E2P_UPDATE is failed!!\n");
			goto HQA_IBF_CMD_FAIL;
		}

		break;

	default:
		break;
	}

HQA_IBF_CMD_FAIL:
	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2,
			(u_char *) &ext_id, sizeof(ext_id));
	hqa_len = 2 + sizeof(ext_id);
	update_hqa_frame(hqa_frame_ctrl, hqa_len, ret);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
			 "End of hqa_set_ibf_value_ext()!!\n");

	if (cmd)
		sys_ad_free_mem(cmd);

	return ret;
}


static s_int32 hqa_update_ibf_chan_prof_update_ext(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int16 hqa_len;
	u_int32 ext_id = 0;
	u_int32 pfmu_idx = 0, subcarr_idx = 0, fg_final_data = 0;
	s_int32 h11 = 0, angle_h11 = 0, h21 = 0, angle_h21 = 0;
	s_int32 h31 = 0, angle_h31 = 0, h41 = 0, angle_h41 = 0;
	s_int32 h51 = 0, angle_h51 = 0;
	u_int32 band_idx = 0;
	char    *cmd;
	INT sRet = 0;


	ret = sys_ad_alloc_mem((u_char **)&cmd,
				sizeof(char) * (HQA_BF_STR_SIZE));
	if (ret != SERV_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"CMD can't be created!!\n");
		goto HQA_PROFILE_UPDATE_FAIL;
	}

	sys_ad_zero_mem(cmd, sizeof(char) * (HQA_BF_STR_SIZE));

	get_param_and_shift_buf(TRUE, sizeof(ext_id),
				(u_char *)&ext_id, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&pfmu_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&subcarr_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&fg_final_data, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&h11, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&angle_h11, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&h21, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&angle_h21, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&h31, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&angle_h31, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&h41, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&angle_h41, hqa_frame_ctrl);
	if (serv_test->test_config.tx_ant > 4) {
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&h51, hqa_frame_ctrl);
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&angle_h51, hqa_frame_ctrl);
	}
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&band_idx, hqa_frame_ctrl);

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);

		if (cmd)
			sys_ad_free_mem(cmd);

		return ret;
	}

	if (serv_test->test_config.tx_ant <= 4) {
		sRet = snprintf(cmd, HQA_BF_STR_SIZE,
			"%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x",
			pfmu_idx, subcarr_idx, fg_final_data,
			h11, angle_h11, h21, angle_h21, h31, angle_h31, h41, angle_h41);
	} else {
		sRet = snprintf(cmd, HQA_BF_STR_SIZE,
			"%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x",
			pfmu_idx, subcarr_idx, fg_final_data,
			h11, angle_h11, h21, angle_h21, h31, angle_h31, h41, angle_h41,
			h51, angle_h51);
	}
	if (os_snprintf_error(HQA_BF_STR_SIZE, sRet))
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR, "snprintf error!\n");
	ret = mt_agent_set_txbf_chan_profile_update(serv_test, cmd);

	if (ret == FALSE) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"is failed!!\n");
		goto HQA_PROFILE_UPDATE_FAIL;
	}

HQA_PROFILE_UPDATE_FAIL:
	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2,
			(u_char *) &ext_id, sizeof(ext_id));
	hqa_len = 2 + sizeof(ext_id);
	update_hqa_frame(hqa_frame_ctrl, hqa_len, ret);

	if (cmd)
		sys_ad_free_mem(cmd);

	return ret;
}


static s_int32 hqa_update_ibf_chan_prof_update_all_ext(
	struct service_test *serv_test, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  *data = hqa_frame_ctrl->hqa_frame_eth->data;
	u_int16 hqa_len;
	u_int32 ext_id = 0;
	u_int32 pfmu_idx = 0;
	u_int32 temp;
	u_int32 band_idx = 0;
	u_int32 ibf_raw_data_offset = 24;

	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&ext_id, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&pfmu_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&band_idx, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&temp, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&temp, hqa_frame_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&temp, hqa_frame_ctrl);

	ret = mt_serv_get_serv_test_by_band(&serv_test, band_idx);
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"ret:%x\n", ret);
		return ret;
	}

	if (serv_test->test_config.tx_ant <= 4) {
		ret = mt_serv_set_txbf_profile_data_Write_20m_all(
			serv_test, pfmu_idx, data + ibf_raw_data_offset);
	} else {
		ret = mt_serv_set_txbf_profile_data_Write_20m_all_5x5(
			serv_test, pfmu_idx, data + ibf_raw_data_offset);
	}

	if (ret != SERV_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"is failed!!\n");
		goto HQA_PROFILE_UPDATE_FAIL;
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
			 "str:%d\n", pfmu_idx);

HQA_PROFILE_UPDATE_FAIL:
	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame_ctrl->hqa_frame_eth->data + 2,
			(u_char *) &ext_id, sizeof(ext_id));
	hqa_len = 2 + sizeof(ext_id);
	update_hqa_frame(hqa_frame_ctrl, hqa_len, ret);

	return ret;
}


s_int32 mt_agent_set_ibf_phase_cal_e2p_update(
	struct service_test *serv_test, u_char *arg)
{
	u_int32  i;
	u_char   group_idx, update_type, *value;
	u_long  buf[3] = {0};
	boolean  fgSx2;
	s_int32  ret = SERV_STATUS_SUCCESS;


	if (arg == NULL) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	if (strlen(arg) != 8) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
					value; value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	group_idx   = (u_char)buf[0];
	fgSx2       = (u_char)buf[1];
	update_type = (u_char)buf[2];

	ret = mt_serv_set_ibf_phase_cal_e2p_update(
						serv_test,
						group_idx,
						fgSx2,
						update_type);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_lna_gain(
	struct service_test *serv_test,
	u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_long lna_gain = 0;


	if (kstrtol(arg, 10, (long *)&lna_gain) == 0)
		ret = mt_serv_set_txbf_lna_gain(serv_test, (u_char)lna_gain);
	else
		ret = SERV_STATUS_AGENT_INVALID_PARAM;

	return ret;
}


s_int32 mt_agent_set_txbf_da(
	struct service_test *serv_test, u_char *arg)
{
	u_int16 wcid = 1;
	u_char *addr = NULL;
	u_int32 Octet;
	u_long  buf[6] = {0};
	char   *mac_tok = NULL, *dash_ptr = NULL, *mac_str = NULL;
	s_int32 ret = SERV_STATUS_SUCCESS;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
			 "Da = %s\n", arg);

	dash_ptr = strstr(arg, "-");
	if (dash_ptr) {
		mac_str = dash_ptr+1;
		*dash_ptr = '\0';
		/* sscanf(arg, "%d", &Octet); */
		ret = kstrtouint(arg, 10, &Octet);
		if (ret) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
		wcid = Octet;
	} else
		mac_str = arg;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(mac_str) != 17) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err_out;
	}

	addr = (u_char *)CONFIG_GET_PADDR(serv_test, addr1[wcid-1]);

	for (Octet = 0, mac_tok = agent_trtok(arg, ":");
			mac_tok; mac_tok = agent_trtok(NULL, ":")) {
		/* sanity check */
		if ((strlen(mac_tok) != 2) ||
			(!serv_isxdigit(*mac_tok)) ||
			(!serv_isxdigit(*(mac_tok + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_LEN;
			goto err_out;
		}

		if (kstrtol(mac_tok, 16, (u_long *)&buf[Octet])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		addr[Octet] = (u_char)buf[Octet];
		Octet++;
	}

	/* sanity check */
	if (Octet != SERV_MAC_ADDR_LEN)
		ret = SERV_STATUS_AGENT_INVALID_LEN;

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_sa(
	struct service_test *serv_test, u_char *addr, u_char *arg)
{
	char    *value;
	s_int32 Octet;
	u_long  buf[6] = {0};
	s_int32 ret = SERV_STATUS_SUCCESS;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
			 "Sa = %s\n", arg);

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(arg) != 17) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err_out;
	}

	addr = (u_char *)CONFIG_GET_PADDR(serv_test, addr3[0]);

	for (Octet = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		/* sanity check */
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[Octet])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		addr[Octet] = (u_char)buf[Octet];
		Octet++;
	}

	/* sanity check */
	if (Octet != SERV_MAC_ADDR_LEN) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err_out;
	}

	ret = mt_serv_set_txbf_sa(serv_test, addr);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_bssid(
	struct service_test *serv_test, u_char *arg)
{
	char    *value;
	s_int32 Octet;
	u_long  buf[6] = {0};
	u_char  *addr = NULL;
	s_int32 ret = SERV_STATUS_SUCCESS;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
			 "BSSID = %s\n", arg);

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(arg) != 17) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err_out;
	}

	addr = (u_char *)CONFIG_GET_PADDR(serv_test, addr2[0]);

	for (Octet = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		/* sanity check */
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_LEN;
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[Octet])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		addr[Octet] = (u_char)buf[Octet];
		Octet++;
	}

	/* sanity check */
	if (Octet != SERV_MAC_ADDR_LEN)
		ret = SERV_STATUS_AGENT_INVALID_LEN;

err_out:
	return ret;
}


s_int32 mt_agent_set_bss_info(
	struct service_test *serv_test, u_char *arg)
{
	u_char  bssid[6];
	u_long  buf[6] = {0};
	char    *value;
	u_char i;
	s_int32 ret = SERV_STATUS_SUCCESS;


	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		bssid[i] = (u_char)buf[i];
		i++;
	}

	ret = mt_serv_set_bss_info(serv_test, bssid);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_tx_apply(
	struct service_test *serv_test, u_char *arg)
{
	char	*value;
	u_long	buf[5] = {0};
	u_char  in_arg[5] = {0};
	s_int32 ret = SERV_STATUS_SUCCESS;
	s_int32	i;


	if (strlen(arg) != 14) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		in_arg[i] = (u_char)buf[i];
		i++;
	}

	ret = mt_serv_set_txbf_tx_apply(serv_test, in_arg);

err_out:
	return ret;
}


s_int32 mt_agent_set_ibf_phase_comp(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;
	u_char  bw,  group_idx,   dbdc_band_idx, *value;
	u_long  buf[6] = {0};
	boolean fg_read_from_e2p, fg_dis_comp,   fg_bw_160nc, fg_jp_band;

	if (arg == NULL) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	if ((strlen(arg) != 14) && (strlen(arg) != 17)) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	bw               = (u_char)buf[0];
	dbdc_band_idx    = (u_char)buf[1];
	group_idx        = (u_char)buf[2];
	fg_read_from_e2p = (u_char)buf[3];
	fg_dis_comp      = (u_char)buf[4];
	fg_bw_160nc     = (i == 6) ? (u_char)buf[4] : 0;
	fg_jp_band      = (group_idx == 1) ? 1 : 0;

	ret = mt_serv_set_ibf_phase_comp(serv_test,
					bw,
					fg_jp_band,
					dbdc_band_idx,
					group_idx,
					fg_read_from_e2p,
					fg_dis_comp);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_profile_tag_read(
	struct service_test *serv_test, u_char *arg)
{
	u_char  pf_idx;
	boolean fg_bfer;
	u_long  buf[2] = {0};
	char	*value;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char	i;


	if (strlen(arg) != 5) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	pf_idx  = (u_char)buf[0];
	fg_bfer = (u_char)buf[1];

	ret = mt_serv_set_txbf_profile_tag_read(serv_test, pf_idx, fg_bfer);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_profile_tag_write(
	struct service_test *serv_test, u_char *arg)
{
	u_long prf_idx = 0;


	if (kstrtol(arg, 10, (u_long *)&prf_idx) == 0)
		return mt_serv_set_txbf_profile_tag_write(
							serv_test,
							(u_char)prf_idx);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_profile_tag_invalid(
	struct service_test *serv_test, u_char *arg)
{
	u_long fg_invalid = 0;


	if (kstrtol(arg, 10, (u_long *)&fg_invalid) == 0)
		return mt_serv_set_txbf_profile_tag_invalid(
							serv_test,
							(u_char)fg_invalid);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_profile_tag_idx(
	struct service_test *serv_test, u_char *arg)
{
	u_long profile_idx = 0;


	if (kstrtol(arg, 10, (u_long *)&profile_idx) == 0)
		return mt_serv_set_txbf_profile_tag_idx(
							serv_test,
							(u_char)profile_idx);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_bf_type(
	struct service_test *serv_test, u_char *arg)
{
	u_long bf_type = 0;


	if (kstrtol(arg, 10, (u_long *)&bf_type) == 0)
		return mt_serv_set_txbf_pfmu_tag_bf_type(
							serv_test,
							(u_char)bf_type);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_dbw(
	struct service_test *serv_test, u_char *arg)
{
	u_long dbw = 0;


	if (kstrtol(arg, 10, (u_long *)&dbw) == 0)
		return mt_serv_set_txbf_pfmu_tag_dbw(
						serv_test,
						(u_char)dbw);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_sumu(
	struct service_test *serv_test, u_char *arg)
{
	u_long su_mu = 0;


	if (kstrtol(arg, 10, (u_long *)&su_mu) == 0)
		return mt_serv_set_txbf_pfmu_tag_sumu(
						serv_test,
						(u_char)su_mu);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_mem(
	struct service_test *serv_test, u_char *arg)
{
	u_long  buf[8] = {0};
	char    *value;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;
	u_char  pfmu_mem_col[4], pfmu_mem_row[4];


	/* mem col0:row0:col1:row1:col2:row2:col3:row3 */
	if (strlen(arg) != 23) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	pfmu_mem_col[0] = (u_char)buf[0];
	pfmu_mem_row[0] = (u_char)buf[1];
	pfmu_mem_col[1] = (u_char)buf[2];
	pfmu_mem_row[1] = (u_char)buf[3];
	pfmu_mem_col[2] = (u_char)buf[4];
	pfmu_mem_row[2] = (u_char)buf[5];
	pfmu_mem_col[3] = (u_char)buf[6];
	pfmu_mem_row[3] = (u_char)buf[7];

	ret = mt_serv_set_txbf_pfmu_tag_mem(
				serv_test,
				pfmu_mem_row,
				pfmu_mem_col);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_pfmu_tag_matrix(
	struct service_test *serv_test, u_char *arg)
{
	u_long  buf[6] = {0};
	char    *value;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;
	u_char  nr, nc, ng, lm, cb, he;


	/* nrow:nol:ng:LM:CodeBook:HtcExist */
	if (strlen(arg) != 17) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	nr = (u_char)buf[0];
	nc = (u_char)buf[1];
	ng = (u_char)buf[2];
	lm = (u_char)buf[3];
	cb = (u_char)buf[4];
	he = (u_char)buf[5];

	ret = mt_serv_set_txbf_pfmu_tag_matrix(
					serv_test,
					nr,
					nc,
					ng,
					lm,
					cb,
					he);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_pfmu_tag_snr(
	struct service_test *serv_test, u_char *arg)
{
	u_long  buf[8] = {0};
	char    snr[8];
	char    *value;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;


	if ((strlen(arg) != 11) && (strlen(arg) != 23)) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value && (i < ARRAY_SIZE(buf)); value = agent_trtok(NULL, ":")) {
		if ((!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		snr[i] = (u_char)buf[i];
		i++;
	}

	ret = mt_serv_set_txbf_pfmu_tag_snr(serv_test, snr);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_pfmu_tag_smart_ant(
	struct service_test *serv_test, u_char *arg)
{
	u_long SmartAnt = 0;


	if (kstrtol(arg, 10, (u_long *)&SmartAnt) == 0)
		return mt_serv_set_txbf_pfmu_tag_smart_ant(
							serv_test,
							SmartAnt);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_se_idx(
	struct service_test *serv_test, u_char *arg)
{
	u_long se_idx = 0;


	if (kstrtol(arg, 10, (u_long *)&se_idx) == 0)
		return mt_serv_set_txbf_pfmu_tag_se_idx(
							serv_test,
							se_idx);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_rmsd_thrd(
	struct service_test *serv_test, u_char *arg)
{
	u_long rmsd_thrd = 0;


	if (kstrtol(arg, 10, (u_long *)&rmsd_thrd) == 0)
		return mt_serv_set_txbf_pfmu_tag_rmsd_thrd(
							serv_test,
							(u_char)rmsd_thrd);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_mcs_thrd(
	struct service_test *serv_test, u_char *arg)
{
	u_long  buf[6] = {0};
	char    *value;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;
	u_char  mcs_lss[3], mcs_sss[3];


	if (strlen(arg) != 17) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value && (i < ARRAY_SIZE(buf)); value = agent_trtok(NULL, ":")) {
		if ((!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	mcs_lss[0] = (u_char)buf[0];
	mcs_sss[0] = (u_char)buf[1];
	mcs_lss[1] = (u_char)buf[2];
	mcs_sss[1] = (u_char)buf[3];
	mcs_lss[2] = (u_char)buf[4];
	mcs_sss[2] = (u_char)buf[5];

	ret = mt_serv_set_txbf_pfmu_tag_mcs_thrd(
						serv_test,
						mcs_lss,
						mcs_sss);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_pfmu_tag_time_out(
	struct service_test *serv_test, u_char *arg)
{
	u_long time_out = 0;


	if (kstrtol(arg, 10, (u_long *)&time_out) == 0)
		return mt_serv_set_txbf_pfmu_tag_time_out(
							serv_test,
							(u_char)time_out);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_desired_bw(
	struct service_test *serv_test, u_char *arg)
{
	u_long desired_bw = 0;


	if (kstrtol(arg, 10, (u_long *)&desired_bw) == 0)
		return mt_serv_set_txbf_pfmu_tag_desired_bw(
							serv_test,
							(u_char)desired_bw);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_desired_nr(
	struct service_test *serv_test, u_char *arg)
{
	u_long desired_nr = 0;;


	if (kstrtol(arg, 10, (u_long *)&desired_nr) == 0)
		return mt_serv_set_txbf_pfmu_tag_desired_nr(
							serv_test,
							(u_char)desired_nr);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_desired_nc(
	struct service_test *serv_test, u_char *arg)
{
	u_long desired_nc = 0;


	if (kstrtol(arg, 10, (u_long *)&desired_nc) == 0)
		return mt_serv_set_txbf_pfmu_tag_desired_nc(
							serv_test,
							(u_char)desired_nc);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_data_write(
	struct service_test *serv_test, u_char *arg)
{
	u_long  buf[18];
	u_int16 pfmu_data[18];
	s_int32 ret = SERV_STATUS_SUCCESS;
	char    *value;
	u_char  i;


	sys_ad_zero_mem(buf, 36);

	/* Profile Select : Subcarrier Select */
	if (strlen(arg) != 60) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value && (i < ARRAY_SIZE(buf)); value = agent_trtok(NULL, ":"), i++) {
		if ((!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		pfmu_data[i] = (u_int16)buf[i];
		i++;
	}

	ret = mt_serv_set_txbf_pfmu_data_write(serv_test, pfmu_data);

err_out:
	return ret;
}


s_int32 mt_agent_set_sta_rec_bf_update(
	struct service_test *serv_test, u_char *arg)
{
	return mt_serv_set_sta_rec_bf_update(serv_test, arg);
}


s_int32 mt_agent_set_sta_rec_bf_read(
	struct service_test *serv_test, u_char *arg)
{
	return mt_serv_set_sta_rec_bf_read(serv_test, arg);
}


s_int32 mt_agent_set_manual_assoc(
	struct service_test *serv_test, u_char *arg)
{
	return mt_serv_set_manual_assoc(serv_test, arg);
}


s_int32 mt_agent_set_tx_pkt_with_ibf(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;
	u_char  wlan_idx, tx_cnt, *value;
	u_long  buf[4] = {0};
	boolean fg_bf, fg_tx_param_update;


	if (arg == NULL) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	if (strlen(arg) != 11) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value && (i < ARRAY_SIZE(buf)); value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	fg_bf    = (u_char)buf[0];
	wlan_idx = (u_char)buf[1];
	tx_cnt   = (u_char)buf[2];
	fg_tx_param_update = (u_char)buf[3];

	ret = mt_serv_set_tx_pkt_with_ibf(
					serv_test,
					wlan_idx,
					tx_cnt,
					fg_bf,
					fg_tx_param_update);

err_out:
	return ret;
}


s_int32 mt_agent_set_ibf_profile_update(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;
	u_char  nc, pfmu_idx, *value;
	u_long  buf[3] = {0};


	if (arg == NULL) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	if (strlen(arg) != 8) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value && (i < ARRAY_SIZE(buf)); value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	pfmu_idx = (u_char)buf[0];
	nc       = (u_char)buf[2];
	ret = mt_serv_set_ibf_profile_update(serv_test, pfmu_idx, nc);

err_out:
	return ret;
}


s_int32 mt_agent_set_ebf_profile_update(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;
	u_char  nc, pfmu_idx, *value;
	u_long  buf[3] = {0};


	if (arg == NULL) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	if (strlen(arg) != 8) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value && (i < ARRAY_SIZE(buf)); value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	pfmu_idx = (u_char)buf[0];
	nc       = (u_char)buf[2];
	ret = mt_serv_set_ebf_profile_update(serv_test, pfmu_idx, nc);

err_out:
	return ret;
}


s_int32 mt_agent_set_ibf_inst_cal(
	struct service_test *serv_test, u_char *arg)
{
	u_char   i;
	u_char   group_idx, group_l_m_h, *value;
	u_long   buf[6] = {0};
	boolean  fg_sx2;
	u_char   phase_cal, phase_lna_gain_level, phase_out_revision = 0;
	s_int32 ret = SERV_STATUS_SUCCESS;

	if (arg == NULL) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	if ((strlen(arg) != 14) && (strlen(arg) != 17)) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value && (i < ARRAY_SIZE(buf)); value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	group_idx            = (u_char)buf[0];
	group_l_m_h          = (u_char)buf[1];
	fg_sx2               = (u_char)buf[2];
	phase_cal            = (u_char)buf[3];
	phase_lna_gain_level = (u_char)buf[4];
	phase_out_revision   = (u_char)buf[5];

	ret = mt_serv_set_ibf_inst_cal(
				serv_test,
				group_idx,
				group_l_m_h,
				fg_sx2,
				phase_cal,
				phase_lna_gain_level,
				phase_out_revision);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_chan_profile_update(
	struct service_test *serv_test, u_char *arg)
{
	u_char  i;
	u_char  *value;
	u_long  buf[11] = {0};
	u_int16 pfmu_data[11] = {0};
	s_int32 ret = SERV_STATUS_SUCCESS;

	if (arg == NULL) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	if (strlen(arg) != 43) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value && (i < ARRAY_SIZE(buf)); value = agent_trtok(NULL, ":")) {
		if ((strlen(value) > 3) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		pfmu_data[i] = (u_int16)buf[i];
		i++;
	}

	ret = mt_serv_set_txbf_chan_profile_update(serv_test, pfmu_data);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_dut_init_proc(
	struct service_test *serv_test, u_char *arg)
{
	u_char  ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	struct  test_operation *op = serv_test->test_op;
	struct  test_wlan_info *winfos = serv_test->test_winfo;
	u_long  in_value = 0;
	boolean fg_dbdc;
	char    cmdStr[24];
	u_char  tx_path = 0;
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	INT sRet = 0;

	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	if (kstrtol(arg, 10, (u_long *)&in_value)) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
		"cmd ptr = 0x%p\n", &arg);

	fg_dbdc = (boolean)(in_value & 1);


	/* set ATESA=00:22:22:22:22:22 */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
			0x22, 0x22, 0x22, 0x22, 0x22);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"snprintf error!\n");
	SetATESa(ad, cmdStr);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
			 "mt_serv_set_device_info()!!\n");

	/* set ATEBSSID=00:22:22:22:22:22 */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
			0x22, 0x22, 0x22, 0x22, 0x22);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"snprintf error!\n");
	SetATEBssid(ad, cmdStr);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
			 "mt_agent_set_bss_info()!!\n");

	/* set ATEDA=00:11:11:11:11:11 */
	sRet = snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
			0x11, 0x11, 0x11, 0x11, 0x11);
	if (os_snprintf_error(sizeof(cmdStr), sRet))
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"snprintf error!\n");
	SetATEDa(ad, cmdStr);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
			 "mt_agent_set_txbf_da()!!\n");


	/* set ATETXMODE=2 */
	CONFIG_SET_PARAM(serv_test, tx_mode, 2);
	mt_serv_set_rate(serv_test);


	/* set ATETXBW=0 */
	mt_agent_set_bw(serv_test, "0");
	/* set ATETXGI=0 */
	CONFIG_SET_PARAM(serv_test, sgi, 0);
	/* Set IPG = 100 */
	CONFIG_SET_PARAM(serv_test, ipg_param.ipg, 100);
	/* Enable i/eBF */
	CONFIG_SET_PARAM(serv_test, ibf, TRUE);
	CONFIG_SET_PARAM(serv_test, ebf, TRUE);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
		"Set txmode, mcs, bw, sgi, ibf and ebf!!\n");

		tx_path = CONFIG_GET_PARAM(serv_test, tx_ant);
		switch (tx_path) {
		case SERV_TX_PATH_2:
			/* set ATETXANT=3 2T */
			mt_agent_set_txant(serv_test, "3");
			/* set ATERXANT=3  2R*/
			mt_agent_set_rxant(serv_test, "3");

			/* set ATETXMCS=15 */
			CONFIG_SET_PARAM(serv_test, mcs, 15);
			break;

		case SERV_TX_PATH_3:
			/* set ATETXANT=7 3T */
			mt_agent_set_txant(serv_test, "7");
			/* set ATERXANT=7  3R*/
			mt_agent_set_rxant(serv_test, "7");

			/* set ATETXMCS=23 */
			CONFIG_SET_PARAM(serv_test, mcs, 23);
			break;

		case SERV_TX_PATH_4:
		default:
			/* set ATETXANT=15 4T */
			mt_agent_set_txant(serv_test, "15");
			/* set ATERXANT=15  4R*/
			mt_agent_set_rxant(serv_test, "15");

			/* set ATETXMCS=31 */
			CONFIG_SET_PARAM(serv_test, mcs, 31);
			break;

		case SERV_TX_PATH_5:
			/* set ATETXANT=31 5T but enable wf0~wf3*/
			mt_agent_set_txant(serv_test, "15");
			/* set ATERXANT=31 5R but enable wf0~wf3*/
			mt_agent_set_rxant(serv_test, "15");

			/* set ATETXNSS=4 */
			CONFIG_SET_PARAM(serv_test, nss, 4);
			/* set ATETXMCS=7 */
			CONFIG_SET_PARAM(serv_test, mcs, 7);
			break;
		}

	CONFIG_SET_PARAM(serv_test, fgEBfEverEnabled, 0);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
			 "Set tx path = %d\n!!\n", tx_path);

	/* Enable Tx MAC HW before trigger sounding */
	if (op->op_set_tr_mac) {
		ret = op->op_set_tr_mac(
				winfos,
				SERV_TEST_MAC_TX,
				TRUE,
				ctrl_band_idx);
	} else {
		ret = SERV_STATUS_SERV_TEST_NOT_SUPPORTED;
		goto err_out;
	}

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
			 "op_set_tr_mac()!!\n");

	/* Init iBF phase calibration */
	if (((in_value >> 4) & 1) == 0)
		ret = mt_serv_set_ibf_inst_cal_init(serv_test);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
		"mt_serv_set_ibf_inst_cal_init()!!\n");

err_out:
	return ret;
}
#endif /* TXBF_SUPPORT */


static struct hqa_cmd_entry CMD_SET6[] = {
	/* cmd id start from 0x1600 */
	{0x1,	hqa_set_channel_ext, "DBDCSetChannel"},
	{0x2,	hqa_set_txcontent_ext, "DBDCSetTXContent"},
	{0x3,	hqa_start_tx_ext, "DBDCStartTX"},
	{0x4,	hqa_start_rx_ext, "DBDCStartRX"},
	{0x5,	hqa_stop_tx_ext, "DBDCStopTX"},
	{0x6,	hqa_stop_rx_ext, "DBDCStopRX"},
	{0x7,	legacy_function, "TODO"},
#ifdef TXBF_SUPPORT
	{0x8,	hqa_set_ibf_value_ext, "SetIbfValue"},
	{0x9,	hqa_get_ibf_status_ext, "GetIbfStatus"},
	{0xa,	hqa_update_ibf_chan_prof_update_ext, "UpdateIbfChanProfUpdate"},
	{0xb,	legacy_function, "TODO"},
	{0xc,	hqa_update_ibf_chan_prof_update_all_ext, "UpdateIbfChanProfUpdateAll"},
#endif /* TXBF_SUPPORT */
	{0x26,	hqa_set_tx_time, "SetTxTime"},
	{0x27,	hqa_off_ch_scan, "OffChScan"},
	{0x2a,	hqa_testmode_at_cmd_get, "AtCmdGet"},
	{0x2b,	hqa_testmode_at_cmd_set, "AtCmdSet"},
};

static struct hqa_cmd_table CMD_TABLES[] = {
	{
		CMD_SET0,
		SERV_ARRAY_SIZE(CMD_SET0),
		0x1000,
	}
	,
	{
		CMD_SET1,
		SERV_ARRAY_SIZE(CMD_SET1),
		0x1100,
	}
	,
	{
		CMD_SET2,
		SERV_ARRAY_SIZE(CMD_SET2),
		0x1200,
	}
	,
	{
		CMD_SET3,
		SERV_ARRAY_SIZE(CMD_SET3),
		0x1300,
	}
	,
	{
		CMD_SET4,
		SERV_ARRAY_SIZE(CMD_SET4),
		0x1400,
	}
	,
	{
		CMD_SET5,
		SERV_ARRAY_SIZE(CMD_SET5),
		0x1500,
	}
	,
	{
		CMD_SET6,
		SERV_ARRAY_SIZE(CMD_SET6),
		0x1600,
	}
	,
};

s_int32 mt_agent_hqa_cmd_string_parser(
	struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int8 i = 0, j = 0, k = 0, pattern_found = 0;
	s_int8 *this_para = NULL;
	u_int16 tmp_cmd_id = 0, str_len = 0;
	for (i = 0; i < (sizeof(CMD_TABLES) / sizeof(struct
		hqa_cmd_table)); i++){
		for (j = 0; j < CMD_TABLES[i].cmd_set_size; j++) {
			if (strncasecmp(hqa_frame_ctrl->hqa_frame_string,
				CMD_TABLES[i].cmd_set[j].cmd_str,
				strlen(CMD_TABLES[i].cmd_set[j].cmd_str)) == 0) {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
					"Found Cmd : %s\n", CMD_TABLES[i].cmd_set[j].cmd_str);
				/*Command Found in table*/
				pattern_found = 1;
				hqa_frame_ctrl->pars_idx += strlen(CMD_TABLES[i].cmd_set[j].cmd_str);
				/*Parsing the parameters if it has*/
				this_para = strstr(hqa_frame_ctrl->hqa_frame_string, "=");
				if (this_para) {
					/*Roll over "=", get Parameters y,y,y,y...*/
					hqa_frame_ctrl->pars_idx++;
					this_para++;
				}
				tmp_cmd_id = CMD_TABLES[i].cmd_offset |
					CMD_TABLES[i].cmd_set[j].index;
				break;
			}
		}
		if (pattern_found == 1)
			break;
	}
	hqa_frame_ctrl->hqa_frame_eth->length = 0;
	if (this_para != NULL) {
		str_len = strlen(this_para);
		for (k = 0; k < str_len; k++) {
			if (this_para && (*this_para == ',')) {
				hqa_frame_ctrl->hqa_frame_eth->length++;
				this_para++;
			} else {
				this_para++;
			}
		}
		hqa_frame_ctrl->hqa_frame_eth->length =
			(hqa_frame_ctrl->hqa_frame_eth->length + 1) * sizeof(u_int32); // one ',' means two parameter, two ',' mean three parameter and mapping to ethernet
	}
	/*pack the hqa command*/
	if (pattern_found) {
		hqa_frame_ctrl->hqa_frame_eth->magic_no = TEST_CMD_MAGIC_NO;
		hqa_frame_ctrl->hqa_frame_eth->type = 1;
		hqa_frame_ctrl->hqa_frame_eth->id = tmp_cmd_id;
		hqa_frame_ctrl->hqa_frame_eth->sequence = 0;
		return SERV_STATUS_SUCCESS;
	} else
		return SERV_STATUS_AGENT_NOT_SUPPORTED;
}

s_int32 mt_agent_hqa_cmd_handler(
	struct service *serv, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct service_test *serv_test;
	u_int16 cmd_id;
	u_int32 table_idx = 0, ext_cmd_id = 0;
	u_int32 magic_no = 0;
	struct hqa_frame *hqa_frame = NULL;
	hqa_frame_ctrl->pars_idx = 0;

	if (!serv) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"service is NULL\n");
		return -SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	if (hqa_frame_ctrl->type == 1) {
		ret = mt_agent_hqa_cmd_string_parser(
		hqa_frame_ctrl);
		/*can return for debug*/
		/*return ret;*/
		if (ret != SERV_STATUS_SUCCESS)
			return ret;
	}

	hqa_frame = hqa_frame_ctrl->hqa_frame_eth;

	if (hqa_frame_ctrl->type == 0) {
		hqa_frame->length = SERV_OS_NTOHS(hqa_frame->length);
		cmd_id = SERV_OS_NTOHS(hqa_frame->id);
		magic_no = SERV_OS_NTOHL(hqa_frame->magic_no);
	} else {
		cmd_id = hqa_frame->id;
		magic_no = hqa_frame->magic_no;
	}

	serv_test = (struct service_test *)serv->serv_handle;

	if (magic_no != TEST_CMD_MAGIC_NO)
		return SERV_STATUS_AGENT_INVALID_PARAM;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"\x1b[0;32m cmd id: 0x%02x\x1b[0m\n", cmd_id);

	while (table_idx < (sizeof(CMD_TABLES) / sizeof(struct hqa_cmd_table))) {
		if ((cmd_id & 0xff00) == CMD_TABLES[table_idx].cmd_offset) {
			u_int32 cmd_loop = 0;
			struct hqa_cmd_entry *cmd_set = NULL;

			cmd_set = CMD_TABLES[table_idx].cmd_set;
			/* Fix me, should align with other command set */
			if (hqa_frame_ctrl->type == 0) {
				if (CMD_TABLES[table_idx].cmd_offset == 0x1600) {
					sys_ad_move_mem(&ext_cmd_id,
						hqa_frame->data,
						sizeof(ext_cmd_id));

					cmd_id = SERV_OS_NTOHL(ext_cmd_id);
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
					"\x1b[0;32m ext cmd id: 0x%02x\x1b[0m\n", cmd_id);
				}
			}

			cmd_id &= 0xff;
			ret = SERV_STATUS_AGENT_NOT_SUPPORTED;
			while (cmd_loop < CMD_TABLES[table_idx].cmd_set_size) {
				if (cmd_id == cmd_set[cmd_loop].index) {
					if (cmd_set[cmd_loop].cmd_str != NULL) {
						MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
							"\x1b[0;32m cmd str: %s\x1b[0m\n", cmd_set[cmd_loop].cmd_str);
					}
					ret = cmd_set[cmd_loop].handler(
						serv_test, hqa_frame_ctrl);

					goto done;
				} else
					cmd_loop++;
			}
		} else
			table_idx++;
	}

done:
	if (cmd_id == TEST_CMD_REQ) {
		hqa_frame->type = TEST_CMD_RSP;
		/* hqa_frame->type = 0x8005; */
	} else
		hqa_frame->type = TEST_CMDRSP;

	return ret;
}

/*****************************************************************************
 *	iwpriv command handler
 *****************************************************************************/

static struct hqa_frame_ctrl hqa_cmd_frame_ctrl;

static struct agent_cli_act_handler cli_act_cmds[] = {
	{"ATESTART", hqa_serv_start},
	{"APSTOP", hqa_serv_start},
	{"ATESTOP", hqa_serv_stop},
	{"APSTART", hqa_serv_stop},
	{"TXCOMMIT", mt_serv_submit_tx},
	{"TXREVERT", mt_serv_revert_tx},
	{"TXFRAME", mt_serv_start_tx},
	{"TXSTOP", mt_serv_stop_tx},
	{"RXFRAME", mt_serv_start_rx},
	{"RXSTOP", mt_serv_stop_rx},
	{"DNLKCLEAN", mt_serv_dnlk_clean},
	{"DNLK2G", mt_serv_dnlk_2g},
	{"DNLK5G", mt_serv_dnlk_5g},
	{"RXGAINCAL", mt_serv_rxgaink},
	{"GROUPREKClean", mt_serv_group_prek_clean},
	{"GROUPREKDump", mt_serv_group_prek_dump},
	{"GROUPREKDump5G", mt_serv_group_prek_dump_5g},
	{"GROUPREKDump6G", mt_serv_group_prek_dump_6g},
	{"GROUPREK", mt_serv_group_prek},
	{"DPDClean", mt_serv_dpd_prek_clean},
	{"DPDDump2G", mt_serv_dpd_prek_dump_2g},
	{"DPDDump5G", mt_serv_dpd_prek_dump_5g},
	{"DPDDump6G", mt_serv_dpd_prek_dump_6g},
	{"DPD6G", mt_serv_dpd_prek_6g},
	{"DPD5G", mt_serv_dpd_prek_5g},
	{"DPD2G", mt_serv_dpd_prek_2g},
	{"DPDREK", mt_serv_dpd_prek},
	{"TXCONT", mt_serv_continuous_tx},
	{"TXCONTSTOP", mt_serv_continuous_tx_stop},
	{"checkTXV", mt_serv_check_txv},
	{"TXCARR", mt_serv_tx_carr},
	{"TXCARRSTOP", mt_serv_tx_carr_stop},
	{"", NULL}	/* the last entry */
};

s_int32 mt_agent_cli_act(u_char *name, struct service *serv)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct service_test *serv_test = NULL;
	struct agent_cli_act_handler *entry = cli_act_cmds;

	if (!serv) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"service is NULL\n");
		return -SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	serv_test = (struct service_test *)serv->serv_handle;

	if (serv_test->test_config.op_mode & OP_MODE_STOP) {
		if (strncmp(name, "ATESTART", 8) != 0) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"op_mode: %x\n", serv_test->test_config.op_mode);
			return SERV_STATUS_AGENT_FAIL;
		}
	}

	while (strlen(entry->name)) {
		if (strcmp(name, entry->name) == 0) {
			ret = entry->handler(serv_test);
			break;
		}

		entry++;
	}

	if (strlen(entry->name) == 0) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				 "No support %s !\n", name);
	}

	return ret;
}

s_int32 mt_agent_show_TxDone(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 tx_cnt = 0;
	struct _net_testmode_chip_info testmode_chip_info;
	struct service_test *serv = serv_test;
	u_char band_num = 0, i = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE, "\n");

	mt_serv_get_tx_info(serv_test);

	mt_serv_get_chip_info(serv_test, &testmode_chip_info);

	band_num = testmode_chip_info.u1chip_band_num;

	for (i = 0; i < band_num; i++) {
		tx_cnt = 0;
		serv = serv_test;
		if (mt_serv_get_serv_test_by_band(&serv, i) == SERV_STATUS_SUCCESS)
			tx_cnt =  CONFIG_GET_PARAM(serv, tx_stat.tx_done_cnt);

		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"tx_cnt = %u, band_idx = %d, band_num = %d\n",
			tx_cnt, i, band_num);
	}

	return ret;
}

s_int32 mt_agent_show_ru_info(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int16 sta_idx = 0;
	struct test_ru_info *ru_sta;

	ru_sta = (struct test_ru_info *)CONFIG_GET_PADDR(serv_test,
							ru_info_list);

	for (sta_idx = 0 ; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
		if (ru_sta[sta_idx].valid) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_OFF,
				"list index[%u] RU index[%u] in Segment[%u]: Allocation[%u]"
				"aid[%u](0: N/A) TX mcs[%u],nss[%u], ldpc[%u], "
				"start spetial stream:%u, mpdu length:%u, alpha:%d\n",
				sta_idx, ru_sta[sta_idx].ru_index,
				(ru_sta[sta_idx].ps160 << 1) | ru_sta[sta_idx].b0, ru_sta[sta_idx].allocation,
				ru_sta[sta_idx].aid, ru_sta[sta_idx].rate, ru_sta[sta_idx].nss,
				ru_sta[sta_idx].ldpc, ru_sta[sta_idx].start_sp_st,
				ru_sta[sta_idx].mpdu_length, ru_sta[sta_idx].alpha);
		}
	}

	return ret;
}

static struct agent_cli_show_handler cli_show_cmds[] = {
	{"ATETXDONE", mt_agent_show_TxDone},
	{"ATERUINFO", mt_agent_show_ru_info},
	{"", NULL}	/* the last entry */
};

s_int32 mt_agent_cli_show(u_char *name, struct service *serv)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct service_test *serv_test = NULL;
	struct agent_cli_show_handler *entry = cli_show_cmds;

	if (!serv) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"service is NULL\n");
		return -SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	serv_test = (struct service_test *)serv->serv_handle;
	if (serv_test->test_config.op_mode & OP_MODE_STOP) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"op_mode: %x\n", serv_test->test_config.op_mode);
		return SERV_STATUS_AGENT_FAIL;
	}

	while (strlen(entry->name)) {
		if (strcmp(name, entry->name) == 0) {
			ret = entry->handler(serv_test);
			break;
		}

		entry++;
	}

	if (strlen(entry->name) == 0) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				 "No support %s !\n", name);
	}

	return ret;
}

static struct agent_cli_set_w_handler cli_set_w_cmds[] = {
	{"", NULL}	/* the last entry */
};

s_int32 mt_agent_cli_set_w(u_char *name, struct service *serv, u_char *param)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_long str2value = 0;
	struct agent_cli_set_w_handler *entry = cli_set_w_cmds;
	struct hqa_frame_ctrl *hqa_cmd = &hqa_cmd_frame_ctrl;
	u_int16 value = 0;
	struct service_test *serv_test = NULL;

	if (!serv) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"service is NULL\n");
		return -SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	serv_test = (struct service_test *)serv->serv_handle;
	if (serv_test->test_config.op_mode & OP_MODE_STOP) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"op_mode %x\n", serv_test->test_config.op_mode);
		return SERV_STATUS_AGENT_FAIL;
	}

	sys_ad_zero_mem(hqa_cmd, sizeof(*hqa_cmd));

	ret = sys_ad_alloc_mem((pu_char *)&(hqa_cmd->hqa_frame_eth),
			sizeof(struct hqa_frame));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");

		return ret;
	}
	sys_ad_zero_mem(hqa_cmd->hqa_frame_eth, sizeof(struct hqa_frame));

	if (kstrtol(param, 10, &str2value) == 0) {
		value = str2value;
		value = SERV_OS_HTONS(value);
		sys_ad_move_mem(hqa_cmd->hqa_frame_eth->data, &value, sizeof(u_int16));

		while (strlen((char *)entry->name)) {
			if (strcmp((char *)name, (char *)entry->name) == 0)
				ret = entry->handler(
				(struct service_test *)serv->serv_handle,
					hqa_cmd);

			entry++;
		}
	} else
		ret = SERV_STATUS_AGENT_INVALID_PARAM;

	if (hqa_cmd->hqa_frame_eth)
		sys_ad_free_mem(hqa_cmd->hqa_frame_eth);

	return ret;
}

static struct agent_cli_set_dw_handler cli_set_dw_cmds[] = {
	{"ATETXMCS", hqa_set_rate},
	{"ATETXNSS", hqa_set_nss},
	{"ATENSS", hqa_set_nss},
	{"ATETXSTBC", hqa_set_stbc},
	{"ATETXMODE", hqa_set_preamble},
	{"ATETXGI", hqa_set_short_gi},
	{"ATETXPE", hqa_set_tx_pe},
	{"ATEFIXEDPAYLOAD", hqa_set_fixed_payload},
	{"ATETXFREQOFFSET", hqa_set_freq_offset},
	{"", NULL}	/* the last entry */
};

s_int32 mt_agent_cli_set_dw(u_char *name, struct service *serv, u_char *param)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_long str2value = 0;
	struct agent_cli_set_dw_handler *entry = cli_set_dw_cmds;
	struct hqa_frame_ctrl *hqa_cmd = &hqa_cmd_frame_ctrl;
	u_int32 value = 0;
	struct service_test *serv_test = NULL;

	if (!serv) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"service is NULL\n");
		return -SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	serv_test = (struct service_test *)serv->serv_handle;
	if (serv_test->test_config.op_mode & OP_MODE_STOP) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"op_mode: %x\n", serv_test->test_config.op_mode);
		return SERV_STATUS_AGENT_FAIL;
	}

	sys_ad_zero_mem(hqa_cmd, sizeof(*hqa_cmd));

	ret = sys_ad_alloc_mem((pu_char *)&(hqa_cmd->hqa_frame_eth),
			sizeof(struct hqa_frame));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");

		return ret;
	}
	sys_ad_zero_mem(hqa_cmd->hqa_frame_eth, sizeof(struct hqa_frame));

	if (kstrtol(param, 10, &str2value) == 0) {
		value = str2value;
		value = SERV_OS_HTONL(value);
		sys_ad_move_mem(hqa_cmd->hqa_frame_eth->data, &value, sizeof(u_int32));

		while (strlen((char *)entry->name)) {
			if (strcmp((char *)name, (char *)entry->name) == 0)
				ret = entry->handler(
				(struct service_test *)serv->serv_handle,
					hqa_cmd);

			entry++;
		}
	} else
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
	if (hqa_cmd->hqa_frame_eth)
		sys_ad_free_mem(hqa_cmd->hqa_frame_eth);
	return ret;
}

s_int32 mt_agent_set_bw(struct service_test *serv_test, u_char *arg)
{
	u_int32 ret = SERV_STATUS_SUCCESS;
	u_long str2value = 0;
	u_int32 param[2] = {0};
	u_int8 i = 0;
	u_char *value;
	struct hqa_frame_ctrl *hqa_cmd = &hqa_cmd_frame_ctrl;
	u_char *data = NULL;

	sys_ad_zero_mem(hqa_cmd, sizeof(*hqa_cmd));
	ret = sys_ad_alloc_mem((pu_char *)&(hqa_cmd->hqa_frame_eth),
			sizeof(struct hqa_frame));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");

		return ret;
	}
	sys_ad_zero_mem(hqa_cmd->hqa_frame_eth, sizeof(struct hqa_frame));

	data = hqa_cmd->hqa_frame_eth->data;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"Bw = %s\n", arg);

	for (i = 0, value = agent_trtok(arg, ":");
		value;
		value = agent_trtok(NULL, ":"), i++) {
		if (i == 2)
			break;

		if (kstrtol(value, 10, &str2value)) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		param[i] = str2value;
	}

	if (param[1] == 0 && i == 1) {
		param[1] = param[0];
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"DBW is not specified, aligned with CBW = %d\n", param[0]);
	}

	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&param[0], &data);
	hqa_set_system_bw(serv_test, hqa_cmd);

	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&param[1], &data);
	hqa_set_per_pkt_bw(serv_test, hqa_cmd);

err_out:
	if (hqa_cmd->hqa_frame_eth)
		sys_ad_free_mem(hqa_cmd->hqa_frame_eth);

	return ret;
}

s_int32 mt_agent_set_ctrl_band(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"is not supported\n");

	return ret;
}

s_int32 mt_agent_set_pwr(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_long input = 0;
	u_int32 value = 0;
	struct hqa_frame_ctrl *hqa_cmd = &hqa_cmd_frame_ctrl;
	u_char *data = NULL;

	sys_ad_zero_mem(hqa_cmd, sizeof(*hqa_cmd));
	ret = sys_ad_alloc_mem((pu_char *)&(hqa_cmd->hqa_frame_eth),
			sizeof(struct hqa_frame));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");

		return ret;
	}
	sys_ad_zero_mem(hqa_cmd->hqa_frame_eth, sizeof(struct hqa_frame));
	data = hqa_cmd->hqa_frame_eth->data;

	if (kstrtol(arg, 10, (long *)&input) == 0) {
		/* power */
		value = input;
		set_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, &data);
		/* band index */
		value = serv_test->ctrl_band_idx;
		set_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, &data);
		/* channel */
		value = CONFIG_GET_PARAM(serv_test, channel);
		set_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, &data);
		/* channel band */
		value = CONFIG_GET_PARAM(serv_test, ch_band);
		set_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, &data);
		/* ant index */
		value = 0;
		set_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, &data);

		hqa_set_tx_power_ext(serv_test, hqa_cmd);
	} else
		ret = SERV_STATUS_AGENT_INVALID_PARAM;

	if (hqa_cmd->hqa_frame_eth)
		sys_ad_free_mem(hqa_cmd->hqa_frame_eth);
	return ret;
}

s_int32 mt_agent_set_channel(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_long str2value = 0;
	u_int32 value = 0;
	u_int32 input[5] = {0};
	u_int8 i = 0;
	struct hqa_frame_ctrl *hqa_cmd = &hqa_cmd_frame_ctrl;
	u_char *data = NULL, *tok = NULL;

	sys_ad_zero_mem(input, 5);
	sys_ad_zero_mem(hqa_cmd, sizeof(*hqa_cmd));
	ret = sys_ad_alloc_mem((pu_char *)&(hqa_cmd->hqa_frame_eth),
			sizeof(struct hqa_frame));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");

		return ret;
	}
	sys_ad_zero_mem(hqa_cmd->hqa_frame_eth, sizeof(struct hqa_frame));
	data = hqa_cmd->hqa_frame_eth->data;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"control_band_idx:%x, Channel = %s\n",
		serv_test->ctrl_band_idx, arg);

	for (i = 0, tok = agent_trtok(arg, ":");
		tok;
		tok = agent_trtok(NULL, ":"), i++) {
		if (i == 5)
			break;

		if (i == 4) {
			if (kstrtol(tok, 16, &str2value)) {
				ret = SERV_STATUS_AGENT_INVALID_PARAM;
				goto err_out;
			}
		} else if (kstrtol(tok, 10, &str2value)) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		input[i] = str2value;
	}

	/* For backward compatibility */
	if (input[0] >= 36 && input[0] <= 181) {
		if (input[1] == 0) {
			input[1] = 1; /* channel_band 5G as 1*/
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
	"\x1b[41m 5G Channel:%d, then force Channel_Band:%d !!\x1b[m\n",
				input[0], input[1]);
		}
	}

	/* ext id */
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* parameters count */
	value = 10;
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* band index */
	value = serv_test->ctrl_band_idx;
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* central channel index */
	value = input[0];
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* 2nd central channel index */
	value = input[3];
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* system bandwidth */
	value = CONFIG_GET_PARAM(serv_test, bw);
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* data band width */
	value = CONFIG_GET_PARAM(serv_test, per_pkt_bw);
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* primary offset */
	value = input[2];
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* reason */
	value = 0;
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* channel band */
	value = input[1];
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	value = 0;
	/* out band frequency */
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);

	/* indication specific */
	value = input[4];
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);

	hqa_cmd->hqa_frame_eth->length = 48;

	hqa_set_channel_ext(serv_test, hqa_cmd);

err_out:
	if (hqa_cmd->hqa_frame_eth)
		sys_ad_free_mem(hqa_cmd->hqa_frame_eth);
	return ret;
}

s_int32 mt_agent_set_ru_cli(struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_AGENT_INVALID_PARAM;
	u_char *value = NULL, i = 0, input_cnt = 0;
	u_int32 seg = 0, argLen = 0;
	struct test_ru_info *ru_info = NULL;
	struct test_ru_allocatoin *ru_allocation = NULL;
	u_int32 band_idx = (u_int32)(serv_test->ctrl_band_idx);


	ru_info = (struct test_ru_info *)CONFIG_GET_PADDR(serv_test,
							ru_info_list[0]);
	ru_allocation = CONFIG_GET_PADDR(serv_test, ru_alloc);
	argLen = strlen(arg);


	if (argLen > 0) {
		sys_ad_zero_mem(ru_info,
				sizeof(struct test_ru_info)*MAX_MULTI_TX_STA);
		sys_ad_set_mem(ru_allocation, sizeof(*ru_allocation), 0xff);

		for (i = 0, value = agent_trtok(arg, ":");
			value && (i < MAX_MULTI_TX_STA);
			value = agent_trtok((char *)NULL, ":")) {
			if (strlen(value) == 1) {
				ret = kstrtouint(value, 10, &seg);
				if (seg >= 4 || ret != 0) {
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
						 "Given segment[%u] is wrong. ret:%d\n",
						  seg, ret);
					ret = SERV_STATUS_AGENT_INVALID_PARAM;
					goto err_out;
				}
			} else {
				input_cnt = sscanf(value,
						"%4x-%u-%u-%u-%u-%u-%u-%u-%u",
						&ru_info[i].allocation,
						&ru_info[i].aid,
						&ru_info[i].ru_index,
						&ru_info[i].rate,
						&ru_info[i].ldpc,
						&ru_info[i].nss,
						&ru_info[i].start_sp_st,
						&ru_info[i].mpdu_length,
						&ru_info[i].alpha);

				if (strlen(value) > 0 && input_cnt == 9) {
					ru_info[i].valid = TRUE;

					ru_info[i].b0 = seg % 2;
					ru_info[i].ps160 = seg / 2;

					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
						 "segment[%d]: alloc:%04x\n",
						  seg,
						  ru_info[i].allocation);
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
						 "\t\t\tru_idx:%d, length:%d,\n",
						  ru_info[i].ru_index,
						  ru_info[i].mpdu_length);
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
						 "\t\t\talpha:%d, rate:0x%x,\n",
						  ru_info[i].alpha,
						  ru_info[i].rate);
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
						 "\t\t\tldpc:%d, nss:%d\n",
						  ru_info[i].ldpc,
						  ru_info[i].nss);
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
						 "\t\t\tb0:%u, ps160:%u, valid:%d\n",
						  ru_info[i].b0,
						  ru_info[i].ps160, ru_info[i].valid);
					i++;
				} else {
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
						"Invalid format, %s ignored!\n", arg);
					goto err_out;
				}
			}
		}
		ret = serv_test->test_op->op_set_ru_info(
		serv_test->test_winfo,
		band_idx,
		ru_info);
	}

err_out:
	return ret;
}

s_int32 mt_agent_set_txant(struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_frame_ctrl *hqa_cmd = &hqa_cmd_frame_ctrl;
	u_char *data = NULL;
	u_int32 value = 0;
	u_long str2value = 0;

	sys_ad_zero_mem(hqa_cmd, sizeof(*hqa_cmd));
	ret = sys_ad_alloc_mem((pu_char *)&(hqa_cmd->hqa_frame_eth),
			sizeof(struct hqa_frame));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");

		return ret;
	}
	sys_ad_zero_mem(hqa_cmd->hqa_frame_eth, sizeof(struct hqa_frame));
	data = hqa_cmd->hqa_frame_eth->data;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"TX PATH = %s\n", arg);

	if (!strchr(arg, ':')) {
		hqa_cmd->hqa_frame_eth->length = 2*sizeof(u_int32);

		if (kstrtol(arg, 10, &str2value) == 0) {
			value = str2value;
			set_param_and_shift_buf(TRUE, sizeof(u_int32),
						(u_char *)&value, &data);
			value = SERV_GET_PARAM(serv_test, ctrl_band_idx);
			set_param_and_shift_buf(TRUE, sizeof(u_int32),
						(u_char *)&value, &data);
			hqa_set_tx_path(serv_test, hqa_cmd);
		} else
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
	} else {
		u_char i = 0, *tok = NULL;
		u_int32 input[2] = {0};

		for (i = 0, tok = agent_trtok(arg, ":");
			tok;
			tok = agent_trtok(NULL, ":"), i++) {
			if (i == 2)
				break;

			if (kstrtol(tok, 10, &str2value)) {
				ret = SERV_STATUS_AGENT_INVALID_PARAM;
				goto err_out;
			}

			input[i] = str2value;
		}

		if (input[0])	{/* user specify SPE index */
			input[1] |= TEST_ANT_USER_DEF;

			hqa_cmd->hqa_frame_eth->length = 2*sizeof(u_int32);
			set_param_and_shift_buf(TRUE, sizeof(u_int32),
						(u_char *)&input[1], &data);
			value = SERV_GET_PARAM(serv_test, ctrl_band_idx);
			set_param_and_shift_buf(TRUE, sizeof(u_int32),
						(u_char *)&value, &data);
			hqa_set_tx_path(serv_test, hqa_cmd);
		}
	}

err_out:
	if (hqa_cmd->hqa_frame_eth)
		sys_ad_free_mem(hqa_cmd->hqa_frame_eth);
	return ret;
}

s_int32 mt_agent_set_rxant(struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_frame_ctrl *hqa_cmd = &hqa_cmd_frame_ctrl;
	u_char *data = NULL;
	u_int32 value = 0;
	u_long str2value = 0;

	sys_ad_zero_mem(hqa_cmd, sizeof(*hqa_cmd));
	ret = sys_ad_alloc_mem((pu_char *)&(hqa_cmd->hqa_frame_eth),
			sizeof(struct hqa_frame));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");

		return ret;
	}
	sys_ad_zero_mem(hqa_cmd->hqa_frame_eth, sizeof(struct hqa_frame));
	data = hqa_cmd->hqa_frame_eth->data;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"RX PATH = %s\n", arg);

	hqa_cmd->hqa_frame_eth->length = 2*sizeof(u_int32);

	if (kstrtol(arg, 10, (long *)&str2value) == 0) {
		value = str2value;
		set_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, &data);
		value = SERV_GET_PARAM(serv_test, ctrl_band_idx);
		set_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, &data);

		hqa_set_rx_path(serv_test, hqa_cmd);
	} else
		ret = SERV_STATUS_AGENT_INVALID_PARAM;

	sys_ad_free_mem(hqa_cmd->hqa_frame_eth);
	return ret;
}

s_int32 mt_agent_set_payload(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 pl_len = 0, fixed_payload = 0, pay_load = 0;
	u_char *p_payload = NULL;
	u_int32 band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	struct test_wlan_info *winfo = serv_test->test_winfo;

	pl_len = strnlen(arg, 4);
	CONFIG_SET_PARAM(serv_test, pl_len, pl_len);
	p_payload = CONFIG_GET_PARAM(serv_test, payload);
	sys_ad_zero_mem(p_payload, TEST_MAX_PATTERN_SIZE);
	sys_ad_move_mem(p_payload, arg, pl_len);

	ret = kstrtou32(p_payload, 16, &pay_load);
	if (ret != SERV_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"Covert fail!!\n");
		return SERV_STATUS_AGENT_FAIL;
	}

	fixed_payload = CONFIG_GET_PARAM(serv_test, fixed_payload);

	net_ad_set(winfo, RF_AT_FUNCID_SET_DBDC_BAND_IDX, band_idx);
	net_ad_set(winfo, RF_AT_FUNCID_SET_PAYLOAD, ((fixed_payload << 16) | pay_load));

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"pl_len:%u payload:%s\n", pl_len, p_payload);

	return ret;
}

s_int32 mt_set_pream_punc(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int16 preampunc = 0;

	ret = kstrtou16(arg, 16, &preampunc);
	if (ret != SERV_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"Covert fail!!\n");
		return SERV_STATUS_AGENT_FAIL;
	}

	serv_test->test_config.preampunc = preampunc;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
		"arg:%s preampunc:%u\n", arg, preampunc);

	return ret;
}

s_int32 mt_agent_set_ch_pwr_offset(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 value = 0;
	struct hqa_frame_ctrl *hqa_cmd = &hqa_cmd_frame_ctrl;
	u_char *data = NULL, *ptemp = NULL;
	long str2value = 0;
	s_int32 param[2] = {0};
	u_int8 i = 0;

	sys_ad_zero_mem(hqa_cmd, sizeof(*hqa_cmd));
	ret = sys_ad_alloc_mem((pu_char *)&(hqa_cmd->hqa_frame_eth),
			sizeof(struct hqa_frame));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");

		return ret;
	}
	sys_ad_zero_mem(hqa_cmd->hqa_frame_eth, sizeof(struct hqa_frame));
	data = hqa_cmd->hqa_frame_eth->data;

	for (i = 0, ptemp = agent_trtok(arg, ":");
		ptemp;
		ptemp = agent_trtok(NULL, ":"), i++) {
		if (i == 2)
			break;

		if (kstrtol(ptemp, 16, &str2value)) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		param[i] = str2value;
	}

	/* band index */
	value = serv_test->ctrl_band_idx;
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* ant index */
	value = param[0];
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* ch power offset */
	value = param[1];
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);

	hqa_set_ch_power_offset(serv_test, hqa_cmd);

err_out:
	if (hqa_cmd->hqa_frame_eth)
		sys_ad_free_mem(hqa_cmd->hqa_frame_eth);
	return ret;
}

s_int32 mt_agent_set_cvt_pwr_offset(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 value = 0;
	struct hqa_frame_ctrl *hqa_cmd = &hqa_cmd_frame_ctrl;
	u_char *data = NULL, *ptemp = NULL;
	long str2value = 0;
	s_int32 param[2] = {0};
	u_int8 i = 0;

	sys_ad_zero_mem(hqa_cmd, sizeof(*hqa_cmd));
	ret = sys_ad_alloc_mem((pu_char *)&(hqa_cmd->hqa_frame_eth),
			sizeof(struct hqa_frame));
	if (ret) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"allocate register memory fail\n");

		return ret;
	}
	sys_ad_zero_mem(hqa_cmd->hqa_frame_eth, sizeof(struct hqa_frame));
	data = hqa_cmd->hqa_frame_eth->data;

	for (i = 0, ptemp = agent_trtok(arg, ":");
		ptemp;
		ptemp = agent_trtok(NULL, ":"), i++) {
		if (i == 2)
			break;

		if (kstrtol(ptemp, 16, &str2value)) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		param[i] = str2value;
	}

	/* band index */
	value = serv_test->ctrl_band_idx;
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* ant index */
	value = param[0];
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* cvt power offset */
	value = param[1];
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);

	hqa_set_cvt_power_offset(serv_test, hqa_cmd);

err_out:
	if (hqa_cmd->hqa_frame_eth)
		sys_ad_free_mem(hqa_cmd->hqa_frame_eth);
	return ret;
}


static struct agent_cli_set_ext_handler cli_set_ext_cmds[] = {
	{"ATECTRLBANDIDX", mt_agent_set_ctrl_band},
	{"ATETXPOW0", mt_agent_set_pwr},
	{"ATECHANNEL", mt_agent_set_channel},
	{"ATETXBW", mt_agent_set_bw},
	{"ATERUINFO", mt_agent_set_ru_cli},
	{"ATETXANT", mt_agent_set_txant},
	{"ATERXANT", mt_agent_set_rxant},
	{"ATEPAYLOAD", mt_agent_set_payload},
	{"ATEPREAMPUNC", mt_set_pream_punc},
	{"ATECHOFFSET", mt_agent_set_ch_pwr_offset},
	{"ATECVTOFFSET", mt_agent_set_cvt_pwr_offset},
	{"", NULL}	/* the last entry */
};

s_int32 mt_agent_cli_set_ext(u_char *name, struct service *serv, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct agent_cli_set_ext_handler *entry = cli_set_ext_cmds;
	struct service_test *serv_test = NULL;

	if (!serv) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"service is NULL\n");
		return -SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	serv_test = (struct service_test *)serv->serv_handle;
	if (serv_test->test_config.op_mode & OP_MODE_STOP) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"op_mode: %x\n", serv_test->test_config.op_mode);
		return SERV_STATUS_AGENT_FAIL;
	}

	while (strlen(entry->name)) {
		if (strcmp(name, entry->name) == 0)
			ret = entry->handler(
				(struct service_test *)serv->serv_handle,
				arg);

		entry++;
	}

	return ret;
}

/*****************************************************************************
 *	Service init/exit handler
 *****************************************************************************/
s_int32 mt_agent_init_service(struct service *serv)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	if (!serv) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"service is NULL\n");
		return -SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	switch (serv->serv_id) {
	case SERV_HANDLE_TEST:
		ret = mt_serv_init_test(
			(struct service_test *)serv->serv_handle);
		break;

	default:
		return SERV_STATUS_AGENT_FAIL;
	}

	return ret;
}

s_int32 mt_agent_exit_service(struct service *serv)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	if (!serv) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"service is NULL\n");
		return -SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	switch (serv->serv_id) {
	case SERV_HANDLE_TEST:
		ret = mt_serv_exit_test(
			(struct service_test *)serv->serv_handle);
		break;

	default:
		return SERV_STATUS_AGENT_FAIL;
	}

	return ret;
}
