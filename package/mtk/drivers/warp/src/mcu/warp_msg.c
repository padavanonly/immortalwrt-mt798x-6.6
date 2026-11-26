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
    warp_msg.c
*/
#include "warp.h"
#include "warp_wo.h"
#include "warp_msg.h"
#include "warp_meminfo_list.h"


char* thread_name[MAX_MODULE_ID] = {
    "warp",
    "wo"
};

struct warp_msg_ctrl *get_warp_msg_ctrl(u8 id, u8 wed_idx)
{
	struct warp_entry *entry = NULL;

	if (id >= MAX_MODULE_ID)
		return NULL;

	entry = warp_entry_get_by_idx(wed_idx);

	if (entry == NULL)
		return NULL;

	return &entry->msg_ctrl[id];
}

static int warp_msg_queue_init(struct msg_queue *msg_q)
{
	if (msg_q->state != MSG_Q_UNKNOWN)
		return -1;

	spin_lock_init(&msg_q->msg_lock);
	spin_lock(&msg_q->msg_lock);
	msg_q->head = NULL;
	msg_q->tail = NULL;
	msg_q->size = 0;
	msg_q->state = MSG_Q_INITED;
	spin_unlock(&msg_q->msg_lock);

	return 0;
}

static int warp_msg_queue_deinit(struct msg_queue *msg_q)
{
	spin_lock(&msg_q->msg_lock);
	msg_q->head = NULL;
	msg_q->tail = NULL;
	msg_q->size = 0;
	msg_q->state = MSG_Q_UNKNOWN;
	spin_unlock(&msg_q->msg_lock);

	return 0;
}


static struct msg_element *warp_msg_ins_init(unsigned char *msg, u32 msg_len)
{
	struct msg_element *msg_ins = NULL;

	warp_os_alloc_mem((unsigned char **)&msg_ins, sizeof(struct msg_element), GFP_ATOMIC);
	if(msg_ins == NULL) {
		return NULL;
	}
	memset(msg_ins, 0, sizeof(struct msg_element));

	warp_os_alloc_mem((unsigned char **)&msg_ins->msg, msg_len, GFP_ATOMIC);
	if(msg_ins->msg == NULL) {
		warp_os_free_mem(msg_ins);
		return NULL;
	}
	memset(msg_ins->msg, 0, msg_len);
	memcpy(msg_ins->msg, msg, msg_len);

	msg_ins->msg_len = msg_len;

	return msg_ins;
}

/*KO Mode*/
static void warp_msg_enq(struct msg_queue *msg_q, struct msg_element *msg)
{
	spin_lock(&msg_q->msg_lock);

	if (msg_q->size == 0)
		msg_q->head = msg;
	else
		msg_q->tail->next = msg;
	msg_q->tail = msg;
	msg->next = NULL;
	msg_q->size++;

	spin_unlock(&msg_q->msg_lock);
}

/*KO Mode*/
static void warp_msg_deq(struct msg_queue *msg_q, struct msg_element **msg)
{
	spin_lock(&msg_q->msg_lock);

	*msg = msg_q->head;

	if (*msg != NULL) {
		msg_q->head = msg_q->head->next;
		msg_q->size--;

		if (msg_q->size == 0)
			msg_q->tail = NULL;
	}
	spin_unlock(&msg_q->msg_lock);
}

#if 0
static void warp_hex_dump(char *str, unsigned char *buf, unsigned int buf_len,
			  int dbglvl)
{
	unsigned char *pt;
	unsigned int x;

	pt = buf;
	warp_dbg(dbglvl, "%s: %p, len = %d\n", str,  buf, buf_len);

	for (x = 0; x < buf_len; x++) {
		if (x % 16 == 0)
			warp_dbg(dbglvl, "0x%04x : ", x);

		warp_dbg(dbglvl, "%02x ", (unsigned char)pt[x]);

		if (x % 16 == 15)
			warp_dbg(dbglvl, "\n");
	}

	warp_dbg(dbglvl, "\n");
}
#endif

