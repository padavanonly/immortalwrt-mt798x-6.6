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
	warp_cmd.c
*/
#include "warp_cmd.h"
#include "warp_msg.h"
/*wo interface*/
#include "warp_woif.h"

static int warp_cmd_sanity(unsigned char *msg, u32 msg_len)
{
	struct warp_cmd_header *hdr = (struct warp_cmd_header *)msg;

	if (hdr->ver != 0) {
		warp_dbg(WARP_DBG_OFF, "%s(): ver(%d) should be zero \n", __func__, hdr->ver);
		return WARP_INVALID_PARA_STATUS;
	}

	if (msg_len < sizeof(struct warp_cmd_header)) {
		warp_dbg(WARP_DBG_OFF, "%s(): msg_len(%d) \n", __func__, msg_len);
		return WARP_INVALID_PARA_STATUS;
	}

	if (hdr->length != msg_len) {
		warp_dbg(WARP_DBG_OFF, "%s(): hdr->length(%d), msg_len(%d)\n",
			 __func__, hdr->length, msg_len);
		return WARP_INVALID_PARA_STATUS;
	}

	return WARP_OK_STATUS;
}


int warp_cmd_process_wo_ring(u8 wed_idx, unsigned char *msg, u32 msg_len)
{
	struct warp_cmd_header *hdr = (struct warp_cmd_header *)msg;

	if (warp_cmd_sanity(msg, msg_len) != 0)
	{
		warp_dbg(WARP_DBG_ERR, "%s(), sanity fail!", __func__);
		return WARP_INVALID_LENGTH_STATUS;
	}

	warp_dbg(WARP_DBG_INF, "%s()[%s]: wed_idx:%d, msg_len: %d, ver:%d, cmd_id:%d, length:%d, uni_id:%d, status:%d, flag:%d\n",
		__func__,
		WARP_CMD_FLAG_IS_RSP(hdr) ? "rsp" : "evt",
		wed_idx, msg_len, hdr->ver, hdr->cmd_id, hdr->length, hdr->uni_id, hdr->status, hdr->flag);


	// rsp
	if (WARP_CMD_FLAG_IS_RSP(hdr)) {
		warp_msg_handle_rsp(wed_idx, MODULE_ID_WO, msg, msg_len);
	} else {
		warp_evt_handle_event(wed_idx, msg, msg_len);
	}

	return WARP_OK_STATUS;
}


int warp_cmd_process_msg(u8 wed_idx, unsigned char *msg, u32 msg_len) {

	struct warp_cmd_header *hdr = (struct warp_cmd_header *)msg;

	// ko mode: wo -> warp
	if (WARP_CMD_FLAG_IS_FROM_TO_WO(hdr)) {
		warp_cmd_process_wo_ring(wed_idx, msg, msg_len);

		return WARP_OK_STATUS;
	}

	switch (hdr->cmd_id) {
		case WARP_CMD_A:
			break;

		default:
			break;
	}

	warp_msg_handle_rsp(wed_idx, MODULE_ID_WARP, msg, msg_len);

	return WARP_OK_STATUS;
}




