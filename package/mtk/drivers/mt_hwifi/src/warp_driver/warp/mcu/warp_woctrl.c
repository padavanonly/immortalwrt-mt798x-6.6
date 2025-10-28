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

	Module Name: wifi_offload
	warp_woctrl.c
*/

#include "warp.h"
#include "warp_hw.h"
#include "warp_woif.h"
#include <linux/device.h>
#include <linux/reset.h>

#define WOCPU_DLM_DEV_NODE "mediatek,wocpu_dlm"

static char *warp_woctrl_state_to_string(u8 state)
{
	switch (state) {
	case WO_STATE_DISABLE:
		return "DISABLE";

	case WO_STATE_ENABLE:
		return "ENABLE";

	case WO_STATE_GATING:
		return "GATING";

	case WO_STATE_HALT:
		return "HALT";

	case WO_STATE_SER_RESET:
		return "PAUSE";

	case WO_STATE_UNDEFINED:
		return "UNDEFINED";

	default:
		warp_dbg(WARP_DBG_OFF, "%s(), state: %d\n", __func__, state);
		return "UNKNOWN";
	}
}

static int warp_woctrl_get_trigger_cr_value(u8 state)
{
	switch (state) {
	case WO_STATE_ENABLE:
		return WOIF_TRIGGER_ENABLE;

	case WO_STATE_GATING:
		return WOIF_TRIGGER_GATING;

	case WO_STATE_HALT:
		return WOIF_TRIGGER_HALT;

	default:
		warp_dbg(WARP_DBG_OFF,
			"%s(), state: %d, %s, unfinded trigger value\n",
			__func__, state, warp_woctrl_state_to_string(state));

		return WOIF_UNDEFINED;
	}
}

static int warp_woctrl_get_done_cr_value(u8 state)
{
	switch (state) {
	case WO_STATE_ENABLE:
		return WOIF_ENABLE_DONE;

	case WO_STATE_DISABLE:
		return WOIF_DISABLE_DONE;

	case WO_STATE_GATING:
		return WOIF_GATING_DONE;

	case WO_STATE_HALT:
		return WOIF_HALT_DONE;

	default:
		warp_dbg(WARP_DBG_OFF,
			"%s(), wrong state: %d, %s, , unfinded done value\n",
			__func__, state, warp_woctrl_state_to_string(state));

		return WOIF_UNDEFINED;
	}
}


static void warp_woctrl_send_msg_and_wait_cr(struct warp_entry *warp, u8 state)
{
	struct wed_entry *wed = &warp->wed;
	struct warp_msg_cmd cmd = {0};
	u32 value = 0;
	int done_value;
	u8 wo_state = state;
	bool print_once = false;
	unsigned long timeout = jiffies + WOCPU_TIMEOUT;

	warp_dbg(WARP_DBG_OFF, "%s(), idx:%d, state:%s\n", __func__,
		warp->idx, warp_woctrl_state_to_string(state));

	/* send wo cmd */
	cmd.param.cmd_id = WO_CMD_CHANGE_STATE;
	cmd.param.to_id = MODULE_ID_WO;
	cmd.param.wait_type = WARP_MSG_WAIT_TYPE_NONE;
	cmd.msg = &wo_state;
	cmd.msg_len = sizeof(u8);

	warp_msg_send_cmd(warp->idx, &cmd);

	/* wait CR */
	done_value = warp_woctrl_get_done_cr_value(state);

	if (done_value != WOIF_UNDEFINED) {
		warp_dbg(WARP_DBG_OFF, "%s(): wait for wocpu\n", __func__);
		do {
			warp_dummy_cr_get(wed, WED_DUMMY_CR_WO_STATUS, &value);

			if (!print_once) {
				warp_dbg(WARP_DBG_OFF, "[tmp]%s(): wait for wocpu, value:%d, value_done:%d\n", __func__, value, done_value);
				print_once = true;
			}
		} while (value != done_value && !time_after(jiffies, timeout));

		warp_dbg(WARP_DBG_OFF, "%s(): wocpu is ready, done_value:%d\n",
			__func__, done_value);
	}

#if 0
	if (state == WO_STATE_HALTED) {
		/* stop polling when dummy cr as 0 */
		do {
			addr = (warp->idx == 0) ? 0x15194200 : 0x15194204;
			warp_io_read32(wed, addr, &value);
		} while (value != 0);

		/* TODO: do reset_ctrl */
		warp_decrease_ref();
		if (warp_get_ref() == 0) {
		}
	}
#endif
}