static int warp_msg_receive(u8 id, u8 wed_idx, struct msg_element **msg_ret)
{
	struct warp_msg_ctrl *ctrl = NULL;
	struct msg_queue *msg_q = NULL;
	struct msg_element *msg_ins = NULL;

	ctrl = get_warp_msg_ctrl(id, wed_idx);

	if (ctrl == NULL)
		return -1;

	warp_dbg(WARP_DBG_LOU, "%s(),wait,%s,%ld,%d\n", __func__,  thread_name[id], (long)ctrl->kthread_task->pid, ctrl->thread_running);

	wait_event_interruptible(ctrl->kthread_q, ctrl->thread_running ||
				 kthread_should_stop());

	warp_dbg(WARP_DBG_OFF, "%s(),go,%s,%ld,%d\n", __func__,  thread_name[id], (long)ctrl->kthread_task->pid, ctrl->thread_running);

	if (kthread_should_stop())
		return -1;

	msg_q = &ctrl->msg_q;

	if (msg_q->state == MSG_Q_UNKNOWN)
		return -1;

	warp_dbg(WARP_DBG_LOU, "%s(),deq,%s,%d\n", __func__,  thread_name[id], msg_q->size);

	warp_msg_deq(msg_q, &msg_ins);

	if (msg_ins == NULL) {
		warp_dbg(WARP_DBG_ERR, "%s(),msg_ins is null,%s,%d\n", __func__,  thread_name[id], msg_q->size);
		return -1;
	}

	warp_dbg(WARP_DBG_LOU, "%s(),deq,%s,%d\n", __func__,  thread_name[id], msg_q->size);

	if ( msg_q->size == 0) {
		ctrl->thread_running = 0;
	} else {
		warp_dbg(WARP_DBG_LOU, "%s(),%s,ser next msg\n", __func__,  thread_name[id]);
	}
	*msg_ret = msg_ins;

	return 0;
}


static int warp_msg_process(void *data)
{
	struct warp_msg_ctrl *ctrl = (struct warp_msg_ctrl *) data;
	int ret = 0;
	struct msg_element *msg_ins = NULL;

	while (1) {
		ret = warp_msg_receive(ctrl->module_id, ctrl->wed_idx, &msg_ins);
		if (ret != 0) {
			warp_dbg(WARP_DBG_OFF,
				 "%s(): module id(%d), wed_idx(%d), something wrong(ret = %d)\n",
				 __func__, ctrl->module_id, ctrl->wed_idx, ret);
			break;
		}

		/* warp_hex_dump("msg", msg, msg_len, WARP_DBG_OFF); */
		if (msg_ins == NULL) {
			warp_dbg(WARP_DBG_OFF, "%s(): msg_ins is null\n", __func__);
			continue;
		}

		ret = ctrl->cmd_event_process(ctrl->wed_idx, msg_ins->msg, msg_ins->msg_len);
		warp_dbg(WARP_DBG_INF,
			 "%s(): cmd_event_process done(ret = %d), id = %d, wed_idx = %d\n",
			 __func__, ret, ctrl->module_id, ctrl->wed_idx);

		warp_os_free_mem(msg_ins->msg);
		warp_os_free_mem(msg_ins);
	}
	return 0;
}


int warp_msg_send_by_msg_q(u8 to_id, u8 wed_idx, unsigned char *msg, u32 msg_len)
{
	struct warp_msg_ctrl *ctrl = NULL;
	struct msg_queue *msg_q = NULL;
	struct msg_element *msg_ins = NULL;

	warp_dbg(WARP_DBG_INF, "%s(): seng msg to %s, msg_len = %d\n", __func__,
		 thread_name[to_id], msg_len);

	ctrl = get_warp_msg_ctrl(to_id, wed_idx);

	if (ctrl == NULL) {
		warp_dbg(WARP_DBG_INF, "%s(): ctrl is null, to_id:%d, wed_idx:%d\n", __func__, to_id, wed_idx);
		return -1;
	}

	warp_dbg(WARP_DBG_OFF, "%s(),to,%s,%ld\n", __func__, thread_name[to_id], (long)ctrl->kthread_task->pid);

	msg_q = &ctrl->msg_q;

	if (msg_q->state == MSG_Q_UNKNOWN)
		return -1;

	msg_ins = warp_msg_ins_init(msg, msg_len);
	if (msg_ins == NULL) {
		warp_dbg(WARP_DBG_INF, "%s(): warp_msg_ins_init fail to_id:%d, wed_idx:%d\n", __func__, to_id, wed_idx);
		return -1;
	}

	warp_dbg(WARP_DBG_LOU, "%s(),enq,%s,%d\n", __func__, thread_name[to_id], msg_q->size);

	warp_msg_enq(msg_q, msg_ins);

	warp_dbg(WARP_DBG_LOU, "%s(),enq,%s,%d\n", __func__, thread_name[to_id], msg_q->size);

	if (ctrl->thread_running == 0) {
		ctrl->thread_running = 1;
		warp_dbg(WARP_DBG_LOU, "%s(),wake\n", __func__);
		wake_up(&ctrl->kthread_q);
	} else {
		warp_dbg(WARP_DBG_INF, "%s(),awake\n", __func__);
	}

	return 0;
}
EXPORT_SYMBOL(warp_msg_send_by_msg_q);


