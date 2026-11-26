/*
 * Copyright (c) [2020], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/

#ifndef _DIAG_H_
#define _DIAG_H_

#ifdef WIFI_DIAG
#include <linux/time.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include "rtmp_comm.h"
#include "rtmp_type.h"
#include "rtmp_os.h"
#include "rtmp.h"

#define DIAG_PROC_PATH					"ctcwifi"
#define DIAG_VAR_PATH					"/var/ctcwifi/"
#define DIAG_STATUS_FILE				"/var/ctcwifi/mt_wifi_diag"
#define DIAG_PROC_FILE_NUM				5
#define DIAG_VAR_FILE_NUM				2
#define DIAG_LOG_FILE_SIZE_MAX			(1024 * 1024) /* 1M bytes */
#define DIAG_PROCESS_NUM_MAX			(8)
#define DIAG_FRAME_SIZE_MAX				(2048)
#define DIAG_LOG_BUFF_SIZE				(200) /*(1000)*/
#define DIAG_ASSOC_ERROR_BUFF_SIZE		(100)
#define DIAG_WRITE_FILE_PER_SECCOND 50

enum _ENUM_DIAG_BAND {
	DIAG_BAND_2G = 0,
	DIAG_BAND_5G,
	DIAG_BAND_MAX
};

enum _ENUM_DIAG_CONN_ERROR_CODE {
	DIAG_CONN_FRAME_LOST = 0,
	DIAG_CONN_CAP_ERROR,
	DIAG_CONN_AUTH_FAIL,
	DIAG_CONN_ACL_BLK,
	DIAG_CONN_STA_LIM,
	DIAG_CONN_DEAUTH,
	DIAG_CONN_BAND_STE,
	DIAG_CONN_ERROR_MAX,
	DIAG_CONN_DEAUTH_COM
};

struct _DIAG_WIFI_PROCESS_ENTRY {
	UCHAR name[RTMP_OS_TASK_NAME_LEN];
	INT32 pid;
	UCHAR is_process;
};

struct _DIAG_WIFI_PROCESS_INFO {
	UCHAR num;
	struct _DIAG_WIFI_PROCESS_ENTRY entry[DIAG_PROCESS_NUM_MAX];
};

struct _DIAG_LOG_ENTRY {
	struct rtc_time tm;
	BOOLEAN isTX;
	UCHAR ssid[MAX_LEN_OF_SSID+1];
	UCHAR ssid_len;
	UCHAR band;
	UCHAR frame_type[32];
	UCHAR sta_addr[6];
	UCHAR data[DIAG_FRAME_SIZE_MAX];
	UINT32 data_len;
};

struct _DIAG_FRAME_INFO {
	UCHAR isTX;
	UCHAR *ssid;
	UCHAR ssid_len;
	UCHAR band; /* 0:2G, 1:5G, get from pAd->LatchRfRegs.Channel */
	UCHAR *pData; /* include 80211 header, refer to pRxBlk (RX)/ pData (TX) */
	UINT32 dataLen; /* packet length */
};

struct DIAG_ASSOC_ERROR_ENTRY {
	struct rtc_time tm;
	UCHAR StaAddr[6];
	UCHAR Ssid[32];
	enum _ENUM_DIAG_CONN_ERROR_CODE errCode;
	UINT32 reason;
};

struct _DIAG_CTRL {
	UINT8	init_flag;
	enum _ENUM_DIAG_BAND diag_band;
	UINT32	diag_duration;
	UINT32	diag_enable;
	ULONG	diag_enable_time;
	UINT32	busy_time;

	UINT32 diag_log_file_offset;
	struct _DIAG_LOG_ENTRY *diag_log_entry;
	UINT32 diag_log_read_idx;
	UINT32 diag_log_write_idx;
	NDIS_SPIN_LOCK diag_log_lock;

	UINT32 assoc_error_file_offset;
	struct DIAG_ASSOC_ERROR_ENTRY *assoc_error_entry;
	UINT32 assoc_error_read_idx;
	UINT32 assoc_error_write_idx;
	NDIS_SPIN_LOCK assoc_error_lock;

	struct _DIAG_WIFI_PROCESS_INFO process_info;
	PRTMP_ADAPTER pAd;
	UCHAR channel;
};
void diag_conn_error(PRTMP_ADAPTER pAd, UCHAR apidx, UCHAR* addr,
	enum _ENUM_DIAG_CONN_ERROR_CODE Code, UINT32 Reason);
void diag_conn_error_write(PRTMP_ADAPTER pAd);
void diag_add_pid(OS_TASK *pTask);
void diag_del_pid(OS_TASK *pTask);
void diag_get_process_info(PRTMP_ADAPTER	pAdapter, RTMP_IOCTL_INPUT_STRUCT	*wrq);
void diag_miniport_mm_request(PRTMP_ADAPTER pAd, UCHAR *pData, UINT Length);
void diag_bcn_tx(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, UCHAR *pBeaconFrame,ULONG FrameLen );
void diag_log_file_write(PRTMP_ADAPTER pAd);
void diag_dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk);
void diag_dev_rx_cntl_frm(RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk);
void diag_ap_mlme_one_sec_proc(PRTMP_ADAPTER pAd);
void diag_ctrl_alloc(PRTMP_ADAPTER pAd);
void diag_ctrl_free(PRTMP_ADAPTER pAd);
BOOLEAN diag_proc_init(PRTMP_ADAPTER pAd);
BOOLEAN diag_proc_exit(PRTMP_ADAPTER pAd);


#endif
#endif /* #ifndef _DIAG_H_ */