static void warp_woctrl_kickout(struct warp_entry *warp, u8 state)
{
	struct wed_entry *wed = &warp->wed;
	struct woif_bus *bus = &warp->woif.bus;
	u32 value = 0;
	int trigger_value, done_value;
	bool print_once = false;
	unsigned long timeout = jiffies + WOCPU_TIMEOUT;

	warp_dbg(WARP_DBG_OFF, "%s(), idx: %d, state:%s\n", __func__,
		warp->idx, warp_woctrl_state_to_string(state));

	/* set CR */
	trigger_value = warp_woctrl_get_trigger_cr_value(state);

	if (trigger_value != WOIF_UNDEFINED)
		warp_dummy_cr_set(wed, WED_DUMMY_CR_WO_STATUS, trigger_value);

	/* kickout */
	bus->kickout(bus);

	/* wait CR */
	done_value = warp_woctrl_get_done_cr_value(state);

	if (done_value != WOIF_UNDEFINED) {
		warp_dbg(WARP_DBG_OFF, "%s(): wait for wocpu\n", __func__);
		do {
			warp_dummy_cr_get(wed, WED_DUMMY_CR_WO_STATUS, &value);

			if (!print_once) {
				warp_dbg(WARP_DBG_OFF,
					"[tmp]%s(): wait for wocpu, value:%d, value_done:%d\n",
					__func__, value, done_value);
				print_once = true;
			}
		} while (value != done_value && !time_after(jiffies, timeout));

		warp_dbg(WARP_DBG_OFF, "%s(): wocpu is ready, done_value:%d\n",
			__func__, done_value);
	}
}

static void warp_woctrl_send_msg_and_wait_rsp(struct warp_entry *warp, u8 state)
{
	struct warp_msg_cmd cmd = {0};
	u8 wo_state = state;

	warp_dbg(WARP_DBG_OFF, "%s(), idx: %d, state:%s\n",
		__func__, warp->idx, warp_woctrl_state_to_string(state));

	/* send wo ENABLED cmd */
	cmd.param.cmd_id = WO_CMD_CHANGE_STATE;
	cmd.param.to_id = MODULE_ID_WO;
	cmd.param.wait_type = WARP_MSG_WAIT_TYPE_RSP_STATUS;
	cmd.param.timeout = 3000;
	cmd.msg = &wo_state;
	cmd.msg_len = sizeof(u8);

	warp_msg_send_cmd(warp->idx, &cmd);

	warp_dbg(WARP_DBG_INF, "%s(): got wocpu msg response\n", __func__);
}

static void warp_woctrl_wakeup_wo(struct warp_entry *warp, u8 state)
{
#if (WED_HWRRO_MODE == WED_HWRRO_MODE_WOCPU)
	warp_woctrl_kickout(warp, state);
#else /* (WED_HWRRO_MODE == WED_HWRRO_MODE_WOCPU) */
	warp_woctrl_send_msg_and_wait_rsp(warp, state);
#endif /* (WED_HWRRO_MODE == WED_HWRRO_MODE_WOCPU) */
}

static int warp_woctrl_reset_wocpu(struct warp_entry *warp)
{
	int ret;
	struct device_node *node = of_find_compatible_node(NULL, NULL, WOCPU_DLM_DEV_NODE);
	struct reset_control *rst_warp = of_reset_control_get(node, "wocpu_rst");

	if (IS_ERR(rst_warp)) {
		warp_dbg(WARP_DBG_OFF, "%s(): can't get reset control, rst_warp=%ld\n",
			__func__, PTR_ERR(rst_warp));
		return WARP_FAIL_STATUS;
	}

	ret = reset_control_assert(rst_warp);
	if (ret) {
		warp_dbg(WARP_DBG_OFF, "%s(): unable to assert, ret:%d\n", __func__, ret);
		return WARP_FAIL_STATUS;
	}

	warp_dbg(WARP_DBG_OFF, "%s(): wait for wocpu assert\n", __func__);

	do {
		ret = reset_control_status(rst_warp);
	} while (ret <= 0);

	warp_dbg(WARP_DBG_OFF, "%s(): wocpu is assert\n", __func__);

	ret = reset_control_deassert(rst_warp);

	if (ret) {
		warp_dbg(WARP_DBG_OFF, "%s(): unable to deassert, ret:%d\n", __func__, ret);
		return WARP_FAIL_STATUS;
	}

	/* clear fwdl parameter */
	warp_fwdl_clear(warp);

	return WARP_OK_STATUS;
}