/* mcu mode */
static int warp_msg_send_by_ccif(u8 wed_idx, unsigned char *msg, u32 msg_len)
{
	warp_dbg(WARP_DBG_INF, "%s(): seng msg to %s, msg_len = %d\n", __func__,
		 thread_name[MODULE_ID_WO], msg_len);

	if (woif_bus_tx(wed_idx, msg, msg_len) < 0) {
		warp_dbg(WARP_DBG_LOU, "%s() fail!\n", __func__);
		return -1;
	}
	return 0;
}
static bool check_if_warp_msg_need_rsp(struct warp_msg_cmd *msg_cmd) {

	return (msg_cmd->param.rsp_hdlr != NULL ||
		msg_cmd->param.wait_type != WARP_MSG_WAIT_TYPE_NONE);
}

static bool check_if_warp_msg_need_wait(enum warp_msg_wait_type wait_type) {

	return (wait_type != WARP_MSG_WAIT_TYPE_NONE);
}


static struct warp_msg_wait_queue *get_warp_msg_wait_queue(u8 to_id, u8 wed_idx)
{
	struct warp_msg_ctrl *ctrl = get_warp_msg_ctrl(to_id, wed_idx);

	if (ctrl == NULL)
		return NULL;

	return &ctrl->msg_wait_q;
}

static struct warp_msg_wait_node *warp_msg_wait_node_init(struct warp_msg_cmd *msg_cmd)
{
	struct warp_msg_wait_node *node = NULL;

	warp_os_alloc_mem((unsigned char **)&node, sizeof(struct warp_msg_wait_node), GFP_ATOMIC);

	if (node != NULL) {
		memset(node, 0, sizeof(struct warp_msg_wait_node));
		node->cmd_id = (u8)msg_cmd->param.cmd_id;
		node->wait_type = msg_cmd->param.wait_type;
		node->rsp_hdlr = msg_cmd->param.rsp_hdlr;
		node->user_data = msg_cmd->param.user_data;
		init_completion(&node->recv_done);
	} else {
		warp_dbg(WARP_DBG_ERR, "%s(), node is null !!\n", __func__);
	}

	return node;
}

static int warp_msg_wait_node_deinit(struct warp_msg_wait_node *node)
{
	int ret = -1;

	if (node != NULL) {
		memset(node, 0, sizeof(struct warp_msg_wait_node));

		warp_os_free_mem(node);

		ret = 0;
	} else {
		warp_dbg(WARP_DBG_ERR, "%s(), node is invalid !!\n", __func__);
	}

	return ret;
}


static int warp_msg_wait_node_enq(
	struct warp_msg_wait_queue *msg_q, struct warp_msg_wait_node *node)
{
	struct warp_msg_wait_node *last;

	warp_dbg(WARP_DBG_LOU, "%s(), cmd_id:%d, node:%p\n", __func__,
			node->cmd_id, node);

	spin_lock_bh(&msg_q->wait_lock);

	if (msg_q->head == NULL) {
		msg_q->head = node;
	} else {
		last = msg_q->head;
		while (last->next) {
			last = last->next;
		}
		last->next = node;
	}

