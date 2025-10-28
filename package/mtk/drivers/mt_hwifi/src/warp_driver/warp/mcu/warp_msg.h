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
    warp_msg.h
*/

#ifndef _WARP_MSG_H_
#define _WARP_MSG_H_

#include "warp_utility.h"
#include <linux/kthread.h>
#include "warp_evt.h"

#define WARP_MSG_TIMEOUT_DEFAULT 3000  /* millisecond */
#define WARP_MSG_MAX_BUFFER_LEN 2048

/* cmd which sends to warp */
enum warp_cmd_id {
	WARP_CMD_A
};

/* synchronous or asynchronous msg */
enum warp_msg_wait_type {
	WARP_MSG_WAIT_TYPE_NONE,
	WARP_MSG_WAIT_TYPE_RSP_STATUS,
	WARP_MSG_WAIT_TYPE_RSP_MSG
};

typedef void (*RSP_MSG_HANDLER)(char *msg, u16 msg_len, void *user_data);


struct msg_element {
	unsigned char *msg;
	u32 msg_len;
	struct msg_element *next;
};

struct msg_queue {
	struct msg_element *head;
	struct msg_element *tail;
	u32 size;
	u8 state;
	spinlock_t msg_lock;
};

struct warp_msg_wait_node {
	/* input */
	u8 cmd_id;    /* for debug */
	u16 uni_id;
	enum warp_msg_wait_type wait_type;
	struct completion recv_done;
	RSP_MSG_HANDLER rsp_hdlr;
	void *user_data;

	/* output */
	unsigned char *rsp_msg;     /* rsp message*/
	u32 rsp_msg_len;            /* rsp message length*/
	int rsp_status;
	struct warp_msg_wait_node *next;
};

struct warp_msg_wait_queue {
	struct warp_msg_wait_node *head;
	u32 size;
	u16 current_uni_id;
	u8 state;
	spinlock_t wait_lock;
};


struct warp_msg_ctrl {
	struct msg_queue msg_q;
	u8 wed_idx;
	u8 module_id;
	wait_queue_head_t kthread_q;
	bool thread_running;
	struct task_struct *kthread_task;
	int (*cmd_event_process)(u8 wed_idx, unsigned char *msg, u32 msg_len);

	struct warp_msg_wait_queue msg_wait_q;
	struct warp_evt_hdlr_queue evt_hdlr_q;
};

struct warp_msg_param {
	/* input */
	u32 to_id;    /* module id */
	u32 cmd_id;   /* warp_cmd_id or wo_cmd_id */

	enum warp_msg_wait_type wait_type;
	u32 timeout;           /* if 0, default: 3000ms */

	RSP_MSG_HANDLER rsp_hdlr;
	void *user_data;
};


struct warp_msg_cmd {
	u8 *msg;
	u32 msg_len;

	struct warp_msg_param param;

	/* output: caller allocates memory */
	u8 *rsp_msg;
	u32 rsp_msg_len;
};

enum {
	MODULE_ID_WARP = 0,
	MODULE_ID_WO = 1,
	MAX_MODULE_ID
};

#define WARP_MSG_DEF_TIMEOUT (0)


struct warp_msg_ctrl *get_warp_msg_ctrl(u8 id, u8 wed_idx);

int warp_msg_register(u8 id, u8 wed_idx, void *cmd_event_process);

int warp_msg_unregister(u8 id, u8 wed_idx);

void warp_msg_init(u8 wed_idx);
void warp_msg_deinit(u8 wed_idx);

int warp_msg_send_cmd(u8 wed_idx, struct warp_msg_cmd *msg);
void warp_msg_handle_rsp(u8 wed_idx, u8 from_id, unsigned char *msg, u32 msg_len);

int warp_msg_send_by_msg_q(u8 to_id, u8 wed_idx, unsigned char *msg, u32 msg_len);

#endif /*_WARP_MSG_H_*/
