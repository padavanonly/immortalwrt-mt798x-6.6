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
/*
 ***************************************************************************
 ***************************************************************************

*/

#ifndef __DBG_CTRL_H__
#define __DBG_CTRL_H__

#define DEFAULT_FW_LOG_DESTINATION "/media/sda1/fw_log.bin"

#define FW_BIN_LOG_MAGIC_NUM    0x44D9C99A
#define FW_BIN_LOG_VERSION      0x01
#define FW_BIN_LOG_RSV          0x00
#ifdef DBG
#ifdef DBG_ENHANCE
#define DBG_PRINT_BUF_SIZE	280
#endif /* DBG_ENHANCE */
#endif /* DBG */

enum {
	FW_LOG_2_HOST_CTRL_OFF = 0,
	FW_LOG_2_HOST_CTRL_2_UART = 1,
	FW_LOG_2_HOST_CTRL_2_HOST = 2,
	FW_LOG_2_HOST_CTRL_2_EMI = 4,
	FW_LOG_2_HOST_CTRL_2_HOST_STORAGE = 8,
	FW_LOG_2_HOST_CTRL_2_HOST_ETHNET = 16,
	FW_LOG_2_HOST_CTRL_2_HOST_CONNSYS = 32,
	FW_LOG_2_HOST_CTRL_2_HOST_MEMORY = 64,
};

typedef enum _ENUM_DGB_LOG_PKT_TYPE_T {
	DBG_LOG_PKT_TYPE_ICS = 0x0C,
	DBG_LOG_PKT_TYPE_PHY_ICS = 0x0D,
	DBG_LOG_PKT_TYPE_TXV = 0x11,
	DBG_LOG_PKT_TYPE_FTRACE = 0x12,
	DBG_LOG_PKT_TYPE_TRIG_FRAME = 0x13,
	DBG_LOG_PKT_TYPE_MCU_TEXT_MSG = 0x100,
	DBG_LOG_PKT_TYPE_HOST_TEXT_MSG = 0x101,
} ENUM_HW_LOG_PKT_TYPE_T;

typedef enum _ENUM_BIN_DBG_LOG_T {
	BIN_DBG_LOG_TRIGGER_FRAME = 0,
	BIN_DBG_LOG_NUM
} ENUM_BIN_DBG_LOG_T;

#ifdef FW_LOG_DUMP
#define SUPPORTED_FW_LOG_TYPE	0x7F
#define FW_LOG_TYPE_COUNT		7
#else
#define SUPPORTED_FW_LOG_TYPE	0x03
#define FW_LOG_TYPE_COUNT		2
#endif /* FW_LOG_DUMP */

typedef struct _ICS_AGG_HEADER {
	UINT16 rxByteCount;
	UINT16 frameCount:5;
	UINT16 reserved1:6;
	UINT16 pktType:5;
	UINT16 reserved2;
	UINT16 pseFid;
} ICS_AGG_HEADER, *PICS_AGG_HEADER;

typedef struct _FW_LOG_CTRL {
	UINT32 fw_log_server_ip;
	UCHAR fw_log_server_mac[MAC_ADDR_LEN];
	UCHAR wmcpu_log_type;
	CHAR fw_log_dest_dir[32];
	UCHAR debug_level_ctrl[BIN_DBG_LOG_NUM];
	UINT16 fw_log_serialID_count;
	RTMP_STRING fw_log_arg[10];
} FW_LOG_CTRL, *PFW_LOG_CTRL;

struct FW_IDX_LOG_CTRL {
	UINT32 round;
	UINT32 alloc_size;
	UINT32 written_size;
	UCHAR *dump_buffer;
};

typedef struct _FW_BIN_LOG_HDR_T {
	UINT32 u4MagicNum;
	UINT8  u1Version;
	UINT8  u1Rsv;
	UINT16 u2SerialID;
	UINT32 u4Timestamp;
	UINT16 u2MsgID;
	UINT16 u2Length;
} FW_BIN_LOG_HDR_T, *P_FW_BIN_LOG_HDR_T;

INT set_fw_log_dest_dir(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

INT set_binary_log(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

NTSTATUS fw_log_open_close_file(
	IN PRTMP_ADAPTER pAd,
	IN PCmdQElmt CMDQelmt);

NTSTATUS fw_log_write_file(
	IN PRTMP_ADAPTER pAd,
	IN PCmdQElmt CMDQelmt);

NTSTATUS fw_log_alloc_memory(
	IN PRTMP_ADAPTER pAd,
	IN PCmdQElmt CMDQelmt);

NTSTATUS fw_log_write_memory(
	IN PRTMP_ADAPTER pAd,
	IN PCmdQElmt CMDQelmt);

VOID fw_log_to_ethernet(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *fw_log,
	IN UINT32 log_len);

UINT16 Checksum16(UINT8 *pData, int len);

NTSTATUS
dbg_log_wrapper(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 ucPktType,
	IN UINT8 *pucData,
	IN UINT16 u2Length);

NTSTATUS
host_log_to_parser(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pucData,
	IN UINT16 u2Length);

void host_dbg_prt(
	IN RTMP_ADAPTER	*pAd,
	IN const INT8 *pFmt,
	...);

void host_dbg_hexdump(
	IN struct _RTMP_ADAPTER *pAd,
	char *str,
	unsigned char *va,
	unsigned int size);

INT32 set_fwlog_serverip(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
INT32 set_fwlog_servermac(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
VOID print_fw_log_header(FW_BIN_LOG_HDR_T log_hdr);

#ifdef DBG
#ifdef DBG_ENHANCE
void mtwf_dbg_option(
	IN const BOOLEAN prtCatLvl,
	IN const BOOLEAN prtIntfName,
	IN const BOOLEAN prtThreadId,
	IN const BOOLEAN prtFuncLine);

void mtwf_dbg_prt(
	IN RTMP_ADAPTER *pAd,
	IN const UINT32 dbgCat,
	IN const UINT32 dbgLvl,
	IN const INT8   *pFunc,
	IN const UINT32 line,
	IN const INT8   *pFmt,
	...);
#endif /* DBG_ENHANCE */
#endif /* DBG */
#endif /* __DBG_CTRL_H__ */