	node->next = NULL;
	node->uni_id = (++msg_q->current_uni_id);
	msg_q->size++;

	spin_unlock_bh(&msg_q->wait_lock);

	warp_dbg(WARP_DBG_LOU, "%s(), cmd_id:%d, uni_id:%d, size:%d\n", __func__,
			node->cmd_id, node->uni_id, msg_q->size);

	return node->uni_id;
}


static void warp_msg_wait_node_deq(
    struct warp_msg_wait_queue *msg_q, struct warp_msg_wait_node *node)
{
	struct warp_msg_wait_node *cur_node, *prev_node = NULL;
	u32 seach_cnt = 0;

	warp_dbg(WARP_DBG_LOU, "%s(), cmd_id:%d, uni_id:%d, size:%d\n", __func__,
		node->cmd_id, node->uni_id, msg_q->size);

	cur_node = msg_q->head;
	seach_cnt = msg_q->size;

	while (cur_node != NULL && cur_node != node) {
		prev_node = cur_node;
		cur_node = cur_node->next;

		if (seach_cnt)
			seach_cnt--;
		else {
			warp_dbg(WARP_DBG_OFF, "%s(%d), surspicious search! break forcedly! last_node:%p\n",
					 __func__, __LINE__, cur_node);
			cur_node = NULL;
			break;
		}
	}

	if (cur_node == NULL) {
		warp_dbg(WARP_DBG_OFF, "%s(), cmd_id:%d, node:%p NOT found! msg_q->size:%d\n", __func__,
				node->cmd_id, node, msg_q->size);
		return;
	} else if (cur_node == msg_q->head) {
		msg_q->head = cur_node->next;
	} else if (prev_node != NULL) {
		prev_node->next = cur_node->next;
	}

	msg_q->size--;

	warp_dbg(WARP_DBG_LOU, "%s(), cmd_id:%d, node:%p found! msg_q->size:%d\n", __func__,
			node->cmd_id, node, msg_q->size);

}

static struct warp_msg_wait_node *warp_msg_get_wait_node_by_uni_id(
    struct warp_msg_wait_queue *msg_q, u16 uni_id)
{
	struct warp_msg_wait_node *node = NULL;
	u32 q_len = 0;

	spin_lock_bh(&msg_q->wait_lock);

	warp_dbg(WARP_DBG_LOU, "%s(), searching uni id:%d, msg_q->size:%d\n", __func__,
				uni_id, msg_q->size);

	node = msg_q->head;
	q_len = msg_q->size;

	while (node != NULL && node->uni_id != uni_id) {
		node = node->next;
		if (q_len)
			q_len--;
		else {
			warp_dbg(WARP_DBG_OFF, "%s(%d), surspicious search! break forcedly! last_node:%p\n",
					 __func__, __LINE__, node);
			node = NULL;
			break;
		}
	}

	if (node) {
		warp_dbg(WARP_DBG_LOU, "%s(), uni id:%d, node:%p found!msg_q->size:%d\n", __func__,
					uni_id, node, msg_q->size);

		warp_msg_wait_node_deq(msg_q, node);
	}

	spin_unlock_bh(&msg_q->wait_lock);

	return node;
}

