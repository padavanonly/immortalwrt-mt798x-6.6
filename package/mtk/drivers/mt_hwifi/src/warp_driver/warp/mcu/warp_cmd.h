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
	warp_cmd.h
*/

#ifndef _WARP_CMD_H_
#define _WARP_CMD_H_

#include "warp_utility.h"

struct warp_cmd_header {
	/*DW0*/
	u8 ver;
	u8 cmd_id;
	u16 length;

	/*DW1*/
	u16 uni_id;
	u16 flag;

	/*DW2*/
	int status;

	/*DW3*/
	u8 reserved[20];
};

enum {
	WARP_CMD_FLAG_RSP           = 1 << 0, /* is responce*/
	WARP_CMD_FLAG_NEED_RSP      = 1 << 1, /* need responce */
	WARP_CMD_FLAG_FROM_TO_WO    = 1 << 2, /* send between host and wo */
};

#define WARP_CMD_FLAG_IS_RSP(_hdr) ((_hdr)->flag & (WARP_CMD_FLAG_RSP))
#define WARP_CMD_FLAG_SET_RSP(_hdr) ((_hdr)->flag |= (WARP_CMD_FLAG_RSP))
#define WARP_CMD_FLAG_IS_NEED_RSP(_hdr) ((_hdr)->flag & (WARP_CMD_FLAG_NEED_RSP))
#define WARP_CMD_FLAG_SET_NEED_RSP(_hdr) ((_hdr)->flag |= (WARP_CMD_FLAG_NEED_RSP))
#define WARP_CMD_FLAG_IS_FROM_TO_WO(_hdr) ((_hdr)->flag & (WARP_CMD_FLAG_FROM_TO_WO))
#define WARP_CMD_FLAG_SET_FROM_TO_WO(_hdr) ((_hdr)->flag |= (WARP_CMD_FLAG_FROM_TO_WO))


/* Support forwarding cmds from wifi driver*/

typedef int(*pf_wificmd_handle)(u8 wed_idx, u8 *msg, u32 msg_len);

struct _wifi_cmd_t {
	u32 u4cmd_id;
	pf_wificmd_handle phandle;
};

enum {
	WARP_STA_REC_BASIC = 0,
	WARP_STA_REC_RA,
	WARP_STA_REC_RA_COMMON_INFO,
	WARP_STA_REC_RA_UPDATE,
	WARP_STA_REC_BF,
	WARP_STA_REC_AMSDU,
	WARP_STA_REC_BA,
	WARP_STA_REC_RED,
	WARP_STA_REC_TXPROC,
	WARP_STA_REC_MAX_NUM,
};

/* align with wo report structure */
struct latency_record_t {
	u32 sn;
	u32 total;
	u32 rro;
	u32 mod;
};

struct wo_cmd_rxcnt_t {
	u16 wlan_idx;
	u16 tid;
	u32 rx_pkt_cnt;
	u32 rx_byte_cnt;
	u32 rx_err_cnt;
	u32 rx_drop_cnt;
};

int warp_cmd_process_msg(u8 wed_idx, unsigned char *msg, u32 msg_len);
int warp_cmd_process_wo_ring(u8 wed_idx, unsigned char *msg, u32 msg_len);
int warp_cmd_send_wocmd(u8 wed_idx, u8 *cmd_hdr, u8 *cmd_param, u16 cmd_len);

int warp_setcmd_setup(u8 wed_idx, u8 *para, u32 para_len, u16 sub_cmd_id);
int warp_evt_release_timer(u8 wed_idx, u8 *para, u32 para_len);
int warp_evt_rxstat_handle(u8 wed_idx, u8 *para, u32 para_len);
int warp_evt_rroirq_handle(u8 wed_idx, u8 *para, u32 para_len);


#endif /*_WARP_CMD_H_*/