static int warp_woctrl_enter_disable_state(struct warp_entry *warp)
{
	struct warp_wo_ctrl *wo_ctrl = &warp->woif.wo_ctrl;

	if (wo_ctrl->cur_state == WO_STATE_ENABLE || wo_ctrl->cur_state == WO_STATE_WF_RESET)
		/* enable -> disable */
		warp_woctrl_send_msg_and_wait_cr(warp, WO_STATE_DISABLE);
	else if (wo_ctrl->cur_state == WO_STATE_GATING) {
		if (wo_ctrl->prev_state == WO_STATE_DISABLE)
			warp_woctrl_wakeup_wo(warp, WO_STATE_DISABLE);
		else if (wo_ctrl->prev_state == WO_STATE_ENABLE) {
			warp_dbg(WARP_DBG_OFF,
				"%s(), warning! it will leave gating\n", __func__);

			/* gating -> enable -> disable */
			warp_woctrl_send_msg_and_wait_rsp(warp, WO_STATE_ENABLE);
			warp_woctrl_send_msg_and_wait_cr(warp, WO_STATE_DISABLE);
		}
	} else {
		warp_dbg(WARP_DBG_OFF, "%s(), wrong state:%s\n", __func__,
			warp_woctrl_state_to_string(wo_ctrl->cur_state));
		return WARP_FAIL_STATUS;
	}

	return WARP_OK_STATUS;
}

static int warp_woctrl_enter_enable_state(struct warp_entry *warp)
{
	struct warp_wo_ctrl *wo_ctrl = &warp->woif.wo_ctrl;

	if (wo_ctrl->cur_state == WO_STATE_DISABLE)
		warp_woctrl_wakeup_wo(warp, WO_STATE_ENABLE);
	else if (wo_ctrl->cur_state == WO_STATE_GATING) {
		if (wo_ctrl->prev_state == WO_STATE_ENABLE)
			warp_woctrl_send_msg_and_wait_rsp(warp, WO_STATE_ENABLE);
		else
			/* gating -> enable */
			warp_woctrl_wakeup_wo(warp, WO_STATE_ENABLE);
	} else if (wo_ctrl->cur_state == WO_STATE_SER_RESET)
		warp_woctrl_send_msg_and_wait_rsp(warp, WO_STATE_ENABLE);
	else {
		warp_dbg(WARP_DBG_OFF, "%s(), wrong state:%s\n", __func__,
			warp_woctrl_state_to_string(wo_ctrl->cur_state));
		return WARP_FAIL_STATUS;
	}

	return WARP_OK_STATUS;
}

static int warp_woctrl_enter_halt_state(struct warp_entry *warp)
{
	struct warp_wo_ctrl *wo_ctrl = &warp->woif.wo_ctrl;
	int ret;

	if (wo_ctrl->cur_state == WO_STATE_DISABLE)
		warp_woctrl_wakeup_wo(warp, WO_STATE_HALT);
	else if (wo_ctrl->cur_state == WO_STATE_ENABLE)
		warp_woctrl_send_msg_and_wait_cr(warp, WO_STATE_HALT);
	else if (wo_ctrl->cur_state == WO_STATE_GATING) {
		if (wo_ctrl->prev_state == WO_STATE_ENABLE) {
			/* gating -> enable -> halt */
			warp_woctrl_send_msg_and_wait_rsp(warp, WO_STATE_ENABLE);
			warp_woctrl_send_msg_and_wait_cr(warp, WO_STATE_HALT);
		} else
			/* gating -> halt */
			warp_woctrl_wakeup_wo(warp, WO_STATE_HALT);
	} else {
		warp_dbg(WARP_DBG_OFF, "%s(), wrong state:%s\n", __func__,
			warp_woctrl_state_to_string(wo_ctrl->cur_state));
		return WARP_FAIL_STATUS;
	}

	ret = warp_woctrl_reset_wocpu(warp);

	return ret;
}

static int warp_woctrl_enter_gating_state(struct warp_entry *warp)
{
	struct warp_wo_ctrl *wo_ctrl = &warp->woif.wo_ctrl;

	if (wo_ctrl->cur_state == WO_STATE_ENABLE)
		warp_woctrl_send_msg_and_wait_cr(warp, WO_STATE_GATING);
	else if (wo_ctrl->cur_state == WO_STATE_DISABLE)
		warp_woctrl_wakeup_wo(warp, WO_STATE_GATING);
	else {
		warp_dbg(WARP_DBG_OFF, "%s(), wrong state:%s\n", __func__,
			warp_woctrl_state_to_string(wo_ctrl->cur_state));
		return WARP_FAIL_STATUS;
	}

	return WARP_OK_STATUS;
}

