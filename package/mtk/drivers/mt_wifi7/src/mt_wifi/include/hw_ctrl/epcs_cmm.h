/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2023, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************
 */

#ifndef __EPCS_CMM_H__
#define __EPCS_CMM_H__

enum EPCS_CMD_T {
	EPCS_ENABLE_STA = 1,
	/*EPCS_SET_PARAM_ENTRY = 2,*/
	/*EPCS_SET_STA_PARAM_IDX = 3,*/
	/*EPCS_QUERY_PARAM_ENTRY = 4,*/
	EPCS_QUERY_STA = 5,
	/*EPCS_RESET_ENTRY = 6*/
};

enum ENUM_EPCS_ENTRY_STATUS_T {
	EPCS_ENTRY_INVALID = 0,
	EPCS_ENTRY_VALID = BIT(0)
};

struct GNU_PACKED EPCS_EDCA_PARAM_T {
	UINT_16		u2TxopLimit;
	UINT_8		ucCWmin;
	UINT_8		ucCWmax;
	UINT_8		ucAifsn;
	UINT_8		aucReserved[3];
	UINT_8		ucMuEdcaCWmin;
	UINT_8		ucMuEdcaCWmax;
	UINT_8		ucMuEdcaAifsn;
	UINT_8		ucMuEdcaTimer;
};

struct GNU_PACKED EPCS_EDCA_ENTRY_T {
	struct EPCS_EDCA_PARAM_T EdcaParam[4];
	UINT_16 u2StatusFlag;
};

struct GNU_PACKED EPCS_CMD_ENABLE_STA_T {
	UINT_16		u2WlanIdx;
	UINT_16		u2Idx;
	BOOLEAN		fgEnable;
	struct EPCS_EDCA_ENTRY_T    EdcaSetting;
};

struct GNU_PACKED EPCS_CMD_QUERY_STA_T {
	UINT_16		u2WlanIdx;
};

struct GNU_PACKED EPCS_EVENT_RESULT_T {
	UINT_32              u4Result;
};

struct GNU_PACKED EPCS_EVENT_QUERY_STA_T {
	UINT_16              u2WlanIdx;
	UINT_16              u2Idx;
	BOOLEAN              fgEnable;
	struct EPCS_EDCA_ENTRY_T    EdcaSetting;
};

INT32 uni_cmd_epcs_ctrl(struct _RTMP_ADAPTER *ad, UINT32 u4Cmd, UINT8 *epcscmd);
#endif