static int warp_msg_send_cmd_with_header(
	u8 wed_idx, struct warp_msg_cmd *msg_cmd, u16 uni_id)
{

	u16 header_len = sizeof(struct warp_cmd_header);
	u16 sendmsg_len = header_len + msg_cmd->msg_len;
	int result;
	u8 *sendmsg_buf;
	struct warp_cmd_header *cmd_header;

	result = warp_os_alloc_mem(&sendmsg_buf, WARP_MSG_MAX_BUFFER_LEN, GFP_ATOMIC);
	if (result)
		return WARP_FAIL_STATUS;

	cmd_header = (struct warp_cmd_header *)sendmsg_buf;

	warp_dbg(WARP_DBG_INF, "%s(), uni_id:%d, msg_len:%d, header_len:%d, sendmsg_buf:%d\n",
		__func__, uni_id, msg_cmd->msg_len, header_len, (u32)sizeof(sendmsg_buf));

	if (sendmsg_len > WARP_MSG_MAX_BUFFER_LEN) {
		warp_dbg(WARP_DBG_ERR, "%s(), invaid length, msg_len:%d, header_len:%d, sendmsg_buf:%d\n",
			__func__, msg_cmd->msg_len, header_len, (u32)sizeof(sendmsg_buf));
		result = WARP_INVALID_LENGTH_STATUS;
		goto end;
	}

	/* header */
	memset(cmd_header, 0, header_len);
	cmd_header->cmd_id = msg_cmd->param.cmd_id;
	cmd_header->length = sendmsg_len;
	cmd_header->uni_id = uni_id;

	if (check_if_warp_msg_need_rsp(msg_cmd)) {
		WARP_CMD_FLAG_SET_NEED_RSP(cmd_header);
	}

	/* sendmsg_buf */
	if (msg_cmd->msg_len > 0) {
		memcpy(sendmsg_buf + header_len, msg_cmd->msg, msg_cmd->msg_len);
	}

	switch (msg_cmd->param.to_id) {

		case MODULE_ID_WARP:
			result = warp_msg_send_by_msg_q(
				MODULE_ID_WARP, wed_idx, sendmsg_buf, sendmsg_len);
			break;

		case MODULE_ID_WO:

			WARP_CMD_FLAG_SET_FROM_TO_WO(cmd_header);

		#if (WED_HWRRO_MODE == WED_HWRRO_MODE_WOCPU)

			result = warp_msg_send_by_ccif(wed_idx, sendmsg_buf, sendmsg_len);

		#else  /* WED_HWRRO_MODE_WOCPU */

			// KO mode
			result = warp_msg_send_by_msg_q(
				MODULE_ID_WO, wed_idx, sendmsg_buf, sendmsg_len);

		#endif /* WED_HWRRO_MODE_WOCPU */
			break;

		default:

			result = WARP_INVALID_PARA_STATUS;
			break;
	}
end:
	warp_os_free_mem(sendmsg_buf);
	return result;
}

static int warp_msg_wait_rsp(
	struct warp_msg_cmd *msg_cmd, struct warp_msg_wait_node *node) {

	int timeout, ret = WARP_OK_STATUS;

	if (node != NULL && check_if_warp_msg_need_wait(msg_cmd->param.wait_type)) {

		// wait
		timeout = (msg_cmd->param.timeout > 0) ?
			msg_cmd->param.timeout : WARP_MSG_TIMEOUT_DEFAULT;

		if (wait_for_completion_timeout(
			&node->recv_done, msecs_to_jiffies(timeout)) == 0) {

			warp_dbg(WARP_DBG_ERR, "%s(), uni_id:%d(cmd_id:%d), timeout\n", __func__, node->uni_id, node->cmd_id);
			return -1;
		}

		warp_dbg(WARP_DBG_INF, "%s(), rsp_status:%d\n", __func__, node->rsp_status);

		// handle output
		ret = node->rsp_status;

		if (msg_cmd->param.wait_type == WARP_MSG_WAIT_TYPE_RSP_MSG) {

			if (msg_cmd->rsp_msg_len == node->rsp_msg_len &&
				msg_cmd->rsp_msg != NULL && node->rsp_msg != NULL) {

				memcpy(msg_cmd->rsp_msg, node->rsp_msg, msg_cmd->rsp_msg_len);

			} else {
				warp_dbg(WARP_DBG_ERR, "%s(): msg_cmd->rsp_msg_len:%d, entity->rsp_msg_len:%d\n",
				__func__, msg_cmd->rsp_msg_len, node->rsp_msg_len);
				ret = WARP_INVALID_LENGTH_STATUS;
			}
		}
	}

	return ret;
}


