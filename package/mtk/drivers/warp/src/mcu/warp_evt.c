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

	Module Name: warp event handler
	warp_evt.c
*/

#include "warp_evt.h"
#include "warp_msg.h"
#include "warp_cmd.h"
#include "warp_meminfo_list.h"

static struct warp_evt_hdlr_queue *get_warp_evt_hdlr_queue(u8 module_id, u8 wed_idx)
{
	struct warp_msg_ctrl *ctrl = get_warp_msg_ctrl(module_id, wed_idx);

	if (ctrl == NULL)
		return NULL;

	return &ctrl->evt_hdlr_q;
}

static struct warp_evt_hdlr_node *warp_evt_get_hdlr_node(
	struct warp_evt_hdlr_queue *hdlr_q, u16 evt_id,
	EVT_MSG_HANDLER func, void *user_data)
{
	struct warp_evt_hdlr_node *node = NULL;

	spin_lock(&hdlr_q->q_lock);

	node = hdlr_q->head;

	while (node != NULL && (node->evt_id != evt_id ||
				node->evt_handler != func ||
				node->user_data != user_data)) {
		node = node->next;
	}

	spin_unlock(&hdlr_q->q_lock);

	return node;
}


static struct warp_evt_hdlr_node *warp_evt_hdlr_node_init(
	u16 evt_id, EVT_MSG_HANDLER hdlr, void *user_data)
{
	struct warp_evt_hdlr_node *node = NULL;

	warp_dbg(WARP_DBG_INF, "%s(), evt_id:%d\n", __func__, evt_id);

	warp_os_alloc_mem((unsigned char **)&node, sizeof(struct warp_evt_hdlr_node), GFP_ATOMIC);
	if (node == NULL)
		return NULL;
	memset(node, 0, sizeof(struct warp_evt_hdlr_node));

	node->evt_id = evt_id;
	node->evt_handler = hdlr;
	node->user_data = user_data;

	return node;
}

static void warp_evt_hdlr_node_enq(
	struct warp_evt_hdlr_queue *hdlr_q, struct warp_evt_hdlr_node *node)
{
	struct warp_evt_hdlr_node *last;

	spin_lock(&hdlr_q->q_lock);

	if (hdlr_q->head == NULL) {
		hdlr_q->head = node;
	} else {
		last = hdlr_q->head;
		while (last->next) {
			last = last->next;
		}
		last->next = node;
	}

	node->next = NULL;
	hdlr_q->size++;

	warp_dbg(WARP_DBG_INF, "%s(), evt_id:%d, size:%d\n",
		__func__, node->evt_id, hdlr_q->size);

	spin_unlock(&hdlr_q->q_lock);
}


static void warp_evt_hdlr_node_deq_and_delete(
	struct warp_evt_hdlr_queue *hdlr_q, struct warp_evt_hdlr_node *node)
{
	struct warp_evt_hdlr_node *cur_node, *prev_node = NULL;

	warp_dbg(WARP_DBG_INF, "%s(), evt_id:%d\n", __func__, node->evt_id);

	spin_lock(&hdlr_q->q_lock);

	cur_node = hdlr_q->head;

	while (cur_node != NULL && cur_node != node) {
		prev_node = cur_node;
		cur_node = cur_node->next;
	}

	if (cur_node == NULL) {
		spin_unlock(&hdlr_q->q_lock);
		return;
	} else if (cur_node == hdlr_q->head) {
		hdlr_q->head = cur_node->next;
	} else if (prev_node != NULL) {
		prev_node->next = cur_node->next;
	}

	hdlr_q->size--;
	spin_unlock(&hdlr_q->q_lock);

	warp_os_free_mem(cur_node);
}

static void warp_evt_notify_hdlr_node(
	struct warp_evt_hdlr_queue *hdlr_queue, unsigned char *evt_msg,
	u32 evt_msg_len, u16 evt_id, u16 uni_id) {

	struct warp_evt_hdlr_node *node = NULL;
	u32 search_cnt = 0;

	warp_dbg(WARP_DBG_INF, "%s(), evt_id:%d, uni_id:%d, evt_msg_len: %d, queue_size:%d\n",
		__func__, evt_id, uni_id, evt_msg_len, hdlr_queue->size);

	if (spin_trylock(&hdlr_queue->q_lock)) {
		search_cnt = hdlr_queue->size;

		node = hdlr_queue->head;

		while (node != NULL) {

			// invoke each event handler in handler queue
			if (node->evt_id == evt_id && node->evt_handler != NULL) {

				node->evt_handler(evt_msg, evt_msg_len, node->user_data);
			}

			node = node->next;

			if (search_cnt)
				search_cnt--;
			else {
				warp_dbg(WARP_DBG_OFF, "%s(%d), suspicious search! break forcedly\n", __func__, __LINE__);
				break;
			}
		}

		spin_unlock(&hdlr_queue->q_lock);
	} else {
		warp_dbg(WARP_DBG_OFF, "%s(%d), suspicious locked! dismissed!\n", __func__, __LINE__);
	}
}

