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
	warp_evt.h
*/

#ifndef _WARP_EVT_H_
#define _WARP_EVT_H_

#include "warp_utility.h"

typedef void (*EVT_MSG_HANDLER)(char *msg, u16 msg_len, void *user_data);

enum {
	MSG_Q_UNKNOWN = 0,
	MSG_Q_INITED = 1,
	MAX_MSG_Q_STATE
};

struct warp_evt_hdlr_node {
	u16 evt_id; /* wo_event_id */
	EVT_MSG_HANDLER evt_handler;
	void *user_data;
	struct warp_evt_hdlr_node *next;
};

struct warp_evt_hdlr_queue {
	struct warp_evt_hdlr_node *head;
	u32 size;
	u8 state;
	spinlock_t q_lock;
};

int register_wo_event_handler (
	u8 wed_idx, u16 evt_id, EVT_MSG_HANDLER func, void *user_data);

void unregister_wo_event_handler (
	u8 wed_idx, u16 evt_id, EVT_MSG_HANDLER func, void *user_data);

void warp_evt_handle_event(
	u8 wed_idx, unsigned char *msg, u32 msg_len);

int warp_evt_hdlr_queue_init(u8 wed_idx);

int warp_evt_hdlr_queue_deinit(u8 wed_idx);


#endif /*_WARP_MSG_H_*/