void warp_msg_handle_rsp(u8 wed_idx, u8 from_id, unsigned char *msg, u32 msg_len) {


	struct warp_msg_wait_queue *wait_queue;
	struct warp_msg_wait_node *node;
	struct warp_cmd_header *hdr = (struct warp_cmd_header *)msg;

	int rsp_msg_len = msg_len - sizeof(struct warp_cmd_header);
	unsigned char *rsp_msg = (rsp_msg_len > 0) ?
		msg + sizeof(struct warp_cmd_header) : NULL;

	wait_queue = get_warp_msg_wait_queue(from_id, wed_idx);
	if (wait_queue == NULL || wait_queue->state != MSG_Q_INITED) {
		warp_dbg(WARP_DBG_ERR, "%s(): queue state isn't inited\n", __func__);
		return;
	}

	node = warp_msg_get_wait_node_by_uni_id(wait_queue, hdr->uni_id);

	if (node == NULL) {
		warp_dbg(WARP_DBG_ERR, "%s(): uni_id:%d, node is not found\n", __func__, hdr->uni_id);
		return;
	}

	// call response handler
	if (node->rsp_hdlr) {

		node->rsp_hdlr(rsp_msg, rsp_msg_len, node->user_data);
	}

	if (check_if_warp_msg_need_wait(node->wait_type)) {

		node->rsp_status = hdr->status;

		if (node->wait_type == WARP_MSG_WAIT_TYPE_RSP_MSG) {

			if (rsp_msg_len == node->rsp_msg_len && node->rsp_msg != NULL) {

				memcpy(node->rsp_msg, rsp_msg, rsp_msg_len);

			} else {
				warp_dbg(WARP_DBG_ERR, "%s(): rsp_msg_len:%d, entity->rsp_msg_len:%d\n",
					__func__, rsp_msg_len, node->rsp_msg_len);

				node->rsp_status = WARP_INVALID_LENGTH_STATUS;
			}
		}

		complete(&node->recv_done);

	} else {

		// de-queue entity
		warp_msg_get_wait_node_by_uni_id(wait_queue, node->uni_id);
	}

}

static void warp_msg_wait_queue_init(u8 wed_idx) {

	struct warp_msg_wait_queue *wait_queue = NULL;
	int to_id;

	warp_dbg(WARP_DBG_INF, "%s(), wed_idx: %d\n", __func__, wed_idx);

	for (to_id = 0; to_id < MAX_MODULE_ID; to_id ++){
		wait_queue = get_warp_msg_wait_queue(to_id, wed_idx);

		if (wait_queue == NULL) {
			warp_dbg(WARP_DBG_ERR, "%s(): wait_queue is NULL, return.\n", __func__);
			return;
		}

		memset(wait_queue, 0, sizeof(struct warp_msg_wait_queue));
		spin_lock_init(&wait_queue->wait_lock);
		wait_queue->state = MSG_Q_INITED;
	}
}


static void warp_msg_wait_queue_deinit(u8 wed_idx) {

	struct warp_msg_wait_queue *wait_queue = NULL;
	int to_id;

	warp_dbg(WARP_DBG_INF, "%s(), wed_idx: %d\n", __func__, wed_idx);

	for (to_id = 0; to_id < MAX_MODULE_ID; to_id ++){
		wait_queue = get_warp_msg_wait_queue(to_id, wed_idx);

		if (wait_queue == NULL) {
			warp_dbg(WARP_DBG_ERR, "%s(), wait_queue is NULL\n", __func__);
		} else {
			// Remove each entity
			while (wait_queue->size > 0)
				warp_msg_wait_node_deq(wait_queue, wait_queue->head);

			wait_queue->head = NULL;
			wait_queue->size = 0;
			wait_queue->current_uni_id = 0;
			wait_queue->state = MSG_Q_UNKNOWN;
		}
	}
}

int warp_msg_register(u8 id, u8 wed_idx, void *cmd_event_process)
{
	struct warp_msg_ctrl *ctrl = NULL;

	ctrl = get_warp_msg_ctrl(id, wed_idx);

	if (ctrl == NULL)
		return -1;

	if (warp_msg_queue_init(&ctrl->msg_q) != 0) {
		warp_dbg(WARP_DBG_OFF, "%s(): msg_q is inited\n", __func__);
		return -1;
	}

	ctrl->thread_running = 0;
	ctrl->module_id = id;
	ctrl->wed_idx = wed_idx;
	init_waitqueue_head(&ctrl->kthread_q);
	ctrl->cmd_event_process = cmd_event_process;
	ctrl->kthread_task = kthread_run(warp_msg_process, (void *)ctrl, thread_name[id]);

	warp_dbg(WARP_DBG_OFF, "%s():%s,pid,%ld,id,%d,wed_idx,%d\n", __func__,thread_name[id], (long)ctrl->kthread_task->pid, id,
		wed_idx);

	return 0;
}
EXPORT_SYMBOL(warp_msg_register);