static int warp_woctrl_enter_ser_reset_state(struct warp_entry *warp)
{
	struct warp_wo_ctrl *wo_ctrl = &warp->woif.wo_ctrl;

	if (wo_ctrl->cur_state == WO_STATE_ENABLE)
		warp_woctrl_send_msg_and_wait_rsp(warp, WO_STATE_SER_RESET);
	else {
		warp_dbg(WARP_DBG_OFF, "%s(), wrong state:%s\n", __func__,
			warp_woctrl_state_to_string(wo_ctrl->cur_state));
		return WARP_FAIL_STATUS;
	}

	return WARP_OK_STATUS;
}

static bool warp_woctrl_sanity_check(struct warp_entry *warp, u8 state)
{
	struct warp_wo_ctrl *wo_ctrl = &warp->woif.wo_ctrl;

	if (state == wo_ctrl->cur_state) {
		warp_dbg(WARP_DBG_OFF, "%s(), alreay in the state, state: %s\n",
			__func__, warp_woctrl_state_to_string(state));

		return false;
	}

#if (WED_HWRRO_MODE != WED_HWRRO_MODE_WOCPU)
	if (state == WO_STATE_GATING) {
		warp_dbg(WARP_DBG_OFF, "%s(), can't enter GATING in non-cpu mode\n", __func__);
		return false;
	}
#endif /* (WED_HWRRO_MODE != WED_HWRRO_MODE_WOCPU) */

	return true;
}

void warp_woctrl_enter_state(struct warp_entry *warp, u8 state)
{
	struct warp_wo_ctrl *wo_ctrl = &warp->woif.wo_ctrl;
	int ret;

	if (!warp_woctrl_sanity_check(warp, state))
		return;

	warp_dbg(WARP_DBG_INF, "%s(), wo_state:%d, prev:%s, cur:%s, req: %s\n",
		__func__, state,
		warp_woctrl_state_to_string(wo_ctrl->prev_state),
		warp_woctrl_state_to_string(wo_ctrl->cur_state),
		warp_woctrl_state_to_string(state));

	switch (state) {
	case WO_STATE_DISABLE:
		ret = warp_woctrl_enter_disable_state(warp);
		break;

	case WO_STATE_ENABLE:
		ret = warp_woctrl_enter_enable_state(warp);
		break;

	case WO_STATE_GATING:
		ret = warp_woctrl_enter_gating_state(warp);
		break;

	case WO_STATE_SER_RESET:
		ret = warp_woctrl_enter_ser_reset_state(warp);
		break;

	case WO_STATE_HALT:
		ret = warp_woctrl_enter_halt_state(warp);
		break;

	default:
		warp_dbg(WARP_DBG_ERR, "%s(), wrong state:%s\n", __func__,
			warp_woctrl_state_to_string(wo_ctrl->cur_state));
		ret = WARP_FAIL_STATUS;
		break;
	}

	if (ret == WARP_OK_STATUS) {
		wo_ctrl->prev_state = wo_ctrl->cur_state;
		wo_ctrl->cur_state = state;
	}
}

void warp_woctrl_init_state(struct warp_entry *warp)
{
	struct warp_wo_ctrl *wo_ctrl = &warp->woif.wo_ctrl;

	warp_dbg(WARP_DBG_OFF, "%s(), wo_state: WO_STATE_ENABLE\n", __func__);

	wo_ctrl->cur_state = WO_STATE_ENABLE;
	wo_ctrl->prev_state = WO_STATE_UNDEFINED;
}

void warp_woctrl_exit_state(struct warp_entry *warp, u8 state)
{
	struct warp_wo_ctrl *wo_ctrl = &warp->woif.wo_ctrl;

	warp_dbg(WARP_DBG_OFF, "%s(), wo_state:%d, prev:%s, cur:%s, req: %s\n",
		__func__, state,
		warp_woctrl_state_to_string(wo_ctrl->prev_state),
		warp_woctrl_state_to_string(wo_ctrl->cur_state),
		warp_woctrl_state_to_string(state));

	if (wo_ctrl->cur_state != state) {
		warp_dbg(WARP_DBG_OFF, "%s(), current state is wrong. prev:%s, cur:%s\n",
			__func__,
			warp_woctrl_state_to_string(wo_ctrl->prev_state),
			warp_woctrl_state_to_string(wo_ctrl->cur_state));

		return;
	}

	if (state == WO_STATE_HALT) {
		warp_fwdl(warp);
		warp_woctrl_init_state(warp);
		/* ToDo: set rro */
	} else
		warp_woctrl_enter_state(warp, wo_ctrl->prev_state);
}