void warp_evt_handle_event(
	u8 wed_idx, unsigned char *msg, u32 msg_len) {

	struct warp_cmd_header *hdr = (struct warp_cmd_header *)msg;

	int evt_msg_len = msg_len - sizeof(struct warp_cmd_header);
	unsigned char *evt_msg = (evt_msg_len > 0) ?
		msg + sizeof(struct warp_cmd_header) : NULL;

	struct warp_evt_hdlr_queue *hdlr_queue;

	hdlr_queue = get_warp_evt_hdlr_queue(MODULE_ID_WO, wed_idx);
	if (hdlr_queue == NULL) {
		warp_dbg(WARP_DBG_OFF, "%s(): warp evt hdlr_queue is NULL!\n", __func__);
		return;
	}

	if (hdlr_queue->state != MSG_Q_INITED) {

		warp_dbg(WARP_DBG_OFF, "%s(): queue state isn't inited\n", __func__);
		return;
	}

	warp_evt_notify_hdlr_node(hdlr_queue, evt_msg, evt_msg_len, hdr->cmd_id, hdr->uni_id);
}

int warp_evt_hdlr_queue_init(u8 wed_idx) {

	struct warp_evt_hdlr_queue *hdlr_queue = NULL;
	int to_id;

	warp_dbg(WARP_DBG_INF, "%s(), wed_idx: %d\n", __func__, wed_idx);

	for (to_id = 0; to_id < MAX_MODULE_ID; to_id ++){
		hdlr_queue = get_warp_evt_hdlr_queue(to_id, wed_idx);
		if (hdlr_queue == NULL) {
			warp_dbg(WARP_DBG_OFF, "%s(): warp evt hdlr_queue is NULL!\n", __func__);
			return WARP_FAIL_STATUS;
		}

		memset(hdlr_queue, 0, sizeof(struct warp_evt_hdlr_queue));
		spin_lock_init(&hdlr_queue->q_lock);
		hdlr_queue->state = MSG_Q_INITED;
	}

	return 0;
}


int warp_evt_hdlr_queue_deinit(u8 wed_idx) {

	struct warp_evt_hdlr_queue *hdlr_queue = NULL;
	int to_id;

	warp_dbg(WARP_DBG_INF, "%s(), wed_idx: %d\n", __func__, wed_idx);

	for (to_id = 0; to_id < MAX_MODULE_ID; to_id ++){
		hdlr_queue = get_warp_evt_hdlr_queue(to_id, wed_idx);
		if (hdlr_queue == NULL) {
			warp_dbg(WARP_DBG_OFF, "%s(): warp evt hdlr_queue is NULL!\n", __func__);
			return WARP_FAIL_STATUS;
		}

		// Remove each entity
		while (hdlr_queue->size > 0) {
			warp_evt_hdlr_node_deq_and_delete(hdlr_queue, hdlr_queue->head);
		}

		hdlr_queue->head = NULL;
		hdlr_queue->size = 0;
		hdlr_queue->state = MSG_Q_UNKNOWN;
	}
	warp_dbg(WARP_DBG_INF, "%s(), done\n", __func__);

	return 0;
}


int register_wo_event_handler (
	u8 wed_idx, u16 evt_id, EVT_MSG_HANDLER func, void *user_data) {

	struct warp_evt_hdlr_queue *hdlr_queue = NULL;
	struct warp_evt_hdlr_node *node;

	warp_dbg(WARP_DBG_INF, "%s(), wed_idx:%d, evt_id:%d\n",
		__func__, wed_idx, evt_id);

	hdlr_queue = get_warp_evt_hdlr_queue(MODULE_ID_WO, wed_idx);

	if (hdlr_queue == NULL)
		return WARP_FAIL_STATUS;

	if (hdlr_queue->state != MSG_Q_INITED) {

		warp_dbg(WARP_DBG_OFF, "%s(): queue state isn't inited\n", __func__);
		return WARP_FAIL_STATUS;
	}

	node = warp_evt_hdlr_node_init(evt_id, func, user_data);
	if (node == NULL) {
		warp_dbg(WARP_DBG_OFF, "%s(): warp_evt_hdlr_node_init fail\n", __func__);
		return WARP_FAIL_STATUS;
	}
	warp_evt_hdlr_node_enq(hdlr_queue, node);

	return WARP_OK_STATUS;
}
EXPORT_SYMBOL(register_wo_event_handler);



void unregister_wo_event_handler (
	u8 wed_idx, u16 evt_id, EVT_MSG_HANDLER func, void *user_data) {

	struct warp_evt_hdlr_queue *hdlr_queue = NULL;
	struct warp_evt_hdlr_node *node;

	warp_dbg(WARP_DBG_INF, "%s(), wed_idx:%d, evt_id: %d\n",
		__func__, wed_idx, evt_id);

	hdlr_queue = get_warp_evt_hdlr_queue(MODULE_ID_WO, wed_idx);
	if (hdlr_queue == NULL) {
		warp_dbg(WARP_DBG_OFF, "%s(): warp evt hdlr_queue is NULL!\n", __func__);
		return;
	}

	node = warp_evt_get_hdlr_node(hdlr_queue, evt_id, func, user_data);

	if (node != NULL) {
		warp_evt_hdlr_node_deq_and_delete(hdlr_queue, node);
	}

}
EXPORT_SYMBOL(unregister_wo_event_handler);