int warp_msg_unregister(u8 id, u8 wed_idx)
{
	struct warp_msg_ctrl *ctrl = NULL;

	ctrl = get_warp_msg_ctrl(id, wed_idx);

	if (ctrl == NULL)
		return -1;

	kthread_stop(ctrl->kthread_task);

	warp_msg_queue_deinit(&ctrl->msg_q);

	ctrl->thread_running = 0;
	ctrl->cmd_event_process = NULL;

	warp_dbg(WARP_DBG_OFF, "%s():done\n", __func__);

	return 0;
}
EXPORT_SYMBOL(warp_msg_unregister);



void warp_msg_init(u8 wed_idx) {

#ifdef WED_RX_D_SUPPORT
	warp_msg_register(MODULE_ID_WARP, wed_idx, warp_cmd_process_msg);
#endif
	warp_msg_wait_queue_init(wed_idx);

	warp_evt_hdlr_queue_init(wed_idx);

}
EXPORT_SYMBOL(warp_msg_init);

void warp_msg_deinit(u8 wed_idx) {

	warp_msg_unregister(MODULE_ID_WARP, wed_idx);

	warp_msg_wait_queue_deinit(wed_idx);

	warp_evt_hdlr_queue_deinit(wed_idx);

}
EXPORT_SYMBOL(warp_msg_deinit);


int warp_msg_send_cmd(u8 wed_idx, struct warp_msg_cmd *msg_cmd)
{
	struct warp_msg_wait_queue *wait_queue = NULL;
	struct warp_msg_wait_node *node = NULL;
	int ret = WARP_OK_STATUS;

	u16 uni_id = 0;

	warp_dbg(WARP_DBG_INF, "%s(), wed_idx:%d, to: %d, cmd_id: %d, wait_type: %d, timeout: %d, msg_len: %d, rsp_msg_len: %d, rsp_hdlr: %s\n",
		__func__,
		wed_idx,
		msg_cmd->param.to_id,
		msg_cmd->param.cmd_id,
		msg_cmd->param.wait_type,
		msg_cmd->param.timeout,
		msg_cmd->msg_len,
		msg_cmd->rsp_msg_len,
		msg_cmd->param.rsp_hdlr == NULL ? "null" : "not null");

	wait_queue = get_warp_msg_wait_queue(msg_cmd->param.to_id, wed_idx);

	if (wait_queue == NULL || wait_queue->state != MSG_Q_INITED) {

		warp_dbg(WARP_DBG_OFF, "%s(): queue state isn't inited\n", __func__);
		return WARP_FAIL_STATUS;
	}

	// before-send: add to entity queue
	if (check_if_warp_msg_need_rsp(msg_cmd)) {

		node = warp_msg_wait_node_init(msg_cmd);
		if (!node) {
			ret = WARP_NULL_POINTER_STATUS;
			goto end;
		}
		uni_id = warp_msg_wait_node_enq(wait_queue, node);
	}

	ret = warp_msg_send_cmd_with_header(wed_idx, msg_cmd, uni_id);

	if (ret != WARP_OK_STATUS) {
		warp_dbg(WARP_DBG_OFF, "%s(): cmd_id:%d send failed!\n",
				 __func__, msg_cmd->param.cmd_id);

		if (uni_id) {
			warp_msg_get_wait_node_by_uni_id(wait_queue, uni_id);
			warp_msg_wait_node_deinit(node);
		}
		goto end;
	}

	if (uni_id) {
		// wait and get response.
		ret = warp_msg_wait_rsp(msg_cmd, node);

		if (ret < 0) {//cmd timeout
			warp_msg_get_wait_node_by_uni_id(wait_queue, uni_id);
		}

		warp_msg_wait_node_deinit(node);
	}

end:

	return ret;

}
EXPORT_SYMBOL(warp_msg_send_cmd);


